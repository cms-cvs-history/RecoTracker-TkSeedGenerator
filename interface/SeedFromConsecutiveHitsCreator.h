#ifndef RecoTracker_TkSeedGenerator_SeedFromConsecutiveHitsCreator_H
#define RecoTracker_TkSeedGenerator_SeedFromConsecutiveHitsCreator_H

#include "RecoTracker/TkSeedGenerator/interface/SeedCreator.h"
#include "RecoTracker/TkSeedingLayers/interface/SeedingHitSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

class FreeTrajectoryState;

class SeedFromConsecutiveHitsCreator : public SeedCreator {
public:

  SeedFromConsecutiveHitsCreator( const edm::ParameterSet & cfg):
    thePropagatorLabel(cfg.getParameter<std::string>("propagator")),
    theBOFFMomentum(cfg.existsAs<double>("SeedMomentumForBOFF") ? cfg.getParameter<double>("SeedMomentumForBOFF") : 5.0)
      {
	if (cfg.exists("probCut")){
	  probCut_ = cfg.getParameter<double>("probCut");
	  edm::LogInfo("SeedFromConsecutiveHitsCreator")<<" ready to cut on cluster probability at :"<<probCut_;
	}
	else
	  probCut_ = -1;
	
	if (cfg.exists("maxSeeds"))
	  {
	    maxSeeds_ = cfg.getParameter<int>("maxSeeds");
	    edm::LogInfo("SeedFromConsecutiveHitsCreator")<<" truncating at: "<<maxSeeds_<<" seeds";
	  }
	else
	  maxSeeds_=-1;
      }

  SeedFromConsecutiveHitsCreator( 
      const std::string & propagator = "PropagatorWithMaterial", double seedMomentumForBOFF = -5.0) 
   : thePropagatorLabel(propagator), theBOFFMomentum(seedMomentumForBOFF) {
    probCut_=-1;
    maxSeeds_=-1;
  }

  //dtor
  virtual ~SeedFromConsecutiveHitsCreator(){}

  virtual const TrajectorySeed * trajectorySeed(TrajectorySeedCollection & seedCollection,
						const SeedingHitSet & ordered,
						const TrackingRegion & region,
						const edm::EventSetup& es);
protected:

  virtual bool checkHit(
      const TrajectoryStateOnSurface &,
      const TransientTrackingRecHit::ConstRecHitPointer &hit,
      const edm::EventSetup& es) const { 
    if (probCut_>0){
      double prob=hit->clusterProbability();
      if (prob< probCut_ )
	LogDebug("SeedFromConsecutiveHitsCreator")<<"a bad hit ?"<< hit->hit()->geographicalId().rawId()<<" : "<<prob;
      return (prob>probCut_);
    }else
      return true;
  }

  virtual GlobalTrajectoryParameters initialKinematic(
      const SeedingHitSet & hits, 
      const TrackingRegion & region, 
      const edm::EventSetup& es) const;

  virtual CurvilinearTrajectoryError initialError(
      const TrackingRegion& region, 
      float sinTheta) const;

  virtual const TrajectorySeed * buildSeed(
      TrajectorySeedCollection & seedCollection,
	const SeedingHitSet & hits,
	const FreeTrajectoryState & fts,
	const edm::EventSetup& es) const;

  virtual TransientTrackingRecHit::RecHitPointer refitHit(
      const TransientTrackingRecHit::ConstRecHitPointer &hit, 
      const TrajectoryStateOnSurface &state) const;

protected:
    std::string thePropagatorLabel;
    double theBOFFMomentum;
    double probCut_;
    int maxSeeds_;
};
#endif 
