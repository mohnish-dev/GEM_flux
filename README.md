# GEM_flux

MOLLER / **remoll** — Photon-background study (**GEM 577** and the busiest detectors) **and electron hit maps**

This repository documents building **remoll**, producing `remollout.root`, and analyzing photon backgrounds across all detectors, followed by detailed studies for **GEM det 577** and the four most active detector IDs (**2001, 2002, 2011, 2012**). It also shows how to determine where those photons were **produced** and where they **hit**. A final section summarizes **electron (e±) hit maps** in GEM 577.


---

## Repository structure

```
scripts/
  whereFrom.C             # compute production-vertex z from `part` for chosen detector IDs
  sliceXY.C               # x–y occupancy at the z-peaks (≈19.5k, 20k, 20.5k, 21k mm)
  sliceEnergy_logY.C      # energy spectrum in the selected z-slices (log y)
  sliceEnergyAllDet.C     # energy-by-detector helper (batch)

figs/                     # plots saved by the macros (PDF/PNG)
```

_Optional helpers present but not required by this README:_  
`sliceEnergy.C`, `pidPrimarySecondary.C`, `color_3d_sliceXY.C`, `GEM_flux.C`.

---

## 1) Build **remoll**

**Prerequisites:** ROOT 6, Geant4 (with GDML/Xerces-C), CMake, and a C++17 compiler.

```bash
git clone https://github.com/JeffersonLab/remoll.git
cd remoll
mkdir build && cd build
cmake ..        # add -DGeant4_DIR=/path/to/geant4 if needed
make -j
```

Run a macro that writes `remollout.root`:

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

**Useful branches**

- `hit.det`, `hit.pid`, `hit.mtrid`, `hit.trid`, `hit.z`, `hit.x`, `hit.y`, `hit.e`
- `part.trid`, `part.vz` (parent track production vertex)

**Secondary-photon selection** used throughout:
```
hit.pid==22 && hit.mtrid!=0
```

---

## 3) Detectors with the most secondary-photon hits

**All detectors (1D)**
```cpp
T->Draw("hit.det>>hDet(6401,-0.5,6400.5)", "hit.pid==22 && hit.mtrid!=0");
gPad->SetLogy();
```

**Trimmed range (example: 30–600)**
```cpp
T->Draw("hit.det>>hDet(571,30.5,600.5)", "hit.pid==22 && hit.mtrid!=0");
gPad->SetLogy();
```

**Top-N table from `hDet`**
```cpp
#include <vector>
#include <algorithm>
std::vector<std::pair<double,int>> v;
for (int b=1;b<=hDet->GetNbinsX();++b){ double c=hDet->GetBinContent(b);
  if (c>0) v.emplace_back(c,(int)hDet->GetBinCenter(b)); }
std::sort(v.begin(),v.end(),[](auto&a,auto&b){return a.first>b.first;});
for (int k=0;k<15 && k<(int)v.size();++k)
  printf("%2d) det %d : %.0f hits\n",k+1,v[k].second,v[k].first);
```

Typical top four: **2002, 2001, 2012, 2011**.

---

## 4) Busiest four: hit-z vs detector (2D)

```cpp
T->Draw("hit.z:hit.det>>h2(12,2000.5,2012.5,400,19000,21200)",
        "hit.pid==22 && hit.mtrid!=0 && (hit.det==2001||hit.det==2002||hit.det==2011||hit.det==2012)",
        "colz");
gPad->SetLogz();
```

---

## 5) Production-z and hit-z per detector

Use `scripts/whereFrom.C` to join `hit` ↔ `part` by `trid` and fill:
- **production-vertex z** (`part[hit.trid].vz`)
- **hit z** (`hit.z`)

```cpp
.L scripts/whereFrom.C+
whereFrom(T, {2001,2002,2011,2012});   // outputs PDFs under figs/
```

---

## 6) GEM **det 577** (photons)

**Production-z and hit-z**
```cpp
.L scripts/whereFrom.C+
whereFrom(T, {577});
```

**x–y occupancy at z-peaks (≈19.5k, 20k, 20.5k, 21k mm)**
```cpp
.L scripts/sliceXY.C+
sliceXY(577);
```

**Energy spectra in those slices (log-y)**
```cpp
.L scripts/sliceEnergy_logY.C+
sliceEnergy_logY(577);

.L scripts/sliceEnergyAllDet.C+
sliceEnergyAllDet({2001,2002,2011,2012,577});
```

