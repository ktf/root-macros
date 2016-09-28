
#include "AliHLTMisc.h"
#include "AliHLTDataDeflater.h"
#include "AliHLTDataDeflaterHuffman.h"
#include "AliHLTTPCRawCluster.h"
#include "AliCDBEntry.h"
#include "TObject.h"
#include "TFile.h"
#include "TStopwatch.h"
#include <iostream>
#include <fstream>

typedef vector<unsigned char> databuffer_t;

int read_file(const char* filename, databuffer_t& buffer)
{
  std::ifstream input(filename, std::ifstream::binary);
  buffer.clear();
  if (input) {
    // get length of file:
    input.seekg (0, input.end);
    int length = input.tellg();
    input.seekg (0, input.beg);

    // allocate memory:
    buffer.resize(length);

    // read data as a block:
    input.read(reinterpret_cast<char*>(&buffer[0]), length);
    if (!input.good()) {
      buffer.clear();
      std::cerr << "failed to read " << length << " byte(s) from file " << filename << std::endl;
    }

    input.close();
    return buffer.size();
  }
  std::cerr << "failed to open file " << filename << std::endl;
  return -1;
}

ostream& operator<<(ostream& stream, const AliHLTTPCRawCluster& cluster)
{
  stream << "AliHLTTPCRawCluster:"
	 << " " << cluster.GetPadRow()
	 << " " << cluster.GetPad()
	 << " " << cluster.GetTime()
	 << " " << cluster.GetSigmaPad2()
	 << " " << cluster.GetSigmaTime2()
	 << " " << cluster.GetCharge()
	 << " " << cluster.GetQMax();
  return stream;
}

class RawClusterArray {
public:
  RawClusterArray() : mBuffer(), mNClusters(0), mClusters(NULL), mClustersEnd(NULL) {}
  RawClusterArray(const char* filename) : mBuffer(), mNClusters(0), mClusters(NULL), mClustersEnd(NULL) {
    init(filename);
  }
  RawClusterArray(unsigned char* buffer, int size) : mBuffer(), mNClusters(0), mClusters(NULL), mClustersEnd(NULL) {
    init(buffer, size);
  }
  ~RawClusterArray() {}

  int init(const char* filename) {
    std::ifstream input(filename, std::ifstream::binary);
    mBuffer.clear();
    if (input) {
      // get length of file:
      input.seekg (0, input.end);
      int length = input.tellg();
      input.seekg (0, input.beg);

      // allocate memory:
      mBuffer.resize(length);

      // read data as a block:
      input.read(reinterpret_cast<char*>(&mBuffer[0]),length);
      if (!input.good()) {
	mBuffer.clear();
	std::cerr << "failed to read " << length << " byte(s) from file " << filename << std::endl;
      }

      input.close();
      return init();
    }
    std::cerr << "failed to open file " << filename << std::endl;
    return -1;
  }

  int init(unsigned char* buffer, int size) {
    if (!buffer || size <= 0) return -1;
    mBuffer.resize(size);
    memcpy(&mBuffer[0], buffer, size);
    return init();
  }

  int GetNClusters() const {return mNClusters;}

  AliHLTTPCRawCluster* begin() {return mClusters;}

  AliHLTTPCRawCluster* end() {return mClustersEnd;}

  AliHLTTPCRawCluster& operator[](int i) {
    if (i + 1 > mNClusters) {
      // runtime exeption?
      static AliHLTTPCRawCluster dummy;
      return dummy;
    }
    return *(mClusters + i);
  }

  void print() {print(std::cout);}

  template<typename StreamT>
  StreamT& print(StreamT& stream) {
    std::cout << "RawClusterArray: " << mNClusters << " cluster(s)" << std::endl;
    for (AliHLTTPCRawCluster* cluster = mClusters; cluster != mClustersEnd; cluster++) {
      std::cout << "  " << *cluster;
    }
    return stream;
  }

private:
  int init() {
    if (mBuffer.size() == 0) return 0;
    if (mBuffer.size() < sizeof(AliHLTTPCRawClusterData)) return -1;
    AliHLTTPCRawClusterData& clusterData = reinterpret_cast<AliHLTTPCRawClusterData&>(mBuffer[0]);

    if (clusterData.fCount * sizeof(AliHLTTPCRawCluster) + sizeof(AliHLTTPCRawClusterData) > mBuffer.size()) {
      std::cerr << "Format error, " << clusterData.fCount << " cluster(s) "
		<< "would require "
		<< (clusterData.fCount * sizeof(AliHLTTPCRawCluster) + sizeof(AliHLTTPCRawClusterData))
		<< " byte(s), but only " << mBuffer.size() << " available" << std::endl;
      return clear(-1);
    }

    mNClusters = clusterData.fCount;
    mClusters = clusterData.fClusters;
    mClustersEnd = mClusters + mNClusters;

    return mNClusters;
  }

  int clear(int returnValue) {
    mNClusters = 0;
    mClusters = NULL;
    mClustersEnd = NULL;
    mBuffer.clear();

    return returnValue;
  }

  vector<unsigned char*> mBuffer;
  int mNClusters;
  AliHLTTPCRawCluster* mClusters;
  AliHLTTPCRawCluster* mClustersEnd;
};

