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
	void advanceGameTimer( class GaGameComponent* Component, BcF32 Tick );
	void onIdle( class GaGameComponent* Component, BcF32 Tick );
	void onBuildPhase( class GaGameComponent* Component, BcF32 Tick );
	void onDefendPhase( class GaGameComponent* Component, BcF32 Tick );
	void onGameOver( class GaGameComponent* Component, BcF32 Tick );
	
};

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
class GaGameComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaGameComponent, ScnComponent );

	GaGameComponent();
	virtual ~GaGameComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	
private:
	friend class GaGameProcessor;

	BcU32 Level_ = 0;
	BcF32 GamePhaseTime_ = 30.0f;
	std::vector< ScnEntity* > StructureTemplates_;

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

	GameState GameState_ = GameState::IDLE;
	InputState InputState_ = InputState::IDLE;

	// Game state specific.
	BcF32 GameTimer_ = 0.0f;

	// Input state specific.
	class GaStructureComponent* BuildStructure_ = nullptr;
	

	class ScnCanvasComponent* Canvas_ = nullptr;
	class ScnFontComponent* Font_ = nullptr;
	

};
