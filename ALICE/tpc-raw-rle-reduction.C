/// @file   tpc-raw-rle-reduction.C
/// @author Matthias.Richter@scieq.net
/// @date   2015-05-21
/// @brief  Read TPC raw data and collect statistics for ALTRO bunch length and RLE reduction
///
/// The macro illustrates the reading of TPC raw data files using the AliAltroRawStreamV3
/// decoder. The file names of input files are read from the standard input. Each line of input
/// must contain the file name, optionally followed by a list of blank-separated size values
/// for the individual events.
/// Usage:
/// cat config.txt | aliroot -b -q -l tpc-raw-rle-reduction.C
///
/// Event sizes for the config file must be calculated elsewhere, e.g from size ofbinary files
/// of reconstructed TPC clusters.
///
/// # Alternatively, by omitting event size numbers, input files can be sent directly to the
/// macro. Replace 'ls rawdata*.root' by appropriate command printing the raw file names
/// ls rawdata*.root | aliroot -b -q -l tpc-raw-rle-reduction.C
///
/// Changelog:
/// 2015-05-21 initial version

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
  tpc_raw_rle_reduction();
}
#else

#include "AliRawReader.h"
#include "AliAltroRawStreamV3.h"
#include "AliHLTHuffman.h"
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

void tpc_raw_rle_reduction()
{
  ////////////////////////////////////////////////////////////////////////////////
  // macro configuration area
  const int maxEvent=-1;

  // signal bit length
  const int signalBitLength=10;
  const int signalRange=0x1<<signalBitLength;

  // the length of one channel
  const int maxChannelLength=1000;

  ////////////////////////////////////////////////////////////////////////////////

  int fileCount=0;
  TGrid* pGrid=NULL;
  TString line;
  TString targetFileName("tpc-raw-rle-statistics.root");
  Int_t EventNumber=-1;
  Int_t NofClusters=0;
  Int_t DDLNumber=-1;
  Int_t HWAddress=-1;
  Int_t RCUId=-1;
  Int_t NofBunches=0;
  Int_t BunchLength[maxChannelLength];
  Float_t RleReduction=1.;

  TTree *tpcrawbunchlenstat = NULL;
  tpcrawbunchlenstat=new TTree("tpcrawbunchlenstat","TPC RAW BunchLength statistics");
  if (tpcrawbunchlenstat) {
    tpcrawbunchlenstat->Branch("EventNumber"  ,&EventNumber  ,"EventNumber/I");
    tpcrawbunchlenstat->Branch("NofClusters"  ,&NofClusters  ,"NofClusters/I");
    tpcrawbunchlenstat->Branch("DDLNumber"    ,&DDLNumber    ,"DDLNumber/I");
    tpcrawbunchlenstat->Branch("HWAddress"    ,&HWAddress    ,"HWAddress/I");
    tpcrawbunchlenstat->Branch("RCUId"        ,&RCUId        ,"RCUId/I");
    tpcrawbunchlenstat->Branch("NofBunches"   ,&NofBunches   ,"NofBunches/I");
    //    tpcrawbunchlenstat->Branch("BunchLength"  ,BunchLength   ,"BunchLength[NofBunches]/i");
    tpcrawbunchlenstat->Branch("RleReduction" ,&RleReduction ,"RleReduction/F");
  }

  TH1* hRleReduction=NULL;
  Int_t nBins=100;
  hRleReduction=new TH1D("hRleReduction", "TPC RLE reduction", nBins, 0, 10);
  hRleReduction->GetXaxis()->SetTitle("RLE reduction factor");
  hRleReduction->GetYaxis()->SetTitle("counts");
  hRleReduction->GetYaxis()->SetTitleOffset(1.4);

  TH1* hBunchLength=NULL;
  hBunchLength=new TH1D("hBunchLength", "TPC Altro bunch length", maxChannelLength+1, 0, maxChannelLength);
  hBunchLength->GetXaxis()->SetTitle("TPC Altro bunch length");
  hBunchLength->GetYaxis()->SetTitle("counts");
  hBunchLength->GetYaxis()->SetTitleOffset(1.4);

  line.ReadLine(cin);
  while (cin.good()) {
    int iToken=0;
    TObjArray* pTokens=line.Tokenize(" ");
    if (!pTokens || pTokens->GetEntriesFast()==0) {
      line.ReadLine(cin);
      continue;
    }
    TString  filename=pTokens->At(iToken++)->GetName();
    if (pGrid==NULL && filename.BeginsWith("alien://")) {
      pGrid=TGrid::Connect("alien");
      if (!pGrid) return;
    }
    cout << "open file " << fileCount << " '" << filename << "'" << endl;    
    AliRawReader* rawreader=AliRawReader::Create(filename);
    AliAltroRawStreamV3* altrorawstream=new AliAltroRawStreamV3(rawreader);
    if (!rawreader || !altrorawstream) {
      cerr << "error: can not open rawreader or altrostream for file " << filename << endl;
    } else {
      fileCount++;
      rawreader->RewindEvents();
      int eventCount=0;
      if (!rawreader->NextEvent()) {
    	cout << "info: no events found in " << filename << endl;
      } else {
    	do {
    	  cout << "processing file " << filename << " event " << eventCount << endl;
	  EventNumber++;
	  if (iToken<pTokens->GetEntriesFast()) {
	    TString strNofClusters=pTokens->At(iToken++)->GetName();
	    NofClusters=strNofClusters.Atoi();
	  }
    	  altrorawstream->Reset();
	  altrorawstream->SelectRawData("TPC");
    	  while (altrorawstream->NextDDL()) {
    	    DDLNumber=altrorawstream->GetDDLNumber();
	    cout << " reading event " << std::setw(4) << eventCount
		 << "  DDL " << std::setw(4) << DDLNumber
		 << " (" << filename << ")"
		 << endl;
    	    while (altrorawstream->NextChannel()) {
    	      if (altrorawstream->IsChannelBad()) continue;
    	      HWAddress=altrorawstream->GetHWAddress();
	      NofBunches=0;
    	      memset(BunchLength, 0, maxChannelLength*sizeof(Int_t));

	      int channelWordCount=0;
	      while (altrorawstream->NextBunch()) {
    	    	BunchLength[NofBunches]=altrorawstream->GetBunchLength();
		channelWordCount+=BunchLength[NofBunches] + 2; // +2 : bunch length and start time words
		if (hBunchLength) hBunchLength->Fill(BunchLength[NofBunches]);
		// cout << "reading bunch:"
		//      << "  DDL "       << DDLNumber
		//      << "  channel "   << HWAddress
		//      << "  startTime " << startTime
		//      << "  length "    << bunchLength
		//      << endl;
		NofBunches++;
	      } // end of bunch loop

	      channelWordCount+=(4-channelWordCount%4); // align to group of 4 10bit words
	      if (channelWordCount>0) {
		RleReduction=maxChannelLength;
		RleReduction/=channelWordCount;
	      } else {
		RleReduction=0.;
	      }
	      if (hRleReduction) hRleReduction->Fill(RleReduction);

	      if (tpcrawbunchlenstat) {
		tpcrawbunchlenstat->Fill();
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
  if (tpcrawbunchlenstat) {
    tpcrawbunchlenstat->Print();
    tpcrawbunchlenstat->Write();
  }
  if (hRleReduction)  hRleReduction->Write();
  if (hBunchLength)   hBunchLength->Write();

  of->Close();
}
#endif
