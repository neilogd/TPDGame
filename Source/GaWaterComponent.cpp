#include "GaWaterComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Renderer/RsContext.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaWaterComponent );

void GaWaterComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Material_", &GaWaterComponent::Material_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY | bcRFF_CONST ),
		new ReField( "NoofSegments_", &GaWaterComponent::NoofSegments_, bcRFF_IMPORTER | bcRFF_CONST ),
	};

	using namespace std::placeholders;
	ReRegisterClass< GaWaterComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry::Update< GaWaterComponent >( "Update Water" ) 
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaWaterComponent::GaWaterComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaWaterComponent::~GaWaterComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// update
void GaWaterComponent::update( BcF32 Tick )
{
	// Update vertices.
	RsCore::pImpl()->updateBuffer( VertexBuffer_.get(), 0, 0, RsResourceUpdateFlags::ASYNC, 
		[	
			NoofSegments = NoofSegments_,
			Timer = Timer_

		]( class RsBuffer*, const RsBufferLock& Lock )
		{
			auto Vertices = static_cast< Vertex* >( Lock.Buffer_ );

			const BcF32 WaveAdvance = BcPIMUL2 / static_cast< BcF32 >( NoofSegments - 1 );
			const BcF32 PositionAdvance = 2.0f / static_cast< BcF32 >( NoofSegments - 1 );
			const BcF32 TexCoordAdvance = 1.0f / static_cast< BcF32 >( NoofSegments - 1 );

			BcF32 WaveX = Timer;
			MaVec4d PositionT( -1.0f,  1.0f, 0.0f, 1.0f );
			MaVec4d PositionB( -1.0f, -1.0f, 0.0f, 1.0f );
			MaVec2d TexCoordT(  0.0f,  0.0f );
			MaVec2d TexCoordB(  0.0f,  1.0f );
			for( BcU32 Idx = 0; Idx < NoofSegments; ++Idx )
			{
				PositionT.y( 
					0.25f + 
					BcSin( WaveX * 0.25f ) * 0.02f +
					BcSin( WaveX * 4.0f ) * 0.005f +
					BcSin( WaveX * 8.0f ) * 0.002f );
				WaveX += WaveAdvance;

				Vertices->Position_ = PositionT;
				Vertices->TexCoord_ = TexCoordT;
				Vertices->Colour_ = 0xffffffff;
				Vertices++;
				Vertices->Position_ = PositionB;
				Vertices->TexCoord_ = TexCoordB;
				Vertices->Colour_ = 0xffffffff;
				Vertices++;

				PositionT += MaVec4d( PositionAdvance, 0.0f, 0.0f, 0.0f );
				PositionB += MaVec4d( PositionAdvance, 0.0f, 0.0f, 0.0f );
				TexCoordT += MaVec2d( TexCoordAdvance, 0.0f );
				TexCoordB += MaVec2d( TexCoordAdvance, 0.0f );
			}
		} );


	// Timer.
	Timer_ += Tick;
	const auto WrappingValue = BcPIMUL2 * 100.0;
	if( Timer_ > WrappingValue )
	{
		Timer_ -= WrappingValue;
	}
}

//////////////////////////////////////////////////////////////////////////
// render
void GaWaterComponent::render( ScnRenderContext & RenderContext )
{
	RsRenderSort Sort = RenderContext.Sort_;
	if( MaterialComponent_ )
	{
		RenderContext.pViewComponent_->setMaterialParameters( MaterialComponent_ );
		MaterialComponent_->setObjectUniformBlock( UniformBuffer_.get() );
		MaterialComponent_->bind( RenderContext.pFrame_, Sort );

		RenderContext.pFrame_->queueRenderNode( Sort,
			[ 
				VertexDecl = VertexDecl_.get(),
				VertexBuffer = VertexBuffer_.get(),
				NoofSegments = NoofSegments_
			] ( RsContext* Context )
			{
				Context->setVertexDeclaration( VertexDecl );
				Context->setVertexBuffer( 0, VertexBuffer, sizeof( Vertex ) );
				Context->drawPrimitives( RsTopologyType::TRIANGLE_STRIP, 0, NoofSegments * 2 );
			} );
	}
}

//////////////////////////////////////////////////////////////////////////
// getAABB
MaAABB GaWaterComponent::getAABB() const
{
	return MaAABB();
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaWaterComponent::onAttach( ScnEntityWeakRef Parent )
{
	BcAssert( Material_ );
	MaterialComponent_ = Parent->attach< ScnMaterialComponent >( 
		BcName::INVALID,
		Material_, ScnShaderPermutationFlags::MESH_STATIC_2D );

	VertexDecl_.reset( RsCore::pImpl()->createVertexDeclaration(
		RsVertexDeclarationDesc( 2 )
			.addElement( RsVertexElement( 0, 0, 4, RsVertexDataType::FLOAT32, RsVertexUsage::POSITION, 0 ) )
			.addElement( RsVertexElement( 0, 16, 2, RsVertexDataType::FLOAT32, RsVertexUsage::TEXCOORD, 0 ) )
			.addElement( RsVertexElement( 0, 24, 4, RsVertexDataType::UBYTE_NORM, RsVertexUsage::COLOUR, 0 ) ) ) );

	VertexBuffer_.reset( RsCore::pImpl()->createBuffer(
		RsBufferDesc( 
			RsBufferType::VERTEX,
			RsResourceCreationFlags::STREAM,
			sizeof( Vertex ) * NoofSegments_ * 2 ) ) );
			
	UniformBuffer_.reset( RsCore::pImpl()->createBuffer(
		RsBufferDesc( 
			RsBufferType::UNIFORM,
			RsResourceCreationFlags::STREAM,
			sizeof( UniformBlock_ ) ) ) );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaWaterComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}
