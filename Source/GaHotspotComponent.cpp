#include "GaHotspotComponent.h"

#include "System/Debug/DsCore.h"
#include "System/Debug/DsImGui.h"
#include "System/Os/OsCore.h"

#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaHotspotProcessor );

void GaHotspotProcessor::StaticRegisterClass()
{
	ReRegisterClass< GaHotspotProcessor, Super >();
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaHotspotProcessor::GaHotspotProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Setup hotspots",
				ScnComponentPriority::ENTITY_UPDATE,
				std::bind(  &GaHotspotProcessor::setupHotspots, this, std::placeholders::_1 ) ),
			ScnComponentProcessFuncEntry(
				"Debug draw",
				ScnComponentPriority::VIEW_RENDER - 1,
				std::bind(  &GaHotspotProcessor::debugDraw, this, std::placeholders::_1 ) )
		} ),
	HotspotComponents_()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
GaHotspotProcessor::~GaHotspotProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaHotspotProcessor::initialise()
{
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, this,
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputMouse >();
			MouseEvents_.push_back( std::make_pair( ID, Event ) );
			return evtRET_PASS;
		} );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, this,
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputMouse >();
			MouseEvents_.push_back( std::make_pair( ID, Event ) );
			return evtRET_PASS;
		} );
}

//////////////////////////////////////////////////////////////////////////
// shutdown
void GaHotspotProcessor::shutdown()
{
	OsCore::pImpl()->unsubscribeAll( this );
}

//////////////////////////////////////////////////////////////////////////
// setupHotspots
void GaHotspotProcessor::setupHotspots( const ScnComponentList& Components )
{
	HotspotComponents_.clear();
	HotspotComponents_.reserve( Components.size() );

	// Copy in.
	for( auto InComponent : Components )
	{		
		BcAssert( InComponent->isTypeOf< GaHotspotComponent >() );
		auto* Component = static_cast< GaHotspotComponent* >( InComponent.get() );
		HotspotComponents_.emplace_back( Component );
	}

	// Sort by their Z position.
	std::sort( HotspotComponents_.begin(), HotspotComponents_.end(), 
		[]( const GaHotspotComponent* A, const GaHotspotComponent* B )
		{
			return A->Layer_ < B->Layer_;
		} );

	// Process pending events.
	for( const auto& EventPair : MouseEvents_ )
	{
		const EvtID ID = EventPair.first;
		const OsEventInputMouse Event = EventPair.second;

		for( auto* Hotspot : HotspotComponents_ )
		{
			auto* ParentEntity = Hotspot->getParentEntity();
			const MaVec2d Position = Hotspot->getPosition();
			const MaVec2d MousePosition( Event.MouseX_, Event.MouseY_ );
			const MaVec2d CornerA = Position - Hotspot->Size_;	
			const MaVec2d CornerB = Position + Hotspot->Size_;				
			if( MousePosition.x() >= CornerA.x() && MousePosition.x() <= CornerB.x() &&
				MousePosition.y() >= CornerA.y() && MousePosition.y() <= CornerB.y() )
			{
				GaHotspotEvent OutEvent;
				OutEvent.ID_ = Hotspot->ID_;
				OutEvent.Position_ = MousePosition;
				OutEvent.RelativePosition_ = MousePosition - Position;
				if( ID == osEVT_INPUT_MOUSEDOWN )
				{
					ParentEntity->publish( gaEVT_HOTSPOT_PRESSED, OutEvent );
					break;
				}
				else if( ID == osEVT_INPUT_MOUSEMOVE )
				{
					ParentEntity->publish( gaEVT_HOTSPOT_HOVER, OutEvent );
					break;
				}				
			}
		}
	}

	// No more events.
	MouseEvents_.clear();

	// No more hotspots.
	HotspotComponents_.clear();
}

//////////////////////////////////////////////////////////////////////////
// debugDraw
void GaHotspotProcessor::debugDraw( const ScnComponentList& Components )
{			
#if !PSY_PRODUCTION
	ImGui::SetNextWindowPos( MaVec2d( -64.0f, -64.0f ), ImGuiSetCond_Always );
	ImGui::SetNextWindowSize( MaVec2d( 0.0f, 0.0f ) );
	if( ImGui::Begin( "", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
	{
		auto DrawList = ImGui::GetWindowDrawList();
		DrawList->PushClipRectFullScreen();

		for( auto InComponent : Components )
		{		
			BcAssert( InComponent->isTypeOf< GaHotspotComponent >() );
			auto* Component = static_cast< GaHotspotComponent* >( InComponent.get() );

			MaVec2d Position = Component->getPosition();
			MaVec2d CornerA = Position;
			MaVec2d CornerB = Position + Component->Size_;
			DrawList->AddRect( CornerA, CornerB, 0xffffffff );
		}

		DrawList->PopClipRect();
		ImGui::End();
	}
#endif // !PSY_PRODUCTION
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaHotspotComponent );

void GaHotspotComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ID_", &GaHotspotComponent::ID_, bcRFF_IMPORTER ),
		new ReField( "Position_", &GaHotspotComponent::Position_, bcRFF_IMPORTER ),
		new ReField( "Size_", &GaHotspotComponent::Size_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< GaHotspotComponent, Super >( Fields )
		.addAttribute( new GaHotspotProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaHotspotComponent::GaHotspotComponent():
	ID_( 0 ),
	Layer_( 0 ),
	Position_( 0.0f, 0.0f ),
	Size_( 0.0f, 0.0f )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaHotspotComponent::GaHotspotComponent( BcU32 ID, BcS32 Layer, MaVec2d Position, MaVec2d Size ):
	ID_( ID ),
	Layer_( Layer ),
	Position_( Position ),
	Size_( Size )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaHotspotComponent::~GaHotspotComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// getID
BcU32 GaHotspotComponent::getID() const
{
	return ID_;
}

//////////////////////////////////////////////////////////////////////////
// getPosition
MaVec2d GaHotspotComponent::getPosition() const
{
	return getParentEntity()->getWorldPosition().xy() + Position_;
}

//////////////////////////////////////////////////////////////////////////
// getSize
MaVec2d GaHotspotComponent::getSize() const
{
	return Size_;
}
