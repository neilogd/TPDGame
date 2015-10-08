#include "GaGameComponent.h"
#include "GaHotspotComponent.h"
#include "GaStructureComponent.h"
#include "GaPositionUtility.h"


#include "System/Debug/DsCore.h"
#include "System/Debug/DsImGui.h"
#include "System/Os/OsCore.h"

#include "System/Scene/ScnCore.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/Rendering/ScnFont.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaGameProcessor );

void GaGameProcessor::StaticRegisterClass()
{
	ReRegisterClass< GaGameProcessor, Super >();
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaGameProcessor::GaGameProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Update game state",
				ScnComponentPriority::DEFAULT_UPDATE,
				std::bind( &GaGameProcessor::update, this, std::placeholders::_1 ) )
		} )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
GaGameProcessor::~GaGameProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaGameProcessor::initialise()
{

}

//////////////////////////////////////////////////////////////////////////
// shutdown
void GaGameProcessor::shutdown()
{
	
}

//////////////////////////////////////////////////////////////////////////
// update
void GaGameProcessor::update( const ScnComponentList& Components )
{
	BcAssert( Components.size() <= 1 );

	BcF32 Tick = SysKernel::pImpl()->getFrameTime();
	for( auto InComponent : Components )
	{
		BcAssert( InComponent->isTypeOf< GaGameComponent >() );
		auto* Component = static_cast< GaGameComponent* >( InComponent.get() );

		Component->update( Tick );
	}
}


//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaGameComponent );

void GaGameComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Level_", &GaGameComponent::Level_, bcRFF_IMPORTER ),
		new ReField( "GamePhaseTime_", &GaGameComponent::GamePhaseTime_, bcRFF_IMPORTER ),
		new ReField( "StructureTemplates_", &GaGameComponent::StructureTemplates_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
	};

	ReRegisterClass< GaGameComponent, Super >( Fields )
		.addAttribute( new GaGameProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaGameComponent::GaGameComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaGameComponent::~GaGameComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Get canvas + font.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >();
	BcAssert( Canvas_ );
	Font_ = Parent->getComponentAnyParentByType< ScnFontComponent >();
	BcAssert( Font_ );

	// Subscribe to hotspot for hover.
	Parent->subscribe( gaEVT_HOTSPOT_HOVER, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< GaHotspotEvent >();

			if( BuildStructure_ )
			{
				BuildStructure_->getParentEntity()->setLocalPosition( 
					MaVec3d( Event.Position_, 0.0f ) );
			}
			return evtRET_PASS;
		} );

	// Subscribe to hotspot for pressed.
	Parent->subscribe( gaEVT_HOTSPOT_PRESSED, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< GaHotspotEvent >();

			if( BuildStructure_ )
			{
				BuildStructure_->getParentEntity()->setLocalPosition( 
					MaVec3d( Event.Position_, 0.0f ) );
				BuildStructure_->setActive( BcTrue );
				BuildStructure_ = nullptr;
				setInputState( InputState::IDLE );
			}
			return evtRET_PASS;
		} );

	// Spawn tentacle things.
	MaMat4d TransformA;
	MaMat4d TransformB;
	auto TentacleA = GaPositionUtility::GetScreenPosition( MaVec2d( 128.0f, 128.0f ), MaVec2d( 32.0f, 32.0f ), GaPositionUtility::TOP | GaPositionUtility::LEFT );
	auto TentacleB = GaPositionUtility::GetScreenPosition( MaVec2d( 128.0f, 128.0f ), MaVec2d( 32.0f, 32.0f ), GaPositionUtility::TOP | GaPositionUtility::RIGHT );
	TransformA.translation( MaVec3d( TentacleA, 0.0f ) );
	TransformB.translation( MaVec3d( TentacleB, 0.0f ) );
	ScnCore::pImpl()->spawnEntity( 
		ScnEntitySpawnParams( 
			BcName::INVALID, "game", "TentacleEntity",
			TransformA, Parent ) );
	ScnCore::pImpl()->spawnEntity( 
		ScnEntitySpawnParams( 
			BcName::INVALID, "game", "TentacleEntity",
			TransformB, Parent ) );

	
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	Parent->unsubscribeAll( this );
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// setGameState
void GaGameComponent::setGameState( GameState GameState )
{
	PSY_LOG( "Changing game state from %u -> %u", GameState_, GameState );
	GameState_ = GameState;
}

