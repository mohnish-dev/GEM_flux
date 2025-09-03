// whereFrom_df.C
{
  // 1) open the file and make an RDataFrame
  ROOT::RDataFrame df("T","remollout.root");

  // 2) apply a filter to only keep hits in detector detID
  const int detID = 577;
  auto dfHits = df.Filter( Form("hit.det==%d", detID) );

  // 3) define a new column “vz” which is the production-vertex z of the parent
  //    (hit.trackID gives the index into the part array)
  auto dfVz = dfHits.Define("vz",
    []( const std::vector<remollEventParticle_t> &parts,
        const std::vector<remollGenericDetectorHit_t> &hits ){
      // for each hit, extract parts[trackID].v_z
      std::vector<double> out;
      out.reserve(hits.size());
      for(auto &h : hits){
        out.push_back( parts[h.trackID].v_z );
      }
      return out;
    },
    {"part","hit"}
  );

  // 4) book and draw a histogram of vz
  auto h = dfVz.Histo1D(
    {"h","Origin z of hits (det="+std::to_string(detID)+")",200,19400,21000},
    "vz"
  );

  TCanvas c("c","where hits originate",800,600);
  c.SetLogy();
  h->Draw();
  c.SaveAs( Form("origin_z_det%d.pdf",detID) );
}
