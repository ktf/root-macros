/// @file   drawtracks.C
/// @author Matthias.Richter@scieq.net
/// @date   2015-03-04
/// @brief  Very simplistic macro to draw HLT TPC spacepoints and tracks
///
/// This first version is based on a little tool macro from spring 2011
/// Expected input files in the folder 'eventdir':
///   - binary TPC clusters per partition following the naming convention
///     event_TPC:CLUSTERS_<data_spec>, e.g. event_TPC:CLUSTERS_0x06060101
///   - clusters.list: text file with complete paths to binary cluster blocks
///   - binary TPC raw clusters per partition following the naming convention
///     event_TPC:CLUSTRAW_<data_spec>
///   - binary TPC track block event_TPC:HLTTRACK_0x23000500
///
/// Other naming conventions can easily be adjusted by editing the macro.
///
/// The number of drawn tracks and clusters can be reduced by using the min
/// max range or the direct index.
void drawtracks(const char* eventdir,
		int runno,
		const char* cdbURI,
		bool merged=kTRUE,
		const char* option="",
		int minTrack=-1,
		int maxTrack=-1,
		int drawTrack=-1
		)
{
AliHLTMisc::Instance().InitCDB(cdbURI);;
AliHLTMisc::Instance().SetCDBRunNo(runno);
AliHLTMisc::Instance().InitMagneticField();
AliHLTSystem* pHLT=AliHLTPluginBase::GetInstance();
pHLT->LoadComponentLibraries("libAliHLTUtil.so libAliHLTTPC.so");
AliHLTTPCSpacePointContainer tpcpoints;
AliHLTTPCRawSpacePointContainer tpcrawpoints;
AliHLTComponentDataType dt;
AliHLTComponent::SetDataType(dt, "CLUSTERS", "TPC ");
TString listfile=eventdir;listfile+="/";listfile+="clusters.list";
tpcpoints.AddInputBlocks(listfile, dt);
AliHLTComponent::SetDataType(dt, "CLUSTRAW", "TPC ");
 for (int slice=0; slice<36; slice++) {
   for (int part=0; part<6; part++) {
     UInt_t specification=(slice<<24)|(slice<<16)|(part<<8)|part;
     TString rawfile;rawfile.Form("%s/event_TPC:CLUSTRAW_0x%08x", eventdir, specification);
     tpcrawpoints.AddInputBlock(rawfile, dt, specification);
   }
 }
TClonesArray array("AliHLTGlobalBarrelTrack");
if (merged) {
  TString trackfile=eventdir; trackfile+="/"; trackfile+="event_TPC:HLTTRACK_0x23000500";
  AliHLTGlobalBarrelTrack::ReadTracks(trackfile, array);
} else {
  TString trackfile=eventdir; trackfile+="/"; trackfile+="slicetracks.list";
  AliHLTGlobalBarrelTrack::ReadTrackList(trackfile, array);
}
c1 = new TCanvas("c1","c1",1200,1200);

 TH2* residualY=new TH2F("residualY", "residualY", 180, 70, 250, 400, -5.1, 5.1);
 TH2* residualZ=new TH2F("residualZ", "residualZ", 180, 70, 250, 400, -5.1, 5.1);
 TH2* residualP=new TH2F("residualP", "residualP", 180, 70, 250, 1000, -50., 50.);
 TH2* residualT=new TH2F("residualT", "residualT", 100, 0, 1000, 1000, -1000., 1000.);

 if (minTrack<0) minTrack=0;
 if (maxTrack<0 || maxTrack>=array.GetEntriesFast()) maxTrack=array.GetEntriesFast();
 for (int tix=0; tix<array.GetEntriesFast(); tix++) {
   if (((tix+1)%100) == 0) cout << tix << " tracks" << endl;
   AliHLTGlobalBarrelTrack* track=(AliHLTGlobalBarrelTrack*)array[tix];
   AliHLTTPCTrackGeometry* trackpoints=new AliHLTTPCTrackGeometry;
   trackpoints->CalculateTrackPoints(*track);
   track->SetTrackGeometry(trackpoints);
   //trackspacepoints->Print();
   if (drawTrack<0 || tix==drawTrack) {
     trackpoints->Draw(option);
     cout << " track " << tix << " id " << track->GetID() << ": " << track->GetNumberOfPoints() << " space points" << endl;
   }

   AliHLTSpacePointContainer& spacepoints=tpcpoints;
   spacepoints.SetTrackID(track->GetID(), track->GetPoints(), track->GetNumberOfPoints());

   AliHLTTPCSpacePointContainer* trackspacepoints=trackpoints->ConvertToSpacePoints();
   TString option1=option;
   option1+=" markersize=3";
   if (drawTrack<0 || tix==drawTrack) {
     //trackspacepoints->Draw(option1);
     //cout << "track points: " << endl; trackspacepoints->Print();
   }
 }

 for (int tix=minTrack; tix<maxTrack; tix++) {
   if (drawTrack<0 || tix==drawTrack) {
     if (((tix+1)%100) == 0) cout << tix << " tracks" << endl;
     AliHLTGlobalBarrelTrack* track=(AliHLTGlobalBarrelTrack*)array[tix];
     AliHLTTPCTrackGeometry* trackpoints=(AliHLTTPCTrackGeometry*)track->GetTrackGeometry();
     AliHLTSpacePointContainer& spacepoints=tpcpoints;
     int result=track->AssociateSpacePoints(trackpoints, spacepoints);
     if (result>=0 && result!=track->GetNumberOfPoints()) {
       cout << "associated only " << result << " of " << track->GetNumberOfPoints() << " space point(s) for track " << tix << " with id " << track->GetID() << endl;
     }
     track->SetSpacePointContainer(&spacepoints);
     trackpoints->FillResidual(0, residualY);
     trackpoints->FillResidual(1, residualZ);
     trackpoints->FillRawResidual(0, residualP, &tpcrawpoints);
     trackpoints->FillRawResidual(1, residualT, &tpcrawpoints);

     AliHLTSpacePointContainer* associatedtrackpoints=trackpoints->ConvertToSpacePoints(true);

     TString option2=option;
     option2+=" markercolor=6 markersize=2";
     if (drawTrack<0 || tix==drawTrack) {
       associatedtrackpoints->Draw(option2);
       //cout << "associated track points: " << endl; associatedtrackpoints->Print();
     }
     //track->Draw(option);
     AliHLTSpacePointContainer* usedspacepoints=spacepoints.SelectByTrack(track->GetID());
     TString option3=option;
     option3+=" markercolor=7";
     usedspacepoints->Draw(option3);
     //cout << "used space points: " << endl; usedspacepoints->Print();
   }
 }

AliHLTSpacePointContainer* unusedtpcpoints=tpcpoints.SelectByTrack(-1);
for (int t=minTrack; t<maxTrack && false; t++) {
  if (((t+1)%100) == 0) cout << t << " tracks" << endl;
  AliHLTGlobalBarrelTrack* track=(AliHLTGlobalBarrelTrack*)array[t];
  AliHLTTPCTrackGeometry* trackpoints=(AliHLTTPCTrackGeometry*)track->GetTrackGeometry();
  trackpoints->SetVerbosity(1);
  int result=trackpoints->AssociateUnusedSpacePoints(*unusedtpcpoints);
  AliHLTSpacePointContainer* addspacepoints=unusedtpcpoints->SelectByTrack(track->GetID());
  TString option4=option;
  option4+=" markercolor=8";
  if (drawTrack<0 || t==drawTrack) addspacepoints->Draw(option4);
  if (result>=0) {
    //cout << "associated " << result << " unused space point(s) with track " << t << endl;
  }
}
cout << "tpc spacepoints: " << tpcpoints.GetNumberOfSpacePoints() << endl;
cout << "unused spacepoints: " << unusedtpcpoints->GetNumberOfSpacePoints() << endl;

 TFile* output=TFile::Open("projections.root", "RECREATE");
 TH1* projUnused=unusedtpcpoints->DrawProjection("xy");
 projUnused->SetName("xyUnused");
 projUnused->SetTitle("xy projection unused clusters");
 projUnused->Write();
 if (true) {
   AliHLTSpacePointContainer* remainingpoints=unusedtpcpoints->SelectByTrack(-1);
   TString option5=option;
   option5+=" markercolor=9";
   //unusedtpcpoints->Draw(option5);
   c2 = new TCanvas("c2","c2",1200,1200);
   remainingpoints->Draw(option5);
   cout << "remaining spacepoints: " << remainingpoints->GetNumberOfSpacePoints() << endl;
   TH1* projRemaining=remainingpoints->DrawProjection("xy");
   projRemaining->SetName("xyRemaining");
   projRemaining->SetTitle("xy projection remaining clusters");
   projRemaining->Write();
 }

 residualY->Write();
 residualZ->Write();
 residualP->Write();
 residualT->Write();

 TTree* tpcpointcoordinates=tpcpoints.FillTree("tpccoordinates", "TPC space point coordinates");
 if (tpccoordinates) tpccoordinates->Write();
 TTree* tpcrawpointcoordinates=tpcrawpoints.FillTree("tpcrawcoordinates", "TPC raw space point coordinates");
 if (tpcrawcoordinates) tpcrawcoordinates->Write();

 output->Close();
}
