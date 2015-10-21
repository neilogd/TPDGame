#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"

//////////////////////////////////////////////////////////////////////////
// GaGameProcessor
class GaGameProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaGameProcessor, ScnComponentProcessor );

	GaGameProcessor();
	virtual ~GaGameProcessor();

	void initialise() override;
	void shutdown() override;

	void update( const ScnComponentList& Components );

private:
	
};

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
class GaGameComponent:
	public ScnComponent,
	public ReIObjectNotify
{
public:
	REFLECTION_DECLARE_DERIVED( GaGameComponent, ScnComponent );

	GaGameComponent();
	virtual ~GaGameComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	void onObjectDeleted( class ReObject* Object ) override;

	const std::vector< class GaStructureComponent* >& getStructures() const;
	const std::vector< class GaTentacleComponent* >& getTentacles() const;

	void createStructureButtons();
	void setSelection( BcU32 SelectedIdx );
	bool buildStructure( BcU32 StructureIdx, MaVec2d Position );
	void destroyStructure( class GaStructureComponent* Structure );

	void launchProjectile( class GaProjectileComponent* Projectile );

	void spawnTentacles();
	class GaTentacleComponent* getNearestTentacle( BcBool IncludeTargetted ) const;

	BcS64 getPlayerScore() const { return PlayerScore_; }
	BcS64 getPlayerResources() const { return PlayerResources_; }
	BcBool spendResources( BcS64 NoofResources );
	void addScore( MaVec2d Position, BcS64 Score );

	MaVec2d getStructurePlacement( MaVec2d Position );

	BcF32 haltonSequence( BcU32 Index, BcU32 Base );

	void spawnPopupText( MaVec2d Position, MaVec2d Velocity, BcF32 Time, const char* Format, ... );

private:
	void update( BcF32 Tick );
	void advanceGameTimer( BcF32 Tick );
	void onIdle( BcF32 Tick );
	void onBuildPhase( BcF32 Tick );
	void onDefendPhase( BcF32 Tick );
	void onGameOver( BcF32 Tick );

	friend class GaGameProcessor;

	BcU32 Level_ = 0;
	BcF32 GamePhaseTime_ = 30.0f;
	std::vector< ScnEntity* > StructureTemplates_;
	ScnEntity* UpgradeMenuTemplate_ = nullptr;
	ScnEntity* ButtonTemplate_ = nullptr;
	ScnEntity* BuildUIEntity_ = nullptr;

	MaVec2d BuildUIEntityTarget_ = MaVec2d( 0.0f, 0.0f );

	enum class GameState
	{
		IDLE,
		BUILD_PHASE,
		DEFEND_PHASE,
		GAME_OVER,

		MAX
	};
	
	enum class InputState
	{
		IDLE,
		BUILD_BUILDING,
		SELECTED_BUILDING,

		MAX
	};

	void setGameState( GameState GameState );
	void setInputState( InputState InputState );

	// Player specific.
	BcS64 PlayerScore_ = 0;
	BcS64 PlayerResources_ = 100;

	// Game state specific.
	GameState GameState_ = GameState::IDLE;
	BcF32 GameTimer_ = 0.0f;

	std::vector< class GaStructureComponent* > Structures_;
	std::vector< class GaTentacleComponent* > Tentacles_;
	std::vector< class GaProjectileComponent* > Projectiles_;

	BcU32 StructureID_ = 0;
	ScnEntity* CurrentModal_ = nullptr;

	// Input state specific.
	InputState InputState_ = InputState::IDLE;
	std::vector< class ScnSpriteComponent* > StructureUISprites_;
	size_t SelectedStructureIdx_ = BcErrorCode;

	class ScnCanvasComponent* Canvas_ = nullptr;
	class ScnFontComponent* Font_ = nullptr;


	struct PopupText
	{
		BcChar Text_[ 256 ];
		MaVec2d Position_;
		MaVec2d Velocity_;
		BcF32 Time_;
	};

	std::vector< PopupText > PopupText_;

};
