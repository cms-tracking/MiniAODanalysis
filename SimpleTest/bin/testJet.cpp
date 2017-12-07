#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "FWCore/FWLite/interface/FWLiteEnabler.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/Math/interface/deltaPhi.h"

#include "PhysicsTools/FWLite/interface/CommandLineParser.h" 


#include "TFile.h"

#include<iostream>

#include <typeinfo>

#include "TROOT.h"
int main (int argc, char* argv[]) 
{

   FWLiteEnabler::enable();

   std::string eos = "root://eoscms.cern.ch//";
   std::string xrd = "root://cms-xrd-global.cern.ch//";

   // Tell people what this analysis code does and setup default options.
   optutl::CommandLineParser parser ("Playing with jets");

   // change default output filename
   parser.stringValue ("outputFile") = "jetInfo.dat";
   
   // Parse the command line arguments
   parser.parseArguments (argc, argv);


  std::vector<std::string> inputFiles = parser.stringVector("inputFiles");
  
  edm::InputTag jetLabel ("slimmedJets");


  auto avgHits  = [](auto jet)->float {
    int nhits=0;
    int cands=0;
    assert(jet.numberOfDaughters());
    for (auto i=0U; i<jet.numberOfDaughters(); ++i) {
      if (jet.daughter(i))
        std::cout << typeid(*jet.daughter(i)).name() << std::endl;
      auto pd = dynamic_cast<pat::PackedCandidate const *>(jet.daughter(i));
      if (!pd) continue;
      //auto const & d = reinterpret_cast<pat::PackedCandidate const&>(*jet.daughter(i));
      auto const & d = *pd;
      if (d.charge() != 0 && d.hasTrackDetails()) {
        nhits += d.pixelLayersWithMeasurement();
        ++cands;
      }
    } 
    return cands>0 ?  float(nhits)/float(cands) : -99.0f;
  };

  float bha =0;

  for(auto const & file : inputFiles){
    // open input file (can be located on castor)
    TFile* inFile = TFile::Open((eos+file).c_str());
    if( !inFile ) {
      std::cout << "skipping file " << file << std::endl;
      continue;
    }

    fwlite::Event ev(inFile);

    std::cout << "opeing file " << file << " with " << ev.size() << " events" << std::endl;

    for(ev.toBegin(); !ev.atEnd(); ++ev){
      edm::Handle<std::vector< pat::Jet>> jetHandle;
      ev.getByLabel (jetLabel, jetHandle);
      assert ( jetHandle.isValid() );  
      for (auto const & jet : (*jetHandle)) {
         if (jet.pt()<15.) continue;
         auto h = avgHits(jet);
         bha += h>0 ? h : 0;
      }

    } // events
  }  // files
  std::cout << "tot "<< bha << std::endl;
  return 0;
}
