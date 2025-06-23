# Proton-Deuteron Analysis

Analysis of LArTPC data to identify and separate proton and deuteron particles based on beamline mass measurements and track reconstruction quality.

## What it does

- Processes experimental data from ROOT files containing raw detector data
- Applies track reconstruction quality cuts (single-track events)
- Identifies proton candidates (600-1600 MeV) with high purity selection
- Identifies deuteron candidates (1600-2750 MeV) with inclusive selection
- Generates event displays for visualization and validation
- Creates datasets for further physics analysis

## Workflow

1. **Data Processing**: ROOT files → CSV with track information
2. **Quality Selection**: Filter for single-track events only
3. **Mass Selection**: Apply beamline mass cuts for particle identification
4. **Event Display**: Generate collection plane heatmaps for selected events
5. **Analysis**: Statistical analysis and validation

## Key Files

### Scripts
- `scripts/preselection.C` - ROOT script for initial data processing and track counting
- `scripts/savedisplay.py` - Command-line tool for batch event display generation
- `scripts/eventdisplay.py` - Interactive GUI event display viewer
- `scripts/to_parquet.py` - Data format conversion utilities

### Notebooks
- `notebooks/onetrack.ipynb` - Single-track event analysis and selection
- `notebooks/picky+match.ipynb` - Mass distribution analysis and particle selection
- `notebooks/plots.ipynb` - Data visualization and quality plots
- `notebooks/preselection.ipynb` - Initial data exploration

### Data
- `LArTPC_Variables.csv` - Processed track reconstruction data
- `preselection_criteria.md` - Detailed methodology documentation
- `requirements.txt` - Python dependencies

## Tools and Features

### Event Display System
- **Interactive Viewer**: GUI-based tool for browsing events one by one
- **Batch Generator**: Command-line tool for creating event displays en masse
- **Collection Plane Focus**: Optimized for collection plane visualization
- **Fast Search**: Parallel processing for efficient file searching

### Data Selection Pipeline
- **Track Quality**: Single-track events only for clean analysis
- **Mass Cuts**: Beamline mass-based particle identification
- **Event Matching**: Links track data with raw detector data
- **Statistics**: Comprehensive event counting and validation

## Usage Examples

### Generate Event Displays
```bash
# Create displays for all single-track proton events
python scripts/savedisplay.py onetrack_events.csv /path/to/raw/data event_displays/

# Interactive event viewer
python scripts/eventdisplay.py
```

### Data Analysis
```python
# Load and analyze single-track events
import pandas as pd
onetrackdf = pd.read_csv('onetrack_events.csv')
print(f"Found {len(onetrackdf)} single-track events")
```

## Results

- **16,935** high-purity proton candidates
- **10,029** deuteron candidates

## Current Results

### Event Statistics
- **Total Events Processed**: From comprehensive ROOT file analysis
- **Single-Track Events**: High-quality events with exactly one reconstructed track
- **Proton Candidates**: Events with beamline mass 600-1600 MeV
- **Deuteron Candidates**: Events with beamline mass 1600-2750 MeV

### Data Quality
- **Track Reconstruction**: Applied strict single-track requirements
- **Event Display Validation**: Visual verification of detector response
- **Mass Distribution**: Clean separation between particle types
- **File Matching**: Successful linking of analysis data with raw detector files

## Directory Structure

```
proton-deuteron/
├── scripts/           # Processing and visualization tools
├── notebooks/         # Analysis notebooks and data exploration
├── data/             # Processed datasets (CSV format)
├── rawprotons/       # Raw ROOT files from detector
└── event_displays/   # Generated event visualization images
```

## Dependencies

Install required packages:
```bash
pip install -r requirements.txt
```

Key packages:
- `pandas` - Data manipulation and analysis
- `numpy` - Numerical computing
- `matplotlib`, `seaborn` - Plotting and visualization
- `uproot` - ROOT file reading
- `awkward` - Array processing
- `tqdm` - Progress bars

## Development Notes

- Optimized for LArIAT detector geometry (240 collection plane wires)
- Supports parallel processing for large datasets
- Event displays focus on collection plane for track visualization
- Compatible with both interactive analysis and batch processing