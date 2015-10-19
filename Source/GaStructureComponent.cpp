#include "GaStructureComponent.h"
#include "GaGameComponent.h"
#include "GaHotspotComponent.h"
#include "GaPhysicsComponent.h"
#include "GaProjectileComponent.h"
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
REFLECTION_DEFINE_DERIVED( GaStructureProcessor );

void GaStructureProcessor::StaticRegisterClass()
{
	ReRegisterClass< GaStructureProcessor, Super >();
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaStructureProcessor::GaStructureProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Update game state",
				ScnComponentPriority::DEFAULT_UPDATE,
				std::bind( &GaStructureProcessor::update, this, std::placeholders::_1 ) )
		} )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
GaStructureProcessor::~GaStructureProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaStructureProcessor::initialise()
{

}

//////////////////////////////////////////////////////////////////////////
// shutdown
void GaStructureProcessor::shutdown()
{
	
}

//////////////////////////////////////////////////////////////////////////
// drawMenus
void GaStructureProcessor::update( const ScnComponentList& Components )
{
	const BcF32 Tick = SysKernel::pImpl()->getFrameTime();

	for( auto InComponent : Components )
	{
		BcAssert( InComponent->isTypeOf< GaStructureComponent >() );
		auto* Component = static_cast< GaStructureComponent* >( InComponent.get() );
		if( Component->Active_ )
		{
			Component->update( Tick );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaStructureComponent );

void GaStructureComponent::StaticRegisterClass()
{
	ReEnumConstant* StructureTypeEnumConstants[] = 
	{
		new ReEnumConstant( "TURRET", (BcU32)GaStructureType::TURRET ),
		new ReEnumConstant( "RESOURCE", (BcU32)GaStructureType::RESOURCE ),
		new ReEnumConstant( "POTATO", (BcU32)GaStructureType::POTATO ),
		new ReEnumConstant( "MINE", (BcU32)GaStructureType::MINE )
	};
	ReRegisterEnum< GaStructureType >( StructureTypeEnumConstants );

	ReField* Fields[] = 
	{
		new ReField( "Level_", &GaStructureComponent::Level_, bcRFF_IMPORTER ),
		new ReField( "Floating_", &GaStructureComponent::Floating_, bcRFF_IMPORTER ),
		new ReField( "Active_", &GaStructureComponent::Active_, bcRFF_IMPORTER ),

		new ReField( "BuildCost_", &GaStructureComponent::BuildCost_, bcRFF_IMPORTER ),
		new ReField( "BaseUpgradeCost_", &GaStructureComponent::BaseUpgradeCost_, bcRFF_IMPORTER ),
		new ReField( "LevelUpgradeCost_", &GaStructureComponent::LevelUpgradeCost_, bcRFF_IMPORTER ),
		new ReField( "CalculatedUpgradeCost_", &GaStructureComponent::CalculatedUpgradeCost_ ),

		new ReField( "BaseFireRate_", &GaStructureComponent::BaseFireRate_, bcRFF_IMPORTER ),
		new ReField( "LevelFireRateMultiplier_", &GaStructureComponent::LevelFireRateMultiplier_, bcRFF_IMPORTER ),
		new ReField( "CalculatedFireRate_", &GaStructureComponent::CalculatedFireRate_ ),

		new ReField( "BaseResourceRate_", &GaStructureComponent::BaseResourceRate_, bcRFF_IMPORTER ),
		new ReField( "LevelResourcesRateMultiplier_", &GaStructureComponent::LevelResourcesRateMultiplier_, bcRFF_IMPORTER ),
		new ReField( "CalculatedResourceRate_", &GaStructureComponent::CalculatedResourceRate_ ),


		new ReField( "StructureType_", &GaStructureComponent::StructureType_, bcRFF_IMPORTER ),
		new ReField( "TemplateProjectile_", &GaStructureComponent::TemplateProjectile_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
	};

	ReRegisterClass< GaStructureComponent, Super >( Fields )
		.addAttribute( new GaStructureProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaStructureComponent::GaStructureComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaStructureComponent::~GaStructureComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// setupTopology
void GaStructureComponent::setupTopology()
{
	// Setup physics.
	std::vector< GaPhysicsPointMass > PointMasses;
	std::vector< GaPhysicsConstraint > Constraints;
	PointMasses.reserve( 4 );
	Constraints.reserve( 6 );

	// TODO: Calculate this *properly* instead of drunkenly guessing?
	const BcF32 Size = 64.0f;
	const BcF32 PointMass = 1.0f;
	const MaVec2d Offsets[3] = 
	{
		MaVec2d( 0.0f, 1.0f ) * Size * 0.5f,
		MaVec2d( -0.9f, -0.5f ) * Size * 0.5f,
		MaVec2d( 0.9f, -0.5f ) * Size * 0.5f,
	};
	MaVec2d Position = getParentEntity()->getWorldPosition().xy();

	// Central point + external constraints.
	PointMasses.emplace_back( GaPhysicsPointMass( Position, 0.5f, 1.0f / PointMass ) );
	Constraints.emplace_back( GaPhysicsConstraint( 0, 1, -1.0f, 0.1f ) );
	Constraints.emplace_back( GaPhysicsConstraint( 0, 2, -1.0f, 0.1f ) );
	Constraints.emplace_back( GaPhysicsConstraint( 0, 3, -1.0f, 0.1f ) );

	// Outer edges.
	for( size_t Idx = 0; Idx < 3; ++Idx )
	{
		const MaVec2d Offset( Offsets[ Idx ] );
		PointMasses.emplace_back( GaPhysicsPointMass( Position + Offset, 0.5f, 1.0f / PointMass ) );
		Constraints.emplace_back( GaPhysicsConstraint( 1 + Idx, 1 + ( ( Idx + 1 ) % 3 ), -1.0f, 1.0f ) );
	}

	Physics_ = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	BcAssert( Physics_ );
	Physics_->setup( std::move( PointMasses ), std::move( Constraints ) );

	// Grab absolute position.
	AbsolutePosition_ = getParentEntity()->getWorldPosition().xy();

	PSY_LOG( "GaStructureComponent::setupTopology: %f, %f", AbsolutePosition_.x(), AbsolutePosition_.y() );
}

//////////////////////////////////////////////////////////////////////////
// setupHotspot
void GaStructureComponent::setupHotspot()
{
	auto Sprite = getComponentByType< ScnSpriteComponent >();
	if( Sprite )
	{
		BcAssert( ID_ != BcErrorCode );
		getParentEntity()->attach< GaHotspotComponent >( 
			BcName::INVALID,
			ID_, 0, 
			Sprite->getPosition() - Sprite->getSize() * 0.5f, 
			Sprite->getSize() );
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaStructureComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Get canvas + font.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >();
	BcAssert( Canvas_ );
	Font_ = Parent->getComponentAnyParentByType< ScnFontComponent >();
	BcAssert( Font_ );
	Physics_ = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	BcAssert( Physics_ );
	Game_ = getParentEntity()->getComponentAnyParentByType< GaGameComponent >();
	BcAssert( Game_ );

	// Subscribe to hotspot for pressed.
	Parent->subscribe( gaEVT_HOTSPOT_PRESSED, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< GaHotspotEvent >();

			return evtRET_PASS;
		} );

	// Begin build phase event
	Game_->getParentEntity()->subscribe( gaEVT_GAME_BEGIN_BUILD_PHASE, this, 
		[ this ]( EvtID, const EvtBaseEvent & Event )
		{
			if( StructureType_ == GaStructureType::RESOURCE )
			{
				Game_->spendResources( -CalculatedResourceRate_ );
			}
			return evtRET_PASS;
		} );

	setActive( Active_ );

	Level_ = 0;
	incLevel();
	Timer_ = BaseFireRate_;
	
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaStructureComponent::onDetach( ScnEntityWeakRef Parent )
{
	setActive( BcFalse );
	Parent->unsubscribeAll( this );
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// setActive
void GaStructureComponent::setActive( BcBool Active )
{
	Active_ = Active;

	if( Active_ )
	{
		setupTopology();
		setupHotspot();		
	}

	auto Sprite = getComponentByType< ScnSpriteComponent >();
	if( Sprite )
	{
		Sprite->setColour( Active_ ? RsColour( 0.0f, 0.5f, 0.0f, 1.0f ) : RsColour( 0.5f, 0.0f, 0.0f, 1.0f ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// incLevel
BcU32 GaStructureComponent::incLevel()
{
	++Level_;

	CalculatedUpgradeCost_ = BaseUpgradeCost_ + ( Level_ * LevelUpgradeCost_ );
	CalculatedFireRate_ = BaseFireRate_ * std::pow( LevelFireRateMultiplier_, static_cast< BcF32 >( Level_ ) );

	CalculatedResourceRate_ = BaseResourceRate_ + ( Level_ * LevelResourcesRateMultiplier_ );

	return Level_;
}

//////////////////////////////////////////////////////////////////////////
// update
void GaStructureComponent::update( BcF32 Tick )
{
	// If we have point masses, calculate position.
	const size_t NoofPointMasses = Physics_->getNoofPointMasses();
	if( Physics_->getNoofPointMasses() > 0 )
	{
#if 1
		MaVec2d Centre( 0.0f, 0.0f );
		for( size_t Idx = 1; Idx < NoofPointMasses; ++Idx )
		{
			Centre += Physics_->getPointMassPosition( Idx );
		}
		Centre /= static_cast< BcF32 >( NoofPointMasses - 1 );
		getParentEntity()->setLocalPosition( MaVec3d( Centre, 0.0f ) );
#else
		MaVec2d Centre( Component->Physics_->getPointMassPosition( 0 ) );
		Component->getParentEntity()->setLocalPosition( MaVec3d( Centre, 0.0f ) );
#endif
		if( Timer_ < CalculatedFireRate_ )
		{
			Timer_ += Tick;
		}
	}	
	
	switch( StructureType_ )
	{
	case GaStructureType::TURRET:
		if( Timer_ >= CalculatedFireRate_ )
		{
			// Find a tentacle.
			GaTentacleComponent* NearestTentacle = Game_->getNearestTentacle( BcFalse );
			if( NearestTentacle == nullptr )
			{
				NearestTentacle = Game_->getNearestTentacle( BcTrue );
			}
				
			// Spawn a projectile.
			if( NearestTentacle && TemplateProjectile_ )
			{
				auto Entity = ScnCore::pImpl()->spawnEntity( ScnEntitySpawnParams( 
					BcName::INVALID, TemplateProjectile_,
					getParentEntity()->getWorldMatrix(),
					Game_->getParentEntity() ) );
				auto Projectile = Entity->getComponentByType< GaProjectileComponent >();
				Projectile->setLevel( Level_ );
				Projectile->setTarget( NearestTentacle->getParentEntity() );
				Game_->launchProjectile( Projectile );

				// Time to spawn!
				Timer_ -= CalculatedFireRate_;
			}
		}
		break;

	case GaStructureType::RESOURCE:
		break;

	case GaStructureType::POTATO:
		break;

	case GaStructureType::MINE:
		break;
	}
}
