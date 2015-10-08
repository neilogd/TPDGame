#include "GaStructureComponent.h"
#include "GaHotspotComponent.h"
#include "GaPhysicsComponent.h"
#include "GaPositionUtility.h"

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

		// If we have point masses, calculate position.
		const size_t NoofPointMasses = Component->Physics_->getNoofPointMasses();
		if( Component->Physics_->getNoofPointMasses() > 0 )
		{
#if 1
			MaVec2d Centre( 0.0f, 0.0f );
			for( size_t Idx = 1; Idx < NoofPointMasses; ++Idx )
			{
				Centre += Component->Physics_->getPointMassPosition( Idx );
			}
			Centre /= static_cast< BcF32 >( NoofPointMasses - 1 );
			Component->getParentEntity()->setLocalPosition( MaVec3d( Centre, 0.0f ) );
#else
			MaVec2d Centre( Component->Physics_->getPointMassPosition( 0 ) );
			Component->getParentEntity()->setLocalPosition( MaVec3d( Centre, 0.0f ) );
#endif
			Component->Timer_ += Tick;

			// Offset the centre bit to give some wobbly goodness.
			MaVec2d Offset( MaVec2d( BcCos( Component->Timer_ ), -BcSin( Component->Timer_ * 4.0f ) ) * 8.0f );
			Component->Physics_->setPointMassPosition( 0, Component->AbsolutePosition_ + Offset );
		}	
	}
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaStructureComponent );

void GaStructureComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Level_", &GaStructureComponent::Level_, bcRFF_IMPORTER ),
		new ReField( "Active_", &GaStructureComponent::Active_, bcRFF_IMPORTER ),
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

	Physics_->setup( std::move( PointMasses ), std::move( Constraints ) );

	// Grab absolute position.
	AbsolutePosition_ = getParentEntity()->getWorldPosition().xy();

	PSY_LOG( "GaStructureComponent::setupTopology: %f, %f", AbsolutePosition_.x(), AbsolutePosition_.y() );
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

	setActive( Active_ );
	
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
	}

	auto Sprite = getComponentByType< ScnSpriteComponent >();
	if( Sprite )
	{
		Sprite->setColour( Active_ ? RsColour( 0.0f, 0.5f, 0.0f, 1.0f ) : RsColour( 0.5f, 0.0f, 0.0f, 1.0f ) );
	}
}
