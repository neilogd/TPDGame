#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Os/OsEvents.h"
	
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
	std::vector< std::pair< EvtID, OsEventInputMouse > > MouseEvents_;
};

//////////////////////////////////////////////////////////////////////////
// GaHotspotComponent
class GaHotspotComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaHotspotComponent, ScnComponent );

	GaHotspotComponent();
	GaHotspotComponent( BcU32 ID, BcS32 Layer, MaVec2d Position, MaVec2d Size );
	virtual ~GaHotspotComponent();

	BcU32 getID() const;
	MaVec2d getPosition() const;
	MaVec2d getSize() const;

private:
	friend class GaHotspotProcessor;

	/// ID to send to event.
	BcU32 ID_;
	/// Layer of hotspot. -ve = further back (process late), +ve = further forward (process early)
	BcS32 Layer_;
	/// Position of hotspot.
	MaVec2d Position_;
	/// Size of hotspot. If -ve, then dimension is divisor on screen size, so -1,-1 = screen size.
	MaVec2d Size_;
	
};