#include <Psybrus.glsl>

#include "TentacleUniforms.glsl"

//////////////////////////////////////////////////////////////////////////
// Vertex shader
#if VERTEX_SHADER

/// X = Idx.
/// Y = Wave offset.
/// Z = Scale wave.
/// W = Scale width.
VS_IN( vec4, InParams_, POSITION );
VS_IN( vec4, InTexCoord_, TEXCOORD0 );

VS_OUT( vec4, VsTexCoord0 );
VS_OUT( vec4, VsTexCoord1 );

void vertexMain()
{
	vec4 Segment = TentacleSegments_[ int(InParams_.x) ];
	vec2 CalculatedPosition = Segment.xy;
	CalculatedPosition += Segment.zw * ( InTexCoord_.x * 2.0f - 1.0f ) * InParams_.w * 0.5;
	float PosUVScale = 4.0f / ( InParams_.w + 3.0 );

	gl_Position = mul( TentacleClipMatrix_, vec4( CalculatedPosition.xy, 0.0, 1.0 ) );
	VsTexCoord0 = InTexCoord_ * vec4( 1.0, PosUVScale, 1.0, 1.0 );
	
	// WAVE SCALEY THING.
	//VsTexCoord1 = VsTexCoord0 + vec4( sin( TentacleTimer_.x + InParams_.y ) * InParams_.z, 0.0, 0.0, 0.0 );
}

#endif

//////////////////////////////////////////////////////////////////////////
// Pixel shader
#if PIXEL_SHADER

PS_IN( vec4, VsTexCoord0 );
PS_IN( vec4, VsTexCoord1 );

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_330
out float4 fragColor;
#endif

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_ES_100
#define fragColor gl_FragData[0]
#endif


//////////////////////////////////////////////////////////////////////////
// pixelTexturedMain
PSY_SAMPLER_2D( TentacleBaseTex );
PSY_SAMPLER_2D( TentacleSuckersTex );
PSY_SAMPLER_2D( TentacleTipTex );

void pixelTexturedMain()
{
	// Tentacle tip:
	// x = Border.
	// y = Base.
	// z = Suckers.
	vec4 TentacleTip = PSY_SAMPLE_2D( TentacleTipTex, VsTexCoord0.xy );
	vec4 TentacleBase = PSY_SAMPLE_2D( TentacleBaseTex, VsTexCoord0.xy );
	vec4 TentacleSuckers = PSY_SAMPLE_2D( TentacleSuckersTex, VsTexCoord0.xy );
	TentacleBase.w *= TentacleTip.y;
	TentacleSuckers.w *= TentacleTip.z;

	// Composite base on to suckers.
	float SrcAlpha = TentacleSuckers.a;
	float InvSrcAlpha = 1.0 - SrcAlpha;
	vec4 Colour = TentacleBase * InvSrcAlpha + TentacleSuckers * SrcAlpha;

	// Border.
	Colour.xyz *= 1.0 - TentacleTip.x;

	fragColor = Colour;
}


#endif
