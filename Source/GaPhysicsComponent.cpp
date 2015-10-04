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
	const BcF32 Tick = SysKernel::pImpl()->getFrameTime();
	const BcF32 TickSquared = Tick * Tick;

	for( auto InComponent : Components )
	{
		BcAssert( InComponent->isTypeOf< GaPhysicsComponent >() );
		auto* Component = static_cast< GaPhysicsComponent* >( InComponent.get() );

		// Update point masses.
		for(auto& PointMass : Component->PointMasses_)
		{
			BcAssert( PointMass.DampingFactor_ >= 0.0f && PointMass.DampingFactor_ <= 1.0f );
			BcAssert( PointMass.InvMass_ >= 0.0f );

			const MaVec2d Velocity = PointMass.CurrPosition_ - PointMass.PrevPosition_;
			PointMass.PrevPosition_= PointMass.CurrPosition_;
			PointMass.CurrPosition_ += Velocity * ( 1.0f - PointMass.DampingFactor_ ) + PointMass.Acceleration_ * TickSquared;
		}

		// Update constraints.
		for(auto& Constraint : Component->Constraints_)
		{
			BcAssert( Constraint.IdxA_ < Component->PointMasses_.size() );
			BcAssert( Constraint.IdxB_ < Component->PointMasses_.size() );
			auto& PointMassA = Component->PointMasses_[ Constraint.IdxA_ ];
			auto& PointMassB = Component->PointMasses_[ Constraint.IdxB_ ];

			const MaVec2d Delta = PointMassA.CurrPosition_ - PointMassB.PrevPosition_;
			const BcF32 Length = Delta.magnitude();
			const MaVec2d Offset = Delta * ( Length - Constraint.Length_ );
			const BcF32 TotalInvMass = PointMassA.InvMass_ + PointMassB.InvMass_;
			if( TotalInvMass > 0.0f )
			{
				const BcF32 InfluenceA = PointMassA.InvMass_ / TotalInvMass;
				const BcF32 InfluenceB = PointMassB.InvMass_ / TotalInvMass;
				PointMassA.CurrPosition_ += Offset * InfluenceA;
				PointMassB.CurrPosition_ += Offset * InfluenceB;
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
				BcAssert( PointMass.DampingFactor_ >= 0.0f && PointMass.DampingFactor_ <= 1.0f );
				BcAssert( PointMass.InvMass_ >= 0.0f );
				DrawList->AddCircle( PointMass.CurrPosition_, 4.0f, 0xffffffff );
			}

			// Draw constraints.
			for(auto& Constraint : Component->Constraints_)
			{
				BcAssert( Constraint.IdxA_ < Component->PointMasses_.size() );
				BcAssert( Constraint.IdxB_ < Component->PointMasses_.size() );
				auto& PointMassA = Component->PointMasses_[ Constraint.IdxA_ ];
				auto& PointMassB = Component->PointMasses_[ Constraint.IdxB_ ];
				DrawList->AddLine( PointMassA.CurrPosition_, PointMassB.CurrPosition_, 0xffffffff, 2.0f );
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
#if 0
	ReField* Fields[] = 
	{
		new ReField( "ID_", &GaPhysicsComponent::ID_, bcRFF_IMPORTER ),
		new ReField( "Position_", &GaPhysicsComponent::Position_, bcRFF_IMPORTER ),
		new ReField( "Size_", &GaPhysicsComponent::Size_, bcRFF_IMPORTER ),
	};
#endif

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
// setPointMasses
void GaPhysicsComponent::setPointMasses( std::vector< GaPhysicsPointMass >&& PointMasses )
{
	PointMasses_ = std::move( PointMasses );
}

//////////////////////////////////////////////////////////////////////////
// setConstraints
void GaPhysicsComponent::setConstraints( std::vector< GaPhysicsConstraint >&& Constraints )
{
	Constraints_ = std::move( Constraints );
}
