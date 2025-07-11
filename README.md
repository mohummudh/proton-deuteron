# Proton-Deuteron Analysis

Analysis of LArIAT data to identify and separate proton and deuteron particles. 


## History of work (still in works)

- Beamline Mass cuts on LArIAT Run II Positive 100A (`picky+match.ipynb`)    
    - Proton candidates -> 600-1600 MeV (**16,935**) // check numbers again from retreived data instead of Bruno's
    - Deuteron candidates -> 1600-2750 MEV (**10,029**)
    - These are then retrieved from Fermilab's GPVMs (raw + reco data).

- Event displays to visually inspect data (`lariat/Event.py`, `event_class.ipynb`, implementation in other notebooks as well)

- Another cut on protons, from reconstructed events, choose only those with no. of tracks = 1 (`onetrack.ipynb`, **7,636**).
    - To identify characteristics that make these events "pure, golden samples", did exploratory data analysis. Plots in `plots.ipynb`
    - Results on track length vs. momentum interesting as relationship breaks down around 80cm. Compare with theory in `NIST/Plot_Proton_Length.ipynb`

*GROUPING TRACK PIXELS.*

- Simple connectivity based clustering on protons performs well
    - Identified where (a bounding box) the cleanest samples exhibit their track verticies. (`clustering_dev.ipynb`)
    - Cut on deuteron candidates to choose only those events which have activity in the same region and > 100 ADC (down to **6,961**)

- Other clustering algorithms developed to identify proton (`clustering_dev.ipynb` -> incorporated into `lariat/Event.py` class for ease of use). 
    - Find longest (largest area cluster) particle (avoids small noise, doesn't work if muon in event)
    - Find max ADC min:max ratio cluster (works robustly in events with protons, if event only has muons - no way to tell)
    - Fix max ADC value in the event, search connected pixels from there (above a threshold = max // 5 [chosen by user, roughly based on empirical observation from dE/dx v. residual range plots])

*LABELLING GROUPED TRACKS (to be able to identify signal (protons or deuteron present) / background (no p / d) events)*

- Ran basic connectivity clustering (ADC threshold = 15) on all events (proton + deuteron candidates, with all the cuts described above), stored in DataFrames (`clusters_data.ipynb` -> total 438,575 clusters, lots of noise)
    - Included 2D image of cluster
    - Include 1D array of max ADC from each wire in each cluster (1x240) - a 1D representation of ADC change for each cluster (helpful representations shown in `event_stats.ipynb`, `padded_data.ipynb`)
        - Protons/Deuterons expected to show steady increase (characteristic Bragg peaks)
        - Muons, low baseline with lots of fluctuations due to delta ray emissions 
        - These were prepped to be put through autoencoders for more intelligent clustering
    - Padded data to make appropriate for autoencoders (`padded_data.ipynb`)
        - Preserve original wire position of the cluster
        - Experimenting with smoothing data to improve autoencoder performance (still in works, `padded_data.ipynb`)

- Autoecoders 
    - Learnt to make FFA autoencoder (`FFA_autoencoder.ipynb`, uses PyTorch)
        - Okay reconstruction, huge errors on muons
    - CNN autoencoder (`CNN_autoencoder.ipynb`)
        - Improved architecture (mainly with the help of Claude, basic made by me), much better performance
        - Tried clustering latent representation of the events, no conclusive results
        - Struggles to differentiate between long protons/deuterons and muons. 
        - Clustering performs better with smaller tracks.
    - Improvements
        - Need to make sure clusters are properly indexed so can find original event 
        - Improve clustering somehow

*IMPROVING GROUPING TRACKS (formally known as track reconstruction?)*

- Incorporating direction into the clustering process
    - Using graph representations (still in works, `direction.ipynb`)
- Time matching between collection and induction plane (still in works, `time_matching.ipynb`)


`data_explore.ipynb` looks at data structure of available data (ROOT, reco, raw, etc.)

*WILD WILD WEST IDEAS*
- Using Meta's Segment Anything (still not implemented properly, `SAM.ipynb`)

## Results

- **16,935** high-purity proton candidates
- **10,029** deuteron candidates

## In Retrieved Data

- **16,624** events for protons -> one track cut -> **7636** proton candidate events
- **9852** events for deuterons -> bounding box cut -> **6961** deuteron candidate events


## Dependencies

Install required packages:
```bash
pip install -r requirements.txt
```