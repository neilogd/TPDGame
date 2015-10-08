#include "GaStructureComponent.h"
#include "GaHotspotComponent.h"
#include "GaPositionUtility.h"

#include "System/Debug/DsCore.h"
#include "System/Debug/DsImGui.h"
#include "System/Os/OsCore.h"

#include "System/Scene/ScnCore.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/Rendering/ScnFont.h"
#include "System/Scene/Rendering/ScnSpriteComponent.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaStructureProcessor );

void GaStructureProcessor::StaticRegisterClass()
{
	ReRegisterClass< GaStructureProcessor, Super >();
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaStructureProcessor::GaStructureProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Update game state",
				ScnComponentPriority::DEFAULT_UPDATE,
				std::bind( &GaStructureProcessor::update, this, std::placeholders::_1 ) )
		} )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
GaStructureProcessor::~GaStructureProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaStructureProcessor::initialise()
{

}

//////////////////////////////////////////////////////////////////////////
// shutdown
void GaStructureProcessor::shutdown()
{
	
}

//////////////////////////////////////////////////////////////////////////
// drawMenus
void GaStructureProcessor::update( const ScnComponentList& Components )
{
	for( auto InComponent : Components )
	{
		BcAssert( InComponent->isTypeOf< GaStructureComponent >() );
		auto* Component = static_cast< GaStructureComponent* >( InComponent.get() );

		
	}
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaStructureComponent );

void GaStructureComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Level_", &GaStructureComponent::Level_, bcRFF_IMPORTER ),
		new ReField( "Active_", &GaStructureComponent::Active_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< GaStructureComponent, Super >( Fields )
		.addAttribute( new GaStructureProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaStructureComponent::GaStructureComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaStructureComponent::~GaStructureComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaStructureComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Get canvas + font.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >();
	BcAssert( Canvas_ );
	Font_ = Parent->getComponentAnyParentByType< ScnFontComponent >();
	BcAssert( Font_ );

	setActive( Active_ );
	
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaStructureComponent::onDetach( ScnEntityWeakRef Parent )
{
	setActive( BcFalse );
	Parent->unsubscribeAll( this );
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// setActive
void GaStructureComponent::setActive( BcBool Active )
{
	Active_ = Active;

	auto Sprite = getComponentByType< ScnSpriteComponent >();
	if( Sprite )
	{
		Sprite->setColour( Active_ ? RsColour::GREEN : RsColour::RED );
	}
}
