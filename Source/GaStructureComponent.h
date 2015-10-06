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

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

private:
	friend class GaStructureProcessor;

	BcU32 Level_ = 0;
	BcBool Active_ = BcFalse;
 
	class ScnCanvasComponent* Canvas_ = nullptr;
	class ScnFontComponent* Font_ = nullptr;
};
