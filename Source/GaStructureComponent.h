#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"

//////////////////////////////////////////////////////////////////////////
// GaStructureType
enum class GaStructureType
{
	TURRET,
	RESOURCE,
	POTATO,
	MINE
};

//////////////////////////////////////////////////////////////////////////
// GaStructureProcessor
class GaStructureProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaStructureProcessor, ScnComponentProcessor );

	GaStructureProcessor();
	virtual ~GaStructureProcessor();

	void initialise() override;
	void shutdown() override;

	void update( const ScnComponentList& Components );

private:
	
};

//////////////////////////////////////////////////////////////////////////
// GaStructureComponent
class GaStructureComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaStructureComponent, ScnComponent );

	GaStructureComponent();
	virtual ~GaStructureComponent();

	void setupTopology();
	void setupHotspot();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	/**
	 * Sets whether the structure is active or not.
	 * @param Active If true, will be active in game, if false then
	 * it will merely render.
	 */
	void setActive( BcBool Active );

	/**
	 * Increment level.
	 * @return Level the structure has been incremented to.
	 */
	BcU32 incLevel();

	void update( BcF32 Tick );



	void setID( BcU32 ID ) { ID_ = ID; }
	BcU32 getID() const { return ID_; }

	BcS64 getBuildCost() const { return BuildCost_; }
	BcS64 getUpgradeCost() const { return CalculatedUpgradeCost_; }

private:
	friend class GaStructureProcessor;

	BcU32 Level_ = 0;
	BcBool Floating_ = BcFalse;
	BcBool Active_ = BcFalse;

	BcS64 BuildCost_ = 0;
	BcS64 BaseUpgradeCost_ = 0;
	BcS64 LevelUpgradeCost_ = 0;
	BcS64 CalculatedUpgradeCost_ = 0;

	GaStructureType StructureType_ = GaStructureType::TURRET;
	ScnEntity* TemplateProjectile_ = nullptr;
	BcF32 FireRate_ = 1.0f;
	BcU32 ID_ = BcErrorCode;


	BcF32 Timer_ = 0.0f;
	MaVec2d AbsolutePosition_;
 
	class ScnCanvasComponent* Canvas_ = nullptr;
	class ScnFontComponent* Font_ = nullptr;
	class GaPhysicsComponent* Physics_ = nullptr;
	class GaGameComponent* Game_ = nullptr;

};
