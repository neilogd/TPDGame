#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"

#include "System/Renderer/RsTypes.h"

#include "Base/BcRandom.h"

#include <map>

//////////////////////////////////////////////////////////////////////////
// GaParticleEmitterShape
enum class GaParticleEmitterShape
{
	/// Emit from a sphere.
	SPHERE
};

//////////////////////////////////////////////////////////////////////////
// GaParticleEmitterVelocityFunction
enum class GaParticleEmitterVelocityFunction
{
	/// Spawn particles relative to centre.
	CENTRE_RELATIVE,
	/// Spawn particles in a direction.
	DIRECTION
};

//////////////////////////////////////////////////////////////////////////
// GaParticleEffect
struct GaParticleEffect
{
public:
	REFLECTION_DECLARE_BASE( GaParticleEffect );

	struct Emitter
	{
		REFLECTION_DECLARE_BASIC_NOAUTOREG( Emitter );

		Emitter(){}

		/// Emission rate in particles/sec.
		BcF32 EmissionRate_ = 1.0f;
		/// Emit all particles at once?
		BcBool EmitBurst_ = BcFalse;

		/// Emitter shape.
		GaParticleEmitterShape EmitterShape_ = GaParticleEmitterShape::SPHERE;
		/// Min emitter position.
		MaVec3d MinEmitterPos_ = MaVec3d( 0.0f, 0.0f, 0.0f );
		/// Max emitter position.
		MaVec3d MaxEmitterPos_ = MaVec3d( 0.0f, 0.0f, 0.0f );

		/// Velocity function.
		GaParticleEmitterVelocityFunction VelocityFunction_ = GaParticleEmitterVelocityFunction::CENTRE_RELATIVE;
		/// Velocity direction.
		MaVec3d VelocityDirection_ = MaVec3d( 1.0f, 0.0f, 0.0f );
		/// Velocity angle range.
		BcF32 VelocityAngleRange_ = 0.0f;

		/// Min lifetime.
		BcF32 MinLifetime_ = 1.0f;
		/// Max lifetime.
		BcF32 MaxLifetime_ = 1.0f;
	
		/// Min scale.
		MaVec2d MinScale_ = MaVec2d( 1.0f, 1.0f );
		/// Max scale.
		MaVec2d MaxScale_ = MaVec2d( 1.0f, 1.0f );

		/// Rotation.
		BcF32 Rotation_ = 0.0f;
		/// Rotation range ( +/- from @a Rotation_)
		BcF32 RotationRange_ = 0.0f;
		/// Min rotation velocity.
		BcF32 MinRotationVelocity_ = 0.0f;
		/// Max rotation velocity.
		BcF32 MaxRotationVelocity_ = 0.0f;

		/// Min colour.
		RsColour MinColour_ = RsColour( 1.0f, 1.0f, 1.0f, 1.0f );
		/// Max colour.
		RsColour MaxColour_ = RsColour( 1.0f, 1.0f, 1.0f, 1.0f );

		/// Min texture index.
		BcU32 MinTextureIndex_ = 0;
		/// Max texture index.
		BcU32 MaxTextureIndex_ = 0;
	
		/// Timer for calculating emission.
		BcF32 EmissionTimer_ = 0.0f;
		/// Last emission position.
		MaVec3d LastEmissionPosition_ = MaVec3d( 0.0f, 0.0f, 0.0f );
	};

	GaParticleEffect(){}

	/// 
	std::vector< Emitter > Emitters_;
};

//////////////////////////////////////////////////////////////////////////
// GaParticleEmitterProcessor
class GaParticleEmitterProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaParticleEmitterProcessor, ScnComponentProcessor );

	GaParticleEmitterProcessor();
	virtual ~GaParticleEmitterProcessor();

	void initialise() override;
	void shutdown() override;

	void emitParticles( const ScnComponentList& Components );

private:	
	BcRandom RNG_;
};

//////////////////////////////////////////////////////////////////////////
// GaParticleEmitterComponent
class GaParticleEmitterComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaParticleEmitterComponent, ScnComponent );

	GaParticleEmitterComponent();
	virtual ~GaParticleEmitterComponent();

	/**
	 * Start effect.
	 */
	void startEffect( BcName Name );

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

private:
	friend class GaParticleEmitterProcessor;

	/// Effects we have.
	std::map< BcName, GaParticleEffect > Effects_;

	/// Effect to start immediartely.
	BcName StartingEffect_;
	
	/// Current effect.
	GaParticleEffect CurrentEffect_;

	/// Particle system.
	class ScnParticleSystemComponent* ParticleSystem_ = nullptr;

	/// Canvas.
	class ScnCanvasComponent* Canvas_ = nullptr;
};
