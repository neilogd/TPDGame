#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
	
////////////////////////////////////////////////////////////////////////////////
// GaHotspotEvents
enum GaHotspotEvents
{
	gaEVT_HOTSPOT_PRESSED,
	gaEVT_HOTSPOT_HOVER,
};

struct GaHotspotEvent : EvtEvent< GaHotspotEvent >
{
	BcU32 ID_;
	MaVec2d Position_;
	MaVec2d RelativePosition_;
};

//////////////////////////////////////////////////////////////////////////
// GaHotspotProcessor
class GaHotspotProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaHotspotProcessor, ScnComponentProcessor );

	GaHotspotProcessor();
	virtual ~GaHotspotProcessor();

	void initialise() override;
	void shutdown() override;

	void setupHotspots( const ScnComponentList& Components );

	void debugDraw( const ScnComponentList& Components );

private:
	std::vector< class GaHotspotComponent* > HotspotComponents_;
	
};

//////////////////////////////////////////////////////////////////////////
// GaHotspotComponent
class GaHotspotComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaHotspotComponent, ScnComponent );

	GaHotspotComponent();
	virtual ~GaHotspotComponent();

	MaVec3d getPosition() const;

private:
	friend class GaHotspotProcessor;

	BcU32 ID_;
	MaVec3d Position_;
	MaVec2d Size_;
	
};