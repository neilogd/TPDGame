#pragma once

#include "Events/EvtEvent.h"
#include "Math/MaVec2d.h"

////////////////////////////////////////////////////////////////////////////////
// GaGameEvents
enum GaGameEvents
{
	gaEVT_GAME_BEGIN_BUILD_PHASE = EVT_MAKE_ID( 'G', 'a', 0 ),
	gaEVT_GAME_BEGIN_DEFEND_PHASE,

	gaEVT_HOTSPOT_PRESSED = EVT_MAKE_ID( 'G', 'a', 1 ),
	gaEVT_HOTSPOT_HOVER,

	gaEVT_PROJECTILE_HIT = EVT_MAKE_ID( 'G', 'a', 2 ),
};

struct GaGameEvent : EvtEvent< GaGameEvent >
{
	GaGameEvent( BcU32 Level ):
		Level_( Level )
	{}

	BcU32 Level_;
};

struct GaHotspotEvent : EvtEvent< GaHotspotEvent >
{
	BcU32 ID_;
	MaVec2d Position_;
	MaVec2d RelativePosition_;
};

struct GaProjectileEvent : EvtEvent< GaProjectileEvent >
{
};
