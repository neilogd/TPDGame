#include "GaTentacleComponent.h"
#include "GaHotspotComponent.h"
#include "GaPhysicsComponent.h"

#include "System/SysKernel.h"

#include "System/Debug/DsCore.h"
#include "System/Debug/DsImGui.h"
#include "System/Os/OsCore.h"

#include "System/Scene/ScnEntity.h"


//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaTentacleProcessor );

void GaTentacleProcessor::StaticRegisterClass()
{
	ReRegisterClass< GaTentacleProcessor, Super >();
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaTentacleProcessor::GaTentacleProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Update simulations",
				ScnComponentPriority::PHYSICS_WORLD_SIMULATE,
				std::bind( &GaTentacleProcessor::update, this, std::placeholders::_1 ) ),
		} )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
GaTentacleProcessor::~GaTentacleProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaTentacleProcessor::initialise()
{

}

//////////////////////////////////////////////////////////////////////////
// shutdown
void GaTentacleProcessor::shutdown()
{
	
}

//////////////////////////////////////////////////////////////////////////
// update
void GaTentacleProcessor::update( const ScnComponentList& Components )
{
	const BcF32 Tick = SysKernel::pImpl()->getFrameTime();
	static BcF32 Timer = 0.0f;
	if( Components.size() > 0 )
	{
		MaVec2d Offset( MaVec2d( BcCos( Timer * 5.0f ), BcSin( Timer ) * 4.0f ) * 64.0f );
		Timer += Tick * 0.25f;
		for( auto InComponent : Components )
		{
			BcAssert( InComponent->isTypeOf< GaTentacleComponent >() );
			auto* Component = static_cast< GaTentacleComponent* >( InComponent.get() );
			auto* Physics = Component->getParentEntity()->getComponentByType< GaPhysicsComponent >();
				
			Physics->setPointMassPosition( 0, Component->getParentEntity()->getWorldPosition().xy() + Offset );
		}
	}
	else
	{
		Timer = 0.0f;
	}
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaTentacleComponent );

void GaTentacleComponent::StaticRegisterClass()
{
#if 0
	ReField* Fields[] = 
	{
		new ReField( "ID_", &GaTentacleComponent::ID_, bcRFF_IMPORTER ),
		new ReField( "Position_", &GaTentacleComponent::Position_, bcRFF_IMPORTER ),
		new ReField( "Size_", &GaTentacleComponent::Size_, bcRFF_IMPORTER ),
	};
#endif

	ReRegisterClass< GaTentacleComponent, Super >()
		.addAttribute( new GaTentacleProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaTentacleComponent::GaTentacleComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaTentacleComponent::~GaTentacleComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// setupComplexTopology
void GaTentacleComponent::setupComplexTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections )
{
	// Setup physics.
	std::vector< GaPhysicsPointMass > PointMasses;
	std::vector< GaPhysicsConstraint > Constraints;

	const BcF32 HalfWidth = Width * 0.5f;

	// Head point used to nagivate.
	PointMasses.emplace_back( GaPhysicsPointMass( MaVec2d( 0.0f, 0.0f ), 1.0f, 0.0f ) );
	Constraints.emplace_back( GaPhysicsConstraint( 0, 1, -1.0f, 1.0f ) );
	Constraints.emplace_back( GaPhysicsConstraint( 0, 2, -1.0f, 1.0f ) );
	size_t PointOffset = 1;
	MaVec2d Offset( 0.0f, SectionHeight );
	for( size_t Idx = 0; Idx < NoofSections; ++Idx )
	{
		PointMasses.push_back( GaPhysicsPointMass( MaVec2d( -HalfWidth, 0.0f ) + Offset, 0.1f, 1.0f / 1.0f ) );
		PointMasses.push_back( GaPhysicsPointMass( MaVec2d(  HalfWidth, 0.0f ) + Offset, 0.1f, 1.0f / 1.0f ) );

		Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 1, -1.0f, 1.0f ) );

		Offset += MaVec2d( 0.0f, SectionHeight );

		// Link to next section.
		if( Idx < ( NoofSections - 1 ) )
		{
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 2, -1.0f, 1.0f ) );
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 1, PointOffset + 3, -1.0f, 1.0f ) );

			Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 1, PointOffset + 2, -1.0f, 0.5f ) );
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 3, -1.0f, 0.5f ) );
			PointOffset += 2;
		}
	}

	for( auto& PointMass : PointMasses )
	{
		PointMass.CurrPosition_ += RootPosition;
		PointMass.PrevPosition_ += RootPosition;
		PointMass.Acceleration_ = MaVec2d( 0.0f, 100.0f );
	}
	PointMasses[0].Acceleration_ = MaVec2d( 0.0f, 0.0f );

	// Pin the end.
	PointMasses[ PointMasses.size() - 2 ].InvMass_ = 0.0f;
	PointMasses[ PointMasses.size() - 1 ].InvMass_ = 0.0f;
	PointMasses[ PointMasses.size() - 2 ].DampingFactor_ = 1.0f;
	PointMasses[ PointMasses.size() - 1 ].DampingFactor_= 1.0f;

	auto Physics = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	Physics->setup( std::move( PointMasses ), std::move( Constraints ) );
}

