# LArIAT Bounding Box Reconstruction Framework

This document provides a detailed explanation of the custom bounding box reconstruction framework implemented for LArIATSoft. This system allows for event-by-event, region-of-interest (ROI) based TPC reconstruction, controlled by a simple CSV file.

## Table of Contents
1.  [Overview](#1-overview)
2.  [System Components](#2-system-components)
    *   [The Core Module: `BoundingBoxCalDataFilter_module.cc`](#the-core-module-boundingboxcaldatafilter_modulecc)
    *   [The Control File: `bboxReco.csv`](#the-control-file-bboxrecocsv)
    *   [The FHiCL Configuration Files](#the-fhicl-configuration-files)
3.  [How It Works: The Data Flow](#3-how-it-works-the-data-flow)
4.  [Key Features & Design Choices](#4-key-features--design-choices)
5.  [How to Use](#5-how-to-use)
    *   [Defining Bounding Boxes](#defining-bounding-boxes)
    *   [Running the Reconstruction](#running-the-reconstruction)
    *   [Debugging](#debugging)
6.  [How to Modify & Extend](#6-how-to-modify--extend)

---

## 1. Overview

The standard LArIAT reconstruction processes the entire TPC volume for every event. Here I wanted to be able to input a bounding box and reconstruct only that part of the LArTPC. The bounding boxes were created to isolate protons and deuterons directly using raw data.

This framework introduces a new `art` producer module, `BoundingBoxCalDataFilter`, which acts as a pre-filter before the hit-finding stage. It reads a list of events and their corresponding 3D bounding boxes from a CSV file. For each event specified in the CSV, the module zeroes out all TPC wire signals that fall outside the defined spatial (wire) and temporal (time tick) boundaries. Events *not* listed in the CSV are passed through unmodified.

The result is a new, filtered collection of calibrated wire data (`recob::Wire`) where only the regions of interest contain signal. The subsequent reconstruction chain (hit finding, clustering, tracking) then operates on this filtered data.

---

## 2. System Components

The system is composed of three main parts: the C++ module that does the work, the CSV file that provides the inputs, and the FHiCL files that configure and run the reconstruction.

### The Core Module: `BoundingBoxCalDataFilter_module.cc`

This C++ file defines an `art::EDProducer` named `BoundingBoxCalDataFilter`. This is the engine of the framework.

**Initialisation (`Constructor` & `loadCSV`):**
*   When the `art` job starts, the module is constructed.
*   It immediately calls the `loadCSV()` function, which reads the user-provided CSV file (`bboxReco.csv`).
*   It parses each line of the CSV, creating an `EventBox` struct containing the run, subrun, event number, and the wire/time boundaries for both collection and induction planes.
*   Crucially, it stores these `EventBox` structs in a `std::map`, using a `std::tuple<int, int, int>` (run, subrun, event) as the key. This is a highly efficient design choice, as it allows for O(1) (constant time) lookup for any given event, rather than re-reading or searching the file for every event.

**Per-Event Processing (`produce` method):**
*   This method is executed for every event in the input ROOT file.
*   It retrieves the current event's run, subrun, and event number.
*   It performs a fast lookup in the `std::map` created during initialisation.
*   **If the event is NOT found in the map:** The module enters a "pass-through" mode. It simply copies the original, unfiltered `recob::Wire` data to its output and does nothing else for this event.
*   **If the event IS found in the map:** The module performs the filtering logic:
    1.  It iterates through every `recob::Wire` in the input data.
    2.  For each wire, it calls the `isWireInBox` helper function. This function uses the `geo::Geometry` service to determine the wire's plane type (collection or induction) and its wire number.
    3.  It checks if the wire number falls within the corresponding `c_w_i`/`c_w_f` (collection) or `i_w_i`/`i_w_f` (induction) range from the CSV.
    4.  **If the wire is inside the box's wire range:** It then applies the time filter. It creates a new signal vector of the same size as the original. It copies the ADC values only for time ticks between `t_i` and `t_f`. All other ticks in that wire's signal are set to `0.0`.
    5.  **If the wire is outside the box's wire range:** It creates a new signal vector where *all* ADC values are `0.0`.
    6.  A new `recob::Wire` object is created with this new, potentially zeroed signal and added to the output collection.

**Robustness and Safety:**
*   The module uses `std::clamp` on all wire and time boundaries read from the CSV. This prevents crashes or undefined behavior if a user provides out-of-range values (e.g., a wire number greater than the number of wires on a plane).
*   It includes detailed error handling for CSV parsing and file access.
*   It correctly uses `fGeometry->SignalType(channel)` to distinguish between collection and induction planes, making it robust against different detector geometries.

### The Control File: `bboxReco.csv`

This is the user-facing control file. It is a simple Comma-Separated Value file with a header and one line per event to be filtered.

**Format:**
`run,subrun,event,c_w_i,c_w_f,c_t_i,c_t_f,i_w_i,i_w_f,i_t_i,i_t_f`

*   `run, subrun, event`: The unique identifier for the event.
*   `c_w_i, c_w_f`: The starting and ending **wire numbers** for the **collection plane**.
*   `c_t_i, c_t_f`: The starting and ending **time ticks** for the **collection plane**.
*   `i_w_i, i_w_f`: The starting and ending **wire numbers** for the **induction plane**.
*   `i_t_i, i_t_f`: The starting and ending **time ticks** for the **induction plane**.

### The FHiCL Configuration Files

These files tell the `art` framework how to run the reconstruction.

*   **`data_multi_pass_reco_bbox.fcl`**: This is the main configuration file.
    *   It includes all the standard LArIAT reconstruction modules.
    *   It defines the new `bboxcaldata` producer, linking it to the `BoundingBoxCalDataFilter` C++ class.
    *   **Crucially, it modifies the reconstruction sequence.** It inserts `bboxcaldata` right after `caldata` and before `gaushitOrigin`. It then tells `gaushitOrigin` to take its input from `bboxcaldata`, effectively inserting the filter into the data stream.
    *   It also configures the output stream to `drop` the intermediate `bboxcaldata` product to save disk space.
    *   This file also controls parameters for reconstruction, `gaushitOrigin` parameters are particularly useful as they set thresholds for finding hits.

*   **`data_multi_pass_reco_bbox_100a.fcl`**: This is an "override" file. It demonstrates a powerful FHiCL feature.
    *   It first `#include`s the base `data_multi_pass_reco_bbox.fcl`.
    *   It then *overrides* specific parameters for a 100A magnetic field run, such as the `BFieldInTesla` value and the output filenames. This avoids duplicating the entire configuration.

*   **`data_multi_pass_reco_bbox_debug.fcl`**: Another override file, this one enables the `DebugMode` flag in the C++ module and turns on verbose logging, which is invaluable for troubleshooting.

---

## 3. How It Works: The Data Flow

The data processing follows this modified chain:

1.  **`art` Framework**: Reads raw detector data (`raw::RawDigit`) from the input ROOT file.
2.  **`caldata` (`CalWireROIT1034`)**: Performs signal processing (e.g., pedestal subtraction, ROI finding) on the raw digits to produce calibrated waveforms on each wire (`recob::Wire`).
3.  **`bboxcaldata` (`BoundingBoxCalDataFilter`)**: **(NEW STEP)**
    *   Receives the `recob::Wire` collection from `caldata`.
    *   Checks the event ID against its internal map loaded from `bboxReco.csv`.
    *   Produces a *new* collection of `recob::Wire` objects where signals outside the defined bounding box are zeroed out.
4.  **`gaushitOrigin` (`GausHitFinder`)**:
    *   Receives the filtered `recob::Wire` collection from `bboxcaldata`.
    *   Finds hits *only* in the regions where signal remains, i.e., inside the bounding box.
5.  **Standard Reconstruction Chain**:
    *   `trajclusterOrigin`, `pmtrackOrigin`, `gaushit` (distorted hit removal), `trajcluster`, `pmtrack`, `calo`, etc., all proceed as normal, but their input is now restricted to the hits found within the bounding box.

---

## 4. How to Use

### Defining Bounding Boxes

1.  Open the file `/exp/lariat/app/users/msultan/bbox/srcs/lariatsoft/LArIATRecoModule/bboxReco.csv` / automate.
2.  Add a new line for each event you want to filter.
3.  Fill in the `run`, `subrun`, and `event` numbers.
4.  Specify the wire and time tick ranges for both collection and induction planes.

### Running the Reconstruction

Execute the `lar` command, pointing it to the appropriate FHiCL configuration file.

**For a standard 100A run:**
```bash
lar -c data_multi_pass_reco_bbox_100a.fcl -s /path/to/your/input_file.root
```

This will use the settings in `data_multi_pass_reco_bbox_100a.fcl`, which automatically includes the base configuration and overrides the magnetic field.

### Debugging

If the output is not as expected, run the debug version:
```bash
lar -c data_multi_pass_reco_bbox_debug.fcl -s /path/to/your/input_file.root
```
This will process only 10 events and print detailed information about which bounding boxes are loaded and how they are applied to each event.

---

## 5. How to Modify & Extend

*   **Change Filter Behavior:** To keep data outside the box instead of zeroing it, change `ZeroOutside: true` to `false` in `data_multi_pass_reco_bbox.fcl`.
*   **Add More Complex Logic:** The C++ module can be extended. For example, you could add logic to support multiple, non-contiguous bounding boxes for a single event.
*   **Change Default Parameters:** The default CSV filename and other parameters can be changed directly in `data_multi_pass_reco_bbox.fcl`.
