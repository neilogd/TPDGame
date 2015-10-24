#pragma once

#include "Events/EvtEvent.h"
#include "System/Scene/ScnComponent.h"

#if PLATFORM_ANDROID
#include "gpg/achievement.h"
#include "gpg/achievement_manager.h"
#include "gpg/builder.h"
#include "gpg/debug.h"
#include "gpg/default_callbacks.h"
#include "gpg/game_services.h"
#include "gpg/leaderboard.h"
#include "gpg/leaderboard_manager.h"
#include "gpg/platform_configuration.h"
#include "gpg/player_manager.h"
#include "gpg/score_page.h"
#include "gpg/types.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// GaGPGEvents
enum GaGPGEvents
{
	gaEVT_GPG_FIRST = EVT_MAKE_ID( 'G', 'p', 0 ),
};

////////////////////////////////////////////////////////////////////////////////
// GaGPGLeaderboard
namespace GaGPGLeaderboard
{
	static const char* Score = "CgkI9tzOuOIfEAIQAA";
	static const char* Level = "CgkI9tzOuOIfEAIQAQ";
	static const char* Resources = "CgkI9tzOuOIfEAIQAg";
}

////////////////////////////////////////////////////////////////////////////////
// GaGPGAchievement
namespace GaGPGAchievement
{
	static const char* FirstNight = "CgkI9tzOuOIfEAIQBA";
}

////////////////////////////////////////////////////////////////////////////////
// gpg forward decls.
namespace gpg
{
	class GameServices;
}

////////////////////////////////////////////////////////////////////////////////
// GaGPGComponent
class GaGPGComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaGPGComponent, ScnComponent );

	GaGPGComponent();
	virtual ~GaGPGComponent();

	/**
	 * Open leaderboards.
	 */
	void openLeaderboards();

	/**
	 * Open achievements.
	 */
	void openAchievements();

	/**
	 * Unlock achivement.
	 * @param ID ID of achievement (set on GPGS).
	 */
	void unlockAchievement( const char* ID );

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

private:
#if PLATFORM_ANDROID
	std::unique_ptr< gpg::GameServices > GameServices_;
#endif
	
};


