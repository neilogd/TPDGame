#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"

////////////////////////////////////////////////////////////////////////////////
// GaGameEvents
enum GaGameEvents
{
	gaEVT_GAME_BEGIN_BUILD_PHASE = EVT_MAKE_ID( 'G', 'a', 'G' ),
	gaEVT_GAME_BEGIN_DEFEND_PHASE,
};

struct GaGameEvent : EvtEvent< GaGameEvent >
{
};

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
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaGameComponent, ScnComponent );

	GaGameComponent();
	virtual ~GaGameComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	const std::vector< class GaStructureComponent* >& getStructures() const;
	const std::vector< class GaTentacleComponent* >& getTentacles() const;

	void buildStructure( class GaStructureComponent* Structure );
	void destroyStructure( class GaStructureComponent* Structure );


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

	std::vector< class GaStructureComponent* > Structures_;
	std::vector< class GaTentacleComponent* > Tentacles_;

	// Input state specific.
	class GaStructureComponent* SelectedStructure_ = nullptr;

	class ScnCanvasComponent* Canvas_ = nullptr;
	class ScnFontComponent* Font_ = nullptr;
	

};
