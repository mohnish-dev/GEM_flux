#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TString.h>

void sliceEnergy() {
  // --- open your remoll output and grab the tree
  auto f = TFile::Open("remollout.root");
  auto T = (TTree*)f->Get("T");

  // --- Z‐slice centers in mm
  const double centers[4] = {19500, 20000, 20500, 21000};

  // --- create a canvas with 4 pads
  auto c = new TCanvas("c","Energy slices", 100,100, 1200,800);
  c->Divide(2,2);

  // --- loop over the four slices
  for(int i=0; i<4; ++i) {
    double zc = centers[i];
    double zmin = zc - 50, zmax = zc + 50;

    // select only Argon hits in that Z‐range
    TString cut = TString::Format(
      "hit.det==577 && hit.z>%g && hit.z<%g",
      zmin, zmax
    );

    c->cd(i+1);

    // make a 1D histogram of hit.e (energy in MeV)
    TString hname = TString::Format("hE%d", i);
    TH1D* h = new TH1D(hname, TString::Format("Energy (Z=[%g,%g] mm)", zmin, zmax),
                       100, 0, 10 // 100 bins, 0–10 MeV; adjust as needed
    );
    h->GetXaxis()->SetTitle("E  [MeV]");
    h->GetYaxis()->SetTitle("Counts");

    // fill it
    T->Draw(TString::Format("hit.e>>%s", hname.Data()), cut, "goff");

    // style & draw
    gPad->SetLogy();       // optional: log‐scale y
    h->SetLineColor(kBlue);
    h->Draw();
  }

  // --- save all four histos in one PDF
  c->SaveAs("all_slices_Energy.pdf");
}
