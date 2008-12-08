import FWCore.ParameterSet.Config as cms

from RecoTracker.TkTrackingRegions.GlobalTrackingRegionWithVertices_cfi import *
from RecoTracker.TkSeedGenerator.SeedFromConsecutiveHitsCreator_cfi import *

globalSeedsFromPairsWithVertices = cms.EDProducer("SeedGeneratorFromRegionHitsEDProducer",
    #include "RecoTracker/PixelStubs/data/SeedComparitorWithPixelStubs.cfi"
    OrderedHitsFactoryPSet = cms.PSet(
        ComponentName = cms.string('StandardHitPairGenerator'),
        SeedingLayers = cms.string('MixedLayerPairs')
    ),
    SeedComparitorPSet = cms.PSet(
        ComponentName = cms.string('none')
    ),
    RegionFactoryPSet = cms.PSet(
        RegionPSetWithVerticesBlock,
        ComponentName = cms.string('GlobalTrackingRegionWithVerticesProducer')
    ),
    SeedCreatorPSet = cms.PSet(SeedFromConsecutiveHitsCreator)
)


