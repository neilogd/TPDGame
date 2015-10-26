#include "GaParticleEmitter.h"

#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/Rendering/ScnParticleSystemComponent.h"
#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_BASE( GaParticleEffect );
REFLECTION_DEFINE_BASIC( GaParticleEffect::Emitter );

void GaParticleEffect::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Emitters_", &GaParticleEffect::Emitters_ ),
	};
	
	auto& Class = ReRegisterClass< GaParticleEffect >( Fields );
	BcUnusedVar( Class );

	{
		ReField* Fields[] = 
		{
			new ReField( "EmissionRate_", &Emitter::EmissionRate_, bcRFF_IMPORTER ),
			new ReField( "EmitBurst_", &Emitter::EmitBurst_, bcRFF_IMPORTER ),
			new ReField( "EmitterShape_", &Emitter::EmitterShape_, bcRFF_IMPORTER ),
			new ReField( "MinEmitterPos_", &Emitter::MinEmitterPos_, bcRFF_IMPORTER ),
			new ReField( "MaxEmitterPos_", &Emitter::MaxEmitterPos_, bcRFF_IMPORTER ),
			new ReField( "VelocityFunction_", &Emitter::VelocityFunction_, bcRFF_IMPORTER ),
			new ReField( "VelocityDirection_", &Emitter::VelocityDirection_, bcRFF_IMPORTER ),
			new ReField( "VelocityAngleRange_", &Emitter::VelocityAngleRange_, bcRFF_IMPORTER ),
			new ReField( "VelocityMultiplier_", &Emitter::VelocityMultiplier_, bcRFF_IMPORTER ),
			new ReField( "MinLifetime_", &Emitter::MinLifetime_, bcRFF_IMPORTER ),
			new ReField( "MaxLifetime_", &Emitter::MaxLifetime_, bcRFF_IMPORTER ),
			new ReField( "MinScale_", &Emitter::MinScale_, bcRFF_IMPORTER ),
			new ReField( "MaxScale_", &Emitter::MaxScale_, bcRFF_IMPORTER ),
			new ReField( "Rotation_", &Emitter::Rotation_, bcRFF_IMPORTER ),
			new ReField( "RotationRange_", &Emitter::RotationRange_, bcRFF_IMPORTER ),
			new ReField( "MinRotationVelocity_", &Emitter::MinRotationVelocity_, bcRFF_IMPORTER ),
			new ReField( "MaxRotationVelocity_", &Emitter::MaxRotationVelocity_, bcRFF_IMPORTER ),
			new ReField( "MinColour_", &Emitter::MinColour_, bcRFF_IMPORTER ),
			new ReField( "MaxColour_", &Emitter::MaxColour_, bcRFF_IMPORTER ),
			new ReField( "MinTextureIndex_", &Emitter::MinTextureIndex_, bcRFF_IMPORTER ),
			new ReField( "MaxTextureIndex_", &Emitter::MaxTextureIndex_, bcRFF_IMPORTER ),

			new ReField( "EmissionTimer_", &Emitter::EmissionTimer_, bcRFF_TRANSIENT ),
		};
	
		auto& Class = ReRegisterClass< Emitter >( Fields );
		BcUnusedVar( Class );
	}

	{
		ReEnumConstant* EnumConstants[] = 
		{
			new ReEnumConstant( "SPHERE", (BcU32)GaParticleEmitterShape::SPHERE ),
		};
		
		ReRegisterEnum< GaParticleEmitterShape >( EnumConstants );
	}


	{
		ReEnumConstant* EnumConstants[] = 
		{
			new ReEnumConstant( "CENTRE_RELATIVE", (BcU32)GaParticleEmitterVelocityFunction::CENTRE_RELATIVE ),
			new ReEnumConstant( "DIRECTION", (BcU32)GaParticleEmitterVelocityFunction::DIRECTION ),
		};

		ReRegisterEnum< GaParticleEmitterVelocityFunction >( EnumConstants );
	}
}


//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaParticleEmitterProcessor );

void GaParticleEmitterProcessor::StaticRegisterClass()
{
	ReRegisterClass< GaParticleEmitterProcessor, Super >();
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaParticleEmitterProcessor::GaParticleEmitterProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Emit particles",
				ScnComponentPriority::DEFAULT_POST_UPDATE,
				std::bind(  &GaParticleEmitterProcessor::emitParticles, this, std::placeholders::_1 ) ),
		} )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
GaParticleEmitterProcessor::~GaParticleEmitterProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaParticleEmitterProcessor::initialise()
{

}

//////////////////////////////////////////////////////////////////////////
// shutdown
void GaParticleEmitterProcessor::shutdown()
{
	
}

