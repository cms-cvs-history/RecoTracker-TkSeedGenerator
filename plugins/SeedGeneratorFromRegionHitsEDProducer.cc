#include "SeedGeneratorFromRegionHitsEDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"

#include "RecoTracker/TkTrackingRegions/interface/TrackingRegionProducerFactory.h"
#include "RecoTracker/TkTrackingRegions/interface/TrackingRegionProducer.h"
#include "RecoTracker/TkTrackingRegions/interface/TrackingRegion.h"

#include "RecoTracker/TkTrackingRegions/interface/OrderedHitsGeneratorFactory.h"
#include "RecoTracker/TkTrackingRegions/interface/OrderedHitsGenerator.h"

#include "RecoTracker/TkSeedingLayers/interface/SeedComparitorFactory.h"
#include "RecoTracker/TkSeedingLayers/interface/SeedComparitor.h"

#include "RecoTracker/TkSeedGenerator/interface/SeedCreatorFactory.h"
#include "RecoTracker/TkSeedGenerator/interface/SeedCreator.h"

#include "RecoTracker/TkSeedGenerator/interface/SeedGeneratorFromRegionHits.h"
#include "RecoPixelVertexing/PixelTriplets/interface/QuadrupletSeedMerger.h"

SeedGeneratorFromRegionHitsEDProducer::SeedGeneratorFromRegionHitsEDProducer(
    const edm::ParameterSet& cfg) 
  : theConfig(cfg), theGenerator(0), theRegionProducer(0), theClusterCheck(cfg.getParameter<edm::ParameterSet>("ClusterCheckPSet"))
{
  theSilentOnClusterCheck = cfg.getParameter<edm::ParameterSet>("ClusterCheckPSet").getUntrackedParameter<bool>("silentClusterCheck",false);
  produces<TrajectorySeedCollection>();
}

SeedGeneratorFromRegionHitsEDProducer::~SeedGeneratorFromRegionHitsEDProducer()
{
}

void SeedGeneratorFromRegionHitsEDProducer::endRun(edm::Run &run, const edm::EventSetup& es) {
  delete theRegionProducer;
  delete theGenerator;
}

void SeedGeneratorFromRegionHitsEDProducer::beginRun(edm::Run &run, const edm::EventSetup& es)
{
  edm::ParameterSet regfactoryPSet = 
      theConfig.getParameter<edm::ParameterSet>("RegionFactoryPSet");
  std::string regfactoryName = regfactoryPSet.getParameter<std::string>("ComponentName");
  theRegionProducer = TrackingRegionProducerFactory::get()->create(regfactoryName,regfactoryPSet);

  edm::ParameterSet hitsfactoryPSet = 
      theConfig.getParameter<edm::ParameterSet>("OrderedHitsFactoryPSet");
  std::string hitsfactoryName = hitsfactoryPSet.getParameter<std::string>("ComponentName");
  OrderedHitsGenerator*  hitsGenerator = 
      OrderedHitsGeneratorFactory::get()->create( hitsfactoryName, hitsfactoryPSet);

  edm::ParameterSet comparitorPSet =
      theConfig.getParameter<edm::ParameterSet>("SeedComparitorPSet");
  std::string comparitorName = comparitorPSet.getParameter<std::string>("ComponentName");
  SeedComparitor * aComparitor = (comparitorName == "none") ? 
      0 :  SeedComparitorFactory::get()->create( comparitorName, comparitorPSet);   

  edm::ParameterSet creatorPSet =
      theConfig.getParameter<edm::ParameterSet>("SeedCreatorPSet");
  std::string creatorName = creatorPSet.getParameter<std::string>("ComponentName");
  SeedCreator * aCreator = SeedCreatorFactory::get()->create( creatorName, creatorPSet);

  theGenerator = new SeedGeneratorFromRegionHits(hitsGenerator, aComparitor, aCreator); 
  
  
}

void SeedGeneratorFromRegionHitsEDProducer::produce(edm::Event& ev, const edm::EventSetup& es)
{
  std::auto_ptr<TrajectorySeedCollection> triplets(new TrajectorySeedCollection());
  std::auto_ptr<TrajectorySeedCollection> quadruplets( new TrajectorySeedCollection() );

  // seed merger & its settings
  QuadrupletSeedMerger theMerger( es );
  edm::ParameterSet mergerPSet = theConfig.getParameter<edm::ParameterSet>( "SeedMergerPSet" );
  theMerger.setTTRHBuilderLabel( mergerPSet.getParameter<std::string>( "ttrhBuilderLabel" ) );
  theMerger.setMergeTriplets( mergerPSet.getParameter<bool>( "mergeTriplets" ) );
  theMerger.setAddRemainingTriplets( mergerPSet.getParameter<bool>( "addRemainingTriplets" ) );
  theMerger.setLayerListName( mergerPSet.getParameter<std::string>( "layerListName" ) );


  //protection for big ass events...
  size_t clustsOrZero = theClusterCheck.tooManyClusters(ev);
  if (clustsOrZero){
    if (!theSilentOnClusterCheck)
	edm::LogError("TooManyClusters") << "Found too many clusters (" << clustsOrZero << "), bailing out.\n";
    ev.put(triplets);
    return ;
  }

  typedef std::vector<TrackingRegion* > Regions;
  typedef Regions::const_iterator IR;
  Regions regions = theRegionProducer->regions(ev,es);

  for (IR ir=regions.begin(), irEnd=regions.end(); ir < irEnd; ++ir) {
    const TrackingRegion & region = **ir;

    // make job
    theGenerator->run(*triplets, region, ev,es);

    // make quadruplets
    // (TODO: can partly be propagated to the merger)
    TrajectorySeedCollection const& tempQuads = theMerger.mergeTriplets( *triplets, region, es ); //@@
    for( TrajectorySeedCollection::const_iterator qIt = tempQuads.begin(); qIt < tempQuads.end(); ++qIt ) {
      quadruplets->push_back( *qIt );
    }
  }

  // clear memory
  for (IR ir=regions.begin(), irEnd=regions.end(); ir < irEnd; ++ir) delete (*ir);

  // put to event
  ev.put(quadruplets);
}
