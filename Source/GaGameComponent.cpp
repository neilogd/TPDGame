#include "GaGameComponent.h"
#include "GaHotspotComponent.h"
#include "GaStructureComponent.h"
#include "GaTentacleComponent.h"
#include "GaPositionUtility.h"

#include "GaEvents.h"

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
		new ReField( "UpgradeMenuTemplate_", &GaGameComponent::UpgradeMenuTemplate_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY )
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

			if( Event.ID_ == 1000 )
			{
				if( SelectedStructure_ )
				{
					SelectedStructure_->getParentEntity()->setWorldPosition( 
						MaVec3d( Event.Position_, 0.0f ) );
				}
			}
			return evtRET_PASS;
		} );

	// Subscribe to hotspot for pressed.
	Parent->subscribe( gaEVT_HOTSPOT_PRESSED, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< GaHotspotEvent >();
			if( Event.ID_ == 1000 )
			{
				if( SelectedStructure_ )
				{
					SelectedStructure_->getParentEntity()->setWorldPosition( 
						MaVec3d( Event.Position_, 0.0f ) );
					buildStructure( SelectedStructure_ );
					SelectedStructure_ = nullptr;
					setInputState( InputState::IDLE );
				}
			}
			return evtRET_PASS;
		} );

	// Spawn tentacle things.

	for( BcF32 X = -480.0f; X <= 480.0f; X += 120.0f )
	{
		MaMat4d TransformA;
		auto TentacleA = GaPositionUtility::GetScreenPosition( MaVec2d( 0.0f, 128.0f ), MaVec2d( 0.0f, 32.0f ), GaPositionUtility::TOP | GaPositionUtility::HCENTRE );
		TransformA.translation( MaVec3d( TentacleA, 0.0f ) + MaVec3d( X, 0.0f, 0.0f ) );

		auto SpawnParamsA = ScnEntitySpawnParams( 
				BcName::INVALID, "tentacles", "TentacleEntity_0",
				TransformA, Parent );

		SpawnParamsA.OnSpawn_ = [ this ]( ScnEntity* Parent )
			{
				Tentacles_.push_back( Parent->getComponentByType< GaTentacleComponent >() );
			};

		ScnCore::pImpl()->spawnEntity( SpawnParamsA );
	}

	
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
// getStructures
const std::vector< class GaStructureComponent* >& GaGameComponent::getStructures() const
{
	return Structures_;
}

//////////////////////////////////////////////////////////////////////////
// getTentacles
const std::vector< class GaTentacleComponent* >& GaGameComponent::getTentacles() const
{
	return Tentacles_;
}

