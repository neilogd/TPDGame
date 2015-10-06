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

	BcU32 Level_;

	enum class GameState
	{
		IDLE,
		BUILD_PHASE,
		DEFEND_PHASE,
		GAME_OVER,

		MAX
	};

	void setState( GameState GameState );

	GameState GameState_ = GameState::IDLE;
	BcF32 GameTimer_ = 0.0f;
	BcF32 GamePhaseTime_ = 30.0f;
	class ScnCanvasComponent* Canvas_ = nullptr;
	class ScnFontComponent* Font_ = nullptr;
	
	

};
