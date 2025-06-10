# Preselection Criteria for Proton-Deuteron Analysis

## Overview

This document describes the preselection criteria and methodology used to identify and separate proton and deuteron candidates from the experimental data based on beamline mass measurements.

## Data Processing Method

### Initial Data Loading and Filtering

The analysis begins with a dataset containing picky and match selection flags along with beamline mass measurements:

1. **Data Source**: `picky+match.csv` containing event-level information
2. **Initial Filtering**: Events with `beamline_mass > 0` are retained to remove invalid measurements
3. **Duplicate Removal**: The dataset is cleaned of any duplicate entries

### Selection Categories

The analysis categorizes events into four distinct groups based on two binary flags:

- **Picky Only**: Events where `p == 1` and `m == 0`
- **Match Only**: Events where `m == 1` and `p == 0` 
- **Picky+Match**: Events where both `p == 1` and `m == 1`
- **Neither**: Events where both `p == 0` and `m == 0`

## Mass Distribution Analysis

### Visualization Approach

Multiple histogram plots were generated to understand the beamline mass distributions across different mass ranges:

- **Full Range**: 0-2750 MeV
- **Intermediate Range (Protons + Deuterons)**: 600-2750 MeV
- **Low Mass Focus**: 0-1600 MeV
- **Proton Region**: 600-1600 MeV
- **Deuteron Region**: 1600-2750 MeV

## Preselection Criteria

### Proton Candidates

**Mass Range**: 600-1600 MeV

**Selection Requirements**:
- Must pass both Picky and Match selection criteria (`p == 1` AND `m == 1`)
- Beamline mass between 600 MeV and 1600 MeV (inclusive)

**Rationale**: This mass range corresponds to the expected proton mass (~938 MeV) with appropriate tolerance for measurement uncertainties and resolution effects. Using both Picky and Match to get a golden sample of protons ensures high purity and reliability in the selection-this will help later on with deuteron identification.

### Deuteron Candidates

**Mass Range**: 1600-2750 MeV

**Selection Requirements**:
- All events in this mass range regardless of Picky/Match status
- Beamline mass greater than 1600 MeV and less than or equal to 2750 MeV

**Rationale**: This mass range encompasses the expected deuteron mass (~1876 MeV) with broader acceptance to account for the smaller expected sample size.

## Output Files

The preselected events are saved to CSV files containing run, subrun, and event IDs:

1. **Proton Candidates**: `preprotons_pm_600_1600.csv`
   - Contains events with Picky+Match selection in 600-1600 MeV range
   
2. **Deuteron Candidates**: `predeuterons_all_1600_2750.csv`
   - Contains all events in 1600-2750 MeV range regardless of selection status

## Selection Statistics

From the analysis of the mass distributions, the following observations were made:

- The proton region (600-1600 MeV) shows clear separation from higher mass particles
- The deuteron region (1600-2750 MeV) contains fewer events but represents the expected mass range for deuterons
- The Picky+Match selection provides the highest purity for proton identification
- For deuterons, a more inclusive approach is used due to lower statistics

## Quality Control

- **Mass Validation**: Only events with positive beamline mass are considered
- **Duplicate Check**: Dataset is verified to be free of duplicate entries
- **Visual Inspection**: Mass distributions are plotted across multiple ranges to verify selection boundaries

This preselection serves as the first stage in particle identification, providing clean samples of proton and deuteron candidates for subsequent detailed analysis.
