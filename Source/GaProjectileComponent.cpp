#include "GaProjectileComponent.h"
#include "GaPhysicsComponent.h"
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
REFLECTION_DEFINE_DERIVED( GaProjectileProcessor );

void GaProjectileProcessor::StaticRegisterClass()
{
	ReRegisterClass< GaProjectileProcessor, Super >();
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaProjectileProcessor::GaProjectileProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Update game state",
				ScnComponentPriority::DEFAULT_UPDATE,
				std::bind( &GaProjectileProcessor::update, this, std::placeholders::_1 ) )
		} )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
GaProjectileProcessor::~GaProjectileProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaProjectileProcessor::initialise()
{

}

//////////////////////////////////////////////////////////////////////////
// shutdown
void GaProjectileProcessor::shutdown()
{
	
}

//////////////////////////////////////////////////////////////////////////
// drawMenus
void GaProjectileProcessor::update( const ScnComponentList& Components )
{
	const BcF32 Tick = SysKernel::pImpl()->getFrameTime();

	for( auto InComponent : Components )
	{
		BcAssert( InComponent->isTypeOf< GaProjectileComponent >() );
		auto* Component = static_cast< GaProjectileComponent* >( InComponent.get() );
		
		const auto TargetPos = Component->Target_->getWorldPosition().xy();
		const auto CurrentPos = Component->Physics_->getPointMassPosition( 0 );

		auto TotalVector = ( TargetPos - CurrentPos );
		auto TotalVectorMagnitude = TotalVector.magnitude();
		auto Vector = ( TotalVector / TotalVectorMagnitude ) * Component->Acceleration_;
		Component->Physics_->setPointMassAcceleration( 0, Vector );

		if( TotalVectorMagnitude < Component->DamageDistance_ )
		{
			Component->Target_->publish( gaEVT_PROJECTILE_HIT, GaProjectileEvent() );

			// TODO: Send damage event.
			ScnCore::pImpl()->removeEntity( Component->getParentEntity() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaProjectileComponent );

void GaProjectileComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "DamageDistance_", &GaProjectileComponent::DamageDistance_, bcRFF_IMPORTER ),
		new ReField( "MaxSpeed_", &GaProjectileComponent::MaxSpeed_, bcRFF_IMPORTER ),
		new ReField( "LevelSpeedMultiplier_", &GaProjectileComponent::LevelSpeedMultiplier_, bcRFF_IMPORTER ),
		new ReField( "CalculatedMaxSpeed_", &GaProjectileComponent::CalculatedMaxSpeed_ ),
		new ReField( "Acceleration_", &GaProjectileComponent::Acceleration_, bcRFF_IMPORTER ),
		new ReField( "Drag_", &GaProjectileComponent::Drag_, bcRFF_IMPORTER ),
		new ReField( "Mass_", &GaProjectileComponent::Mass_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< GaProjectileComponent, Super >( Fields )
		.addAttribute( new GaProjectileProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaProjectileComponent::GaProjectileComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaProjectileComponent::~GaProjectileComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// setupTopology
void GaProjectileComponent::setupTopology()
{
	// Setup physics.
	std::vector< GaPhysicsPointMass > PointMasses;
	std::vector< GaPhysicsConstraint > Constraints;
	PointMasses.reserve( 1 );
	Constraints.reserve( 1 );

	PointMasses.emplace_back( GaPhysicsPointMass( getParentEntity()->getWorldPosition().xy(), Drag_, 1.0f / 1.0f, CalculatedMaxSpeed_ ) );

	Physics_->setup( std::move( PointMasses ), std::move( Constraints ) );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaProjectileComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Get canvas + font.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >();
	BcAssert( Canvas_ );
	Physics_ = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	BcAssert( Physics_ );

	setupTopology();
	
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaProjectileComponent::onDetach( ScnEntityWeakRef Parent )
{
	Parent->unsubscribeAll( this );
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// setLevel
void GaProjectileComponent::setLevel( BcU32 Level )
{
	auto Levelf = static_cast< BcF32 >( Level );
	CalculatedMaxSpeed_ = MaxSpeed_ * ( 1.0f + Levelf );
}

//////////////////////////////////////////////////////////////////////////
// setTarget
void GaProjectileComponent::setTarget( ScnEntity* Target )
{
	Target_ = Target;
}

