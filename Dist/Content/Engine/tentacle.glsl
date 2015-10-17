#include <Psybrus.glsl>

#include "TentacleUniforms.glsl"

//////////////////////////////////////////////////////////////////////////
// Vertex shader
#if VERTEX_SHADER

VS_IN( vec4, InPosition_, POSITION );
VS_IN( vec4, InTexCoord_, TEXCOORD0 );

VS_OUT( vec4, VsTexCoord0 );

void vertexMain()
{
	vec4 Segment = TentacleSegments_[ int(InPosition_.w) ];
	vec2 CalculatedPosition = Segment.xy;
	CalculatedPosition += Segment.zw * InPosition_.x * InPosition_.z * 0.5;
	float PosUVScale = 4.0f / ( InPosition_.z + 3.0 );

    gl_Position = mul( TentacleClipMatrix_, vec4( CalculatedPosition.xy, 0.0, 1.0 ) );
    VsTexCoord0 = InTexCoord_ * TentacleUVScale_ * vec4( 1.0, PosUVScale, 1.0, 1.0 );
}

#endif

//////////////////////////////////////////////////////////////////////////
// Pixel shader
#if PIXEL_SHADER

PS_IN( vec4, VsTexCoord0 );

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_330
out float4 fragColor;
#endif

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_ES_100
#define fragColor gl_FragData[0]
#endif


//////////////////////////////////////////////////////////////////////////
// pixelTexturedMain
PSY_SAMPLER_2D( DiffuseTex );

void pixelTexturedMain()
{
	vec4 Colour = PSY_SAMPLE_2D( DiffuseTex, VsTexCoord0.xy );
	fragColor = Colour;
}

#endif