//////////////////////////////////////////////////////////////////////////
// updateSimulations
void GaParticleEmitterProcessor::emitParticles( const ScnComponentList& Components )
{
	BcF32 Tick = SysKernel::pImpl()->getFrameTime();
	for( auto InComponent : Components )
	{
		BcAssert( InComponent->isTypeOf< GaParticleEmitterComponent >() );
		auto* Component = static_cast< GaParticleEmitterComponent* >( InComponent.get() );

		MaVec3d BasePosition = Component->getParentEntity()->getWorldPosition();

		for( auto& Emitter : Component->CurrentEffect_.Emitters_ )
		{
			// NOTE: Only *needs* to be done once.
			Component->ParticleSystem_->setTransform( Component->Canvas_->getMatrix() );
			Emitter.EmissionTimer_ += Tick * Emitter.EmissionRate_;
			if( Emitter.EmissionTimer_ > 1.0f )
			{
				const BcF32 EmissionIncr = 1.0f / Emitter.EmissionTimer_;
				const BcU32 NoofEmitters = static_cast< BcU32 >( Emitter.EmissionTimer_ );
				Emitter.EmissionTimer_ -= static_cast< BcF32 >( NoofEmitters );

				const MaVec3d StartPosition = Emitter.LastEmissionPosition_;
				const MaVec3d EndPosition = BasePosition;
				Emitter.LastEmissionPosition_ = BasePosition;
				BcF32 EmissionDelta = 0.0f;
				
				ScnParticle* Particle = nullptr;
				for( BcU32 Idx = 0; Idx < NoofEmitters; ++Idx )
				{
					if( Component->ParticleSystem_->allocParticle( Particle ) )
					{
						MaVec3d CentrePosition;
						CentrePosition.lerp( StartPosition, EndPosition, EmissionDelta );
						switch( Emitter.EmitterShape_ )
						{
						case GaParticleEmitterShape::SPHERE:
							{
								MaVec3d Position(
									RNG_.randRealRange( -1.0f, 1.0f ),
									RNG_.randRealRange( -1.0f, 1.0f ),
									RNG_.randRealRange( -1.0f, 1.0f ) );
								MaVec3d Radius(
									RNG_.randRealRange( 
										Emitter.MinEmitterPos_.x(),
										Emitter.MaxEmitterPos_.x() ),
									RNG_.randRealRange( 
										Emitter.MinEmitterPos_.y(),
										Emitter.MaxEmitterPos_.y() ),
									RNG_.randRealRange( 
										Emitter.MinEmitterPos_.z(),
										Emitter.MaxEmitterPos_.z() ) );
								Particle->Position_ = CentrePosition + Position.normal() * Radius;
							}
							break;
						}

						switch( Emitter.VelocityFunction_ )
						{
						case GaParticleEmitterVelocityFunction::CENTRE_RELATIVE:
							{
								MaVec3d Velocity( Particle->Position_ - CentrePosition );
								MaVec3d RandomVelocity = MaVec3d(
										RNG_.randRealRange( -1.0f, 1.0f ),
										RNG_.randRealRange( -1.0f, 1.0f ),
										RNG_.randRealRange( -1.0f, 1.0f ) ).normal();
								Particle->Velocity_ = Velocity + ( RandomVelocity * Emitter.VelocityMultiplier_ );
							}
							break;
						case GaParticleEmitterVelocityFunction::DIRECTION:
							{
								MaVec3d RandomVelocity = MaVec3d(
										RNG_.randRealRange( -1.0f, 1.0f ),
										RNG_.randRealRange( -1.0f, 1.0f ),
										RNG_.randRealRange( -1.0f, 1.0f ) ).normal();
								Particle->Velocity_ = Emitter.VelocityDirection_ + ( RandomVelocity * Emitter.VelocityMultiplier_ );
							}
							break;
						}
						//
						Particle->MinScale_ = Emitter.MinScale_;
						Particle->MaxScale_ = Emitter.MaxScale_;
						Particle->Scale_ = Particle->MinScale_;
						Particle->MinColour_ = Emitter.MinColour_;
						Particle->MaxColour_ = Emitter.MaxColour_;
						Particle->Colour_ = Emitter.MaxColour_;
						Particle->Rotation_ = Emitter.Rotation_ + 
							RNG_.randRealRange( -Emitter.RotationRange_, Emitter.RotationRange_ );
						Particle->RotationMultiplier_ = 
							RNG_.randRealRange( Emitter.MinRotationVelocity_, Emitter.MaxRotationVelocity_ );
						Particle->MaxTime_ = RNG_.randRealRange( Emitter.MinLifetime_, Emitter.MaxLifetime_ );
						Particle->TextureIndex_ = RNG_.randRange( Emitter.MinTextureIndex_, Emitter.MaxTextureIndex_ );
						Particle->CurrentTime_ = 0.0f;
						Particle->Alive_ = BcTrue;
					}
					EmissionDelta += EmissionIncr;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaParticleEmitterComponent );

void GaParticleEmitterComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Effects_", &GaParticleEmitterComponent::Effects_, bcRFF_IMPORTER ),
		new ReField( "StartingEffect_", &GaParticleEmitterComponent::StartingEffect_, bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< GaParticleEmitterComponent, Super >( Fields )
		.addAttribute( new GaParticleEmitterProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaParticleEmitterComponent::GaParticleEmitterComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaParticleEmitterComponent::~GaParticleEmitterComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaParticleEmitterComponent::onAttach( ScnEntityWeakRef Parent )
{
	ParticleSystem_ = Parent->getComponentAnyParentByType< ScnParticleSystemComponent >();
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >();

	startEffect( StartingEffect_ );
	
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaParticleEmitterComponent::onDetach( ScnEntityWeakRef Parent )
{
	ParticleSystem_ = nullptr;
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// startEffect
void GaParticleEmitterComponent::startEffect( BcName Name )
{
	// Copy in all new emitters.
	auto It = Effects_.find( Name );
	if( It != Effects_.end() )
	{
		CurrentEffect_ = It->second;
	}
}
