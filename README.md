# GEM_flux

MOLLER / **remoll** — Photon‑background study (**GEM 577** + busiest detectors) **and electron maps**

This repo documents how I built **remoll**, produced `remollout.root`, and analyzed photon backgrounds—first across all detectors and then in detail for **GEM det 577** and the four busiest IDs (**2001, 2002, 2011, 2012**). It also shows how to locate where those photons were *produced* and where they *hit*. A final section shows **electron (e±) hit maps** in GEM 577, matching the included plot.

---

## Contents

```
scripts/
  whereFrom.C             # compute production-vertex z from `part` for chosen det IDs
  sliceXY.C               # XY slices at the z-peaks (e.g., 19.5k, 20k, 20.5k, 21k)
  sliceEnergy_logY.C      # energy spectrum in selected z-slices (log y)
  sliceEnergyAllDet.C     # energy-by-detector helper (batch)

# (optional helpers you may add later)
# topDetectors.C          # print N busiest detectors (secondary photons only)
# whichDetLightUp.C       # 1D counts of photon hits vs detector ID (log-y)
# fourDetSlices.C         # 2D hit.z vs det for a short det-range
# whereFrom_df.C          # RDataFrame variant of production-vertex calculation
figs/                     # plots saved by the macros
```
> This README only shows how to **run** things. The C++ implementations live under `scripts/`, so you can keep them short and reusable.

---

## 1) Build **remoll** (very short)

Requirements: **ROOT 6**, **Geant4** (with **GDML/XercesC**), **CMake**, a C++17 compiler.

```bash
# Get remoll
git clone https://github.com/JeffersonLab/remoll.git
cd remoll
mkdir build && cd build
cmake ..                 # add -DGeant4_DIR=/path/to/geant4 if needed
make -j
```

Produce an output file (use your preferred macro; ensure it writes `remollout.root`):

```bash
./remoll ../macros/runexample.mac
# -> remollout.root
```

---

## 2) Start a ROOT analysis session

```bash
root -l remollout.root
```
```cpp
TTree* T = (TTree*)_file0->Get("T");
```

Handy fields:
- `hit.det`, `hit.pid`, `hit.mtrid`, `hit.trid`, `hit.z`, `hit.x`, `hit.y`, `hit.e`
- `part.trid`, `part.vz` (parent track’s production vertex)

**Secondary photons only** selection used throughout:
```
hit.pid==22 && hit.mtrid!=0
```

---

## 3) “Which detectors are lighting up?”

**Quick 1D map (all detectors)**
```cpp
T->Draw("hit.det>>hDet(6401,-0.5,6400.5)", "hit.pid==22 && hit.mtrid!=0");
gPad->SetLogy();
```

**Trim to a sensible range (e.g., 30–600)**  
(helps isolate certain subsystems; adjust as needed)
```cpp
T->Draw("hit.det>>hDet(571,30.5,600.5)", "hit.pid==22 && hit.mtrid!=0");
gPad->SetLogy();
```

---

## 4) Top‑N photon‑hit detectors (text table)

```cpp
// Fill hDet first (see §3), then:
#include <vector>
#include <algorithm>
std::vector<std::pair<double,int>> v;
for (int b=1;b<=hDet->GetNbinsX();++b){
  double c=hDet->GetBinContent(b);
  if (c>0) v.emplace_back(c,(int)hDet->GetBinCenter(b));
}
std::sort(v.begin(),v.end(),[](auto&a,auto&b){return a.first>b.first;});
for(int k=0;k<15 && k<(int)v.size();++k)
  printf("%2d) det %d : %.0f hits\n",k+1,v[k].second,v[k].first);
```

Typical top 4 we saw: **2002, 2001, 2012, 2011**.

---

## 5) Busiest four: quick 2D “where are the hits?”

```cpp
T->Draw("hit.z:hit.det>>h2(12,2000.5,2012.5,400,19000,21200)",
        "hit.pid==22 && hit.mtrid!=0 && (hit.det==2001||hit.det==2002||hit.det==2011||hit.det==2012)",
        "colz");
gPad->SetLogz();
```

---

## 6) For each detector: **where were the photons produced?** and **where did they hit?**

Use `scripts/whereFrom.C` which joins `hit` to `part` by `trid` and fills two histograms per detector:  
- **production‑vertex `z`** = `part[hit.trid].vz`  
- **hit `z`** = `hit.z`

```cpp
.L scripts/whereFrom.C+
whereFrom(T, {2001,2002,2011,2012});   // saves prodZ_detXXXX.pdf and hitZ_detXXXX.pdf under figs/
```

---

## 7) GEM **det 577** deep‑dive (photons)

**A) Production‑`z` & Hit‑`z` (secondary photons)**  
```cpp
.L scripts/whereFrom.C+
whereFrom(T, {577});   // writes figs/det577_prodZ.pdf and figs/det577_hitZ.pdf
```

**B) XY slices at z‑peaks (≈19.5k, 20k, 20.5k, 21k mm)**  
```cpp
.L scripts/sliceXY.C+
sliceXY(577);  // writes figs/…_XY_*.pdf
```

**C) Energy spectra in those slices (log‑y)**  
```cpp
.L scripts/sliceEnergy_logY.C+
sliceEnergy_logY(577);

.L scripts/sliceEnergyAllDet.C+
sliceEnergyAllDet({2001,2002,2011,2012,577});
```

