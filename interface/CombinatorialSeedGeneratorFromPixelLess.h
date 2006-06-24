#ifndef CombinatorialSeedGeneratorFromPixelLess_H
#define CombinatorialSeedGeneratorFromPixelLess_H

/** \class CombinatorialSeedGeneratorFromPixelLess
 *  A concrete seed generator providing seeds constructed 
 *  from combinations of hits in pairs of SiStrip layers 
 */
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"    
#include "RecoTracker/TkTrackingRegions/interface/GlobalTrackingRegion.h"
#include "RecoTracker/TkSeedGenerator/interface/SeedGeneratorFromLayerPairs.h"
#include "RecoTracker/TkHitPairs/interface/PixelLessSeedLayerPairs.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DMatchedLocalPosCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DLocalPosCollection.h"


class CombinatorialSeedGeneratorFromPixelLess : public SeedGeneratorFromLayerPairs {
 public:
 
  CombinatorialSeedGeneratorFromPixelLess(const edm::ParameterSet& conf);
  ~CombinatorialSeedGeneratorFromPixelLess(){delete stripLayers;}

  void init(const SiStripRecHit2DMatchedLocalPosCollection &collmatch,
	    const SiStripRecHit2DLocalPosCollection &collstereo ,
	    const SiStripRecHit2DLocalPosCollection &collrphi,
	    const edm::EventSetup& c);
  void  run(TrajectorySeedCollection &,const edm::EventSetup& c);
 private:
  //  edm::ParameterSet conf_;
  GlobalTrackingRegion region;
  PixelLessSeedLayerPairs* stripLayers;
 
};
#endif


