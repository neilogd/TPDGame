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
		InvMass_( 0.0f )
	{}

	GaPhysicsPointMass( MaVec2d Position, BcF32 DampingFactor, BcF32 InvMass ):
		PrevPosition_( Position ),
		CurrPosition_( Position ),
		Acceleration_( 0.0f, 0.0f ),
		DampingFactor_( DampingFactor ),
		InvMass_( InvMass )
	{}

	MaVec2d PrevPosition_;
	MaVec2d CurrPosition_;
	MaVec2d Acceleration_;
	BcF32 DampingFactor_;
	BcF32 InvMass_;
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

	GaPhysicsConstraint( size_t IdxA, size_t IdxB, BcF32 Length ):
		IdxA_( IdxA ),
		IdxB_( IdxB ),
		Length_( Length )
	{}

	size_t IdxA_;
	size_t IdxB_;
	BcF32 Length_;
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

private:
	
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

	void setPointMasses( std::vector< GaPhysicsPointMass >&& PointMasses );
	void setConstraints( std::vector< GaPhysicsConstraint >&& Constraints );

private:
	friend class GaPhysicsProcessor;

	std::vector< GaPhysicsPointMass > PointMasses_;
	std::vector< GaPhysicsConstraint > Constraints_;
	
};