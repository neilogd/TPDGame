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
		} ){
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
// setupHotspots
void GaPhysicsProcessor::updateSimulations( const ScnComponentList& Components )
{
	const BcF32 Tick = 1.0f / 240.0f;
	const BcF32 TickSquared = Tick * Tick;

	TickAccumulator_ += SysKernel::pImpl()->getFrameTime();
	while( TickAccumulator_ > Tick )
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
				const MaVec2d Velocity = PointMass.CurrPosition_ - PointMass.PrevPosition_;
				PointMass.PrevPosition_= PointMass.CurrPosition_;
				PointMass.CurrPosition_ += Velocity * ( 1.0f - PointMass.DampingFactor_ ) + PointMass.Acceleration_ * TickSquared;
			}

			// Update constraints.
			for( size_t Idx = 0; Idx < Component->ConstraintIterations_; ++Idx )
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

//////////////////////////////////////////////////////////////////////////
// debugDraw
void GaPhysicsProcessor::debugDraw( const ScnComponentList& Components )
{			
	ImGui::SetNextWindowPos( MaVec2d( 0.0f, 0.0f ), ImGuiSetCond_Always );
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

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaPhysicsComponent );

void GaPhysicsComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ConstraintIterations_", &GaPhysicsComponent::ConstraintIterations_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< GaPhysicsComponent, Super >( Fields )
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
