#pragma once

#include "GaStructureBehaviour.h"

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"

//////////////////////////////////////////////////////////////////////////
// GaStructureProcessor
class GaStructureProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaStructureProcessor, ScnComponentProcessor );

	GaStructureProcessor();
	virtual ~GaStructureProcessor();

	void initialise() override;
	void shutdown() override;

	void update( const ScnComponentList& Components );

private:
	
};

//////////////////////////////////////////////////////////////////////////
// GaStructureComponent
class GaStructureComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaStructureComponent, ScnComponent );

	GaStructureComponent();
	virtual ~GaStructureComponent();

	void setupTopology();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	void setActive( BcBool Active );

private:
	friend class GaStructureProcessor;

	BcU32 Level_ = 0;
	BcBool Floating_ = BcFalse;

	BcBool Active_ = BcFalse;

	BcF32 Timer_ = 0.0f;

	MaVec2d AbsolutePosition_;
 
	class ScnCanvasComponent* Canvas_ = nullptr;
	class ScnFontComponent* Font_ = nullptr;
	class GaPhysicsComponent* Physics_ = nullptr;
};
