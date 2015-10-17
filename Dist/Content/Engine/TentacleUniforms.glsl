#pragma once

#include <PsybrusUniforms.glsl>

////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
BEGIN_CBUFFER( GaTentacleUniformBlockData )
	ENTRY( GaTentacleUniformBlockData, float4, TentacleSegments_[32] )
	ENTRY( GaTentacleUniformBlockData, float4x4, TentacleClipMatrix_ )
	ENTRY( GaTentacleUniformBlockData, float4, TentacleTimer_ )
END_CBUFFER

#if !PSY_USE_CBUFFER

#  define TentacleSegments_ GaTentacleUniformBlockDataVS_XTentacleSegments_
#  define TentacleClipMatrix_ GaTentacleUniformBlockDataVS_XTentacleClipMatrix_
#  define TentacleTimer_ GaTentacleUniformBlockDataVS_XTentacleTimer_

#endif

