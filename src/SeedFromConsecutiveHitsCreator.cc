#include "RecoTracker/TkSeedGenerator/interface/SeedFromConsecutiveHitsCreator.h"
#include "RecoTracker/TkTrackingRegions/interface/TrackingRegion.h"

#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"
#include "RecoTracker/TkSeedGenerator/interface/FastHelix.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h" 
#include "TrackingTools/Records/interface/TransientRecHitRecord.h" 
#include "TrackingTools/MaterialEffects/interface/PropagatorWithMaterial.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "TrackingTools/GeomPropagators/interface/PropagationExceptions.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"

template <class T> T sqr( T t) {return t*t;}

const TrajectorySeed * SeedFromConsecutiveHitsCreator::trajectorySeed(
    TrajectorySeedCollection & seedCollection,
    const SeedingHitSet & hits,
    const TrackingRegion & region,
    const edm::EventSetup& es)
{
  if ( hits.size() < 2) return 0;

  GlobalTrajectoryParameters kine = initialKinematic(hits, region, es);
  float sinTheta = sin(kine.momentum().theta());

  CurvilinearTrajectoryError error = initialError(region,  sinTheta);
  FreeTrajectoryState fts(kine, error);

  return buildSeed(seedCollection,hits,fts,es); 
}


GlobalTrajectoryParameters SeedFromConsecutiveHitsCreator::initialKinematic(
      const SeedingHitSet & hits, 
      const TrackingRegion & region, 
      const edm::EventSetup& es) const
{
  edm::ESHandle<MagneticField> bfield;
  es.get<IdealMagneticFieldRecord>().get(bfield);

  GlobalTrajectoryParameters kine;

  TransientTrackingRecHit::ConstRecHitPointer tth1 = hits[0];
  TransientTrackingRecHit::ConstRecHitPointer tth2 = hits[1];
  const GlobalPoint& vertexPos = region.origin();

  FastHelix helix(tth2->globalPosition(), tth1->globalPosition(), vertexPos, es);
  if (helix.isValid()) {
    kine = helix.stateAtVertex().parameters();
  } else {
    GlobalVector initMomentum(tth2->globalPosition() - vertexPos);
    initMomentum *= (100./initMomentum.perp()); 
    kine = GlobalTrajectoryParameters(vertexPos, initMomentum, 1, &*bfield);
  } 

  bool isBOFF = ( std::abs(bfield->inTesla(GlobalPoint(0,0,0)).z()) < 1e-3 );
  if (isBOFF && (theBOFFMomentum > 0)) {
    kine = GlobalTrajectoryParameters(kine.position(),
                              kine.momentum().unit() * theBOFFMomentum,
                              kine.charge(),
                              &*bfield);
  }
  return kine;
}



CurvilinearTrajectoryError SeedFromConsecutiveHitsCreator::
   initialError( const TrackingRegion& region, float sinTheta) const
{
  // Set initial uncertainty on track parameters, using only P.V. constraint and no hit
  // information.
  GlobalError vertexErr( sqr(region.originRBound()), 0, sqr(region.originRBound()),
                   0, 0, sqr(region.originZBound()));

  float ptMin = region.ptMin();


  AlgebraicSymMatrix55 C = ROOT::Math::SMatrixIdentity();

// FIXME: minC00. Prevent apriori uncertainty in 1/P from being too small, 
// to avoid instabilities.
// N.B. This parameter needs optimising ...
  float sin2th = sqr(sinTheta);
  float minC00 = 1.0;
  C[0][0] = std::max(sin2th/sqr(ptMin), minC00);
  float zErr = vertexErr.czz();
  float transverseErr = vertexErr.cxx(); // assume equal cxx cyy
  C[3][3] = transverseErr;
  C[4][4] = zErr*sin2th + transverseErr*(1-sin2th);

  return CurvilinearTrajectoryError(C);
}

const TrajectorySeed * SeedFromConsecutiveHitsCreator::buildSeed(
    TrajectorySeedCollection & seedCollection,
    const SeedingHitSet & hits,
    const FreeTrajectoryState & fts,
    const edm::EventSetup& es) const
{
  // get tracker
  edm::ESHandle<TrackerGeometry> tracker;
  es.get<TrackerDigiGeometryRecord>().get(tracker);
  
  // get propagator
  edm::ESHandle<Propagator>  propagatorHandle;
  es.get<TrackingComponentsRecord>().get(thePropagatorLabel, propagatorHandle);
  const Propagator*  propagator = &(*propagatorHandle);
  
  // get updator
  KFUpdator  updator;
  
  // Now update initial state track using information from seed hits.
  
  TrajectoryStateOnSurface updatedState;
  edm::OwnVector<TrackingRecHit> seedHits;
  
  const TrackingRecHit* hit = 0;
  for ( unsigned int iHit = 0; iHit < hits.size(); iHit++) {
    hit = hits[iHit]->hit();
    TrajectoryStateOnSurface state = (iHit==0) ? 
      propagator->propagate(fts,tracker->idToDet(hit->geographicalId())->surface())
      : propagator->propagate(updatedState, tracker->idToDet(hit->geographicalId())->surface());
    if (!state.isValid()) return 0;
    
    TransientTrackingRecHit::ConstRecHitPointer tth = hits[iHit]; 
    
    TransientTrackingRecHit::RecHitPointer newtth = refitHit( tth, state);
    
    if (!checkHit(state,newtth,es)) return 0;

    updatedState =  updator.update(state, *newtth);
    if (!updatedState.isValid()) return 0;
    
    seedHits.push_back(newtth->hit()->clone());
  } 

  TrajectoryStateTransform transformer;
  boost::shared_ptr<PTrajectoryStateOnDet> PTraj(
      transformer.persistentState(updatedState, hit->geographicalId().rawId()));
  
  seedCollection.push_back( TrajectorySeed(*PTraj,seedHits,alongMomentum));
  return &seedCollection.back();
}

TransientTrackingRecHit::RecHitPointer SeedFromConsecutiveHitsCreator::refitHit(
      const TransientTrackingRecHit::ConstRecHitPointer &hit, 
      const TrajectoryStateOnSurface &state) const
{
  return hit->clone(state);
}
