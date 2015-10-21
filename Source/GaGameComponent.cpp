#include "GaGameComponent.h"
#include "GaHotspotComponent.h"
#include "GaProjectileComponent.h"
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
#include "System/Scene/Rendering/ScnSpriteComponent.h"

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
		new ReField( "UpgradeMenuTemplate_", &GaGameComponent::UpgradeMenuTemplate_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "ButtonTemplate_", &GaGameComponent::ButtonTemplate_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
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
	OsClient* Client = OsCore::pImpl()->getClient( 0 );
	MaVec2d Dimensions( Client->getWidth(), Client->getHeight() );

	// Get canvas + font.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >();
	BcAssert( Canvas_ );
	Font_ = Parent->getComponentAnyParentByType< ScnFontComponent >();
	BcAssert( Font_ );

	BuildUIEntity_ = Parent->getComponentByType< ScnEntity >( "UIEntity" );
	BcAssert( BuildUIEntity_ );

	// Spawn hotspot for placement area.
	Parent->attach< GaHotspotComponent >( 
		BcName::INVALID,
		1000, -1000,
		MaVec2d( 64.0f, 64.0f + 128.0f ),
		Dimensions - MaVec2d( 128.0f, 512.0f ) );

	// Create buttons.
	createStructureButtons();

	// Subscribe to hotspot for hover.
	Parent->subscribe( gaEVT_HOTSPOT_HOVER, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< GaHotspotEvent >();

			if( Event.ID_ == 1000 )
			{
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
				if( SelectedStructureIdx_ != BcErrorCode )
				{
					auto Position = getStructurePlacement( Event.Position_ );

					buildStructure( SelectedStructureIdx_, Position );
					setSelection( BcErrorCode );
				}
			}
			return evtRET_PASS;
		} );
	
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
// onObjectDeleted
void GaGameComponent::onObjectDeleted( class ReObject* Object )
{
	auto Structure = std::find( Structures_.begin(), Structures_.end(), Object );
	auto Tentacle = std::find( Tentacles_.begin(), Tentacles_.end(), Object );
	auto Projectile = std::find( Projectiles_.begin(), Projectiles_.end(), Object );

	if( Structure != Structures_.end() )
	{
		Structures_.erase( Structure );
	}

	if( Tentacle != Tentacles_.end() )
	{
		Tentacles_.erase( Tentacle );
	}

	if( Projectile != Projectiles_.end() )
	{
		Projectiles_.erase( Projectile );
	}
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
// createStructureButtons
void GaGameComponent::createStructureButtons()
{
	// Total width.
	auto ButtonSize = ButtonTemplate_->getComponentByType< ScnSpriteComponent >()->getSize();
	const BcF32 MarginSize = ButtonSize.x() * 0.25f;
	auto TotalSize = MaVec2d(
		( ButtonSize.x() * StructureTemplates_.size() ) + 
			( MarginSize * ( StructureTemplates_.size() - 1 ) ), 
		ButtonSize.y() + MarginSize * 2.0f );

	auto CentrePosition = GaPositionUtility::GetScreenPosition( MaVec2d( 0.0f, -MarginSize ), TotalSize, GaPositionUtility::HCENTRE | GaPositionUtility::BOTTOM );
	
	BcU32 ID = 0;
	for( auto TemplateEntity : StructureTemplates_ )
	{
		MaMat4d Transform;
		Transform.translation( MaVec3d( CentrePosition.x(), CentrePosition.y(), 0.0f ) );
		CentrePosition += MaVec2d( ButtonSize.x() + MarginSize, 0.0f );

		auto ButtonEntity = ScnCore::pImpl()->spawnEntity( 
			ScnEntitySpawnParams(
				BcName::INVALID,
				ButtonTemplate_,
				Transform, BuildUIEntity_ ) );
		BcAssert( ButtonEntity );

		StructureUISprites_.push_back( ButtonEntity->getComponentByType< ScnSpriteComponent >() );
		
		// Subscribe to event on button entity.
		ButtonEntity->subscribe( gaEVT_HOTSPOT_PRESSED, this,
			[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
			{
				const auto& Event = InEvent.get< GaHotspotEvent >();
				if( Event.ID_ < StructureTemplates_.size() )
				{
					if( GameState_ == GameState::BUILD_PHASE )
					{
						if( InputState_ == InputState::BUILD_BUILDING )
						{
							setSelection( BcErrorCode );
						}

						if( InputState_ == InputState::IDLE )
						{
							setSelection( Event.ID_ );
						}
					}
				}
				return evtRET_PASS;
			} );

		auto TemplateSprite = TemplateEntity->getComponentByType< ScnSpriteComponent >();
		BcAssert( TemplateSprite );
		auto Sprite = ButtonEntity->getComponentByType< ScnSpriteComponent >();
		BcAssert( Sprite );

		auto Hotspot = ButtonEntity->getComponentByType< GaHotspotComponent >();
		BcAssert( Hotspot );
		Hotspot->setID( ID++ );

		Sprite->setSpriteIndex( TemplateSprite->getSpriteIndex() );
	}
}

//////////////////////////////////////////////////////////////////////////
// setSelection
void GaGameComponent::setSelection( BcU32 SelectedIdx )
{
	bool ShouldSelect = SelectedStructureIdx_ != SelectedIdx;

	// Undo structure selection + refund cost.
	if( SelectedStructureIdx_ != BcErrorCode )
	{
		auto OldStructureSprite = StructureUISprites_[ SelectedStructureIdx_ ];
		OldStructureSprite->setColour( RsColour::WHITE );
		SelectedStructureIdx_ = BcErrorCode;
		setInputState( InputState::IDLE );
	}

	if( ShouldSelect && SelectedIdx != BcErrorCode )
	{
		auto StructureEntity = StructureTemplates_[ SelectedIdx ];
		auto Structure = StructureEntity->getComponentByType< GaStructureComponent >();
		auto Cost = Structure->getBuildCost();
		if( Cost <= PlayerResources_ )
		{
			// Set input state (clears selected already)
			setInputState( InputState::BUILD_BUILDING );

			auto NewStructureSprite = StructureUISprites_[ SelectedIdx ];
			NewStructureSprite->setColour( RsColour::GREEN );
			SelectedStructureIdx_ = SelectedIdx;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// buildStructure
bool GaGameComponent::buildStructure( BcU32 StructureIdx, MaVec2d Position )
{
	BcAssert( ( Position - getStructurePlacement( Position ) ).magnitude() < 1e-6f );

	// Check position is not taken.
	for( auto Structure : Structures_ )
	{
		auto StructurePos = Structure->getParentEntity()->getWorldPosition().xy();
		if( ( Position - StructurePos ).magnitude() < 1e-6f )
		{
			return false;
		}
	}

	// Setup new structure.
	auto StructureEntity = StructureTemplates_[ StructureIdx ];
	auto Structure = StructureEntity->getComponentByType< GaStructureComponent >();
	BcAssert( Structure );

	if( spendResources( Structure->getBuildCost() ) )
	{
		auto SpawnParams = ScnEntitySpawnParams(
			StructureEntity->getName(), StructureEntity, MaMat4d(), getParentEntity() );

		SpawnParams.OnSpawn_ = [ this ]( ScnEntity* Parent )
			{
				auto Structure = Parent->getComponentByType< GaStructureComponent >();
				BcAssert( Structure );
				Structures_.push_back( Structure );
				Structure->setID( StructureID_++ );
				Structure->setActive( BcTrue );
			};

		auto SpawnedEntity = ScnCore::pImpl()->spawnEntity( SpawnParams );
		BcAssert( SpawnedEntity );
		Structure = SpawnedEntity->getComponentByType< GaStructureComponent >();
		BcAssert( Structure );

		SpawnedEntity->setWorldPosition( 
			MaVec3d( Position, 0.0f ) );

		spawnPopupText( Position - MaVec2d( 0.0f, 32.0f ), MaVec2d( 0.0f, -8.0f ), 2.0f, "Build!" );

		// Listen for when structure is press.
		SpawnedEntity->subscribe( gaEVT_HOTSPOT_PRESSED, this,
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
									spawnPopupText( Structure->getParentEntity()->getWorldPosition().xy() - MaVec2d( 0.0f, 32.0f ), MaVec2d( 0.0f, -8.0f ), 2.0f, "Upgrade!" );
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
		return true;
	}
	return false;
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
// launchProjectile
void GaGameComponent::launchProjectile( GaProjectileComponent* Projectile )
{
	Projectiles_.push_back( Projectile );
	Projectile->addNotifier( this );
}

//////////////////////////////////////////////////////////////////////////
// spawnTentacles
void GaGameComponent::spawnTentacles()
{
	// Spawn tentacle thing.
	auto X = haltonSequence( Level_, 2 ) * 960.0f - 480.0f;

	MaMat4d TransformA;
	auto TentacleA = GaPositionUtility::GetScreenPosition( MaVec2d( 0.0f, 800.0f ), MaVec2d( 0.0f, 32.0f ), GaPositionUtility::TOP | GaPositionUtility::HCENTRE );
	TransformA.translation( MaVec3d( TentacleA, 0.0f ) + MaVec3d( X, 0.0f, 0.0f ) );

	auto SpawnParamsA = ScnEntitySpawnParams( 
			BcName::INVALID, "tentacles", "TentacleEntity_0",
			TransformA, getParentEntity() );

	SpawnParamsA.OnSpawn_ = [ this ]( ScnEntity* Parent )
		{
			Tentacles_.push_back( Parent->getComponentByType< GaTentacleComponent >() );
		};

	ScnCore::pImpl()->spawnEntity( SpawnParamsA );
}

//////////////////////////////////////////////////////////////////////////
// getNearestTentacle
class GaTentacleComponent* GaGameComponent::getNearestTentacle( BcBool IncludeTargetted ) const
{
	GaTentacleComponent* NearestTentacle = nullptr;
	if( Tentacles_.size() > 0 )
	{
		auto NearestDistance = std::numeric_limits< BcF32 >::max();
		for( size_t Idx = 0; Idx < Tentacles_.size(); ++Idx )
		{
			auto* Tentacle = Tentacles_[ Idx ];
			if( Tentacle->getTargetStructure() )
			{
				// Check if projectile is targetting tentacle already.
				auto FoundProjectile = 
					IncludeTargetted ? Projectiles_.end() :
						std::find_if( Projectiles_.begin(), Projectiles_.end(),
							[ Tentacle ]( GaProjectileComponent* Projectile )
							{
								return Projectile->getTarget() == Tentacle->getParentEntity();
							} );

				if( FoundProjectile == Projectiles_.end() )
				{
					auto Distance = ( getParentEntity()->getWorldPosition().xy() - Tentacle->getParentEntity()->getWorldPosition().xy() ).magnitudeSquared();
					if( Distance < NearestDistance )
					{
						NearestTentacle = Tentacle;
						NearestDistance = Distance;
					}
				}
			}	
		}
	}
	return NearestTentacle;
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
// addScore
void GaGameComponent::addScore( MaVec2d Position, BcS64 Score )
{
	PlayerScore_ += Score;
	spawnPopupText( Position, MaVec2d( 0.0f, -32.0f ), 3.0f, "+%u", static_cast< BcU32 >( Score ) );
}

//////////////////////////////////////////////////////////////////////////
// getStructurePlacement
MaVec2d GaGameComponent::getStructurePlacement( MaVec2d Position )
{
	const auto Spacing = 128.0f;
	const auto HalfSpacing = Spacing * 0.5f;
	return MaVec2d(
		std::roundf( Position.x() / Spacing ) * Spacing,
		std::roundf( Position.y() / Spacing ) * Spacing );
}

//////////////////////////////////////////////////////////////////////////
// haltonSequence
BcF32 GaGameComponent::haltonSequence( BcU32 Index, BcU32 Base )
{
	BcF32 Result = 0;
	BcF32 F = 1.0f;
	BcU32 Idx = Index;
	while( Idx > 0 )
	{
		F = F / static_cast< BcF32 >( Base );
		Result = Result + F * ( Idx % Base );
		Idx = Idx / Base;
	}
	return Result;
}

//////////////////////////////////////////////////////////////////////////
// spawnPopupText
void GaGameComponent::spawnPopupText( MaVec2d Position, MaVec2d Velocity, BcF32 Time, const char* Format, ... )
{
	PopupText PopupText = 
	{
		{ 0 }, Position, Velocity, Time
	};
	va_list ArgList;
	va_start( ArgList, Format);
	BcVSPrintf( PopupText.Text_, sizeof( PopupText.Text_ ) - 1, Format, ArgList );
	va_end( ArgList );

	PopupText_.push_back( PopupText );
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
			{
				Level_++;
				getParentEntity()->publish( gaEVT_GAME_BEGIN_BUILD_PHASE, GaGameEvent( Level_ ) );
				BuildUIEntityTarget_ = MaVec2d( 0.0f, 0.0f );

				// Score for each structure.
				for( auto& Structure : Structures_ )
				{
					BcS64 Score = 10 * Structure->getLevel();
					addScore( Structure->getParentEntity()->getWorldPosition().xy(), Score );
				}
			}
			break;
		case GaGameComponent::GameState::DEFEND_PHASE:
			{
				if( CurrentModal_ )
				{
					ScnCore::pImpl()->removeEntity( CurrentModal_ );
					CurrentModal_ = nullptr;
				}
				spawnTentacles();
				getParentEntity()->publish( gaEVT_GAME_BEGIN_DEFEND_PHASE, GaGameEvent( Level_ ) );
				BuildUIEntityTarget_ = MaVec2d( 0.0f, 240.0f );
			}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setInputState
void GaGameComponent::setInputState( InputState InputState )
{
	PSY_LOG( "Changing input state from %u -> %u", InputState_, InputState );

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
	
	// Move UI.
	auto UIEntityPosition = BuildUIEntity_->getLocalPosition().xy();
	UIEntityPosition = UIEntityPosition * 0.9f + BuildUIEntityTarget_ * 0.1f;
	BuildUIEntity_->setLocalPosition( MaVec3d( UIEntityPosition, 0.0f ) );

	// Render popup text.
	ScnFontDrawParams PopupDrawParams;
	PopupDrawParams.setAlignment( ScnFontAlignment::HCENTRE | ScnFontAlignment::VCENTRE );
	PopupDrawParams.setSize( 20.0f );
	PopupDrawParams.setMargin( 0.0f );
	PopupDrawParams.setTextSettings( MaVec4d( 0.35f, 0.5f, 0.0f, 0.0f ) );
	PopupDrawParams.setTextColour( RsColour::BLACK );
	PopupDrawParams.setLayer( 1000 );
	for( auto It = PopupText_.begin(); It != PopupText_.end(); )
	{
		auto& PopupText = *It;
		if( PopupText.Time_ > 0.0f )
		{
			Font_->drawText( Canvas_, PopupDrawParams,
				PopupText.Position_,
				MaVec2d( 0.0f, 0.0f ),
				PopupText.Text_ );
			PopupText.Time_ -= Tick;
			PopupText.Position_ += PopupText.Velocity_ * Tick;
			++It;
		}
		else
		{
			It = PopupText_.erase( It );
		}
	}

#if !PSY_PRODUCTION
	if( ImGui::Begin( "Game Debug" ) )
	{
		ImGui::Text( "Player score: %lld", PlayerScore_ );
		ImGui::Text( "Player resources: %lld", PlayerResources_ );
		ImGui::Text( "Game timer: %f", GameTimer_ );
		ImGui::Text( "Game state: %u", GameState_ );
		ImGui::Text( "Input state: %u", InputState_ );
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
	}
	ImGui::End();
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
		setSelection( BcErrorCode );
	}
	else if( GameState_ == GameState::DEFEND_PHASE &&
		LevelGameTimer < HalfGamePhaseTime )
	{
		setGameState( GameState::BUILD_PHASE );
		setSelection( BcErrorCode );
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

