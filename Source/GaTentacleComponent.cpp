#include "GaTentacleComponent.h"
#include "GaGameComponent.h"
#include "GaPhysicsComponent.h"
#include "GaStructureComponent.h"

#include "GaEvents.h"

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
		// Grab game component.
		auto FirstComponent = Components[0];
		auto Game = static_cast< GaGameComponent* >( FirstComponent.get() )->getComponentAnyParentByType< GaGameComponent >();
		BcAssert( Game );
		const auto& Structures = Game->getStructures();
	
		// Update each tentacle.
		Timer += Tick * 0.25f;
		for( auto InComponent : Components )
		{
			BcAssert( InComponent->isTypeOf< GaTentacleComponent >() );
			auto* Component = static_cast< GaTentacleComponent* >( InComponent.get() );
			auto* Physics = Component->getParentEntity()->getComponentByType< GaPhysicsComponent >();

			// If we have a target, move closer.
			if( Component->TargetStructure_ != nullptr )
			{
				Component->TargetPosition_ = Component->TargetStructure_->getParentEntity()->getWorldPosition().xy();
			}

			const BcF32 MoveSpeed = Component->MoveSpeed_ * Tick;
			auto ComponentPos = Component->getParentEntity()->getWorldPosition().xy();
			auto TargetVec = Component->TargetPosition_ - ComponentPos;
			if( TargetVec.magnitude() < 32.0f )
			{
				if( Component->TargetStructure_ )
				{
					Game->destroyStructure( Component->TargetStructure_ );
					Component->targetHome();
				}
			}

			if( TargetVec.magnitude() > MoveSpeed )
			{
				TargetVec = TargetVec.normal() * MoveSpeed;
			}

			ComponentPos += TargetVec;
			Component->getParentEntity()->setLocalPosition( MaVec3d( ComponentPos, 0.0f ) );
			
			// Set first point mass to world position. Should be a soft constraint to make it move smoothly.
			Physics->setPointMassPosition( 0, Component->getParentEntity()->getWorldPosition().xy() );
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
	ReField* Fields[] = 
	{
		new ReField( "MoveSpeed_", &GaTentacleComponent::MoveSpeed_, bcRFF_IMPORTER ),
		new ReField( "HeadDamping_", &GaTentacleComponent::HeadDamping_, bcRFF_IMPORTER ),
		new ReField( "HeadConstraintRigidity_", &GaTentacleComponent::HeadConstraintRigidity_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< GaTentacleComponent, Super >( Fields )
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
	Constraints.emplace_back( GaPhysicsConstraint( 0, 1, 0.0f, HeadConstraintRigidity_ ) );

	PointMasses.emplace_back( GaPhysicsPointMass( MaVec2d( 0.0f, 0.0f ), HeadDamping_, 1.0f / 1.0f ) );
	Constraints.emplace_back( GaPhysicsConstraint( 1, 2, -1.0f, 1.0f ) );
	Constraints.emplace_back( GaPhysicsConstraint( 1, 3, -1.0f, 1.0f ) );
	size_t PointOffset = 2;
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
	//PointMasses[ PointMasses.size() - 2 ].InvMass_ = 0.0f;
	PointMasses[ PointMasses.size() - 1 ].InvMass_ = 0.0f;
	//PointMasses[ PointMasses.size() - 2 ].DampingFactor_ = 1.0f;
	PointMasses[ PointMasses.size() - 1 ].DampingFactor_= 1.0f;

	auto Physics = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	BcAssert( Physics );
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

	// Head point for moving it round.
	PointMasses.emplace_back( GaPhysicsPointMass( MaVec2d( 0.0f, 0.0f ), 1.0f, 0.0f ) );
	Constraints.emplace_back( GaPhysicsConstraint( 0, 1, 0.0f, HeadConstraintRigidity_ ) );

	// Head point used to nagivate.
	size_t PointOffset = 1;
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
	Constraints.emplace_back( GaPhysicsConstraint( 0, 1, 0.0f, HeadConstraintRigidity_ ) );

	PointMasses.emplace_back( GaPhysicsPointMass( MaVec2d( 0.0f, 0.0f ), HeadDamping_, 1.0f / 1.0f ) );
	Constraints.emplace_back( GaPhysicsConstraint( 1, 2, -1.0f, 1.0f ) );
	size_t PointOffset = 2;
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
// addPhysicsNoise
void GaTentacleComponent::addPhysicsNoise()
{
	// Add some tiny offsets to all the point masses.
	// This is to force there to be no cases where the structure can't
	// move to the side. Should solve any folding in errors upon initial spawning
	// and such.
	const BcF32 TinyMultiplier = 0.001f;
	const BcF32 Increment = 0.0001f;
	BcF32 Counter = 0.0f;
	auto Physics = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	auto NoofPointMasses = Physics->getNoofPointMasses();
	for( size_t Idx = 0; Idx < NoofPointMasses; ++Idx )
	{
		MaVec2d Offset( BcCos( Counter ), BcSin( Counter ) * TinyMultiplier );
		Physics->setPointMassPosition( Idx, Physics->getPointMassPosition( Idx ) + Offset );
		Counter += Increment;
	}
}

//////////////////////////////////////////////////////////////////////////
// targetStructure
void GaTentacleComponent::targetStructure()
{
	auto ComponentPos = getParentEntity()->getWorldPosition().xy();
	GaStructureComponent* NearestStructure = nullptr;
	auto ShortestDistance = std::numeric_limits< BcF32 >::max();
	const auto& Structures = Game_->getStructures();
	for( auto Structure : Structures )
	{
		auto TargetPos = Structure->getParentEntity()->getWorldPosition().xy();
		auto Distance = ( TargetPos - ComponentPos ).magnitude();
		if( Distance < ShortestDistance )
		{
			ShortestDistance = Distance;
			NearestStructure = Structure;
		}
	}

	if( NearestStructure != nullptr )
	{
		TargetStructure_ = NearestStructure;
		TargetStructure_->addNotifier( this );
	}
}

//////////////////////////////////////////////////////////////////////////
// targetHome
void GaTentacleComponent::targetHome()
{
	// Get last pos.
	auto Physics = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	BcAssert( Physics );
	
	TargetPosition_ = Physics->getPointMassPosition( Physics->getNoofPointMasses() - 1 ) + MaVec2d( 256.0f, 0.0f );

	if( TargetStructure_ != nullptr )
	{
		TargetStructure_->removeNotifier( this );
	}
	TargetStructure_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getTargetStructure
GaStructureComponent* GaTentacleComponent::getTargetStructure() const
{
	return TargetStructure_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaTentacleComponent::onAttach( ScnEntityWeakRef Parent )
{
	Game_ = getComponentAnyParentByType< GaGameComponent >();
	BcAssert( Game_ );

	Game_->getParentEntity()->subscribe( gaEVT_GAME_BEGIN_BUILD_PHASE, this, 
		[ this ]( EvtID, const EvtBaseEvent & Event )
		{
			targetHome();
			return evtRET_PASS;
		} );
	
	Game_->getParentEntity()->subscribe( gaEVT_GAME_BEGIN_DEFEND_PHASE, this,
		[ this ]( EvtID, const EvtBaseEvent & Event )
		{
			targetStructure();
			return evtRET_PASS;
		} );

	getParentEntity()->subscribe( gaEVT_PROJECTILE_HIT, this, 
		[ this ]( EvtID, const EvtBaseEvent & Event )
		{
			targetHome();
			return evtRET_PASS;
		} );

	setupComplexTopology( getParentEntity()->getWorldPosition().xy(), 32.0f, 32.0f, 20 );
	addPhysicsNoise();
	targetHome();

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaTentacleComponent::onDetach( ScnEntityWeakRef Parent )
{
	if( Game_ )
	{
		if( Game_->getParentEntity() )
		{
			Game_->getParentEntity()->unsubscribeAll( this );
		}
	}

	if( TargetStructure_ )
	{
		TargetStructure_->removeNotifier( this );
	}

	getParentEntity()->unsubscribeAll( this );
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onObjectDeleted
void GaTentacleComponent::onObjectDeleted( class ReObject* Object )
{
	if( Object == TargetStructure_ )
	{
		TargetStructure_ = nullptr;
	}
}