//////////////////////////////////////////////////////////////////////////
// buildStructure
void GaGameComponent::buildStructure( GaStructureComponent* Structure )
{
	BcAssert( Structure );
	Structures_.push_back( Structure );
	Structure->setID( StructureID_++ );
	Structure->setActive( BcTrue );

	// Listen for when structure is press.
	Structure->getParentEntity()->subscribe( gaEVT_HOTSPOT_PRESSED, this,
		[ this, Structure ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< GaHotspotEvent >();
			BcAssert( Structure->getID() == Event.ID_ );

			if( GameState_ == GameState::BUILD_PHASE )
			{
				CurrentModal_ = ScnCore::pImpl()->spawnEntity( 
					ScnEntitySpawnParams(
						BcName::INVALID,
						UpgradeMenuTemplate_,
						MaMat4d(), getParentEntity() ) );
				BcAssert( CurrentModal_ );

				// Subscribe to modal buttons.
				CurrentModal_->subscribe( gaEVT_HOTSPOT_PRESSED, this,
					[ this, Structure ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
					{
						const auto& Event = InEvent.get< GaHotspotEvent >();
						if( Event.ID_ == 0 )
						{
							if( spendResources( Structure->getUpgradeCost() ) )
							{
								Structure->incLevel();
							}
							ScnCore::pImpl()->removeEntity( CurrentModal_ );
							CurrentModal_ = nullptr;
						}
						else if( Event.ID_ == 1 )
						{
							ScnCore::pImpl()->removeEntity( CurrentModal_ );
							CurrentModal_ = nullptr;
						}
						return evtRET_PASS;
					} );
			}

			return evtRET_PASS;
		} );

}

//////////////////////////////////////////////////////////////////////////
// destroyStructure
void GaGameComponent::destroyStructure( GaStructureComponent* Structure )
{
	BcAssert( Structure );
	Structure->setActive( BcFalse );
	Structures_.erase( std::find( Structures_.begin(), Structures_.end(), Structure ) );
	ScnCore::pImpl()->removeEntity( Structure->getParentEntity() );
}

//////////////////////////////////////////////////////////////////////////
// incScore
void GaGameComponent::incScore( BcS64 NoofPoints )
{
	PlayerScore_ += NoofPoints;
}

//////////////////////////////////////////////////////////////////////////
// spendResources
BcBool GaGameComponent::spendResources( BcS64 NoofResources )
{
	if( NoofResources <= 0 || PlayerResources_ >= NoofResources )
	{
		PlayerResources_ -= NoofResources;
		return BcTrue;
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// setGameState
void GaGameComponent::setGameState( GameState GameState )
{
	if( GameState_ != GameState )
	{
		PSY_LOG( "Changing game state from %u -> %u", GameState_, GameState );
		GameState_ = GameState;

		switch( GameState_ )
		{
		case GaGameComponent::GameState::BUILD_PHASE:
			Level_++;
			getParentEntity()->publish( gaEVT_GAME_BEGIN_BUILD_PHASE, GaGameEvent( Level_ ) );
			break;
		case GaGameComponent::GameState::DEFEND_PHASE:
			if( CurrentModal_ )
			{
				ScnCore::pImpl()->removeEntity( CurrentModal_ );
				CurrentModal_ = nullptr;
			}
			getParentEntity()->publish( gaEVT_GAME_BEGIN_DEFEND_PHASE, GaGameEvent( Level_ ) );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setInputState
void GaGameComponent::setInputState( InputState InputState )
{
	PSY_LOG( "Changing input state from %u -> %u", InputState_, InputState );

	if( SelectedStructure_ )
	{
		// Give cost back.
		auto* StructureComponent = SelectedStructure_->getComponentByType< GaStructureComponent >();
		auto Cost = StructureComponent->getBuildCost();
		spendResources( -Cost );

		ScnCore::pImpl()->removeEntity( SelectedStructure_->getParentEntity() );
		SelectedStructure_ = nullptr;
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
		ImGui::Text( "Player score: %lld", PlayerScore_ );
		ImGui::Text( "Player resources: %lld", PlayerResources_ );
		ImGui::Text( "Game timer: %f", GameTimer_ );
		ImGui::Text( "Game state: %u", GameState_ );
		ImGui::Text( "Game level: %u", Level_ );
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
						// Get cost of structure.
						auto Cost = StructureComponent->getBuildCost();
						if( spendResources( Cost ) )
						{
							// Set input state (clears selected already)
							setInputState( InputState::BUILD_BUILDING );

							// Setup new structure.
							auto SpawnedEntity = ScnCore::pImpl()->spawnEntity( ScnEntitySpawnParams(
								StructureEntity->getName(), StructureEntity, MaMat4d(), getParentEntity() ) );
							BcAssert( SpawnedEntity );
							SelectedStructure_ = SpawnedEntity->getComponentByType< GaStructureComponent >();
						}
					}
				}
			}
			else if( InputState_ == InputState::BUILD_BUILDING )
			{
				ImGui::Text( "Selected to build: %s", (*SelectedStructure_->getName()).c_str() );
				if( ImGui::Button( "Cancel build" ) )
				{
					setInputState( InputState::IDLE );
					SelectedStructure_ = nullptr;
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
