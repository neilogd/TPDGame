#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
	
//////////////////////////////////////////////////////////////////////////
// GaTentacleProcessor
class GaTentacleProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaTentacleProcessor, ScnComponentProcessor );

	GaTentacleProcessor();
	virtual ~GaTentacleProcessor();

	void initialise() override;
	void shutdown() override;

	void update( const ScnComponentList& Components );

private:
	
};

//////////////////////////////////////////////////////////////////////////
// GaTentacleComponent
class GaTentacleComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaTentacleComponent, ScnComponent );

	GaTentacleComponent();
	virtual ~GaTentacleComponent();

	void setupComplexTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections );
	void setupDiamondTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections );
	void setupSimpleTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections );


	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	
private:
	friend class GaTentacleProcessor;

	// TODO: Replace with something more predictable, like a spline.
	MaVec2d TargetPosition_;
	
};