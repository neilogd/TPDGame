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

#include "Base/BcRandom.h"


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
	if( Components.size() > 0 )
	{
		// Grab game component.
		auto FirstComponent = Components[0];
		auto Game = static_cast< GaGameComponent* >( FirstComponent.get() )->getComponentAnyParentByType< GaGameComponent >();
		BcAssert( Game );
		const auto& Structures = Game->getStructures();
	
		// Update each tentacle.
		Timer_ += Tick;
		for( auto InComponent : Components )
		{
			BcAssert( InComponent->isTypeOf< GaTentacleComponent >() );
			auto* Component = static_cast< GaTentacleComponent* >( InComponent.get() );
			auto* Physics = Component->getParentEntity()->getComponentByType< GaPhysicsComponent >();

			// If we have a target, move closer.
			if( Component->TargetStructure_ != nullptr )
			{
				Component->TargetPosition_ = Component->TargetStructure_->getParentEntity()->getWorldPosition().xy();
				Component->TargetPosition_ -= MaVec2d( 0.0f, 32.0f );
			}

			const auto MoveSpeed = Component->CalculatedMoveSpeed_ * Tick;
			const auto TargetPosition = Component->TargetPosition_;

			auto ComponentPos = Component->SoftHeadPosition_;
			auto TargetVec = TargetPosition - ComponentPos;
			auto TargetDistance = TargetVec.magnitude();

			// Calculate sway based on prelim distance.
			const auto Timer = ( Timer_ * Component->TimerRandMult_ ) + Component->TimerRandOffset_;
			const auto MaxSway = 64.0f;
			const auto HeadSway = Component->TargetStructure_ ? MaxSway / 2.0f : MaxSway / 8.0f;
			const auto DistanceDivHeadSwayClamped = BcClamp( TargetDistance / HeadSway, 0.0f, 1.0f );
			const auto SmoothHeadSwayMult = BcSmoothStep( DistanceDivHeadSwayClamped );
			Component->HeadSwaySmoothed_ = ( Component->HeadSwaySmoothed_ * 0.99f ) + ( HeadSway * SmoothHeadSwayMult * 0.01f );
			const auto SwayOffset = MaVec2d( BcCos( Timer ), BcSin( Timer  ) ) * Component->HeadSwaySmoothed_;

			// Recalc target vec + distance to include sway.
			TargetVec = TargetPosition - ( ComponentPos + SwayOffset );
			TargetDistance = TargetVec.magnitude();

			/// Clamp move speed.
			if( TargetVec.magnitude() > MoveSpeed )
			{
				TargetVec = TargetVec.normal() * MoveSpeed;
			}

			// Do the attacky thing.
			// TODO: Make tentacle whack it hard.
			if( TargetDistance < 32.0f )
			{
				if( Component->TargetStructure_ )
				{
					Game->destroyStructure( Component->TargetStructure_ );
					Component->targetHome();
				}
			}

			ComponentPos += TargetVec;
			Component->SoftHeadPosition_ = ComponentPos;


			// Set first + last point mass to world position. Should be a soft constraint to make it move smoothly.

			const auto LastIdx = Physics->getNoofPointMasses() - 1;
			const auto HeadPosition = Component->SoftHeadPosition_ + SwayOffset;
			const auto DistanceMult = 1.0f;//( 800.0f / HeadPosition.y() );
			const auto TailPosition = Physics->getPointMassPosition( LastIdx );
			Physics->setPointMassPosition( 0, HeadPosition );
			Physics->setPointMassPosition( LastIdx, 
				MaVec2d( 
					Component->TailPosition_.x() + ( BcSin( Timer * 2.0f ) * MaxSway ), 
					HeadPosition.y() + Component->HeadTailDistance_ * DistanceMult ) );
				
			Component->getParentEntity()->setLocalPosition( MaVec3d( HeadPosition, 0.0f ) );

			// TODO: Set end points to a new location too. Move them at ~1/4-1/2 the rate to prevent the
			// tentacle bunching up so badly.
		}
	}
	else
	{
		Timer_  = 0.0f;
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
		new ReField( "MoveSpeedMultiplier_", &GaTentacleComponent::MoveSpeedMultiplier_, bcRFF_IMPORTER ),
		new ReField( "HeadDamping_", &GaTentacleComponent::HeadDamping_, bcRFF_IMPORTER ),
		new ReField( "HeadConstraintRigidity_", &GaTentacleComponent::HeadConstraintRigidity_, bcRFF_IMPORTER ),

		new ReField( "VerticalRigidity_", &GaTentacleComponent::VerticalRigidity_, bcRFF_IMPORTER ),
		new ReField( "HorizontalRigidity_", &GaTentacleComponent::HorizontalRigidity_, bcRFF_IMPORTER ),
		new ReField( "DiagonalRigidity_", &GaTentacleComponent::DiagonalRigidity_, bcRFF_IMPORTER ),
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

	const BcF32 Horizontal = Width;
	const BcF32 Vertical = SectionHeight;
	const BcF32 Diagonal = std::sqrtf( ( Horizontal * Horizontal ) + ( Vertical * Vertical ) );

	for( size_t Idx = 0; Idx < NoofSections; ++Idx )
	{
		PointMasses.push_back( GaPhysicsPointMass( MaVec2d( -HalfWidth, 0.0f ) + Offset, 0.1f, 1.0f / 1.0f ) );
		PointMasses.push_back( GaPhysicsPointMass( MaVec2d(  HalfWidth, 0.0f ) + Offset, 0.1f, 1.0f / 1.0f ) );

		Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 1, Horizontal, HorizontalRigidity_ ) );

		// Only advance half the distance.
		Offset += MaVec2d( 0.0f, Vertical * 0.9f );

		const BcF32 Rate = ( BcPIMUL2 / static_cast< BcF32 >( NoofSections ) );
		const BcF32 Mult = Horizontal * 4.0f;
		Offset.x( BcSin( static_cast< BcF32 >( Idx ) * Rate ) * Mult );

		// Link to next section.
		if( Idx < ( NoofSections - 1 ) )
		{
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 2, Vertical, VerticalRigidity_ ) );
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 1, PointOffset + 3, Vertical, VerticalRigidity_) );

			Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 1, PointOffset + 2, Diagonal, DiagonalRigidity_ ) );
			Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 3, Diagonal, DiagonalRigidity_ ) );
			PointOffset += 2;
		}
	}

	HeadTailDistance_ = Offset.y();


	// Add last point to move on the Y axis with the head.
	PointMasses.push_back( GaPhysicsPointMass( MaVec2d( 0.0f, 0.0f ) + Offset, 1.0f, 0.0f ) );

	Constraints.emplace_back( GaPhysicsConstraint( PointOffset, PointOffset + 2, Diagonal, 0.8f ) );
	Constraints.emplace_back( GaPhysicsConstraint( PointOffset + 1, PointOffset + 2, Diagonal, 0.8f ) );

	for( auto& PointMass : PointMasses )
	{
		PointMass.CurrPosition_ += RootPosition;
		PointMass.PrevPosition_ += RootPosition;
		PointMass.Acceleration_ = MaVec2d( 0.0f, 0.0f );
	}
	
	// Pin the end.
	const auto LastIdx = PointMasses.size() - 1;
	PointMasses[ LastIdx ].InvMass_ = 0.0f;
	PointMasses[ LastIdx ].DampingFactor_= 1.0f;


	SoftHeadPosition_ = RootPosition;
	TailPosition_ = PointMasses[ LastIdx ].CurrPosition_;

	auto Physics = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	BcAssert( Physics );
	Physics->setup( std::move( PointMasses ), std::move( Constraints ) );

	// Add noise,.
	//addPhysicsNoise();
}


