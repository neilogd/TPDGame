#include <Psybrus.glsl>

//////////////////////////////////////////////////////////////////////////
// Vertex shader
#if VERTEX_SHADER

VS_IN( vec4, InPosition_, POSITION );
VS_IN( vec4, InTexCoord_, TEXCOORD0 );
VS_IN( vec4, InColour_, COLOUR0 );

VS_OUT( vec4, VsColour0 );
VS_OUT( vec4, VsTexCoord0 );
VS_OUT( vec4, VsTexCoord1 );
VS_OUT( vec4, VsTexCoord2 );
VS_OUT( vec4, VsTexCoord3 );
VS_OUT( vec4, VsTexCoord4 );
VS_OUT( vec4, VsTexCoord5 );
VS_OUT( vec4, VsTexCoord6 );
VS_OUT( vec4, VsTexCoord7 );

void vertexMain()
{
    gl_Position = float4( InPosition_.xy, 0.0, 1.0 );

    vec4 Offset0 = normalize( vec4( -0.0545764,  0.1893472, 0.0, 0.0 ) );
    vec4 Offset1 = normalize( vec4(  0.0589671,  0.1457642, 0.0, 0.0 ) );
    vec4 Offset2 = normalize( vec4(  0.0000000,  0.1968292, 0.0, 0.0 ) );
    vec4 Offset3 = normalize( vec4(  0.0784761,  0.1739212, 0.0, 0.0 ) );
    vec4 Offset4 = normalize( vec4(  0.0000000,  1.0000000, 0.0, 0.0 ) );

    VsTexCoord0 = ( InTexCoord_ + Offset0 * ViewTime_.wwww * 0.2 );
    VsTexCoord1 = ( InTexCoord_ + Offset1 * ViewTime_.wwww * 0.2 );
    VsTexCoord2 = ( InTexCoord_ + Offset2 * ViewTime_.wwww * 0.2 );
    VsTexCoord3 = ( InTexCoord_ + Offset3 * ViewTime_.wwww * 0.2 );

    VsTexCoord4 = ( InTexCoord_ + Offset4 * ViewTime_.wwww * 0.2 );
    VsTexCoord5 = ( InTexCoord_ + Offset4 * ViewTime_.zzzz * 0.2 );
    VsTexCoord6 = ( InTexCoord_ + Offset4 * ViewTime_.yyyy * 0.2 );
    VsTexCoord7 = ( InTexCoord_ + Offset4 * ViewTime_.xxxx * 0.2 );
    VsColour0 = InColour_;
}

#endif

//////////////////////////////////////////////////////////////////////////
// Pixel shader
#if PIXEL_SHADER

PS_IN( vec4, VsColour0 );
PS_IN( vec4, VsTexCoord0 );
PS_IN( vec4, VsTexCoord1 );
PS_IN( vec4, VsTexCoord2 );
PS_IN( vec4, VsTexCoord3 );
PS_IN( vec4, VsTexCoord4 );
PS_IN( vec4, VsTexCoord5 );
PS_IN( vec4, VsTexCoord6 );
PS_IN( vec4, VsTexCoord7 );

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_330
out float4 fragColor;
#endif

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_ES_100
#define fragColor gl_FragData[0]
#endif

//////////////////////////////////////////////////////////////////////////
// pixelDefaultMain
void pixelDefaultMain()
{
	fragColor = VsColour0;
}

//////////////////////////////////////////////////////////////////////////
// pixelTexturedMain
PSY_SAMPLER_2D( NoiseTex );
PSY_SAMPLER_2D( WaveTex );

void pixelTexturedMain()
{
	vec4 Sample0 = PSY_SAMPLE_2D( NoiseTex, VsTexCoord0.xy );
	vec4 Sample1 = PSY_SAMPLE_2D( NoiseTex, VsTexCoord1.xy );
	//vec4 Sample2 = PSY_SAMPLE_2D( NoiseTex, VsTexCoord2.xy );
	//vec4 Sample3 = PSY_SAMPLE_2D( NoiseTex, VsTexCoord3.xy );
	vec4 WaveSample0 = PSY_SAMPLE_2D( WaveTex, VsTexCoord5.yx );
	//vec4 WaveSample1 = PSY_SAMPLE_2D( WaveTex, VsTexCoord5.yx );
	//vec4 WaveSample2 = PSY_SAMPLE_2D( WaveTex, VsTexCoord6.yx );
	//vec4 WaveSample3 = PSY_SAMPLE_2D( WaveTex, VsTexCoord7.yx );

	//float MinSample = min( Sample0.x, Sample1.y );
	//float MaxSample = max( Sample0.x, Sample1.y );
	float MulSample = Sample0.x * Sample1.y;
	//float MinSample = min( min( min( Sample0.x, Sample1.y ), Sample2.z ), Sample3.x );
	//float MaxSample = max( max( max( Sample0.x, Sample1.y ), Sample2.z ), Sample3.x );
	//float MulSample = Sample0.x * Sample1.y * Sample2.z * Sample3.x;

#if 0
	vec4 FoamAdditive = vec4( 1.0, 1.0, 1.0, 0.0 );
	float FoamSample = min( 1.0, MulSample * Sample2.z + 0.5 );
	float FoamThreshold = 0.9;
	FoamAdditive.a = max( 0.0, FoamSample - FoamThreshold ) / ( 1.0 - FoamThreshold );

	//float WaveMultiplier = min( 1.0, 0.8 + WaveSample0.x + WaveSample1.x * 0.9 );
	//float WaveMultiplier = min( 1.0, 0.5 + WaveSample0.x + WaveSample1.x * 0.5 + WaveSample2.x * 0.25 + WaveSample3.x * 0.125 );

	vec4 WaterColour = vec4( vec3( 0.6, 0.8, 1.0 ) * MulSample, 0.5 ) * VsColour0;

	// Composite foam onto water.
	float SrcAlpha = FoamAdditive.a;
	float InvSrcAlpha = 1.0 - SrcAlpha;
	fragColor = WaterColour * InvSrcAlpha + FoamAdditive * SrcAlpha;
#else
	float ColourMult = min( 1.0, 0.5 + WaveSample0.x );
	vec4 WaterColour = vec4( vec3( 0.6, 0.8, 1.0 ) * MulSample * ColourMult, 0.5 ) * VsColour0;
	fragColor = WaterColour;
#endif
}

#endif
