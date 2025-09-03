void sliceXY_markers() {
  auto f = TFile::Open("remollout.root");
  auto T = (TTree*)f->Get("T");
  const double centers[4] = {19500, 20000, 20500, 21000};
  TCanvas* c = new TCanvas("c","slices (markers)",100,100,1000,800);
  c->Divide(2,2);

  for(int i=0; i<4; ++i) {
    double zc   = centers[i];
    double zmin = zc - 50, zmax = zc + 50;
    c->cd(i+1);

    // Create a temporary histogram name
    TString hname = TString::Format("hXY_mark_%d", i);
    TString cut   = TString::Format("hit.det==577 && hit.z>%g && hit.z<%g", zmin, zmax);

    // Draw points instead of a color map
    T->Draw(
      TString::Format("hit.y:hit.x>>%s(200,-1200,1200,200,-1200,1200)", hname.Data()),
      cut.Data(),
      "P"
    );

    // Style the markers
    auto hist = (TH2*)gDirectory->Get(hname);
    hist->SetMarkerStyle(20);
    hist->SetMarkerSize(0.5);
    hist->SetTitle(TString::Format("GEM Argon slice Z=[%g, %g] mm", zmin, zmax));
    hist->GetXaxis()->SetTitle("x [mm]");
    hist->GetYaxis()->SetTitle("y [mm]");
  }

  c->SaveAs("all_slices_XY_markers.pdf");
}
