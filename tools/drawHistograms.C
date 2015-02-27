void drawHistograms(const char* filename, int colw=3, int colh=2, int offset=0, float statScale=0.0)
{
  TFile* file=TFile::Open(filename);
  if (!file || file->IsZombie()) {
    cerr << "can not open file " << filename << endl;
    return;
  }

  TList* keys=file->GetListOfKeys();
  if (!keys || keys->GetEntries()==0) {
    cerr << "can not find any keys in file " << filename << endl;
    return;
  }

  gStyle->SetFillColor(0);
  if (statScale>0.) {
    float statW=gStyle->GetStatW();
    float statH=gStyle->GetStatH();
    statW*=statScale;
    statH*=statScale;
    gStyle->SetStatW(statW);
    gStyle->SetStatH(statH);
  }
  TString cname(filename); cname.ReplaceAll(".root","");
  int width=400*colw;
  int height=400*colh;
  TCanvas* c1=new TCanvas("c1",cname, width,height);
  c1->Divide(colw,colh);
  int padno=1;
  TObject* pObj=NULL;
  TObject* pKey=NULL;
  TIter nextkey(keys);

  // skip a number of histograms
  for (int i=0; i<offset; i++) pKey=nextkey();

  while ((pKey=nextkey())!=NULL && padno<=(colw*colh)) {
    file->GetObject(pKey->GetName(), pObj);
    if (!pObj || !pObj->InheritsFrom("TH1")) continue;
    c1->cd(padno++);
    pObj->Draw();
  }
  c1->cd();
  TString outfile(filename);
  outfile.ReplaceAll(".root","_plot.png");
  c1->SaveAs(outfile);
  outfile.ReplaceAll(".png",".root");
  c1->SaveAs(outfile);  
}