A typical result for **det 577** on our samples:  
⟨**production z**⟩ ≈ **0.7 m** (RMS ≈ **0.22 m**) and ⟨**hit z**⟩ ≈ **20.5 m**.

---

## 8) **Electrons (e±) in GEM 577**

We also mapped **electron** hits in det 577 (see `figs/electrons_in_det577_XY.pdf` / attached example).

**XY occupancy (heatmap):**
```cpp
// e− only:
T->Draw("hit.y:hit.x>>hXYe(240,-1200,1200,240,-1200,1200)",
        "hit.det==577 && hit.pid==11","colz");

// either charge (e±):
// T->Draw("hit.y:hit.x>>hXYe(240,-1200,1200,240,-1200,1200)",
//         "hit.det==577 && abs(hit.pid)==11","colz");

gPad->SetRightMargin(0.15); gPad->SetLogz(); // nicer colorbar & dynamic range
```

**Optional, electron energy spectrum (log‑y):**
```cpp
T->Draw("hit.e>>hEe(240,0,200)","hit.det==577 && abs(hit.pid)==11"); gPad->SetLogy();
```

**Optional, electron hit‑z distribution:**
```cpp
T->Draw("hit.z>>hZe(200,19000,21200)","hit.det==577 && abs(hit.pid)==11"); gPad->SetLogy();
```

> These commands reproduce the electron hit map we showed. You can copy the same style to any detector by replacing `577` with another ID.

---

## 9) Map **z** to physical locations (geometry)

By convention in these files, **z = 0** is at/near the **LH₂ target**.  
- **z ≈ 300–900 mm** → **collimator / toroid‑entrance** region.  
- **z ~ 19.5–21.0 m** → **Detector pipe / GEM region** where hits are recorded.

For precise, named components, read the **GDML** and query node positions.

**Flatten includes and import the full geometry:**
```bash
# outside ROOT
xmllint --xinclude geometry/mollerMother.gdml -o /tmp/moller_flat.gdml
```
```cpp
// inside ROOT
TGeoManager::Import("/tmp/moller_flat.gdml");

// toy snippet to print z of nodes whose names contain keywords:
TGeoIterator it(gGeoManager->GetTopVolume());
while (auto* n = (TGeoNode*)it()) {
  TString nm = n->GetName();
  if (nm.Contains("Toroid") || nm.Contains("Collim") || nm.Contains("Pipe")) {
    TString path = gGeoManager->GetPath(); gGeoManager->cd(path);
    Double_t loc[3]={0,0,0}, mas[3]; gGeoManager->LocalToMaster(loc,mas);
    printf("%-40s z = %8.1f mm  path=%s\n", nm.Data(), mas[2], path.Data());
  }
}
```

---

## 10) Reproduce the key plots quickly

```cpp
// 0) open
TFile* f = TFile::Open("remollout.root");
TTree* T = (TTree*)f->Get("T");

// 1) which detectors light up (secondary photons)
T->Draw("hit.det>>hDet(6401,-0.5,6400.5)", "hit.pid==22 && hit.mtrid!=0"); gPad->SetLogy();

// 2) busiest four, 2D hit-z vs det
T->Draw("hit.z:hit.det>>h2(12,2000.5,2012.5,400,19000,21200)",
        "hit.pid==22 && hit.mtrid!=0 && (hit.det==2001||hit.det==2002||hit.det==2011||hit.det==2012)",
        "colz"); gPad->SetLogz();

// 3) production-z and hit-z per detector (also for 577)
.L scripts/whereFrom.C+; whereFrom(T, {2001,2002,2011,2012,577});

// 4) GEM 577 XY slices at z-peaks
.L scripts/sliceXY.C+; sliceXY(577);

// 5) electrons in GEM 577 (XY map)
T->Draw("hit.y:hit.x>>hXYe(240,-1200,1200,240,-1200,1200)",
        "hit.det==577 && abs(hit.pid)==11","colz"); gPad->SetLogz();
```

---

## 11) Troubleshooting

- **Production‑`z` histogram empty or zero mean?**  
  Ensure you **join `hit.trid` → `part.trid`** (don’t just plot `hit.vz`). Use `whereFrom.C`.
- **GDML import errors about XInclude:**  
  Flatten with `xmllint --xinclude` then `TGeoManager::Import` the flattened file.
- **Sparse plots after widening axes:**  
  Turn on `gPad->SetLogz()` / `SetLogy()` and use sufficient bins for the shown range.

---

## 12) Results snapshot (typical from my runs)

- **Top secondary‑photon detectors:** 2002, 2001, 2012, 2011 (then 38, 2008, …).  
- Their production vertices cluster around **~0.4–0.8 m** downstream of the target (collimator/toroid entrance), while their **hits** are around **~20–21 m** in the detector pipe.  
- **GEM 577:** ⟨prod z⟩ ≈ **0.70 m**, ⟨hit z⟩ ≈ **20.5 m** — consistent with secondaries born near the collimator/toroid entrance and striking the GEM region.
- **Electrons in GEM 577:** multiple ring‑like XY clusters; reproduce with the electron commands in §8.

(Your exact numbers will depend on configuration/statistics.)

---

## 13) Slides & figures

The macros write PDFs in `figs/` (e.g., `det577_prodZ.pdf`, `det577_hitZ.pdf`, `electrons_in_det577_XY.pdf`) ready to drop into a talk.

---

**Author:** Mohnish Sao  
**Advisor:** Wenliang “Bill” Li
