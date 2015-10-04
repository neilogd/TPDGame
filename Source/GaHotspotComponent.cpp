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
		[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputMouse >();

			for( auto* Hotspot : HotspotComponents_ )
			{
				auto* ParentEntity = Hotspot->getParentEntity();
				const MaVec3d Position = Hotspot->getPosition();
				const MaVec2d MousePosition( Event.MouseX_, Event.MouseY_ );
				const MaVec2d CornerA = Position.xy() - Hotspot->Size_;	
				const MaVec2d CornerB = Position.xy() + Hotspot->Size_;				
				if( MousePosition.x() >= CornerA.x() && MousePosition.x() <= CornerB.x() &&
					MousePosition.y() >= CornerA.y() && MousePosition.y() <= CornerB.y() )
				{
					GaHotspotEvent OutEvent;
					OutEvent.ID_ = Hotspot->ID_;
					OutEvent.Position_ = MousePosition;
					OutEvent.RelativePosition_ = MousePosition - Position.xy();
					ParentEntity->publish( gaEVT_HOTSPOT_PRESSED, OutEvent );
					return evtRET_PASS;
				}
			}

			return evtRET_PASS;
		} );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )->eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputMouse >();

			for( auto* Hotspot : HotspotComponents_ )
			{
				auto* ParentEntity = Hotspot->getParentEntity();
				const MaVec3d Position = Hotspot->getPosition();
				const MaVec2d MousePosition( Event.MouseX_, Event.MouseY_ );
				const MaVec2d CornerA = Position.xy() - Hotspot->Size_;	
				const MaVec2d CornerB = Position.xy() + Hotspot->Size_;				
				if( MousePosition.x() >= CornerA.x() && MousePosition.x() <= CornerB.x() &&
					MousePosition.y() >= CornerA.y() && MousePosition.y() <= CornerB.y() )
				{
					GaHotspotEvent OutEvent;
					OutEvent.ID_ = Hotspot->ID_;
					OutEvent.Position_ = MousePosition;
					OutEvent.RelativePosition_ = MousePosition - Position.xy();
					ParentEntity->publish( gaEVT_HOTSPOT_HOVER, OutEvent );
					return evtRET_PASS;
				}
			}

			return evtRET_PASS;
		} );}

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
			return A->getPosition().z() < B->getPosition().z();
		} );
}

//////////////////////////////////////////////////////////////////////////
// debugDraw
void GaHotspotProcessor::debugDraw( const ScnComponentList& Components )
{			
	ImGui::SetNextWindowPos( MaVec2d( 0.0f, 0.0f ), ImGuiSetCond_Always );
	ImGui::SetNextWindowSize( MaVec2d( 0.0f, 0.0f ) );
	if( ImGui::Begin( "", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
	{
		auto DrawList = ImGui::GetWindowDrawList();
		DrawList->PushClipRectFullScreen();

		for( auto InComponent : Components )
		{		
			BcAssert( InComponent->isTypeOf< GaHotspotComponent >() );
			auto* Component = static_cast< GaHotspotComponent* >( InComponent.get() );

			MaVec3d Position = Component->getParentEntity()->getWorldPosition() + Component->Position_;

			MaVec2d CornerA = Position.xy() - Component->Size_;
			MaVec2d CornerB = Position.xy() + Component->Size_;
			DrawList->AddRectFilled( CornerA, CornerB, 0x20ffffff, 1.0f, 15 );
		}

		DrawList->PopClipRect();
		ImGui::End();
	}
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
	Position_( 0.0f, 0.0f, 0.0f ),
	Size_( 0.0f, 0.0f )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaHotspotComponent::~GaHotspotComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// getPosition
MaVec3d GaHotspotComponent::getPosition() const
{
	return getParentEntity()->getWorldPosition() + Position_;
}
