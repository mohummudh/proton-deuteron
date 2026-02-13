# Proton-Deuteron/Kaon Separation in LArIAT

Numerical methods for identifying and separating proton and deuteron/kaon particles in the LArIAT (Liquid Argon In A Testbed) experiment, using signal processing, deep learning, and statistical hypothesis testing on raw TPC data from Run II Positive 100A.

## Overview

The LArIAT TPC records particle interactions across 480 channels (240 induction + 240 collection plane wires) with 3,072 time ticks per event. Protons and deuterons are selected via beamline mass cuts, then distinguished from background (primarily muons) using track reconstruction, autoencoders, and chi-squared tests on ionisation signatures.

### Dataset

| Stage | Protons | Deuterons | Kaons |
|---|---|---|---|
| Beamline mass preselection | 16,935 (600–1600 MeV) | 10,029 (1600–2750 MeV) | ~3,420 (350–650 MeV) |
| Retrieved from GPVM | 16,624 | 9,852 | — |
| After quality cuts | 7,636 (one-track) | 6,961 (bounding box) | — |

Preselection performed with `scripts/preselection.C` (ROOT macro). CSV lists in `cuts/`. Kaon selection disables the picky+match condition to increase statistics.

## Project Structure

```
lariat/                  Core library — Event class for loading, clustering, visualising raw data
├── event.py             Event loading from ROOT files, clustering algorithms, plotting

autoencoder/             Deep learning models for dimensionality reduction and separation
├── vae.ipynb            Variational Autoencoder with Mahalanobis distance in latent space
├── 1dCNN_autoencoder    1D CNN on per-wire ADC sequences
├── 2dCNN.ipynb          2D CNN on collection plane images
├── 2dCNNinduction.ipynb 2D CNN on induction plane images
├── k-fold.ipynb         K-fold cross-validation
├── latent.ipynb         Latent space analysis and visualisation
├── separation.ipynb     Proton/deuteron separation in latent space
├── anomaly.ipynb        Anomaly detection using reconstruction error
├── GAN.ipynb            Generative adversarial network experiments
└── hyperparameters.ipynb Hyperparameter tuning

kaons/                   Classification for protons vs. kaons
├── chi2.ipynb           Chi-squared hypothesis testing with dE/dx vs residual range
├── protons-kaons-vae    VAE for separation
├── kaons_clustering     Kaon track clustering
└── data.ipynb           Kaon data preparation

clustering/              Track pixel grouping and signal extraction
├── clustering_dev.ipynb Algorithm development (connectivity, max ADC, flood-fill)
├── clusters_data.ipynb  Batch cluster extraction (438k+ clusters)
├── event_stats.ipynb    Statistical properties of extracted tracks
└── direction.ipynb      Direction-aware clustering with graph representations

cuts/                    Event selection and filtering
├── masscut.ipynb        Beamline mass filtering
├── onetrack.ipynb       Single-track proton selection
├── pickyprotons.ipynb   Picky reconstruction + matching
├── pickydeuterons.ipynb Picky reconstruction for deuterons
└── momentum_tof.ipynb   Momentum and time-of-flight analysis

analysis/                High-level results
├── protons.ipynb        Proton characterisation
├── deuterons.ipynb      Deuteron characterisation
├── both.ipynb           Combined analysis
└── separation_limit     Theoretical separation limits for proton/deuteron candidates in the data

notebooks/               Exploratory work
├── chi2.ipynb           Chi-squared analysis (proton/deuteron)
├── plots.ipynb          Exploratory data visualisation
├── matching.ipynb       Collection/induction plane matching
├── signalnoise.ipynb    Signal-to-noise studies
├── SAM.ipynb            Meta Segment Anything experiments
└── NIST/                Theory comparisons (track length vs momentum)

ssvd/                    SSVD + autoencoder experiments
├── sparse.ipynb         SSVD autoencoder training
├── inference.ipynb      Model inference and scoring
└── cnn_model.onnx       Exported ONNX model

scripts/                 Production tools
├── preselection.C       ROOT macro for initial beamline mass cuts
├── eventdisplay.py      Tkinter GUI for interactive event viewing
├── group_particles.py   Dask-based parallel particle grouping
├── to_parquet.py        CSV → Parquet conversion (DuckDB)
└── bboxReco/            Custom LArIAT art module for bounding-box reconstruction
```

## Methods

### 1. Track Clustering

Four algorithms in `lariat/event.py` for grouping track pixels:

- **Connected regions** — binary mask above threshold, connected component labelling
- **Longest cluster** — largest connected region by area (fails with muon contamination)
- **Max ADC ratio** — cluster with highest max/min ADC ratio (robust for proton events)
- **Flood-fill from max ADC** — BFS from peak ADC pixel, threshold = max/6 (most reliable)

### 2. Autoencoders

Trained on padded cluster images (normalised 0–1) to learn compressed representations:

- **Feedforward** — baseline, large reconstruction errors on muons
- **1D CNN** — processes per-wire ADC as 1D sequences, improved over feedforward
- **2D CNN** — operates on full wire×tick images, best reconstruction quality
- **VAE** — variational autoencoder producing structured latent space; separation measured via Mahalanobis distance/density mapping the protons. Trained with MS-SSIM + VAE loss. Latent dimensions: 4–16

### 3. Chi-Squared Particle ID

Statistical hypothesis testing using reconstructed dE/dx vs residual range:

- Computes chi-squared against proton, kaon, and deuteron hypotheses
- Results visualised in the VAE latent space
- Located in `kaons/chi2.ipynb`

### 4. Bounding Box Reconstruction

Custom C++ art producer module (`scripts/bboxReco/`) that filters TPC signals within per-event spatial regions defined by CSV, enabling clean isolated track reconstruction within the official LArIAT framework.

## Setup

```bash
pip install -r requirements.txt
```

Key dependencies: PyTorch, TensorFlow/Keras, uproot, scikit-learn, scikit-image, pytorch-msssim, plotly, segment-anything.

GPU acceleration supported via CUDA and MPS (macOS Metal).
