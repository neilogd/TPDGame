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
	static const char* SCORE = "CgkI9tzOuOIfEAIQAA";
	static const char* LEVEL = "CgkI9tzOuOIfEAIQAQ";
	static const char* RESOURCES = "CgkI9tzOuOIfEAIQAg";
}

////////////////////////////////////////////////////////////////////////////////
// GaGPGAchievement
namespace GaGPGAchievement
{
	static const char* FIRST_NIGHT = "CgkI9tzOuOIfEAIQBA";
	static const char* TOUGH_TEN = "CgkI9tzOuOIfEAIQCg";
	static const char* TWISTED_TWENTY = "CgkI9tzOuOIfEAIQCw";
	static const char* THIRSTY_THIRTY = "CgkI9tzOuOIfEAIQDA";
	static const char* INFURIATING_FIFTY = "CgkI9tzOuOIfEAIQDQ";
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
	 * Submit score.
	 * @param ID ID of the leaderboard.
	 * @param Value Score to submit.
	 */
	void submitScore( const char* ID, BcS64 Score );

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


