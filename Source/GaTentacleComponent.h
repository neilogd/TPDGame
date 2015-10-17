#pragma once

#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
	
//////////////////////////////////////////////////////////////////////////
// GaTentacleUniformBlockData
struct GaTentacleUniformBlockData
{
	REFLECTION_DECLARE_BASIC( GaTentacleUniformBlockData );
	GaTentacleUniformBlockData(){};

	static const BcU32 MAX_SEGMENTS = 32;

	/// xy = Position, zw = Tangent.
	std::array< MaVec4d, MAX_SEGMENTS > TentacleSegments_;
	/// Tentacle clip matrix.
	MaMat4d TentacleClipMatrix_;
};

//////////////////////////////////////////////////////////////////////////
// GaTentacleProcessor
class GaTentacleProcessor:
	public ScnComponentProcessor
{
public:
	REFLECTION_DECLARE_DERIVED( GaTentacleProcessor, ScnComponentProcessor );

	GaTentacleProcessor();
	virtual ~GaTentacleProcessor();

	void initialise() override;
	void shutdown() override;

	void update( const ScnComponentList& Components );

private:
	BcF32 Timer_ = 0.0f;
	
};

//////////////////////////////////////////////////////////////////////////
// GaTentacleComponent
class GaTentacleComponent:
	public ScnRenderableComponent,
	public ReIObjectNotify
{
public:
	REFLECTION_DECLARE_DERIVED( GaTentacleComponent, ScnRenderableComponent );

	GaTentacleComponent();
	virtual ~GaTentacleComponent();

	void setupComplexTopology( MaVec2d RootPosition, BcF32 Width, BcF32 SectionHeight, BcU32 NoofSections );
	void addPhysicsNoise();

	void calculateLevelStats( BcU32 Level );

	void targetStructure();
	void targetHome();
	class GaStructureComponent* getTargetStructure() const;


	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	void render( ScnRenderContext & RenderContext ) override;
	MaAABB getAABB() const override;
		
	void onObjectDeleted( class ReObject* Object ) override;


	
private:
	friend class GaTentacleProcessor;

	class ScnMaterial* Material_ = nullptr;

	BcF32 MoveSpeed_ = 64.0f;
	BcF32 MoveSpeedMultiplier_ = 0.1f;
	BcF32 HeadDamping_ = 0.5f;
	BcF32 HeadConstraintRigidity_ = 0.5f;

	BcF32 VerticalRigidity_ = 0.8f;
	BcF32 HorizontalRigidity_ = 0.8f;
	BcF32 DiagonalRigidity_ = 0.4f;

	BcF32 CalculatedMoveSpeed_ = 0.0f;

	BcF32 HeadTailDistance_ = 0.0f;

	BcF32 HeadSwaySmoothed_ = 0.0f;

	BcF32 TimerRandMult_ = 1.0f;
	BcF32 TimerRandOffset_ = 1.0f;

	MaVec2d SoftHeadPosition_ = MaVec2d( 0.0f, 0.0f );
	MaVec2d TailPosition_ = MaVec2d( 0.0f, 0.0f );

	BcU32 NoofSegments_ = 30;

	class GaGameComponent* Game_ = nullptr;
	class ScnCanvasComponent* Canvas_ = nullptr;

	// TODO: Replace with something more predictable, like a spline.
	class GaStructureComponent* TargetStructure_ = nullptr;
	MaVec2d TargetPosition_;

	// Rendering.
	struct Vertex
	{
		MaVec4d Position_;
		MaVec2d TexCoord_;
	};

	class ScnMaterialComponent* MaterialComponent_ = nullptr;
	RsVertexDeclarationUPtr VertexDecl_;
	RsBufferUPtr VertexBuffer_;
	RsBufferUPtr UniformBuffer_;
	GaTentacleUniformBlockData UniformBlock_;
	SysFence RenderFence_;
};