#pragma once

#include "Events/EvtEvent.h"

#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnShaderFileData.h"

////////////////////////////////////////////////////////////////////////////////
// GaWaterComponent
class GaWaterComponent:
	public ScnRenderableComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaWaterComponent, ScnRenderableComponent );

	GaWaterComponent();
	virtual ~GaWaterComponent();

	void update( BcF32 Tick );
	MaVec2d getWaterSurfacePosition( MaVec2d WorldPosition ) const;

	void render( ScnRenderContext & RenderContext ) override;
	MaAABB getAABB() const override;

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

private:
	struct Vertex
	{
		MaVec4d Position_;
		MaVec2d TexCoord_;
		BcU32 Colour_;
	};

	class ScnMaterial* Material_ = nullptr;
	BcU32 NoofSegments_ = 32;
		
	BcF32 Timer_ = 0.0f;

	class ScnMaterialComponent* MaterialComponent_ = nullptr;

	RsVertexDeclarationUPtr VertexDecl_;
	RsBufferUPtr VertexBuffer_;
	RsGeometryBindingUPtr GeometryBinding_;
	RsBufferUPtr UniformBuffer_;
	ScnShaderObjectUniformBlockData UniformBlock_;

	MaMat4d ClipTransform_;
	MaMat4d InvClipTransform_;

};