AliHLTDataDeflaterHuffman* createHuffmanDeflater(const char* huffmanConfigurationFile = NULL)
{
  // huffman deflater
  AliHLTDataDeflaterHuffman* deflater = new AliHLTDataDeflaterHuffman(false);

  if (!deflater->IsTrainingMode()) {
    TObject* pConf=NULL;
    if (huffmanConfigurationFile == NULL) {
      TString cdbPath("HLT/ConfigTPC/");
      cdbPath += "TPCDataCompression";
      cdbPath += "HuffmanTables";
      pConf=AliHLTMisc::Instance().ExtractObject(AliHLTMisc::Instance().LoadOCDBEntry(cdbPath));
    } else {
      // load huffman table directly from file
      TFile* tablefile = TFile::Open(huffmanConfigurationFile);
      if (!tablefile || tablefile->IsZombie()) return NULL;
      TObject* obj = NULL;
      AliCDBEntry* cdbentry = NULL;
      tablefile->GetObject("AliCDBEntry", obj);
      if (obj == NULL || (cdbentry = dynamic_cast<AliCDBEntry*>(obj))==NULL) {
	std::cerr << "can not read configuration object from file " << huffmanConfigurationFile << std::endl;;
	return NULL;
      }
      std::cout << "reading huffman table configuration object from file " << huffmanConfigurationFile << std::endl;
      pConf = cdbentry->GetObject();
    }
    if (!pConf) return NULL;
    if (dynamic_cast<TList*>(pConf)==NULL) {
      std::cerr << "huffman table configuration object of inconsistent type" << std::endl;
      return NULL;
    }
    deflater->InitDecoders(dynamic_cast<TList*>(pConf));
  }

  deflater->AddParameterDefinition("padrow",   6);
  deflater->AddParameterDefinition("pad",     14);
  deflater->AddParameterDefinition("time",    15);
  deflater->AddParameterDefinition("sigmaY2",  8);
  deflater->AddParameterDefinition("sigmaZ2",  8);
  deflater->AddParameterDefinition("charge",  16);
  deflater->AddParameterDefinition("qmax",    10);
  //deflater->EnableStatistics();

  return deflater;
}

int benchClusterCompression(RawClusterArray& ca, AliHLTDataDeflater* pDeflater)
{
  unsigned long long int dummybuffer[32];
  int nBits = 0;
  uint16_t lastPadrow = 0;
  for (AliHLTTPCRawCluster* cluster = ca.begin(); cluster != ca.end(); cluster++) {
    unsigned int parameterID = 0;
    pDeflater->InitBitDataOutput(reinterpret_cast<unsigned char*>(dummybuffer), sizeof(dummybuffer));
    uint16_t padrow = cluster->GetPadRow();
    unsigned long long int value = padrow - lastPadrow;
    lastPadrow = padrow;
    pDeflater->OutputParameterBits(parameterID++, value);
    float padval = cluster->GetPad() * 60;
    value = padval;
    pDeflater->OutputParameterBits(parameterID++, value);
    float timeval = cluster->GetTime() * 25;
    value = timeval;
    pDeflater->OutputParameterBits(parameterID++, value);
    float sigmaPad2val = cluster->GetSigmaPad2() * 25;
    value = sigmaPad2val;
    if (value > 255) value = 255;
    pDeflater->OutputParameterBits(parameterID++, value);
    float sigmaTime2val = cluster->GetSigmaTime2() * 10;
    value = sigmaTime2val;
    if (value > 255) value = 255;
    pDeflater->OutputParameterBits(parameterID++, value);
    value = cluster->GetCharge();
    pDeflater->OutputParameterBits(parameterID++, value);
    value = cluster->GetQMax();
    pDeflater->OutputParameterBits(parameterID++, value);

    nBits += pDeflater->GetBitDataOutputSizeBytes()*8 + 7-pDeflater->GetCurrentBitOutputPosition();
    //std::cout << *cluster << " " << pDeflater->GetBitDataOutputSizeBytes() << " " << pDeflater->GetCurrentBitOutputPosition() << std::endl;
  }
  std::cout << "wrote " << ca.GetNClusters() << " cluster(s)";
  if (ca.GetNClusters() > 0)
    std::cout << " " << (nBits+7)/8 << " byte(s) " << float(nBits)/(77 * ca.GetNClusters());
  std::cout << std::endl;
  return ca.GetNClusters();
}

int standalone_tpc_cluster_compression(const char* filename = NULL)
{
  AliHLTDataDeflaterHuffman* pDeflater = createHuffmanDeflater("huffmanConfiguration.root");
  //pDeflater->PrintTable();

  TStopwatch timer;
  TString line;
  int totalNofClusters = 0;
  int fileCount = 0;
  while (line.ReadLine(cin) && cin.good()) {
    RawClusterArray ca(line.Data());
    timer.Continue();
    totalNofClusters += benchClusterCompression(ca, pDeflater);
    timer.Stop();
    ++fileCount;
  }

  std::cout << fileCount << " file(s) processed"
	    << ", " << totalNofClusters << " cluster(s)"
	    << "  realtime " << timer.RealTime()
	    << " cputime " << timer.CpuTime()
	    << std::endl;
  return 0;
}
