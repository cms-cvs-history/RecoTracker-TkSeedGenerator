#ifndef CombinatorialSeedGeneratorFromMixed_H
#define CombinatorialSeedGeneratorFromMixed_H

/** \class CombinatorialSeedGeneratorFromMixed
 *  A concrete seed generator providing seeds constructed 
 *  from combinations of hits in pairs of SiStrip layers 
 */
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"    
#include "RecoTracker/TkTrackingRegions/interface/GlobalTrackingRegion.h"
#include "RecoTracker/TkSeedGenerator/interface/SeedGeneratorFromLayerPairs.h"
#include "RecoTracker/TkHitPairs/interface/MixedSeedLayerPairs.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHitCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DMatchedLocalPosCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DLocalPosCollection.h"


class CombinatorialSeedGeneratorFromMixed : public SeedGeneratorFromLayerPairs {
 public:
 
  CombinatorialSeedGeneratorFromMixed(const edm::ParameterSet& conf);
  ~CombinatorialSeedGeneratorFromMixed(){delete stripLayers;}

  void init(const SiPixelRecHitCollection &collpxl,
	    const SiStripRecHit2DMatchedLocalPosCollection &collmatch,
	    const SiStripRecHit2DLocalPosCollection &collstereo ,
	    const SiStripRecHit2DLocalPosCollection &collrphi,
	    const edm::EventSetup& c);
  void  run(TrajectorySeedCollection &,const edm::EventSetup& c);
 private:
  //  edm::ParameterSet conf_;
  GlobalTrackingRegion region;
  MixedSeedLayerPairs* stripLayers;
 
};
#endif