//////////////////////////////////////////////////////////////////////////
// setupDiamondTopology
void GaTentacleComponent::setupDiamondTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections )
{
	// Setup physics.
	std::vector< GaPhysicsPointMass > PointMasses;
	std::vector< GaPhysicsConstraint > Constraints;

	const BcF32 HalfWidth = Width * 0.5f;
	const BcF32 HalfSectionHeight = SectionHeight * 0.5f;

	// Head point used to nagivate.
	size_t PointOffset = 0;
	MaVec2d Offset( 0.0f, SectionHeight );
	for( size_t Idx = 0; Idx < NoofSections; ++Idx )
	{
		PointMasses.push_back( GaPhysicsPointMass( MaVec2d(  0.0f, 0.0f ) + Offset, 0.1f, 1.0f / 1.0f ) );
		PointMasses.push_back( GaPhysicsPointMass( MaVec2d( -HalfWidth, HalfSectionHeight ) + Offset, 0.1f, 1.0f / 1.0f ) );
		PointMasses.push_back( GaPhysicsPointMass( MaVec2d(  HalfWidth, HalfSectionHeight ) + Offset, 0.1f, 1.0f / 1.0f ) );

		Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 1, -1.0f, 1.0f ) );
		Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 2, -1.0f, 1.0f ) );
		Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 1, PointOffset + 2, -1.0f, 1.0f ) );
		Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 3, -1.0f, 1.0f ) );

		Offset += MaVec2d( 0.0f, SectionHeight );

		// Link to next section.
		if( Idx < ( NoofSections - 2 ) )
		{
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 1, PointOffset + 3, -1.0f, 1.0f ) );
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 2, PointOffset + 3, -1.0f, 1.0f ) );
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 1, PointOffset + 4, -1.0f, 0.9f ) );
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 2, PointOffset + 5, -1.0f, 0.9f ) );
			PointOffset += 3;
		}
	}

	for( auto& PointMass : PointMasses )
	{
		PointMass.CurrPosition_ += RootPosition;
		PointMass.PrevPosition_ += RootPosition;
		PointMass.Acceleration_ = MaVec2d( 0.0f, 100.0f );
	}
	PointMasses[0].Acceleration_ = MaVec2d( 0.0f, 0.0f );

	// Pin the end.
	PointMasses[ PointMasses.size() - 5 ].InvMass_ = 0.0f;
	PointMasses[ PointMasses.size() - 4 ].InvMass_ = 0.0f;
	PointMasses[ PointMasses.size() - 3 ].InvMass_ = 0.0f;
	PointMasses[ PointMasses.size() - 2 ].InvMass_ = 0.0f;
	PointMasses[ PointMasses.size() - 1 ].InvMass_ = 0.0f;
	PointMasses[ PointMasses.size() - 5 ].DampingFactor_ = 1.0f;
	PointMasses[ PointMasses.size() - 4 ].DampingFactor_ = 1.0f;
	PointMasses[ PointMasses.size() - 3 ].DampingFactor_ = 1.0f;
	PointMasses[ PointMasses.size() - 2 ].DampingFactor_ = 1.0f;
	PointMasses[ PointMasses.size() - 1 ].DampingFactor_= 1.0f;

	auto Physics = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	Physics->setup( std::move( PointMasses ), std::move( Constraints ) );
}

