# GEM_flux
MOLLER / remoll — Photon-background study (GEM 577 + busiest detectors)

This repo documents how I built remoll, produced remollout.root, and analyzed photon backgrounds—first across all detectors and then in detail for GEM det 577 and the four busiest IDs (2001, 2002, 2011, 2012). It also shows how to locate where those photons were produced and where they hit.

Contents
scripts/
  topDetectors.C            # print N busiest detectors (secondary photons only)
  whichDetLightUp.C         # 1D counts of photon hits vs detector ID (log-y)
  fourDetSlices.C           # 2D hit.z vs det for {2001,2002,2011,2012}
  whereFrom.C               # compute production-vertex z from 'part' for chosen det IDs
  sliceXY.C                 # XY slices at the z-peaks (e.g., 19.5k, 20k, 20.5k, 21k)  :contentReference[oaicite:0]{index=0}
  sliceEnergy_logY.C        # energy spectrum in selected z-slices (log y)
  sliceEnergyAllDet.C       # energy-by-detector helper (batch)
  whereFrom_df.C            # RDataFrame variant of production-vertex calculation  :contentReference[oaicite:1]{index=1}
figs/                        # plots saved by the macros


Note: This README shows the ROOT one-liners and how to run the macros; the C++ implementations live in scripts/ (kept small and reusable).

1) Build remoll (very short)

Requirements: ROOT 6, Geant4 (with GDML/XercesC), CMake, a C++17 compiler.

# Get remoll
git clone https://github.com/JeffersonLab/remoll.git
cd remoll
mkdir build && cd build
cmake ..                 # add -DGeant4_DIR=/path/to/geant4 if needed
make -j


Produce an output file (any production/run macro you use is fine):
