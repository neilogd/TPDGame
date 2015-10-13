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
	public ScnComponent,
	public ReIObjectNotify
{
public:
	REFLECTION_DECLARE_DERIVED( GaTentacleComponent, ScnComponent );

	GaTentacleComponent();
	virtual ~GaTentacleComponent();

	void setupComplexTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections );
	void setupDiamondTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections );
	void setupSimpleTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections );
	void addPhysicsNoise();

	void calculateLevelStats( BcU32 Level );

	void targetStructure();
	void targetHome();
	class GaStructureComponent* getTargetStructure() const;


	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;
	
	void onObjectDeleted( class ReObject* Object ) override;
	
private:
	friend class GaTentacleProcessor;

	BcF32 MoveSpeed_ = 64.0f;
	BcF32 MoveSpeedMultiplier_ = 0.1f;
	BcF32 HeadDamping_ = 0.5f;
	BcF32 HeadConstraintRigidity_ = 0.5f;

	BcF32 CalculatedMoveSpeed_ = 0.0f;

	class GaGameComponent* Game_ = nullptr;

	// TODO: Replace with something more predictable, like a spline.
	class GaStructureComponent* TargetStructure_ = nullptr;
	MaVec2d TargetPosition_;
};