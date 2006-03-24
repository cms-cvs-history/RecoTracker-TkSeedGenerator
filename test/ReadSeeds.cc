// File: ReadSeeds.cc
// Description:  see ReadSeeds.h
// Author:  O. Gutsche
// Creation Date:  OGU Aug. 1 2005 Initial version.
//
//--------------------------------------------
#include <memory>
#include <string>
#include <iostream>

#include "RecoTracker/TkSeedGenerator/test/ReadSeeds.h"

#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
#include "FWCore/Framework/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GeomDetType.h"
#include "Geometry/CommonDetUnit/interface/GeomDetUnit.h"



ReadSeeds::ReadSeeds(edm::ParameterSet const& conf) : 
  conf_(conf)
{
}

// Virtual destructor needed.
ReadSeeds::~ReadSeeds() { }  

// Functions that gets called by framework every event
void ReadSeeds::analyze(const edm::Event& e, const edm::EventSetup& es)
{
  using namespace edm;
  
  // Step A: Get Inputs 
  edm::Handle<TrajectorySeedCollection> coll;
  e.getByType(coll);
  
  std::cout <<" FOUND "<<(coll.product())->size()<<" Seeds Hits"<<std::endl;
  
  
}