//////////////////////////////////////////////////////////////////////////
// setupSimpleTopology
void GaTentacleComponent::setupSimpleTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections )
{
	// Setup physics.
	std::vector< GaPhysicsPointMass > PointMasses;
	std::vector< GaPhysicsConstraint > Constraints;

	// Head point used to nagivate.
	PointMasses.emplace_back( GaPhysicsPointMass( MaVec2d( 0.0f, 0.0f ), 1.0f, 0.0f ) );
	Constraints.emplace_back( GaPhysicsConstraint( 0, 1, -1.0f, 1.0f ) );
	size_t PointOffset = 1;
	size_t Distance = 8;
	BcF32 LargeConstraintSize = 0.1f;
	MaVec2d Offset( 0.0f, SectionHeight );
	for( size_t Idx = 0; Idx < NoofSections; ++Idx )
	{
		PointMasses.push_back( GaPhysicsPointMass( MaVec2d( 0, 0.0f ) + Offset, 0.1f, 1.0f / 1.0f ) );

		Offset += MaVec2d( 0.0f, SectionHeight );

		// Link to next section.
		if( Idx < ( NoofSections - 1 ) )
		{
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 1, -1.0f, 1.0f ) );
			PointOffset += 1;

			size_t NextIdx = std::min( Idx + Distance, ( (size_t)NoofSections - 1 ) );
			Constraints.emplace_back( GaPhysicsConstraint( Idx, NextIdx, -1.0f, LargeConstraintSize ) );
			LargeConstraintSize *= 1.0f;
		}
	}

	for( auto& PointMass : PointMasses )
	{
		PointMass.CurrPosition_ += RootPosition;
		PointMass.PrevPosition_ += RootPosition;
		PointMass.Acceleration_ = MaVec2d( 0.0f, 100.0f );
	}
	PointMasses[ 0 ].Acceleration_ = MaVec2d( 0.0f, 0.0f );
	PointMasses[ PointMasses.size() - 1 ].Acceleration_ = MaVec2d( 0.0f, 0.0f );
	PointMasses[ PointMasses.size() - 1 ].InvMass_ = 0.0f;
	PointMasses[ PointMasses.size() - 1 ].DampingFactor_ = 1.0f;

	auto Physics = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	Physics->setup( std::move( PointMasses ), std::move( Constraints ) );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaTentacleComponent::onAttach( ScnEntityWeakRef Parent )
{
	setupComplexTopology( Parent->getWorldPosition().xy(), 32.0f, 32.0f, 20 );

	// Setup callback for clicking.
	Parent->subscribe( gaEVT_HOTSPOT_HOVER, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< GaHotspotEvent >();
			if( Event.ID_ == 1000 )
			{
				auto Physics = getParentEntity()->getComponentByType< GaPhysicsComponent >();

				const MaVec2d Position = Physics->getPointMassPosition( 0 );
				const BcF32 MaxVelocity = 32.0f;
				MaVec2d Velocity = ( Event.Position_ - Position );
				if( Velocity.magnitude() > MaxVelocity )
				{
					Velocity = ( Velocity / Velocity.magnitude() ) * MaxVelocity;
				}

				Physics->setPointMassPosition( 0, Position + Velocity );
			}
			return evtRET_PASS;
		} );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaTentacleComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

