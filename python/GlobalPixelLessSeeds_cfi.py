import FWCore.ParameterSet.Config as cms

from RecoTracker.TkTrackingRegions.GlobalTrackingRegion_cfi import *
globalPixelLessSeeds = cms.EDProducer("SeedGeneratorFromRegionHitsEDProducer",
    OrderedHitsFactoryPSet = cms.PSet(
        ComponentName = cms.string('StandardHitPairGenerator'),
        SeedingLayers = cms.string('PixelLessLayerPairs')
    ),
    SeedComparitorPSet = cms.PSet(
        ComponentName = cms.string('none')
    ),
    RegionFactoryPSet = cms.PSet(
        RegionPSetBlock,
        ComponentName = cms.string('GlobalRegionProducer')
    ),
    propagator = cms.string('PropagatorWithMaterial'),
# The fast-helix fit works well, except for large impact parameter pixel pair seeding.                                      
    UseFastHelix = cms.bool(True),
# Following parameter not relevant for UseFastHelix = False.                                                                         
    SeedMomentumForBOFF = cms.double(5.0), 
    TTRHBuilder = cms.string('WithTrackAngle')
)


