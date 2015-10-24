#include "Psybrus.h"

#include "System/Scene/ScnCore.h"

//////////////////////////////////////////////////////////////////////////
// PsyGameInit
void PsyGameInit()
{
	GPsySetupParams = PsySetupParams( 
		"Tentacle Potato Defense",
#if PSY_PRODUCTION
		psySF_GAME_FINAL,
#else
		psySF_GAME_DEV,
#endif
		1.0f / 60.0f );
}

//////////////////////////////////////////////////////////////////////////
// PsyLaunchGame
void PsyLaunchGame()
{
	// Main entity params.
	ScnEntitySpawnParams MainEntityParams( 
		BcName::INVALID, "default", "MainEntity",
		MaMat4d(), nullptr );
	MainEntityParams.OnSpawn_ = []( ScnEntity* Entity )
		{
			// Once main entity has spawned, create menu.
			ScnEntitySpawnParams MenuEntityParams( 
				BcName::INVALID, "menus", "SplashMenu",
				MaMat4d(), Entity );
			ScnCore::pImpl()->spawnEntity( MenuEntityParams );
		};
	
	// Spawn main entity.
	ScnCore::pImpl()->spawnEntity( MainEntityParams );
}
