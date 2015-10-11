#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
	

//////////////////////////////////////////////////////////////////////////
// GaPhysicsPointMass
struct GaPhysicsPointMass
{
	GaPhysicsPointMass():
		PrevPosition_( 0.0f, 0.0f ),
		CurrPosition_( 0.0f, 0.0f ),
		Acceleration_( 0.0f, 0.0f ),
		DampingFactor_( 0.0f ),
		InvMass_( 0.0f ),
		MaxVelocity_( 0.0f )
	{}

	GaPhysicsPointMass( MaVec2d Position, BcF32 DampingFactor, BcF32 InvMass, BcF32 MaxVelocity = 0.0f ):
		PrevPosition_( Position ),
		CurrPosition_( Position ),
		Acceleration_( 0.0f, 0.0f ),
		DampingFactor_( DampingFactor ),
		InvMass_( InvMass ),
		MaxVelocity_( MaxVelocity )
	{}

	MaVec2d PrevPosition_;
	MaVec2d CurrPosition_;
	MaVec2d Acceleration_;
	BcF32 DampingFactor_;
	BcF32 InvMass_;
	BcF32 MaxVelocity_;
};

//////////////////////////////////////////////////////////////////////////
// GaPhysicsConstraint
struct GaPhysicsConstraint
{
	GaPhysicsConstraint():
		IdxA_( BcErrorCode ),
		IdxB_( BcErrorCode ),
		Length_( 0.0f )
	{}

	GaPhysicsConstraint( size_t IdxA, size_t IdxB, BcF32 Length, BcF32 Rigidity ):
		IdxA_( IdxA ),
		IdxB_( IdxB ),
		Length_( Length ),
		Rigidity_( Rigidity )
	{}

	size_t IdxA_;
	size_t IdxB_;
	BcF32 Length_;
	BcF32 Rigidity_;
};

//////////////////////////////////////////////////////////////////////////
// GaPhysicsProcessor
class GaPhysicsProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaPhysicsProcessor, ScnComponentProcessor );

	GaPhysicsProcessor();
	virtual ~GaPhysicsProcessor();

	void initialise() override;
	void shutdown() override;

	void updateSimulations( const ScnComponentList& Components );
	void debugDraw( const ScnComponentList& Components );

	BcF32 getTickRate() const { return TickRate_; }

private:
	BcF32 TickAccumulator_ = 0.0f;

	BcF32 TickRate_ = 1.0f / 120.0f;
	BcU32 Iterations_ = 1;
	BcF64 TimeTaken_ = 0.0f;
	
};

//////////////////////////////////////////////////////////////////////////
// GaPhysicsComponent
class GaPhysicsComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaPhysicsComponent, ScnComponent );

	GaPhysicsComponent();
	virtual ~GaPhysicsComponent();

	void setup( std::vector< GaPhysicsPointMass >&& PointMasses, std::vector< GaPhysicsConstraint >&& Constraints );

	const MaVec2d& getPointMassPosition( size_t Idx ) const { return PointMasses_[ Idx ].CurrPosition_; }
	void setPointMassPosition( size_t Idx, const MaVec2d& Position ) { PointMasses_[ Idx ].CurrPosition_ = Position; }

	void setPointMassAcceleration( size_t Idx, const MaVec2d& Acceleration ) { PointMasses_[ Idx ].Acceleration_ = Acceleration; }

	size_t getNoofPointMasses() const { return PointMasses_.size(); }
	size_t getNoofConstraints() const { return Constraints_.size(); }

private:
	friend class GaPhysicsProcessor;

	std::vector< GaPhysicsPointMass > PointMasses_;
	std::vector< GaPhysicsConstraint > Constraints_;
};