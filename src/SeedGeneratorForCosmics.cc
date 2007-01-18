#include "RecoTracker/TkSeedGenerator/interface/SeedGeneratorForCosmics.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "RecoTracker/TkHitPairs/interface/CosmicLayerPairs.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/CommonDetAlgo/interface/GlobalError.h"
#include "RecoTracker/TkSeedGenerator/interface/SeedFromConsecutiveHits.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
#include "RecoTracker/TransientTrackingRecHit/interface/TkTransientTrackingRecHitBuilder.h"
#include "TrackingTools/Records/interface/TransientRecHitRecord.h" 
void 
SeedGeneratorForCosmics::init(const SiStripRecHit2DCollection &collstereo,
			      const SiStripRecHit2DCollection &collrphi ,
			      const SiStripMatchedRecHit2DCollection &collmatched,
			      const edm::EventSetup& iSetup)
{
  iSetup.get<IdealMagneticFieldRecord>().get(magfield);
  iSetup.get<TrackerDigiGeometryRecord>().get(tracker);
  thePropagatorAl=    new PropagatorWithMaterial(alongMomentum,0.1057,&(*magfield) );
  thePropagatorOp=    new PropagatorWithMaterial(oppositeToMomentum,0.1057,&(*magfield) );
  theUpdator=       new KFUpdator();
  
  // get the transient builder
  //

  edm::ESHandle<TransientTrackingRecHitBuilder> theBuilder;

  iSetup.get<TransientRecHitRecord>().get(builderName,theBuilder);
  TTTRHBuilder = theBuilder.product();

  CosmicLayerPairs cosmiclayers;
  cosmiclayers.init(collstereo,collrphi,collmatched,geometry,iSetup);
  thePairGenerator=new CosmicHitPairGenerator(cosmiclayers,iSetup);
  HitPairs.clear();
  thePairGenerator->hitPairs(region,HitPairs,iSetup);
}

SeedGeneratorForCosmics::SeedGeneratorForCosmics(edm::ParameterSet const& conf): SeedGeneratorFromTrackingRegion(conf),
  conf_(conf)
{  

  float ptmin=conf_.getParameter<double>("ptMin");
  float originradius=conf_.getParameter<double>("originRadius");
  float halflength=conf_.getParameter<double>("originHalfLength");
  float originz=conf_.getParameter<double>("originZPosition");
  builderName = conf_.getParameter<std::string>("TTRHBuilder");   
  geometry=conf_.getUntrackedParameter<std::string>("GeometricStructure","STANDARD");
  region=GlobalTrackingRegion(ptmin,originradius,
 			      halflength,originz);

  edm::LogInfo("SeedGeneratorForCosmics")<<" PtMin of track is "<<ptmin<< 
    " The Radius of the cylinder for seeds is "<<originradius <<"cm" ;



}

void SeedGeneratorForCosmics::run(TrajectorySeedCollection &output,const edm::EventSetup& iSetup){
  seeds(output,iSetup,region);
  delete thePairGenerator;
}
void SeedGeneratorForCosmics::seeds(TrajectorySeedCollection &output,
				    const edm::EventSetup& iSetup,
				    const TrackingRegion& region){
 
  

 
 
  //  if(HitPairs.size()>0){
  for(uint is=0;is<HitPairs.size();is++){

    //   stable_sort(HitPairs.begin(),HitPairs.end(),CompareHitPairsY(iSetup));
    
    
    GlobalPoint inner = tracker->idToDet(HitPairs[is].inner()->geographicalId())->surface().toGlobal(HitPairs[is].inner()->localPosition());
    GlobalPoint outer = tracker->idToDet(HitPairs[is].outer()->geographicalId())->surface().toGlobal(HitPairs[is].outer()->localPosition());
    
    LogDebug("CosmicSeedFinder") <<"inner point of the seed "<<inner <<" outer point of the seed "<<outer; 
    //RC const TransientTrackingRecHit* outrhit=TTTRHBuilder->build(HitPairs[is].outer());  
    TransientTrackingRecHit::ConstRecHitPointer outrhit=TTTRHBuilder->build(HitPairs[is].outer());

    edm::OwnVector<TrackingRecHit> hits;
    hits.push_back(HitPairs[is].outer()->clone());
    //    hits.push_back(HitPairs[is].inner()->clone());

    for (int i=0;i<2;i++){
      //FIRST STATE IS CALCULATED CONSIDERING THAT THE CHARGE CAN BE POSITIVE OR NEGATIVE
      int predsign=(2*i)-1;
      if((outer.y()-inner.y())>0){
	GlobalTrajectoryParameters Gtp(outer,
				       inner-outer,
				       predsign, 
				       &(*magfield));
	
	FreeTrajectoryState CosmicSeed(Gtp,
				       CurvilinearTrajectoryError(AlgebraicSymMatrix(5,1)));
	
	
	LogDebug("CosmicSeedFinder") << " FirstTSOS "<<CosmicSeed;
	//First propagation
	const TSOS outerState =
	  thePropagatorAl->propagate(CosmicSeed,
				     tracker->idToDet(HitPairs[is].outer()->geographicalId())->surface());
	if ( outerState.isValid()) {
	  LogDebug("CosmicSeedFinder") <<"outerState "<<outerState;
	  const TSOS outerUpdated= theUpdator->update( outerState,*outrhit);
	  if ( outerUpdated.isValid()) {
	    LogDebug("CosmicSeedFinder") <<"outerUpdated "<<outerUpdated;
	    
	    PTrajectoryStateOnDet *PTraj=  
	      transformer.persistentState(outerUpdated, HitPairs[is].outer()->geographicalId().rawId());
	    
	    TrajectorySeed *trSeed=new TrajectorySeed(*PTraj,hits,alongMomentum);
	    output.push_back(*trSeed);
	    
	  }else      edm::LogWarning("CosmicSeedFinder") << " SeedForCosmics first update failed ";
	}else      edm::LogWarning("CosmicSeedFinder") << " SeedForCosmics first propagation failed ";
      
      
      }
      else{
	GlobalTrajectoryParameters Gtp(outer,
				       outer-inner,
				       predsign, 
				       &(*magfield));
	FreeTrajectoryState CosmicSeed(Gtp,
				       CurvilinearTrajectoryError(AlgebraicSymMatrix(5,1)));
	LogDebug("CosmicSeedFinder") << " FirstTSOS "<<CosmicSeed;
	//First propagation
	const TSOS outerState =
	  thePropagatorAl->propagate(CosmicSeed,
				     tracker->idToDet(HitPairs[is].outer()->geographicalId())->surface());
	if ( outerState.isValid()) {
	  
	  LogDebug("CosmicSeedFinder") <<"outerState "<<outerState;
	  const TSOS outerUpdated= theUpdator->update( outerState,*outrhit);
	  if ( outerUpdated.isValid()) {
	  LogDebug("CosmicSeedFinder") <<"outerUpdated "<<outerUpdated;
	  PTrajectoryStateOnDet *PTraj=  
	    transformer.persistentState(outerUpdated, HitPairs[is].outer()->geographicalId().rawId());
	  
	  TrajectorySeed *trSeed=new TrajectorySeed(*PTraj,hits,oppositeToMomentum);
	  output.push_back(*trSeed);
	
	  }else      edm::LogWarning("CosmicSeedFinder") << " SeedForCosmics first update failed ";
	}else      edm::LogWarning("CosmicSeedFinder") << " SeedForCosmics first propagation failed ";
      }
      
    }
  }
}
