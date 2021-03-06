#include "GaPhysicsComponent.h"

#include "System/SysKernel.h"

#include "System/Debug/DsCore.h"
#include "System/Debug/DsImGui.h"
#include "System/Os/OsCore.h"

#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaPhysicsProcessor );

void GaPhysicsProcessor::StaticRegisterClass()
{
	ReRegisterClass< GaPhysicsProcessor, Super >();
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaPhysicsProcessor::GaPhysicsProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Update simulations",
				ScnComponentPriority::PHYSICS_WORLD_SIMULATE,
				std::bind(  &GaPhysicsProcessor::updateSimulations, this, std::placeholders::_1 ) ),
			ScnComponentProcessFuncEntry(
				"Update simulations",
				ScnComponentPriority::VIEW_RENDER - 1,
				std::bind(  &GaPhysicsProcessor::debugDraw, this, std::placeholders::_1 ) )
		} )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
GaPhysicsProcessor::~GaPhysicsProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaPhysicsProcessor::initialise()
{

}

//////////////////////////////////////////////////////////////////////////
// shutdown
void GaPhysicsProcessor::shutdown()
{
	
}

//////////////////////////////////////////////////////////////////////////
// updateSimulations
void GaPhysicsProcessor::updateSimulations( const ScnComponentList& Components )
{
	BcTimer Timer;
	Timer.mark();
	const BcF32 Tick = TickRate_;
	const BcF32 TickSquared = Tick * Tick;

	if( Active_ )
	{
		TickAccumulator_ += SysKernel::pImpl()->getFrameTime();
		while( TickAccumulator_ > Tick && Timer.time() < 0.01 )
		{
			TickAccumulator_ -= Tick;
			for( auto InComponent : Components )
			{
				BcAssert( InComponent->isTypeOf< GaPhysicsComponent >() );
				auto* Component = static_cast< GaPhysicsComponent* >( InComponent.get() );

				// Update point masses.
				for( auto& PointMass : Component->PointMasses_ )
				{
					BcAssert( PointMass.InvMass_ >= 0.0f );
					MaVec2d Velocity = PointMass.CurrPosition_ - PointMass.PrevPosition_;
					Velocity = Velocity * ( 1.0f - PointMass.DampingFactor_ ) + ( PointMass.Acceleration_ * TickSquared );
					auto MaxVelocityFrame = PointMass.MaxVelocity_ * Tick;
					if( PointMass.MaxVelocity_ > 0.0f )
					{
						if( Velocity.magnitudeSquared() > ( MaxVelocityFrame * MaxVelocityFrame ) )
						{
							Velocity = Velocity.normal() * MaxVelocityFrame;
						}
					}

					PointMass.PrevPosition_= PointMass.CurrPosition_;
					PointMass.CurrPosition_ += Velocity;
				}

				// Update constraints.
				for( size_t Idx = 0; Idx < Iterations_; ++Idx )
				{
					for( auto& Constraint : Component->Constraints_ )
					{
						auto& PointMassA = Component->PointMasses_[ Constraint.IdxA_ ];
						auto& PointMassB = Component->PointMasses_[ Constraint.IdxB_ ];
						const MaVec2d Delta = PointMassB.CurrPosition_ - PointMassA.CurrPosition_;
						const BcF32 Length = Delta.magnitude();
						const MaVec2d Offset = Delta.normal() * ( Length - Constraint.Length_ );
						const BcF32 TotalInvMass = PointMassA.InvMass_ + PointMassB.InvMass_;
						if( TotalInvMass > 0.0f )
						{
							const BcF32 InfluenceA = PointMassA.InvMass_ / TotalInvMass;
							const BcF32 InfluenceB = PointMassB.InvMass_ / TotalInvMass;
							PointMassA.CurrPosition_ += Offset * InfluenceA * Constraint.Rigidity_;
							PointMassB.CurrPosition_ -= Offset * InfluenceB * Constraint.Rigidity_;
						}
					}
				}
			}
		}
	}
	TimeTaken_ = Timer.time();

#if !PSY_PRODUCTION
	if ( ImGui::Begin( "Game Debug" ) )
	{
		ImGui::Checkbox( "Physics Active", &Active_ );
		ImGui::Checkbox( "Physics Debug Draw", &DebugDraw_ );

		size_t NoofPointMasses = 0;
		size_t NoofConstraints = 0;
		for( auto InComponent : Components )
		{
			BcAssert( InComponent->isTypeOf< GaPhysicsComponent >() );
			auto* Component = static_cast< GaPhysicsComponent* >( InComponent.get() );
			NoofPointMasses += Component->PointMasses_.size();
			NoofConstraints += Component->Constraints_.size();
		}

		BcF32 InvTick = 1.0f / TickRate_;
		if( ImGui::InputFloat( "Physics Tick rate (hz)", &InvTick ) )
		{
			if( InvTick > 0.0f && InvTick < 480.0f )
			{
				TickRate_ = 1.0f / InvTick;
			}
		}

		int Iterations = static_cast< int >( Iterations_ );
		if( ImGui::InputInt( "Physics Iterations", &Iterations ) )
		{
			if( Iterations >= 1 && Iterations <= 32 )
			{
				Iterations_ = static_cast< BcU32 >( Iterations );
			}
		}

		ImGui::Text( "Point mases: %u",  NoofPointMasses );
		ImGui::Text( "Constraints: %u",  NoofConstraints );
		ImGui::Text( "Time spent: %f ms",  TimeTaken_ * 1000.0f );
		ImGui::Separator();
	}
	ImGui::End();
#endif // !PSY_PRODUCTION
}