//////////////////////////////////////////////////////////////////////////
// addPhysicsNoise
void GaTentacleComponent::addPhysicsNoise()
{
	// Add some tiny offsets to all the point masses.
	// This is to force there to be no cases where the structure can't
	// move to the side. Should solve any folding in errors upon initial spawning
	// and such.
	const BcF32 TinyMultiplier = 1.0f;
	const BcF32 Increment = 0.01f;
	BcF32 Counter = 0.0f;
	auto Physics = getParentEntity()->getComponentByType< GaPhysicsComponent >();
	auto NoofPointMasses = Physics->getNoofPointMasses();
	for( size_t Idx = 0; Idx < NoofPointMasses; ++Idx )
	{
		const auto Multiplier = 1.0f - Physics->getPointMass( Idx ).DampingFactor_;
		const MaVec2d Offset( BcCos( Counter ), BcSin( Counter ) * TinyMultiplier );
		Physics->setPointMassPosition( Idx, Physics->getPointMassPosition( Idx ) + ( Offset * Multiplier ) );
		Counter += Increment;
	}
}

//////////////////////////////////////////////////////////////////////////
// calculateLevelStats
void GaTentacleComponent::calculateLevelStats( BcU32 Level )
{
	BcF32 Levelf = static_cast< BcF32 >( Level );
	CalculatedMoveSpeed_ = MoveSpeed_ * ( 1.0f + Levelf * MoveSpeedMultiplier_ );
}

//////////////////////////////////////////////////////////////////////////
// targetStructure
void GaTentacleComponent::targetStructure()
{
	auto ComponentPos = getParentEntity()->getWorldPosition().xy();
	GaStructureComponent* NearestStructure = nullptr;
	auto ShortestDistance = std::numeric_limits< BcF32 >::max();
	const auto& Structures = Game_->getStructures();

	// Potato first.
	for( auto Structure : Structures )
	{
		if( Structure->getStructureType() == GaStructureType::POTATO )
		{
			auto TargetPos = Structure->getParentEntity()->getWorldPosition().xy();
			auto Distance = ( TargetPos - ComponentPos ).magnitude();
			if( Distance < ShortestDistance )
			{
				ShortestDistance = Distance;
				NearestStructure = Structure;
			}
		}
	}

	// Now other structures.
	if( NearestStructure == nullptr )
	{
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
	
	TargetPosition_ = MaVec2d( TailPosition_.x(), 800.0f );

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
	
	Game_->getParentEntity()->subscribe( gaEVT_GAME_BEGIN_BUILD_PHASE, this,
		[ this ]( EvtID, const EvtBaseEvent & Event )
		{
			calculateLevelStats( Event.get< GaGameEvent >().Level_ );
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

	setupComplexTopology( getParentEntity()->getWorldPosition().xy(), 32.0f, 32.0f, 30 );
	targetHome();
	calculateLevelStats( 1 );

	TimerRandMult_ = BcRandom::Global.randRange( 0.8f, 1.2f );
	TimerRandOffset_ = BcRandom::Global.randRange( 0.0f, BcPIMUL2 );

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
		targetHome();
	}
}