//////////////////////////////////////////////////////////////////////////
// setInputState
void GaGameComponent::setInputState( InputState InputState )
{
	PSY_LOG( "Changing input state from %u -> %u", InputState_, InputState );

	if( BuildStructure_ )
	{
		getParentEntity()->detach( BuildStructure_->getParentEntity() );
		BuildStructure_ = nullptr;
	}

	InputState_ = InputState;
}

//////////////////////////////////////////////////////////////////////////
// update
void GaGameComponent::update( BcF32 Tick )
{
	// Handle game state specific stuff.
	switch( GameState_ )
	{
	case GameState::IDLE:
		onIdle( Tick );
		break;
	case GameState::BUILD_PHASE:
		onBuildPhase( Tick );
		break;
	case GameState::DEFEND_PHASE:
		onDefendPhase( Tick );
		break;
	case GameState::GAME_OVER:
		break;
	}

#if !PSY_PRODUCTION
	if( ImGui::Begin( "Game Debug" ) )
	{
		ImGui::Text( "Game timer: %f", GameTimer_ );
		ImGui::Text( "Game state: %u", GameState_ );
		if( ImGui::Button( "Back to Main Menu" ) )
		{
			ScnEntitySpawnParams SpawnParams( 
					BcName::INVALID, "menus", "MainMenu",
					MaMat4d(), getParentEntity()->getParentEntity() );
			SpawnParams.OnSpawn_ = [ this ]( ScnEntity* Entity )
				{
					ScnCore::pImpl()->removeEntity( getParentEntity() );
				};

			ScnCore::pImpl()->spawnEntity( SpawnParams );

		}
		ImGui::Separator();
		ImGui::End();
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// advanceGameTimer
void GaGameComponent::advanceGameTimer( BcF32 Tick )
{
	GameTimer_ += Tick;

	const BcF32 LevelGameTimer = std::fmodf( GameTimer_, GamePhaseTime_ );
	const BcF32 HalfGamePhaseTime = GamePhaseTime_ * 0.5f;

	// Swap between states at the half way/wrap points.
	if( GameState_ == GameState::BUILD_PHASE &&
		LevelGameTimer > HalfGamePhaseTime )
	{
		setGameState( GameState::DEFEND_PHASE );
		setInputState( InputState::IDLE );
	}
	else if( GameState_ == GameState::DEFEND_PHASE &&
		LevelGameTimer < HalfGamePhaseTime )
	{
		setGameState( GameState::BUILD_PHASE );
		setInputState( InputState::IDLE );
	}
}

//////////////////////////////////////////////////////////////////////////
// onIdle
void GaGameComponent::onIdle( BcF32 Tick )
{
	setGameState( GameState::BUILD_PHASE );
}

//////////////////////////////////////////////////////////////////////////
// onBuildPhase
void GaGameComponent::onBuildPhase( BcF32 Tick )
{
	advanceGameTimer( Tick );

	// TODO BUILD MENU.
#if !PSY_PRODUCTION
		if( ImGui::Begin( "Game Debug" ) )
		{
			if( InputState_ == InputState::IDLE )
			{
				for( auto StructureEntity : StructureTemplates_ )
				{
					auto* StructureComponent = StructureEntity->getComponentByType< GaStructureComponent >();
					std::string ButtonText = std::string( "Build " ) + (*StructureComponent->getName());
					if( ImGui::Button( ButtonText.c_str() ) )
					{
						// Set input state (clears selected already)
						setInputState( InputState::BUILD_BUILDING );

						// Setup new structure.
						auto SpawnedEntity = ScnCore::pImpl()->spawnEntity( ScnEntitySpawnParams(
							StructureEntity->getName(), StructureEntity, MaMat4d(), getParentEntity() ) );
						BcAssert( SpawnedEntity );
						BuildStructure_ = SpawnedEntity->getComponentByType< GaStructureComponent >();
					}
				}
			}
			else if( InputState_ == InputState::BUILD_BUILDING )
			{
				ImGui::Text( "Selected to build: %s", (*BuildStructure_->getName()).c_str() );
				if( ImGui::Button( "Cancel build" ) )
				{
					setInputState( InputState::IDLE );
					BuildStructure_ = nullptr;
				}
			}

			ImGui::Separator();
			ImGui::End();
		}
#endif
}

//////////////////////////////////////////////////////////////////////////
// onDefendPhase
void GaGameComponent::onDefendPhase( BcF32 Tick )
{
	advanceGameTimer( Tick );

}

//////////////////////////////////////////////////////////////////////////
// onGameOver
void GaGameComponent::onGameOver( BcF32 Tick )
{

}
