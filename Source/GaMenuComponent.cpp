#include "GaMenuComponent.h"
#include "GaHotspotComponent.h"
#include "GaPositionUtility.h"

#include "System/Debug/DsCore.h"
#include "System/Debug/DsImGui.h"
#include "System/Os/OsCore.h"

#include "System/Scene/ScnCore.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/Rendering/ScnFont.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_BASIC( GaMenuEntry );

void GaMenuEntry::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Text_", &GaMenuEntry::Text_, bcRFF_IMPORTER ),
		new ReField( "EntityPackage_", &GaMenuEntry::EntityPackage_, bcRFF_IMPORTER ),
		new ReField( "EntityName_", &GaMenuEntry::EntityName_, bcRFF_IMPORTER )
	};

	ReRegisterClass< GaMenuEntry >( Fields );
}

GaMenuEntry::GaMenuEntry():
	Text_(),
	EntityPackage_(),
	EntityName_()
{
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaMenuProcessor );

void GaMenuProcessor::StaticRegisterClass()
{
	ReRegisterClass< GaMenuProcessor, Super >();
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaMenuProcessor::GaMenuProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Update simulations",
				ScnComponentPriority::DEFAULT_UPDATE,
				std::bind( &GaMenuProcessor::drawMenus, this, std::placeholders::_1 ) )
		} )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
GaMenuProcessor::~GaMenuProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaMenuProcessor::initialise()
{

}

//////////////////////////////////////////////////////////////////////////
// shutdown
void GaMenuProcessor::shutdown()
{
	
}

//////////////////////////////////////////////////////////////////////////
// drawMenus
void GaMenuProcessor::drawMenus( const ScnComponentList& Components )
{
	for( auto InComponent : Components )
	{
		BcAssert( InComponent->isTypeOf< GaMenuComponent >() );
		auto* Component = static_cast< GaMenuComponent* >( InComponent.get() );

		auto* Canvas = Component->Canvas_;
		auto* Font = Component->Font_;

		ScnFontDrawParams FontParams;
		FontParams.setAlignment( ScnFontAlignment::HCENTRE | ScnFontAlignment::VCENTRE );
		FontParams.setSize( 64.0f );
		FontParams.setMargin( 0.0f );
		FontParams.setTextSettings( MaVec4d( 0.4f, 0.6f, 0.0f, 0.0f ) );
		FontParams.setTextColour( RsColour::WHITE );

		// Draw title.
		MaVec2d TitleSize = MaVec2d( 0.0f, 64.0f );
		MaVec2d TitlePosition = GaPositionUtility::GetScreenPosition(
			MaVec2d( Component->EntryMargin_, Component->EntryMargin_ ), TitleSize, GaPositionUtility::HCENTRE | GaPositionUtility::TOP );

		Font->drawText( Canvas, FontParams,
			TitlePosition,
			TitleSize,
			Component->Title_ );

		FontParams.setSize( 32.0f );

		// Draw entries.
		for( size_t Idx = 0; Idx < Component->Entries_.size(); ++Idx )
		{
			const auto& Entry = Component->Entries_[ Idx ];
			const auto* Hotspot = Component->Hotspots_[ Idx ];

			Font->drawText( Canvas, FontParams,
				Hotspot->getPosition(),
				Hotspot->getSize(),
				Entry.Text_ );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaMenuComponent );

void GaMenuComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Title_", &GaMenuComponent::Title_, bcRFF_IMPORTER ),
		new ReField( "Entries_", &GaMenuComponent::Entries_, bcRFF_IMPORTER ),
		new ReField( "EntrySize_", &GaMenuComponent::EntrySize_, bcRFF_IMPORTER ),
		new ReField( "EntryMargin_", &GaMenuComponent::EntryMargin_, bcRFF_IMPORTER ),
		
		new ReField( "Hotspots_", &GaMenuComponent::Hotspots_, bcRFF_TRANSIENT ),
		new ReField( "Canvas_", &GaMenuComponent::Hotspots_, bcRFF_TRANSIENT ),
		new ReField( "Font_", &GaMenuComponent::Font_, bcRFF_TRANSIENT )
	};

	ReRegisterClass< GaMenuComponent, Super >( Fields )
		.addAttribute( new GaMenuProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaMenuComponent::GaMenuComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaMenuComponent::~GaMenuComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaMenuComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Create hotspots.
	BcU32 ID = 0;
	if( Entries_.size() > 0 )
	{
		const MaVec2d TotalSize = MaVec2d( EntrySize_.x(), ( EntrySize_.y() * Entries_.size() ) + ( EntryMargin_ * ( Entries_.size() - 1 ) ) );
		MaVec2d EntryPosition = GaPositionUtility::GetScreenPosition(
			MaVec2d( EntryMargin_, EntryMargin_ ),
			TotalSize, 
			GaPositionUtility::HCENTRE | GaPositionUtility::VCENTRE );
		
		Hotspots_.reserve( Entries_.size() );
		for( const auto& Entry : Entries_ )
		{
			BcUnusedVar( Entry );
			Hotspots_.emplace_back( Parent->attach< GaHotspotComponent >( 
				GaHotspotComponent::StaticGetTypeName().getUnique(),
				ID++, 0,
				EntryPosition,
				EntrySize_ ) );

			EntryPosition += MaVec2d( 0.0f, EntrySize_.y() + EntryMargin_ );
		}
	}

	// Get canvas + font.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >();
	BcAssert( Canvas_ );
	Font_ = Parent->getComponentAnyParentByType< ScnFontComponent >();
	BcAssert( Font_ );

	// Subscribe for hotspot events.
	// Setup callback for clicking.
	Parent->subscribe( gaEVT_HOTSPOT_PRESSED, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< GaHotspotEvent >();
			const auto& Entry = Entries_[ Event.ID_ ];
			ScnEntitySpawnParams SpawnParams( 
					BcName::INVALID, Entry.EntityPackage_, Entry.EntityName_,
					MaMat4d(), getParentEntity()->getParentEntity() );
			SpawnParams.OnSpawn_ = [ this ]( ScnEntity* Entity )
				{
					ScnCore::pImpl()->removeEntity( getParentEntity() );
				};

			ScnCore::pImpl()->spawnEntity( SpawnParams );

			PSY_LOG( "Spawning menu %s.%s", Entry.EntityPackage_.c_str(), Entry.EntityName_.c_str() );
						
			return evtRET_PASS;
		} );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaMenuComponent::onDetach( ScnEntityWeakRef Parent )
{
	Parent->unsubscribeAll( this );
	Super::onDetach( Parent );
}
