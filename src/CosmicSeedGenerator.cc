//
// Package:         RecoTracker/TkSeedGenerator
// Class:           GlobalPixelLessSeedGenerator
// 

#include <iostream>
#include <memory>
#include <string>

#include "RecoTracker/TkSeedGenerator/interface/CosmicSeedGenerator.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DCollection.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
#include "FWCore/Framework/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

using namespace std;
CosmicSeedGenerator::CosmicSeedGenerator(edm::ParameterSet const& conf) : 
  conf_(conf) ,cosmic_seed(conf)
 {
  edm::LogInfo ("CosmicSeedGenerator")<<"Enter the CosmicSeedGenerator";
  produces<TrajectorySeedCollection>();
}


// Virtual destructor needed.
CosmicSeedGenerator::~CosmicSeedGenerator() { }  

// Functions that gets called by framework every event
void CosmicSeedGenerator::produce(edm::Event& e, const edm::EventSetup& es)
{

 

  
  // get Inputs
  edm::Handle<SiStripRecHit2DCollection> rphirecHits;
  e.getByLabel("SiStripRecHits2D","rphiRecHit" ,rphirecHits);
  edm::Handle<SiStripRecHit2DCollection> stereorecHits;
  e.getByLabel("SiStripRecHits2D","stereoRecHit" ,stereorecHits);
  edm::Handle<SiStripMatchedRecHit2DCollection> matchedrecHits; 	 
  e.getByLabel("SiStripRecHits2D","matchedRecHit" ,matchedrecHits);
 

  std::auto_ptr<TrajectorySeedCollection> output(new TrajectorySeedCollection);
  //
 
  cosmic_seed.init(*stereorecHits,*rphirecHits,*matchedrecHits,es);
 
  // invoke the seed finding algorithm
  cosmic_seed.run(*output,es);

  // write output to file
  LogDebug("Algorithm Performance")<<" number of seeds = "<< output->size();


  e.put(output);
}
