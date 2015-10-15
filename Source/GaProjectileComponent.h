#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"

//////////////////////////////////////////////////////////////////////////
// GaProjectileProcessor
class GaProjectileProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaProjectileProcessor, ScnComponentProcessor );

	GaProjectileProcessor();
	virtual ~GaProjectileProcessor();

	void initialise() override;
	void shutdown() override;

	void update( const ScnComponentList& Components );

private:
	
};

//////////////////////////////////////////////////////////////////////////
// GaProjectileComponent
class GaProjectileComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaProjectileComponent, ScnComponent );

	GaProjectileComponent();
	virtual ~GaProjectileComponent();

	void setupTopology();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	void setLevel( BcU32 Level );
	void setTarget( ScnEntity* Target );
	ScnEntity* getTarget() const { return Target_; }

private:
	friend class GaProjectileProcessor;

	BcF32 DamageDistance_ = 32.0f;
	BcF32 MaxSpeed_ = 128.0f;
	BcF32 LevelSpeedMultiplier_ = 0.1f;
	BcF32 CalculatedMaxSpeed_ = MaxSpeed_;
	BcF32 Acceleration_ = 1024.0f;
	BcF32 Drag_ = 0.05f;
	BcF32 Mass_ = 1.0f;
	ScnEntity* Target_ = nullptr;


	class ScnCanvasComponent* Canvas_ = nullptr;
	class GaPhysicsComponent* Physics_ = nullptr;
};