Representative values from these runs:  
⟨production-z⟩ ≈ **0.70 m** (RMS ≈ **0.22 m**); ⟨hit-z⟩ ≈ **20.5 m**.

---

## 7) Electrons (e±) in GEM 577

**x–y occupancy (heatmap)**
```cpp
// electron-only:
T->Draw("hit.y:hit.x>>hXYe(240,-1200,1200,240,-1200,1200)",
        "hit.det==577 && hit.pid==11","colz");

// or either sign (e±):
// T->Draw("hit.y:hit.x>>hXYe(240,-1200,1200,240,-1200,1200)",
//         "hit.det==577 && abs(hit.pid)==11","colz");

gPad->SetRightMargin(0.15); gPad->SetLogz();
```

**Optional**
```cpp
// energy spectrum (log-y)
T->Draw("hit.e>>hEe(240,0,200)", "hit.det==577 && abs(hit.pid)==11"); gPad->SetLogy();

// electron hit-z
T->Draw("hit.z>>hZe(200,19000,21200)", "hit.det==577 && abs(hit.pid)==11"); gPad->SetLogy();
```

---

## 8) Relating z to hardware locations

By convention, **z = 0** is at/near the **LH₂ target**.
- **z ≈ 0.3–0.9 m** → collimator / toroid-entrance region  
- **z ≈ 19.5–21.0 m** → detector pipe / GEM region

For precise component names, import the flattened **GDML** and query node positions:

```bash
xmllint --xinclude geometry/mollerMother.gdml -o /tmp/moller_flat.gdml
```

```cpp
TGeoManager::Import("/tmp/moller_flat.gdml");
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

## 9) Quick reproduction checklist

```cpp
// open
TFile* f = TFile::Open("remollout.root"); TTree* T=(TTree*)f->Get("T");

// detectors that light up
T->Draw("hit.det>>hDet(6401,-0.5,6400.5)", "hit.pid==22 && hit.mtrid!=0"); gPad->SetLogy();

// busiest four, 2D
T->Draw("hit.z:hit.det>>h2(12,2000.5,2012.5,400,19000,21200)",
        "hit.pid==22 && hit.mtrid!=0 && (hit.det==2001||hit.det==2002||hit.det==2011||hit.det==2012)",
        "colz"); gPad->SetLogz();

// production-z & hit-z (incl. 577)
.L scripts/whereFrom.C+; whereFrom(T, {2001,2002,2011,2012,577});

// GEM 577 slices and energy
.L scripts/sliceXY.C+;           sliceXY(577);
.L scripts/sliceEnergy_logY.C+;  sliceEnergy_logY(577);

// electrons (e±) in GEM 577
T->Draw("hit.y:hit.x>>hXYe(240,-1200,1200,240,-1200,1200)",
        "hit.det==577 && abs(hit.pid)==11","colz"); gPad->SetLogz();
```

---

## 10) Troubleshooting

- **Empty or zero production-z:** ensure you join `hit.trid` to `part.trid` (use `whereFrom.C`).
- **GDML import errors (XInclude):** flatten with `xmllint --xinclude` before importing.
- **Sparse 2D plots:** enable logarithmic color scale (`gPad->SetLogz()`) and use appropriate binning.

---

## 11) Results (snapshot from these runs)

- **Top secondary‑photon detectors:** 2002, 2001, 2012, 2011 (followed by 38, 2008, …).
- **Production‑vertex z (mm)** for those four:
  - det **2002**: mean ≈ **774** (σ ≈ **96**)
  - det **2001**: mean ≈ **438** (σ ≈ **135**)
  - det **2012**: mean ≈ **401** (σ ≈ **120**)
  - det **2011**: mean ≈ **492** (σ ≈ **145**)
- **Hit‑z** for the same detectors clusters around **19.5–21.0 m**, consistent with the detector‑pipe/GEM region.
- **GEM 577 (photons):** ⟨prod z⟩ ≈ **705.6 mm** (RMS **221.5 mm**), ⟨hit z⟩ ≈ **20.5 m**.
- **Electrons in GEM 577:** multiple compact x–y clusters; reproduce with the electron commands in §7.

> Exact numbers depend on the simulation configuration and statistics.

---

**Author:** Mohnish Sao  
**Advisor:** Dr. Wenliang “Bill” Li
