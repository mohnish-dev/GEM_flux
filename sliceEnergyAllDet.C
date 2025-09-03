#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TString.h>
#include <vector>

void sliceEnergyByDetectors() {
  // open the file & tree
  auto f = TFile::Open("remollout.root");
  auto T = (TTree*)f->Get("T");
  if (!T) {
    ::Error("sliceEnergyByDetectors","Cannot open tree T in remollout.root");
    return;
  }

  // the four Z‐slice centers (mm)
  const double centers[4]  = {19500, 20000, 20500, 21000};
  const double halfWidth   = 50;     // ±50 mm around each center
  const int    nSlices     = 4;

  // loop over detector IDs 32–35
  for (int det = 32; det <= 35; ++det) {
    // collect only the non-empty histograms
    struct Slice { TH1D* h; double zmin, zmax; };
    std::vector<Slice> good;

    // first pass — make and fill all 4 histos, but keep only the nonzero ones
    for (int i = 0; i < nSlices; ++i) {
      double zc   = centers[i];
      double zmin = zc - halfWidth;
      double zmax = zc + halfWidth;
      TString cut = TString::Format(
        "hit.det==%d && hit.z>%g && hit.z<%g", det, zmin, zmax
      );

      // create the histogram
      TString hname = TString::Format("hE_det%d_%d", det, i);
      auto h = new TH1D(hname,
                        TString::Format("Det %d: Energy (Z=[%g,%g] mm)",
                                        det, zmin, zmax),
                        100, 0, 10);
      h->GetXaxis()->SetTitle("E [MeV]");
      h->GetYaxis()->SetTitle("Counts");

      // fill silently
      T->Draw(TString::Format("hit.e>>%s",hname.Data()), cut, "goff");
      h->SetLineColor(kBlue);

      // keep non-empty ones
      if (h->GetEntries() > 0) {
        good.push_back({h, zmin, zmax});
      } else {
        delete h;
      }
    }

    if (good.empty()) {
      ::Info("sliceEnergyByDetectors",
             "Detector %d has no hits in any slice — skipping", det);
      continue;
    }

    // make a canvas with #pads = number of good slices
    int n = good.size();
    int nx = (n<=2 ? n : 2);
    int ny = (n<=2 ? 1 : (n+1)/2);
    TString cname = TString::Format("c_det%d", det);
    auto c = new TCanvas(cname, TString::Format("Det %d Energy slices",det),
                         100, 100, 400*nx, 300*ny);
    c->Divide(nx, ny);

    // draw each one
    for (int i = 0; i < n; ++i) {
      c->cd(i+1);
      gPad->SetLogx();    // log-scale x if you want to zoom
      gPad->SetLogy();    // log-scale y
      good[i].h->Draw();
    }

    // save out
    TString out = TString::Format("Energy_slices_det%d.pdf", det);
    c->SaveAs(out);
  }
}
