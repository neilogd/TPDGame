#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"

//////////////////////////////////////////////////////////////////////////
// GaMenuEntry
struct GaMenuEntry
{
	REFLECTION_DECLARE_BASIC( GaMenuEntry );

	GaMenuEntry();

	std::string Text_;
	std::string EntityPackage_;
	std::string EntityName_;
	BcU32 ID_ = BcErrorCode;
};

//////////////////////////////////////////////////////////////////////////
// GaMenuProcessor
class GaMenuProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaMenuProcessor, ScnComponentProcessor );

	GaMenuProcessor();
	virtual ~GaMenuProcessor();

	void initialise() override;
	void shutdown() override;

	void drawMenus( const ScnComponentList& Components );

private:
	
};

//////////////////////////////////////////////////////////////////////////
// GaMenuComponent
class GaMenuComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaMenuComponent, ScnComponent );

	GaMenuComponent();
	virtual ~GaMenuComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

private:
	friend class GaMenuProcessor;

	std::string Title_;
	BcBool Modal_ = BcFalse;
	std::vector< GaMenuEntry > Entries_;
	MaVec2d EntrySize_ = MaVec2d( 400.0f, 64.0f );
	BcF32 EntryMargin_ = 16.0f;
	

	std::vector< class GaHotspotComponent* > Hotspots_;
	class ScnCanvasComponent* Canvas_ = nullptr;
	class ScnFontComponent* Font_ = nullptr;
};
