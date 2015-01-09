/// @file   read-tpc-raw.C
/// @author Matthias.Richter@scieq.net
/// @date   2015-01-06
/// @brief  Read TPC Raw Data and convert to tree
///
/// The macro illustrates the reading of TPC raw data files using the AliAltroRawStreamV3
/// decoder. The file names of input files are read from the standard input
/// Usage:
/// ls rawdata*.root | aliroot -b -q -l read-tpc-raw.C
/// # replace 'ls rawdata*.root' by appropriate command printing the raw file names
///

// for performance reasons, the macro is compiled into a temporary library
// and the compiled function is called
// Note: when using new classes the corresponding header files need to be
// add in the include section
#if defined(__CINT__) && !defined(__MAKECINT__)
{
  gSystem->AddIncludePath("-I$ROOTSYS/include -I$ALICE_ROOT/include");
  TString macroname=gInterpreter->GetCurrentMacroName();
  macroname+="+";
  gROOT->LoadMacro(macroname);
  read_tpc_raw();
}
#else

#include "AliRawReader.h"
#include "AliAltroRawStreamV3.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TSystem.h"
#include "TList.h"
#include "TPad.h"
#include "TH1.h"
#include "TGrid.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <memory>
using namespace std;

void read_tpc_raw()
{
  const int maxEvent=-1;
  const int maxChannelLength=1024;
  int fileCount=0;
  TGrid* pGrid=NULL;
  TString line;
  TString targetFileName("tpc-raw-statistics.root");
  Int_t DDLNumber=-1;
  Int_t HWAddress=-1;
  Int_t RCUId=-1;
  Int_t ChannelLength=maxChannelLength;
  Int_t Signals[maxChannelLength];
  Int_t SignalDiffs[maxChannelLength];

  TTree *tpcrawstat = NULL;
  /// It seems, the tree-approach is not suitable for such an
  /// amount of data, the processing hangs when saving the tree
  // tpcrawstat=new TTree("tpcrawstat","TPC RAW statistics");
  if (tpcrawstat) {
    tpcrawstat->Branch("DDLNumber"    ,&DDLNumber    ,"DDLNumber/I");
    tpcrawstat->Branch("HWAddress"    ,&HWAddress    ,"HWAddress/I");
    tpcrawstat->Branch("RCUId"        ,&RCUId        ,"RCUId/I");
    tpcrawstat->Branch("ChannelLength",&ChannelLength,"ChannelLength/I");
    tpcrawstat->Branch("Signals"      ,Signals       ,"Signals[ChannelLength]/i");
    tpcrawstat->Branch("SignalDiffs"  ,SignalDiffs   ,"SignalDiffs[ChannelLength]/i");
  }

  TH1* hSignalDiff=NULL;
  Int_t binMargin=50; // some margin on both sides of the signal distribution
  Int_t nBins=2*(maxChannelLength+binMargin)+1;
  hSignalDiff=new TH1D("hSignalDiff", "Differences in TPC RAW signal", nBins, -nBins/2, nBins/2);

  line.ReadLine(cin);
  while (cin.good()) {
    if (pGrid==NULL && line.BeginsWith("alien://")) {
      pGrid=TGrid::Connect("alien");
      if (!pGrid) return;
    }
    cout << "open file " << fileCount << " '" << line << "'" << endl;    
    AliRawReader* rawreader=AliRawReader::Create(line);
    AliAltroRawStreamV3* altrorawstream=new AliAltroRawStreamV3(rawreader);
    if (!rawreader || !altrorawstream) {
      cerr << "error: can not open rawreader or altrostream for file " << line << endl;
    } else {
      fileCount++;
      rawreader->RewindEvents();
      int eventCount=0;
      if (!rawreader->NextEvent()) {
    	cout << "info: no events found in " << line << endl;
      } else {
    	do {
    	  cout << "processing file " << line << " event " << eventCount << endl;
    	  altrorawstream->Reset();
	  altrorawstream->SelectRawData("TPC");
    	  while (altrorawstream->NextDDL()) {
    	    DDLNumber=altrorawstream->GetDDLNumber();
	    cout << " reading event " << std::setw(4) << eventCount
		 << "  DDL " << std::setw(4) << DDLNumber
		 << " (" << line << ")"
		 << endl;
    	    while (altrorawstream->NextChannel()) {
    	      HWAddress=altrorawstream->GetHWAddress();
    	      memset(Signals, 0, maxChannelLength*sizeof(Int_t));
    	      memset(SignalDiffs, 0, maxChannelLength*sizeof(Int_t));
    	      Int_t lastSignal=0;
    	      if (!altrorawstream->IsChannelBad()) {
    	    	while (altrorawstream->NextBunch()) {
    	    	  // process all signal values of the bunch and set the
    	    	  // according time bins in the buffer
    	    	  Int_t startTime=altrorawstream->GetStartTimeBin();
    	    	  Int_t bunchLength=altrorawstream->GetBunchLength();
    	    	  const UShort_t* signals=altrorawstream->GetSignals();
    	    	  Int_t i=0;
    	    	  // cout << "reading bunch:"
    	    	  //      << "  DDL "       << DDLNumber
    	    	  //      << "  channel "   << HWAddress
    	    	  //      << "  startTime " << startTime
    	    	  //      << "  length "    << bunchLength
    	    	  //      << endl;
    	    	  for (; i<bunchLength; i++) {
		    int timeBin=startTime-i;
    	    	    //if (timeBin>=maxChannelLength) continue;
    	    	    Signals[timeBin]=signals[i];
    	    	    SignalDiffs[timeBin]=Signals[timeBin];
    	    	    if (timeBin<maxChannelLength) {
    	    	      SignalDiffs[timeBin]-=Signals[timeBin+1];
    	    	    }
    	    	  } // end of bunch signal loop
    	    	  // we are now behind the bunch, have to set the difference
    	    	  // value in the first bin of the gap, where the signal value
    	    	  // is assumed to be zero again
    	    	  if (startTime-i>=0) {
    	    	    SignalDiffs[startTime-i]=-Signals[startTime-i+1];
    	    	  }
    	    	} // end of bunch loop
    	      } // masked bad channel
	      //	      cout << "   DDL " << DDLNumber << "  Channel " << HWAddress << endl;
	      if (tpcrawstat) {
		tpcrawstat->Fill();
	      }
	      if (hSignalDiff) {
		for (int i=0; i<maxChannelLength; i++) {
		  hSignalDiff->Fill(SignalDiffs[i], 1);
		}
	      }
    	    } // end of channel loop
    	  } // end of ddl loop
	  cout << "finished event " << eventCount << endl;
    	  eventCount++;
    	} while (rawreader->NextEvent() && (maxEvent<0 || eventCount<maxEvent));
      }
      if (rawreader) delete rawreader;
      rawreader=NULL;
      if (altrorawstream) delete altrorawstream;
      altrorawstream=NULL;
    }
    line.ReadLine(cin);
  }

  cout << " total " << fileCount << " file(s) " << endl;
  // TODO: create statistics from the total equipment size of the TPC

  TFile* of=TFile::Open(targetFileName, "RECREATE");
  if (!of || of->IsZombie()) {
    cerr << "can not open file " << targetFileName << endl;
    return;
  }

  of->cd();
  if (tpcrawstat) {
    tpcrawstat->Print();
    tpcrawstat->Write();
  }
  if (hSignalDiff) {
    hSignalDiff->Write();
  }
  of->Close();
}
#endif
