/*=============================================================================
	Database.h: Flu Objects Database initialization.
	Created by Vlad Gordienko, Nov. 2017.
=============================================================================*/
#pragma once

#define FOREACH_CLASS(method)\
	FObject						::method();\
	FComponent					::method();\
	FExtraComponent				::method();\
	FEmitterComponent			::method();\
	FPhysEmitterComponent		::method();\
	FLissajousEmitterComponent	::method();\
	FWeatherEmitterComponent	::method();\
	FPainterComponent			::method();\
	FKeyframeComponent			::method();\
	FPuppetComponent			::method();\
	FLogicComponent				::method();\
	FInputComponent				::method();\
	FLightComponent				::method();\
	FLabelComponent				::method();\
	FParallaxLayerComponent		::method();\
	FAnimatedSpriteComponent	::method();\
	FSpriteComponent			::method();\
	FDecoComponent				::method();\
	FBaseComponent				::method();\
	FModelComponent				::method();\
	FBrushComponent				::method();\
	FRectComponent				::method();\
	FZoneComponent				::method();\
	FSkyComponent				::method();\
	FPhysicComponent			::method();\
	FArcadeBodyComponent		::method();\
	FRigidBodyComponent			::method();\
	FMoverComponent				::method();\
	FJointComponent				::method();\
	FHingeComponent				::method();\
	FSpringComponent			::method();\
	FPortalComponent			::method();\
	FWarpComponent				::method();\
	FMirrorComponent			::method();\
	FSkeletonComponent			::method();\
	FEntity						::method();\
	FModifier					::method();\
	FResource					::method();\
	FScript						::method();\
	FTexture					::method();\
	FBitmap						::method();\
	FFont						::method();\
	FLevel						::method();\
	FSound						::method();\
	FMusic						::method();\
	FAnimation					::method();\
	FSkeleton					::method();\
	FProjectInfo				::method();\
	//FDemoBitmap				::method();\
	//FPlasmaBitmap				::method();\
	//FFireBitmap				::method();\
	//FGlassBitmap				::method();\
	//FWaterBitmap				::method();\
	//FTechBitmap				::method();\
	//FHarmonicBitmap			::method();\
	//FMaterial					::method();\
	//FMaterialLayer			::method();\
	//FDiffuseLayer				::method();\


//
// Register all classes.
//
inline void RegisterAll()
{
	mem::enterKnownMemLeaksZone();
	{
		// Register classes hierarchy.
		FOREACH_CLASS(AutoRegisterClass)

		// Register classes properties and methods.
		FOREACH_CLASS(AutoRegisterFields)
	}
	mem::leaveKnownMemLeaksZone();
}

//
// Register all classes.
//
inline void UnRegisterAll()
{
	// Register classes hierarchy.
	//FOREACH_CLASS(AutoUnRegisterClass)
	// todo: implement this function
}

#undef FOREACH_CLASS

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/