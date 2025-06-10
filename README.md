# Proton-Deuteron Analysis

Analysis of LArTPC data to identify and separate proton and deuteron particles based on beamline mass measurements.

## What it does

- Processes experimental data from ROOT files
- Identifies proton candidates (600-1600 MeV) with high purity selection
- Identifies deuteron candidates (1600-2750 MeV) with inclusive selection
- Generates CSV files with selected events for further analysis

## Key files

- `scripts/preselection.C` - ROOT script for initial data processing
- `notebooks/picky+match.ipynb` - Mass distribution analysis and particle selection
- `preselection_criteria.md` - Detailed methodology documentation

## Results

- **16,935** high-purity proton candidates
- **10,029** deuteron candidates