//////////////////////////////////////////////////////////////////////////
// debugDraw
void GaPhysicsProcessor::debugDraw( const ScnComponentList& Components )
{			
#if !PSY_PRODUCTION
	if( DebugDraw_ )
	{
		ImGui::SetNextWindowPos( MaVec2d( -64.0f, -64.0f ), ImGuiSetCond_Always );
		ImGui::SetNextWindowSize( MaVec2d( 0.0f, 0.0f ) );
		if( ImGui::Begin( "", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
		{
			auto DrawList = ImGui::GetWindowDrawList();
			DrawList->PushClipRectFullScreen();

			for( auto InComponent : Components )
			{		
				BcAssert( InComponent->isTypeOf< GaPhysicsComponent >() );
				auto* Component = static_cast< GaPhysicsComponent* >( InComponent.get() );
			
				// Draw point masses.
				for(auto& PointMass : Component->PointMasses_)
				{
					if( PointMass.InvMass_ > 0.0f )
					{
						DrawList->AddCircle( PointMass.CurrPosition_, 4.0f, 0xff00ff00 );
					}
					else
					{
						DrawList->AddCircle( PointMass.CurrPosition_, 4.0f, 0xff0000ff );
					}
				}

				// Draw constraints.
				for(auto& Constraint : Component->Constraints_)
				{
					RsColour Colour( 1.0f, 1.0f, 1.0f, Constraint.Rigidity_ );
					auto& PointMassA = Component->PointMasses_[ Constraint.IdxA_ ];
					auto& PointMassB = Component->PointMasses_[ Constraint.IdxB_ ];
					DrawList->AddLine( PointMassA.CurrPosition_, PointMassB.CurrPosition_, Colour.asABGR(), 2.0f );
				}
			}

			DrawList->PopClipRect();
			ImGui::End();
		}
	}
#endif // !PSY_PRODUCTION
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaPhysicsComponent );

void GaPhysicsComponent::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{
		new ReField( "ConstraintIterations_", &GaPhysicsComponent::ConstraintIterations_, bcRFF_IMPORTER ),
	};
	*/

	ReRegisterClass< GaPhysicsComponent, Super >()
		.addAttribute( new GaPhysicsProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaPhysicsComponent::GaPhysicsComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaPhysicsComponent::~GaPhysicsComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// setup
void GaPhysicsComponent::setup( std::vector< GaPhysicsPointMass >&& PointMasses, std::vector< GaPhysicsConstraint >&& Constraints )
{
	PointMasses_ = std::move( PointMasses );
	for(auto& PointMass : PointMasses_ )
	{
		BcAssert( PointMass.DampingFactor_ >= 0.0f && PointMass.DampingFactor_ <= 1.0f );
		BcAssert( PointMass.InvMass_ >= 0.0f );
	}

	Constraints_ = std::move( Constraints );
	for(auto& Constraint : Constraints_)
	{
		BcAssert( Constraint.IdxA_ < PointMasses_.size() );
		BcAssert( Constraint.IdxB_ < PointMasses_.size() );
		BcAssert( Constraint.Rigidity_ >= 0.0f && Constraint.Rigidity_ <= 1.0f );
		if( Constraint.Length_ < 0.0f )
		{
			Constraint.Length_ = ( PointMasses_[ Constraint.IdxA_ ].CurrPosition_ - PointMasses_[ Constraint.IdxB_ ].CurrPosition_ ).magnitude() * -( Constraint.Length_ );
		}
	}
}
