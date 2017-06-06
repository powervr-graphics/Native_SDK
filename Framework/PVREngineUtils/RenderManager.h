/*!
\brief The RenderManager class. Provides basic engine rendering functionality. See class documentation for basic
use.
\file PVREngineUtils/RenderManager.h
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVREngineUtils/EffectApi_2.h"
#include "PVREngineUtils/AssetUtils.h"
#include <deque>

namespace pvr {
namespace utils {

struct Attribute
{
	StringHash semantic;
	std::string variableName;
	types::DataType datatype;
	uint16 offset;
	uint16 width;
	Attribute() : datatype(types::DataType::None), offset(0), width(0) {}
	Attribute(StringHash semantic, types::DataType datatype, uint16 width, uint16 offset, const std::string& variableName) :
		semantic(semantic), variableName(variableName), datatype(datatype), offset(offset), width(width) {}
};

struct AttributeLayout : public DynamicArray<Attribute>
{
	uint32 stride;
	AttributeLayout() : stride(0) {}
};

struct RendermanModel;
class RenderManager;

struct RendermanSubpassGroupModel;
struct RendermanPipeline;
struct RendermanSubpassMaterial;
struct RendermanSubpass;
struct RendermanPass;
struct RendermanEffect;
struct RendermanNode;
struct RendermanSubpassGroup;

/// <summary>This struct is used to store buffers such as ubo and ssbo</summary>
struct RendermanBufferDefinition
{
	StringHash name; //!< buffer name
	StructuredMemoryView buffer;
	types::BufferBindingUse allSupportedBindings;
	bool isDynamic;
	types::VariableScope scope; //!< buffer scope
	uint16 numBuffers;
	uint32 numDynamicClients;
	RendermanBufferDefinition() : numBuffers(1), numDynamicClients(0), isDynamic(false) {}
};

/// <summary>This struct is used to store a material.</summary>
/// <remarks>(Exists to avoid duplication between different textures.)</remarks>
struct RendermanMaterial
{
	RendermanModel* renderModel_;//!< the model this material belongs to
	std::map<StringHash, api::TextureView> textures;//!< material textures
	assets::MaterialHandle assetMaterial;
	uint32 assetMaterialId;//!< material id

	/// <summary>Return RendermanModel which own this object (const).</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToRendermanModel() const;

	/// <summary>Return RendermanModel which own this object</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToRendermanModel();

	/// <summary>Return RenderManager which own this object (const)</summary>
	/// <returns>Return RenderManager</returns>
	const RenderManager& backToRenderManager()const;

	/// <summary>Return RenderManager which own this object</summary>
	/// <returns>Return RenderManager</returns>
	RenderManager& backToRenderManager();
};


/// <summary>Part of RendermanStructure. This class is used to store VBOs/IBOs. Unique per mesh RendermanNodes
/// inside passes/subpasses reference these items by pointer.</summary>
struct RendermanMesh
{
	RendermanModel* renderModel_;
	assets::MeshHandle assetMesh;
	uint32 assetMeshId;
	DynamicArray<api::Buffer> vbos; // ONLY ONE - OPTIMISED FOR ALL PIPELINES
	api::Buffer ibo;        // ONLY ONE - OPTIMISED FOR ALL PIPELINES
	types::IndexType indexType;

	/// <summary>Return RendermanModel which owns this object (const).</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToRendermanModel()const { return *renderModel_; }

	/// <summary>Return RendermanModel which owns this object.</summary>
	/// <returns>RendermanModel</returns>
	RendermanModel& backToRendermanModel() { return *renderModel_; }

	/// <summary>Return RenderManager which own this object (const)</summary>
	/// <returns>Return RenderManager</returns>
	const RenderManager& backToRenderManager()const;

	/// <summary>Return RenderManager which own this object</summary>
	/// <returns>Return RenderManager</returns>
	RenderManager& backToRenderManager();
};


typedef bool(*ModelSemanticSetter)(TypedMem& mem, const RendermanModel& model);


/// <summary>Part of RendermanStructure. This class is used to store RendermanMeshes. Unique per model
/// RendermanModelEffects inside passes/subpasses reference these items by pointer.</summary>
struct RendermanModel
{
	RenderManager* mgr_;//!< render manager
	assets::ModelHandle assetModel; //!< handle to the model
	std::deque<RendermanMesh> meshes;//!< renderable meshes
	std::deque<RendermanMaterial> materials;//< materials

	/// <summary>Get model semantic data</summary>
	/// <param name="semantic">Semantic</param>
	/// <param name="memory">Data returned</param>
	/// <returns>Return true if found</returns>
	bool getModelSemantic(const StringHash& semantic, TypedMem& memory) const;

	/// <summary>Return model semantic setter</summary>
	/// <param name="semantic">Semantic</param>
	ModelSemanticSetter getModelSemanticSetter(const StringHash& semantic) const;

	/// <summary>Return RenderManager which own this object (const)</summary>
	/// <returns>Return RenderManager</returns>
	const RenderManager& backToRenderManager()const { return *mgr_; }

	/// <summary>Return RenderManager which own this object</summary>
	/// <returns>Return RenderManager</returns>
	RenderManager& backToRenderManager() { return *mgr_; }

	/// <summary>Get renderman mesh object belongs to this model</summary>
	/// <param name="mesh">Mesh index</param>
	/// <returns>RendermanMesh</returns>
	RendermanMesh& toRendermanMesh(uint32 mesh)
	{
		debug_assertion(mesh < meshes.size(), "Mesh index out of bound");
		return meshes[mesh];
	}

	/// <summary>Get renderman mesh object belongs to this model (const)</summary>
	/// <param name="mesh">Mesh index</param>
	/// <returns>RendermanMesh</returns>
	const RendermanMesh& toRendermanMesh(uint32 mesh) const
	{
		debug_assertion(mesh < meshes.size(), "Mesh index out of bound");
		return meshes[mesh];
	}

	/// <summary>Get renderman material object belongs to this model</summary>
	/// <param name="material">Material index</param>
	/// <returns>RendermanMaterial</returns>
	RendermanMaterial& toRendermanMaterial(uint32 material)
	{
		debug_assertion(material < materials.size(), "material index out of bound");
		return materials[material];
	}

	RendermanMaterial* toRendermanMaterial(const StringHash& name)
	{
		auto it = std::find_if(materials.begin(), materials.end(),
		                       [&](RendermanMaterial & mat)
		{
			return name == mat.assetMaterial->getName();
		});
		return (it != materials.end() ? & (*it) : NULL);
	}

	/// <summary>Get renderman material object belongs to this model (const)</summary>
	/// <param name="material">Material index</param>
	/// <returns>RendermanMaterial</returns>
	const RendermanMaterial& toRendermanMaterial(uint32 material) const
	{
		debug_assertion(material < materials.size(), "material index out of bound");
		return materials[material];
	}
};

/// <summary>Contains information to bind a buffer to a specific pipeline's descriptor sets.</summary>
struct RendermanBufferBinding
{
	RendermanBufferDefinition* bufferDefinition;
	StringHash semantic;
	types::DescriptorType type;
	uint8 set;
	uint8 binding;
	int16 node_dynamic_offset_address;
};

//Part of RendermanStructure. This class is a Material's instances as used by a pipeline
struct RendermanMaterialSubpassPipeline
{
	RendermanPipeline* pipeline_;
	RendermanSubpassMaterial* materialSubpass_;
	Multi<api::DescriptorSet> sets[4];

	/// <summary>Get RendermanPipeline object (const)</summary>
	/// <returns>RendermanPipeline</returns>
	const RendermanPipeline& toPipeline() const;

	/// <summary>Get RendermanPipeline object</summary>
	/// <returns>RendermanPipeline</returns>
	RendermanPipeline& toPipeline();

	/// <summary>Return the RendermanSubpassMaterial object which owns this object (const)</summary>
	/// <returns>RendermanSubpassMaterial</returns>
	const RendermanSubpassMaterial& backToSubpassMaterial() const { return *materialSubpass_; }

	/// <summary>Return the RendermanSubpassMaterial object which owns this object</summary>
	/// <returns>RendermanSubpassMaterial</returns>
	RendermanSubpassMaterial& backToSubpassMaterial() { return *materialSubpass_; }
};

//Part of RendermanStructure. This class contains the Material's instances that are used by a pipeline
//The reason is that a pipeline is selected BOTH by material AND by mesh, making it possible for one
//material in one subpass to be used by different pipelines.
struct RendermanSubpassMaterial
{
	std::vector<RendermanMaterialSubpassPipeline> materialSubpassPipelines;
	RendermanSubpassGroupModel* modelSubpass_;
	RendermanMaterial* material;

	/// <summary>Return RendermanMaterialSubpassPipeline object</summary>
	/// <param name="index">RendermanMaterialSubpassPipeline index</param>
	/// <returns>RendermanMaterialSubpassPipeline</returns>
	RendermanMaterialSubpassPipeline& toMaterialSubpassPipeline(uint32 index)
	{
		debug_assertion(index < materialSubpassPipelines.size(), "Material subpass pipeline index out of bound");
		return materialSubpassPipelines[index];
	}

	/// <summary>Return RendermanMaterialSubpassPipeline object (const)</summary>
	/// <param name="index">RendermanMaterialSubpassPipeline index</param>
	/// <returns>RendermanMaterialSubpassPipeline</returns>
	const RendermanMaterialSubpassPipeline& toMaterialSubpassPipeline(uint32 index)const
	{
		debug_assertion(index < materialSubpassPipelines.size(), "Material subpass pipeline index out of bound");
		return materialSubpassPipelines[index];
	}

	/// <summary>Return RendermanSubpassModel object which owns this object (const).</summary>
	/// <returns>RendermanSubpassModel</returns>
	RendermanSubpassGroupModel& backToSubpassGroupModel() { return *modelSubpass_; }

	/// <summary>Return RendermanSubpassModel object which owns this object (const).</summary>
	/// <returns>RendermanSubpassModel</returns>
	const RendermanSubpassGroupModel& backToSubpassGroupModel()const { return *modelSubpass_; }

	/// <summary>Return RendermanModel which owns this object (const)</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToModel()const;

	/// <summary>Return RendermanModel which owns this object</summary>
	/// <returns>RendermanModel</returns>
	RendermanModel& backToModel();

	/// <summary>Return RendermanSubpass which owns this object.</summary>
	/// <returns>RendermanSubpass</returns>
	RendermanSubpassGroup& backToSubpassGroup();

	/// <summary>Return RendermanSubpass which owns this object (const).</summary>
	/// <returns>RendermanSubpass</returns>
	const RendermanSubpassGroup& backToSubpassGroup()const;

	/// <summary>Return material</summary>
	/// <returns>RendermanMaterial</returns>
	RendermanMaterial& toMaterial() { return  *material; }

	/// <summary>Return material (const)</summary>
	/// <returns>RendermanMaterial</returns>
	const RendermanMaterial& toMaterial()const { return  *material; }
};

//Part of RendermanStructure. This class is a Mesh's instances as used by a pipeline
//The "usedByPipelines" is only a helper.
struct RendermanSubpassMesh
{
	RendermanSubpassGroupModel* modelSubpass_;
	RendermanMesh* rendermesh_;
	std::set<RendermanPipeline*> usedByPipelines;

	/// <summary>Return RendermanSubpassModel object which owns this object</summary>
	/// <returns>RendermanSubpassModel</returns>
	RendermanSubpassGroupModel& backToSubpassGroupModel() { return *modelSubpass_; }

	/// <summary>Return RendermanSubpassModel object which owns this object (const)</summary>
	/// <returns>RendermanSubpassModel</returns>
	const RendermanSubpassGroupModel& backToSubpassGroupModel()const { return *modelSubpass_; }

	/// <summary>Return RendermanModel which owns this object</summary>
	/// <returns>RendermanModel</returns>
	RendermanModel& backToModel();

	/// <summary>Return RendermanModel which owns this object (const).</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToModel()const;

	/// <summary>Return RendermanSubpass which owns this object (const).</summary>
	/// <returns>RendermanSubpass</returns>
	const RendermanSubpassGroup& backToSubpassGroup()const;

	/// <summary>Return RendermanSubpass which owns this object.</summary>
	/// <returns>RendermanSubpass</returns>
	RendermanSubpassGroup& backToSubpassGroup();

	/// <summary>Return RendermanMesh object which owns this object (const)</summary>
	/// <returns>RendermanMesh</returns>
	RendermanMesh& backToMesh() { return *rendermesh_; }

	/// <summary>Return RendermanMesh object which owns this object (const)</summary>
	/// <returns>RendermanMesh</returns>
	const RendermanMesh& backToMesh()const { return *rendermesh_; }
};

struct BufferEntrySemantic
{
	StructuredMemoryView* buffer;
	uint16 setId;
	int16 dynamicOffsetNodeId; //In the node's array of dynamic client id's, the actual offset. So, for each node, use dynamicClientIds[setId][dynamicOffsetNodeId]
	uint16 entryIndex;
};

struct UniformSemantic
{
	StringHash variablename;
	int32 uniformLocation;
	TypedMem memory;
};

typedef bool(*NodeSemanticSetter)(TypedMem& mem, const RendermanNode& node);
struct AutomaticNodeBufferEntrySemantic
{
	const StringHash* semantic;
	StructuredMemoryView* buffer;
	uint16 entryIndex;
	NodeSemanticSetter semanticSetFunc;
	uint16 setId;
	int16 dynamicOffsetNodeId;

	/// <summary>Return if the buffer is multibuffered</summary>
	/// <returns>true if the buffer is multibuffered, false otherwise</returns>
	bool multibuffered()
	{
		return buffer->getMultibufferCount() > 0;
	}

	/// <summary>Using the information if the buffer is multibuffered, modify the swapId to its correct value (its value
	/// if multibuffered, otherwise 0)</summary>
	/// <param name="swapId">The current swapchain index</param>
	/// <returns>0 if it is not multi buffered, else <paramref name="swapId"/></returns>
	uint32 calcSwapId(uint32 swapId)
	{
		return multibuffered() ? swapId : 0;
	}
};

struct AutomaticNodeUniformSemantic
{
	const StringHash* semantic;
	TypedMem* memory;
	NodeSemanticSetter semanticSetFunc;
};

struct AutomaticModelBufferEntrySemantic
{
	const StringHash* semantic;
	RendermanModel* model;
	StructuredMemoryView* buffer;
	uint16 entryIndex;
	ModelSemanticSetter semanticSetFunc;

	/// <summary>Check if the buffer is multibuffered</summary>
	/// <returns>true if the buffer is multibuffered, otherwise false</returns>
	bool multibuffered()
	{
		return buffer->getMultibufferCount() > 0;
	}

	/// <summary>Using the information if the buffer is multibuffered, modify the swapId to its correct value (its value
	/// if multibuffered, otherwise 0)</summary>
	/// <param name="swapId">The current swapchain index</param>
	/// <returns>0 if it is not multi buffered, else <paramref name="swapId"/></returns>
	uint32 calcSwapId(uint32 swapId)
	{
		return multibuffered() ? swapId : 0;
	}
};

struct AutomaticModelUniformSemantic
{
	const StringHash* semantic;
	RendermanModel* model;
	TypedMem* memory;
	ModelSemanticSetter semanticSetFunc;
};

//Part of RendermanStructure. This class matches everything together:
//A pipelineMaterial, with a RendermanMeshSubpass, to render.
//Unique per rendering node AND mesh bone batch combination. NOTE: if bone batching is used, then multiple nodes will
//be generated per meshnode.
//Contains a reference to the mesh, the material, and also contains the dynamic offsets required to render with it.
//Loop through those to render.
struct RendermanNode
{
	assets::NodeHandle assetNode;
	uint32 assetNodeId;
	RendermanSubpassMesh* subpassMesh_;
	RendermanMaterialSubpassPipeline* pipelineMaterial_;
	uint32 batchId;
	DynamicArray<uint32> dynamicClientId[4];
	DynamicArray<uint32> dynamicOffset[4];
	DynamicArray<RendermanBufferDefinition*> dynamicBuffer[4];
	pvr::ContiguousMap<StringHash, UniformSemantic> uniformSemantics;

	std::vector<AutomaticNodeBufferEntrySemantic> automaticEntrySemantics;
	std::vector<AutomaticNodeUniformSemantic> automaticUniformSemantics;

	/// <summary>Get a reference of the semantic of this node. The value is saved in a provided memory object.
	/// </summary>
	/// <param name="semantic">The semantic name to get the value of</param>
	/// <param name="memory">The semantic value is returned here</param>
	/// <returns>Return true if found, otherwise false</returns>
	bool getNodeSemantic(const StringHash& semantic, TypedMem& memory) const;

	/// <summary>Get the function object (NodeSemanticSetter) that will be used for a specific semantic</summary>
	/// <param name="semantic">A node-specific semantic name (WORLDMATRIX, BONECOUNT, BONEMATRIXARRAY0 etc.)
	/// </param>
	/// <returns>A NodeSemanticSetter function object that can be called to set this node semantic.</returns>
	NodeSemanticSetter getNodeSemanticSetter(const StringHash& semantic) const;

	/// <summary>Update the value of a semantic of this node</summary>
	/// <param name="semantic">The semantic's name</param>
	/// <param name="value">The value to set the semantic to</param>
	/// <param name="swapid">The swapchain id to set the value for. You should always pass the current swapchain index
	/// - even if the semantic is not in multibuffered storage, the case will be handled correctly.</param>
	/// <returns>Return true if successful.</returns>
	bool updateNodeValueSemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid);

	/// <summary>Iterates any per-node semantics, and updates their values to their automatic per-node values. In order for
	/// this function to work, createAutomaticSemantics needs to have been called before to create the connections of
	/// the automatic semantics.</summary>
	/// <param name="swapidx">The current swapchain (framebuffer) image.</param>
	void updateAutomaticSemantics(uint32 swapidx);

	/// <summary>Generates a list of all the semantics that this node's pipeline requires, that are defined per-node (e.g.
	/// MV/MVP matrices etc). Then, searches the connected asset node (model, mesh, material etc.) for these
	/// semantics, and creates connections between them so that when updateAutomaticSemantics is called, the updated
	/// values are updated in the semantics so that they can be read (with setUniformPtr, or updating buffers etc.).
	/// </summary>
	void createAutomaticSemantics();

	/// <summary>Get the commands necessary to render this node (bind pipeline, descriptor sets, draw commands etc.)
	/// Assumes correctly begun render passes, subpasses etc. All commands generated can be enabled/disabled in order
	/// to allow custom rendering.</summary>
	/// <param name="cbuff">A command buffer to record the commands into</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="recordBindPipeline">If set to false, do not generate any the bind pipeline command (use to
	/// optimize nodes rendered with the same pipelines)</param>
	/// <param name="recordBindDescriptorSets">If set to false, do not generate the bind descriptor sets commands
	/// (use to optimize nodes rendered with the same sets)</param>
	/// <param name="recordUpdateUniforms">If set to false, skip the generation of any update uniforms commands
	/// </param>
	/// <param name="recordBindVboIbo">If set to false, skip the generation of the bind vertex / index buffer
	/// commands (use to optimize nodes rendering the same mesh)</param>
	/// <param name="recordDrawCalls">If set to false, skip the generation of the draw calls.</param>
	void recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapIdx, bool recordBindPipeline = true,
	                             bool* recordBindDescriptorSets = NULL, bool recordUpdateUniforms = true,
	                             bool recordBindVboIbo = true, bool recordDrawCalls = true);

	/// <summary>Navigate(in the Rendering structure) to the RendermanPipeline object that is used by this node</summary>
	/// <returns>A reference to the pipeline object that is used by this node</returns>
	RendermanPipeline& toRendermanPipeline() { return *pipelineMaterial_->pipeline_; }

	/// <summary>Navigate(in the Rendering structure) to the RendermanPipeline object that is used by this node</summary>
	/// <returns>A reference to the pipeline object that is used by this node</returns>
	const RendermanPipeline& toRendermanPipeline() const { return *pipelineMaterial_->pipeline_; }

	/// <summary>Navigate(in the Rendering structure) to the RendermanMesh object that is used by this node</summary>
	/// <returns>A reference to the mesh object that is used by this node</returns>
	RendermanMesh& toRendermanMesh() { return *subpassMesh_->rendermesh_; }

	/// <summary>Navigate(in the Rendering structure) to the RendermanMesh object that is used by this node</summary>
	/// <returns>A reference to the mesh object that is used by this node</returns>
	const RendermanMesh& toRendermanMesh() const { return *subpassMesh_->rendermesh_; }
};
struct RendermanSubpassGroup;

/// <summary>Part of RendermanStructure. This class stores RendermanNodes and RendermanMaterialEffects The list of
/// nodes here references the list of materials. It references the Models in the original RendermanModelStore list.
/// </summary>
struct RendermanSubpassGroupModel
{
	RendermanSubpassGroup* renderSubpassGroup_;
	RendermanModel* renderModel_;
	std::deque<RendermanSubpassMesh> subpassMeshes; //STORAGE: Deque, so that we can insert elements without invalidating pointers.
	std::deque<RendermanSubpassMaterial> materialEffects;
	std::deque<RendermanNode> nodes;


	uint32 getNumRendermanNodes()const { return (uint32)nodes.size(); }

	const RendermanNode& toRendermanNode(uint32 index)const
	{
		debug_assertion(index < getNumRendermanNodes(), "RendermanNode index out of bound");
		return nodes[index];
	}

	RendermanNode& toRendermanNode(uint32 index)
	{
		debug_assertion(index < getNumRendermanNodes(), "RendermanNode index out of bound");
		return nodes[index];
	}

	/// <summary>Update the current frame in the used Model object (the call is forwarded as setCurrentFrame to the Model
	/// object)</summary>
	/// <param name="frame">The frame to set the Model object to.</param>
	void updateFrame(float32 frame);

	/// <summary>Get the commands necessary to render this SubpassModel. All calls are forwarded to the nodes of this
	/// model. Optimizes bind pipeline etc. calls between nodes. Assumes correctly begun render passes, subpasses etc.
	/// </summary>
	/// <param name="cbuff">A command buffer to record the commands into</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="recordUpdateUniforms">If set to false, skip the generation of any update uniforms commands
	/// </param>
	void recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapIdx, bool recordUpdateUniforms = true);

	/// <summary>Navigate(in the Rendering structure) to the RendermanModel object that is used by this</summary>
	/// <returns>A reference to the Renderman Model object that this object belongs to</returns>
	RendermanModel& backToModel();

	/// <summary>Navigate to the root RenderManager object</summary>
	/// <returns>A reference to the RenderManager</returns>
	RenderManager& backToRenderManager();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Subpass this object belongs to</summary>
	/// <returns>A reference to the Renderman Subpass this object belongs to</returns>
	RendermanSubpassGroup& backToRendermanSubpassGroup();

	RendermanSubpass& backToRendermanSubpass();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Pass this object belongs to</summary>
	/// <returns>A reference to the Renderman Pass this object belongs to</returns>
	RendermanPass& backToRendermanPass();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Effect this object belongs to</summary>
	/// <returns>A reference to the Renderman Effect this object belongs to</returns>
	RendermanEffect& backToRendermanEffect();


	/*!
	   \brief Generates a list of semantics that the pipeline requires, but are changed per-node.
	          Necessary to use updateAutomaticSemantics afterwards.
	 */
	void createAutomaticSemantics()
	{
		for (auto& node : nodes)
		{
			node.createAutomaticSemantics();
		}
	}
};

/// <summary>Part of RendermanStructure. This class is a cooked EffectPipeline, exactly mirroring the PFX
/// pipelines. It is affected on creation time by the meshes that use it (for the Vertex Input configuration) but
/// after that it is used for rendering directly when traversing the scene.</summary>
struct RendermanPipeline
{
	struct RendermanSubpassGroup* subpassGroup_;
	std::vector<RendermanSubpassMaterial*> subpassMaterials;
	api::GraphicsPipeline apiPipeline;
	utils::effect::PipelineDef* pipelineInfo;

	Multi<api::DescriptorSet> fixedDescSet[4];
	bool descSetIsFixed[4]; // If it is "fixed", it means that it is set by the PFX and no members of it are exported through semantics
	bool descSetIsMultibuffered[4]; // If it is "multibuffered", it means that it points to different buffers based on the swapchain index
	bool descSetExists[4]; // If it does not "exist", do not do anything for it...

	StringHash name;
	std::map<StringHash, RendermanBufferBinding> bufferBindings;

	std::map<StringHash, StructuredMemoryView*> bufferSemantics;
	std::map<StringHash, BufferEntrySemantic> bufferEntrySemantics;
	pvr::ContiguousMap<StringHash, UniformSemantic> uniformSemantics;

	std::vector<AutomaticModelBufferEntrySemantic> automaticModelBufferEntrySemantics;
	std::vector<AutomaticModelUniformSemantic> automaticModelUniformSemantics;

	/// <summary>Navigate (in the Rendering structure) to the Renderman Subpass this object belongs to</summary>
	/// <returns>A reference to the Renderman Subpass this object belongs to</returns>
	RendermanSubpassGroup& backToSubpassGroup();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Pass this object belongs to</summary>
	/// <returns>A reference to the Renderman Pass this object belongs to</returns>
	RendermanSubpass& backToSubpass();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Effect this object belongs to</summary>
	/// <returns>A reference to the Renderman Effect this object belongs to</returns>
	RendermanEffect& backToRendermanEffect();

	/// <summary>Generate Uniform updating commands (setUniformPtr) for all the uniform semantics of this pipeline,
	/// including Effect and Model semantics, but excluding node semantics. The commands will read the values from the
	/// built-in Semantics objects.</summary>
	/// <param name="cbuff">A command buffer to record the commands to</param>
	void recordUpdateAllUniformSemantics(api::CommandBufferBase cbuff);

	/// <summary>Generate Uniform updating commands (setUniformPtr) for all Effect scope uniform semantics The commands
	/// will read the values from the built-in Semantics objects.</summary>
	/// <param name="cbuff">A command buffer to record the commands to</param>
	void recordUpdateAllUniformEffectSemantics(api::CommandBufferBase cbuff);

	/// <summary>Generate Uniform updating commands (setUniformPtr) for all Model scope uniform semantics The commands will
	/// read the values from the built-in Semantics objects.</summary>
	/// <param name="cbuff">A command buffer to record the commands to</param>
	void recordUpdateAllUniformModelSemantics(api::CommandBufferBase cbuff);

	/// <summary>Generate Uniform updating commands (setUniformPtr) for a specific Node The commands will read the values
	/// from the built-in Semantics objects.</summary>
	/// <param name="cbuff">A command buffer to record the commands to</param>
	/// <param name="node">The node to record commands for</param>
	void recordUpdateAllUniformNodeSemantics(api::CommandBufferBase cbuff, RendermanNode& node);

	/// <summary>Generate Uniform update commands (setUniformPtr) for a specific Model semantic The commands will read the
	/// values from the built-in Semantics objects.</summary>
	/// <param name="cbuff">A command buffer to record the commands to</param>
	/// <param name="semantic">The Model semantic to update</param>
	/// <returns>true on success</returns>
	bool recordUpdateUniformCommandsModelSemantic(api::CommandBufferBase cbuff, const StringHash& semantic);

	/// <summary>Generate Uniform update commands (setUniformPtr) for a specific Effect semantic The commands will read the
	/// values from the built-in Semantics objects.</summary>
	/// <param name="cbuff">A command buffer to record the commands to</param>
	/// <param name="semantic">The effect semantic to update</param>
	/// <returns>Return true on success</returns>
	bool recordUpdateUniformCommandsEffectSemantic(api::CommandBufferBase cbuff, const StringHash& semantic);

	/// <summary>Generate Uniform update commands (setUniformPtr) for a specific Node semantic</summary>
	/// <param name="cbuff">A command buffer to record the commands to</param>
	/// <param name="semantic">The effect semantic to update</param>
	/// <param name="node">The node whose semantics to update</param>
	/// <returns>Return true on success</returns>
	bool recordUpdateUniformCommandsNodeSemantic(api::CommandBufferBase cbuff, const StringHash& semantic, RendermanNode& node);

	/// <summary>Update the value of a Model Uniform semantic (the updated value will be read when the corresponding
	/// recorded update command is executed).</summary>
	/// <param name="semantic">The Model semantic to update</param>
	/// <param name="value">The new value, contained in a TypedMem object.</param>
	/// <returns>true on success, false if the semantic is not found</returns>
	bool updateUniformModelSemantic(const StringHash& semantic, const TypedMem& value);

	/// <summary>Update the value of an Effect Uniform semantic (the updated value will be read when the corresponding
	/// recorded update command is executed).</summary>
	/// <param name="semantic">The Effect semantic to update</param>
	/// <param name="value">The new value, contained in a TypedMem object.</param>
	/// <returns>true on success, false if the semantic is not found</returns>
	bool updateUniformEffectSemantic(const StringHash& semantic, const TypedMem& value);

	/// <summary>Update the value of a per-Node Uniform semantic (the updated value will be read when the corresponding
	/// recorded update command is executed).</summary>
	/// <param name="semantic">The Node semantic to update</param>
	/// <param name="value">The new value, contained in a TypedMem object.</param>
	/// <param name="node">The Node for which to update the uniform</param>
	/// <returns>true on success, false if the semantic is not found</returns>
	bool updateUniformNodeSemantic(const StringHash& semantic, const TypedMem& value, RendermanNode& node);

	/// <summary>Update the value of a per-Model Buffer Entry semantic. The value is updated immediately in the
	/// corresponding buffer.</summary>
	/// <param name="semantic">The Model semantic to update</param>
	/// <param name="value">The new value to set</param>
	/// <param name="swapid">The current swapchain index</param>
	/// <param name="dynamicClientId">(Optional) In the case of a Dynamic buffer, the "dynamic client id" is the index of the
	/// "slice" of the buffer. Built-in functionality does NOT use this parameter for Model semantics. Default 0.
	/// </param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntryModelSemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid,
	                                    uint32 dynamicClientId = 0);

	/// <summary>Update the value of multiple per-Effect Buffer Entry semantics. The values are updated immediately
	/// in the corresponding buffer.</summary>
	/// <param name="semantics">The Effect semantic to update</param>
	/// <param name="values">The new values to set</param>
	/// <param name="numSemantics">The number of semantics to set.</param>
	/// <param name="swapid">The current swapchain index</param>
	/// <param name="dynamicClientId">(Optional) In the case of a Dynamic buffer, the "dynamic client id" is the index of the
	/// "slice" of the buffer. Built-in functionality does NOT use this parameter for Effect semantics. Default 0.
	/// </param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntryModelSemantics(const StringHash* semantics, const FreeValue* values, uint32 numSemantics,
	                                     uint32 swapid, uint32 dynamicClientId);


	/*!
	   \brief Update buffer entry effect semantic
	   \param semantic Effect semantic to update
	   \param value New value
	   \param swapid swapchain id
	   \param dynamicClientId
	   \return  Return true on success
	 */
	bool updateBufferEntryEffectSemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid, uint32 dynamicClientId = 0);

	/// <summary>Update the value of a per-Effect or Per-Model Buffer Entry semantic. The value is updated immediately in
	/// the corresponding buffer.</summary>
	/// <param name="semantic">The Effect or Model semantic to update</param>
	/// <param name="value">The new value to set</param>
	/// <param name="swapid">The current swapchain index</param>
	/// <param name="dynamicClientId">(Optional) In the case of a Dynamic buffer, the "dynamic client id" is the index of the
	/// "slice" of the buffer. Built-in functionality does NOT use this parameter for either Effect or Model
	/// semantics. Default 0.</param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntrySemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid, uint32 dynamicClientId = 0);

	/// <summary>Update the value of a per-Node Buffer Entry semantic. The value is updated immediately in the
	/// corresponding buffer. The dynamic client id of the buffer (i.e. the Offset into the dynamic buffer) is
	/// automatically retrieved from the Node.</summary>
	/// <param name="semantic">The Node semantic to update</param>
	/// <param name="value">The new value to set</param>
	/// <param name="swapid">The current swapchain index</param>
	/// <param name="node">The RendermanNode for which to set the value.</param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntryNodeSemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid, RendermanNode& node);


	bool updateBufferEntryNodeSemantics(const StringHash* semantics, const FreeValue* values, uint32 numSemantics,
	                                    uint32 swapid, RendermanNode& node)
	{
		bool result = true;
		for (uint32 i = 0; i < numSemantics; ++i)
		{
			result = result && updateBufferEntryNodeSemantic(semantics[i], values[i], swapid, node);
		}
		return result;
	}

	/// <summary>Update the value of multiple per-Effect Buffer Entry semantic. The values is updated immediately in the
	/// corresponding buffer. The dynamic client id of the buffer (i.e. the Offset into the dynamic buffer) is
	/// automatically retrieved from the Node.</summary>
	/// <param name="semantics">A c-style array of Semantic names. Must point to at least
	/// <paramref name="numSemantics"/>elements</param>
	/// <param name="values">A c-style array of the Values to set. Must point to at least
	/// <paramref name="numSemantics"/>elements</param>
	/// <param name="numSemantics">The number of semantics to set</param>
	/// <param name="swapid">The current swapchain index</param>
	/// <param name="dynamicClientId">(Optional) In the case of a Dynamic buffer, the "dynamic client id" is the index of the
	/// "slice" of the buffer. Built-in functionality does NOT use this parameter for either Effect or Model
	/// semantics. Default 0.</param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntryEffectSemantics(const StringHash* semantics, const FreeValue* values, uint32 numSemantics,
	                                      uint32 swapid, uint32 dynamicClientId = 0);

	/// <summary>Generates a list of all the semantics that this pipeline requires, that are defined per-Model (e.g. V/VP
	/// matrices, light positions etc). Then, searches the connected asset Model for these semantics, and creates
	/// connections between them so that when updateAutomaticSemantics is called, the new values are updated in the
	/// semantics so that they can be read (with setUniformPtr, or updating buffers etc.).</summary>
	/// <param name="useMainModelId">The model ID to use to read the values from. Default O (the first model).
	/// </param>
	/// <returns>true if successful, false on any error</returns>
	bool createAutomaticModelSemantics(uint32 useMainModelId = 0);

	/// <summary>Update the value of all automatic per-Model semantics. The values are updated immediately in the
	/// corresponding buffer, and where the Uniform values are located, but Uniform values will only be visible in
	/// rendering when the recorded update uniform commands are executed (i.e. the commands generated by
	/// recordUpdateUniformCommandsXXXXX)</summary>
	/// <param name="swapIdx">The swapchain index to generate commands for (ignored for Uniforms)</param>
	/// <returns>Return true on success, false on any error</returns>
	bool updateAutomaticModelSemantics(uint32 swapIdx);
};

struct RendermanSubpassGroup
{
	StringHash name;
	RendermanSubpass* subpass_;
	std::deque<RendermanPipeline> pipelines;
	std::deque<RendermanSubpassGroupModel> subpassGroupModels;
	std::deque<RendermanModel*> allModels;
	const RendermanSubpass& backToSubpass()const;


	uint32 getNumSubpassGroupModels()const { return (uint32)subpassGroupModels.size(); }

	const RendermanSubpassGroupModel& toSubpassGroupModel(uint32 model)const
	{
		debug_assertion(model < subpassGroupModels.size(), "Model index out of bound");
		return subpassGroupModels[model];
	}

	RendermanSubpassGroupModel& toSubpassGroupModel(uint32 model)
	{
		debug_assertion(model < subpassGroupModels.size(), "Model index out of bound");
		return subpassGroupModels[model];
	}

	void updateAllModelsFrame(float32 frame)
	{
		for (auto groupModels : subpassGroupModels)
		{
			groupModels.updateFrame(frame);
		}
	}

	RendermanPipeline& toRendermanPipeline(uint32 pipeline)
	{
		debug_assertion(pipeline < pipelines.size(), "Pipeline index out of bound");
		return pipelines[pipeline];
	}


	const RendermanPipeline& toRendermanPipeline(uint32 pipeline)const
	{
		debug_assertion(pipeline < pipelines.size(), "Pipeline index out of bound");
		return pipelines[pipeline];
	}


	RendermanSubpass& backToSubpass();

	/*!
	     \brief Record rendering commands for this subpass
	     \param cbuff Recording Commandbuffer
	     \param swapIdx Swapchain index
	     \param recordUpdateUniforms
	   */
	void recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapIdx, bool recordUpdateUniforms = true);



	/*!
	\brief Generates a list of semantics that the pipeline requires, but are changed per-node.
	Necessary to use updateAutomaticSemantics afterwards.
	*/
	void createAutomaticSemantics()
	{
		for (RendermanSubpassGroupModel& subpassModel : subpassGroupModels)
		{
			for (RendermanNode& node : subpassModel.nodes)
			{
				node.createAutomaticSemantics();
			}
		}
		for (RendermanPipeline& pipe : pipelines)
		{
			pipe.createAutomaticModelSemantics();
		}
	}


	/*
	\brief Part of RendermanStructure. This class contains the different pipelines, exactly mirroring the PFX subpass
	with the different models. Contained in the RendermanPass objects
	*/
	void updateAutomaticSemantics(uint32 swapidx)
	{
		for (RendermanSubpassGroupModel& subpassModel : subpassGroupModels)
		{
			for (RendermanPipeline& pipe : pipelines)
			{
				pipe.updateAutomaticModelSemantics(swapidx);
			}
			for (RendermanNode& node : subpassModel.nodes)
			{
				node.updateAutomaticSemantics(swapidx);
			}
		}
	}

};


struct RendermanSubpass
{
	RendermanPass* renderingPass_;
	std::deque<RendermanSubpassGroup> groups;
	/// <summary>Return the RendermanPass to which this object belongs (const).</summary>
	/// <returns>The RendermanPass to which this object belongs (const).</returns>
	const RendermanPass& backToRendermanPass()const { return *renderingPass_; }

	/// <summary>Return the RendermanPass to which this object belongs.</summary>
	/// <returns>The RendermanPass to which this object belongs (const).</returns>
	RendermanPass& backToRendermanPass() {  return *renderingPass_; }

	/// <summary>Return the RendermanEffect to which this object belongs. 2 lvls:Pass->Effect.</summary>
	/// <returns>The RendermanEffect to which this object belongs (const).</returns>
	const RendermanEffect& backToRendermanEffect()const;

	/// <summary>Return the RendermanEffect to which this object belongs. 2 lvls:Pass->Effect.</summary>
	/// <returns>The RendermanEffect to which this object belongs (const).</returns>
	RendermanEffect& backToRendermanEffect();

	/// <summary>Return the RenderManager which this object belongs to (const).</summary>
	/// <returns>RenderManager</returns>
	const RenderManager& backToRenderManager()const;

	const RendermanSubpassGroup& toSubpassGroup(uint32 index)const
	{
		debug_assertion(index < groups.size(), "Subpass group index out of bound");
		return groups[index];
	}

	RendermanSubpassGroup& toSubpassGroup(uint32 index)
	{
		debug_assertion(index < groups.size(), "Subpass group index out of bound");
		return groups[index];
	}

	/// <summary>Return the RendermanManager to which this object belongs. 3 lvls:Pass->Effect->RenderManager.</summary>
	/// <returns>The RendermanManager to which this object belongs.</returns>
	RenderManager& backToRenderManager();

	/// <summary>Get the commands necessary to render this entire Subpass (for each node, bind pipeline, descriptor
	/// sets, draw commands etc.) Calls are forwarded to the respective rendering nodes. Assumes correctly begun
	/// render passes, and (if necessary) any nextSubpass calls already recorded.</summary>
	/// <param name="cbuff">A command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="recordUpdateUniforms">If set to false, skip the generation of any update uniforms commands.
	/// </param>
	/// <remarks>If you need to create a SecondaryCommandBuffer for this subpass, use this overload.</remarks>
	void recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapIdx, bool recordUpdateUniforms = true)
	{
		for (auto& group : groups)
		{
			group.recordRenderingCommands(cbuff, swapIdx, recordUpdateUniforms);
		}
	}


	/// <summary>Get the commands necessary to render this entire Subpass (for each node, bind pipeline, descriptor
	/// sets, draw commands etc.) into a Primary command buffer (not secondary command buffer) Allows to configure if
	/// the nextSubpass commands will be recorded, and if the</summary>
	/// <param name="cbuff">A command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="beginWithNextSubpassCommand">Record a nextSubpassInline() command at the beginning of this function.
	/// </param>
	/// <param name="recordUpdateUniforms">If set to false, skip the generation of any update uniforms commands.
	/// </param>
	/// <remarks>This overload can only be used if you need to create a Primary command buffer. Use the other overload
	/// of this function to create secondary command buffers.</remarks>
	void recordRenderingCommands(api::CommandBuffer& cbuff, uint16 swapIdx, bool beginWithNextSubpassCommand,
	                             bool recordUpdateUniforms = true)
	{
		if (beginWithNextSubpassCommand)
		{
			cbuff->nextSubPassInline();
		}

		api::CommandBufferBase base(cbuff);

		recordRenderingCommands(base, swapIdx, recordUpdateUniforms);
	}


	/// <summary>Generates all semantic list connections for all subobjects of this subpass (pipelines, nodes) by
	/// recursively calling createAutomaticSemantics on nodes and pipelines of this subpass. This function must have
	/// been called to be able to call updateAutomaticSemantics afterwards.</summary>
	void createAutomaticSemantics()
	{
		for (auto& group : groups)
		{
			group.createAutomaticSemantics();
		}
	}


	/// <summary>Iterates all the nodes semantics per-pipeline and per-model, per-node, and updates their values to their
	/// specific per-node values. createAutomaticSemantics must have been called before.</summary>
	/// <param name="swapidx">swapchain index</param>
	void updateAutomaticSemantics(uint32 swapidx)
	{
		for (auto& group : groups)
		{
			group.updateAutomaticSemantics(swapidx);
		}
	}

	/*!
	   \brief Return number groups in this subpass
	   \return
	 */
	uint32 getNumSubpassGroups()const { return (uint32)groups.size(); }
};


/*
\brief Part of RendermanStructure. This class contains the different subpasses, exactly mirroring the PFX pass.
*/
struct RendermanPass
{
	api::FboSet fbo;
	RendermanEffect* renderEffect_;
	std::deque<RendermanSubpass> subpasses;


	const RendermanEffect& backToEffect()const { return *renderEffect_ ; }

	RendermanEffect& backToEffect() { return *renderEffect_ ; }

	/// <summary>Get the commands necessary to render this entire Pass (for each subpass, for each node, bind
	/// pipeline, descriptor sets, draw commands etc.) into a Primary command buffer (secondary command buffers cannot
	/// be used at this level) Allows to configure if the begin/end renderpass and/or updateUniform commands will be
	/// recorded</summary>
	/// <param name="cbuff">A Primary command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="beginEndRendermanPass">If set to true, record a beginRenderPass() at the beginning and
	/// endRenderPass() at the end of this function. If loadOp is "clear". If the loadop is "clear", the first Model's
	/// clear colour will be used.</param>
	/// <param name="recordUpdateUniforms">If set to false, skip the generation of any update uniforms commands.
	/// </param>
	void recordRenderingCommands(api::CommandBuffer& cbuff, uint16 swapIdx, bool beginEndRendermanPass, bool recordUpdateUniforms = true);

	/// <summary>Get the commands necessary to render this entire Pass (for each subpass, for each node, bind
	/// pipeline, descriptor sets, draw commands etc.) into a Primary command buffer (secondary command buffers cannot
	/// be used at this level) Will call begin/end renderpass commands at start and finish, and allows to explicitly
	/// specify the clear color.</summary>
	/// <param name="cbuff">A Primary command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="clearColor">The color to which to clear the framebuffer. Ignored if loadop is not clear.</param>
	/// <param name="recordUpdateUniforms">If set to false, skip the generation of any update uniforms commands.
	/// </param>
	void recordRenderingCommandsWithClearColor(api::CommandBuffer& cbuff, uint16 swapIdx, const glm::vec4& clearColor = glm::vec4(.0f, 0.0f, 0.0f, 1.0f), bool recordUpdateUniforms = true)
	{
		recordRenderingCommands_(cbuff, swapIdx, recordUpdateUniforms, &clearColor);
	}

	/// <summary>Generates all semantic list connections for all subobjects of this pass (subpasses->pipelines, nodes) by
	/// recursively calling createAutomaticSemantics on subpasses'pipelines of this subpass. This function must have
	/// been called to be able to call updateAutomaticSemantics afterwards.</summary>
	void createAutomaticSemantics()
	{
		for (RendermanSubpass& subpass : subpasses)
		{
			subpass.createAutomaticSemantics();
		}
	}


	/// <summary>Iterates all the nodes semantics per-effect, per-pass, per-subpass, per-model, per-node, and updates their
	/// values to the ones retrieved by the Model (model,mesh,node,material) retrieved values.
	/// createAutomaticSemantics needs to have been called before.</summary>
	/// <param name="swapidx">The current swapchain index (used to select which buffer to update in case of
	/// multibuffering. Multi-buffers and non multi-buffers are handled automatically, so always pass the current
	/// value).</param>
	void updateAutomaticSemantics(uint32 swapidx)
	{
		for (RendermanSubpass& subpass : subpasses)
		{
			subpass.updateAutomaticSemantics(swapidx);
		}
	}

	const api::Fbo& getFbo(uint32 swapIndex)const { return fbo[swapIndex]; }

	/// <summary>Navigate to a subpass object of this pass (const)</summary>
	/// <param name="subpass">The Subpass index (its order of appearance in the pass)</param>
	/// <returns>The subpass index. There is always at least one subpass per pass.</returns>
	/// <remarks>It is undefined behaviour to pass an index that does not exist.</remarks>
	const RendermanSubpass& toRendermanSubpass(uint16 subpass)const
	{
		assertion(subpass < subpasses.size(), "Subpass index out of bound");
		return subpasses[subpass];
	}

	/// <summary>Navigate to a subpass object of this pass (const)</summary>
	/// <param name="subpass">The Subpass index (its order of appearance in the pass). It is undefined behaviour to
	/// pass an index that does not exist.</param>
	/// <returns>The Subpass.</returns>
	/// <remarks>There is always at least one subpass per pass, even if no subpasses have been defined in the PFX.It is
	/// undefined behaviour to pass an index that does not exist.</remarks>
	RendermanSubpass& toRendermanSubpass(uint16 subpass)
	{
		assertion(subpass < subpasses.size(), "Subpass index out of bound");
		return subpasses[subpass];
	}


	uint32 getNumSubpass()const { return (uint32)subpasses.size(); }

private:
	void recordRenderingCommands_(api::CommandBuffer& cbuff, uint16 swapIdx, bool recordUpdateUniforms, const glm::vec4* clearColor);
};

/// <summary>Part of RendermanStructure. This class contains the different passes, exactly mirroring the PFX
/// effect. Contains the original EffectApi.</summary>
struct RendermanEffect
{
	RenderManager* manager_;
	std::deque<RendermanPass> passes;
	std::deque<RendermanBufferDefinition> bufferDefinitions;

	std::map<StringHash, StructuredMemoryView*> bufferSemantics;
	std::map<StringHash, BufferEntrySemantic> bufferEntrySemantics;
	std::map<StringHash, UniformSemantic> uniformSemantics;
	bool isUpdating[4];
	utils::effect::EffectApi effect;

	/// <summary>Constructor</summary>
	RendermanEffect() { memset(isUpdating, 0, sizeof(isUpdating)); }

	/// <summary>Return the RenderManager which owns this object (const)</summary>
	/// <returns>The root RenderManager object</returns>
	const RenderManager& backToRenderManager() const;

	/// <summary>Return the RenderManager which owns this object (const)</summary>
	/// <returns>The root RenderManager object</returns>
	RenderManager& backToRenderManager();

	/// <summary>Signify that there will be a batch of (buffer) updates, so that any possible buffers that are updated are
	/// only mapped and unmapped once. If you do NOT call this method before updating, the buffers will be mapped and
	/// unmapped with every single operation.</summary>
	/// <param name="swapChainIndex">The swapchain index to be updated.</param>
	/// <remarks>Unless really doing a one-off operation, always call this function before calling any of the
	/// updateAutomaticSemantics and similar operations</remarks>
	void beginBufferUpdates(uint32 swapChainIndex) { isUpdating[swapChainIndex] = true; }

	/// <summary>Signify that the updates for the specified swapchain index have finished. Any and all mapped buffers will
	/// now be unmapped (once). If you do not call this function after you have called beginBufferUpdates, the data in
	/// your buffers are undefined.</summary>
	/// <param name="swapChainIndex">The swapchain index to be updated.</param>
	void endBufferUpdates(uint32 swapChainIndex);

	/// <summary>Get the commands necessary to render this entire Effect (for pass, subpass, node: bind pipeline,
	/// descriptor sets, draw commands etc.) into a Primary command buffer (secondary command buffers cannot be used
	/// at this level)</summary>
	/// <param name="cbuff">A Primary command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="beginEnderRenderPasses">If set to False, the beginRenderPass and endRenderPass commands will not
	/// be recorded</param>
	/// <param name="recordUpdateUniforms">If set to false, skip the generation of any update uniforms commands.
	/// </param>
	void recordRenderingCommands(api::CommandBuffer& cbuff, uint16 swapIdx, bool beginEnderRenderPasses, bool recordUpdateUniforms = true);

	/// <summary>Generates all semantic list connections for all subobjects of this effect (passes->subpasses->pipelines,
	/// nodes) by recursively calling createAutomaticSemantics on all passes. This function must have been called to
	/// be able to call updateAutomaticSemantics afterwards.</summary>
	void createAutomaticSemantics()
	{
		for (RendermanPass& pass : passes)
		{
			pass.createAutomaticSemantics();
		}
	}


	/// <summary>Iterates all the nodes semantics per-effect, per-pass, per-subpass, per-model, per-node, and updates their
	/// values to their updated per-node values. CreateAutomaticSemantics must have been called otherwise no semantics
	/// will have been generated.</summary>
	/// <param name="swapidx">The current swap chain (framebuffer image) index to record commands for.</param>
	void updateAutomaticSemantics(uint32 swapidx)
	{
		bool wasUpdating = isUpdating[swapidx];
		if (!wasUpdating) // Optimization - avoid multiple map/unmap. But only if the user has not taken care of it.
		{
			beginBufferUpdates(swapidx);
		}
		for (RendermanPass& pass : passes)
		{
			pass.updateAutomaticSemantics(swapidx);
		}
		if (!wasUpdating) // If it was not mapped, unmap it. Otherwise leave it alone...
		{
			endBufferUpdates(swapidx);
		}
	}

	/// <summary>Navigate to a RendermanPass object of this effect by the pass ID</summary>
	/// <param name="toPass">The Pass index (its order of appearance in the pass)</param>
	/// <returns>The RendermanPass object with id <paramref>toPass</paramref>.
	/// There is always at least one subpass per pass.</returns>
	/// <remarks>It is undefined behaviour to pass an index that does not exist.</remarks>
	RendermanPass& toRendermanPass(uint16 toPass) { return passes[toPass]; }

	/// <summary>Navigate to a RendermanPass object of this effect by the pass ID</summary>
	/// <param name="toPass">The Pass index (its order of appearance in the pass)</param>
	/// <returns>The RendermanPass object with id <paramref>toPass</paramref>.
	/// There is always at least one subpass per pass.</returns>
	/// <remarks>It is undefined behaviour to pass an index that does not exist.</remarks>
	const RendermanPass& toRendermanPass(uint16 toPass)const { return passes[toPass]; }
};

struct RendermanStructure
{
	std::deque<RendermanEffect> effects;
};


//The RendermanStructure. This class contains all the different effects that have been added.
//RendermanEffect[]
//	map<StringHash, StructuredMemoryView> effectBuffers
//	EffectApi
//	RendermanPass[]
//		RendermanSubpass[]
//			RendermanModel*[]
//			RendermanPipeline[]
//				api::GraphicsPipeline
//				effect::PipelineDef*
//				RendermanSubpassModel[]
//					map<stringHash, PerBoneBatchBuffers*>
//					map<stringHash, PerModelBuffers*>
//					RendermanSubpassMaterial[]
//					RendermanModel*
//					RendermanNode[]
//						assets::Node*
//						RendermanMesh*
//						RendermanSubpassMaterial*
//						dynamicOffsets
//					RendermanMaterialEffect[]
//						RendermanSubpassModel*
//						RendermanMaterial*
//						DescriptorSets


//RendermanModel[]
//	RendermanMaterial[]
//		assetMaterial
//		textures[]
//	RendermanMesh[]
//		assetMesh
//		vbos[]
//		ibo[]
//		indexType



/// <summary>The RenderManager is a rendering automation class, with class responsibilities such as: - Putting
/// together PFX files (Effects) with POD models (Models) to render - Creating Graphics Pipelines, Descriptor Sets,
/// VBOs, IBOs, UBOs, etc. - Creating and configuring render - to - texture targets - Automatically generate
/// command buffers for rendering - Automatically update textures/uniforms/buffers in the rendering api with info
/// provided by the the model (textures, matrices etc)</summary>
/// <remarks>Basic use: Create a RenderManager object Add Effects to it (usually one) : addEffect(...) Add Models to
/// specific parts of the Effect (Normally, a model is added to be rendered by a specific subpass
/// (addModelToSubpass), but shortcut methods are provided to add it to entire renderpasses, or even all
/// renderpasses) Cook the RenderManager : buildRenderObjects(...) Get rendering commands in command buffers :
/// recordRenderingCommands(...) (For complete automation) : createAutomaticSemantics(...) For each frame:
/// updateAutomaticSemantics(...) submitCommandBuffer(..) Semantics are "slots" where pieces of information can be
/// put to renders. For example, a "DIFFUSETEXTURE" semantic may exist where a texture must be added in order to
/// function as the diffuse texture for a shader, or a "MVP" semantic may exist where a matrix must be uploaded for
/// the vertex transformation. Automatic semantics are "connections" where this information will be automatically
/// retrieved from the Model (the scene object). It is important to realize that semantics exist on different parts
/// of the object, and are expected to be updated with different rates. A) Effect - things like, for example, the
/// clear color, that may be common among all objects that use an effect. B) Model - similar to the effect, it is
/// common for an entire model. Might be the Projection matrix or an ambient colour. C) Node - things that are
/// specific to an object. Commonly exist on dynamic buffers. Things like the MVP matrix or Textures. PFX Bonebatch
/// scope items also end up in nodes (as one node is generated per bonebatch). Some things to look out for: The
/// final "renderable" is the Node. Each nodes carries enough information (either directly or through pointers to
/// "outer" object to render itself. One Node is created for each bonebatch of a node of a model There are many
/// different intermediate objects to avoid duplications, and different parts of storage items around the objects.
/// The distinct phases that can be discerned are: - Setup (adding Effect(s) and Model(s)) - Object generation
/// (buildRenderObjects()) - Command generation (recordCommandBuffers()) - Memory updates (updateSemantics,
/// updateAutomaticSemantics)</remarks>
class RenderManager
{
	friend class RenderManagerNodeIterator;
public:
	typedef RendermanNode Renderable;

	/// <summary>A special iterator class that is used to iterate through ALL renderable objects (nodes) of the
	/// entire render manager. Unidirectional, sequential. Provides methods to know when the pass, subpass or pipeline
	/// changed with the last advance. The effect of iterating with this class is identical to iterating for each
	/// pass, each subpass, each subpassmodel, each node.</summary>
	struct RendermanNodeIterator
	{
		friend class RenderManager;
	private:
		RenderManager& mgr;
		RendermanNode* cached;
		uint32 nodeId;
		uint32 subpassModelId;
		uint32 subpassId;
		uint32 subpassGroupId;
		uint32 passId;
		uint32 effectId;
		bool passChanged_;
		bool subpassChanged_;
		bool pipelineChanged_;
		bool subpassGroupChanged_;
		api::GraphicsPipeline::ElementType* pipeline;

		RendermanNodeIterator(RenderManager& mgr, bool begin) : mgr(mgr), nodeId(0), subpassModelId(0),
			subpassGroupId(0), subpassId(0), passId(0), pipeline(NULL)
		{
			effectId = begin ? 0 : (uint32)mgr.renderObjects().effects.size();
			cached = begin ? &mgr.toSubpassGroupModel(0, 0, 0, 0, 0).nodes[0] : NULL;
		}

		void advanceNode(RendermanEffect& eff, RendermanPass& pass, RendermanSubpass& spass, RendermanSubpassGroup& group,
		                 RendermanSubpassGroupModel& spmodel)
		{
			api::GraphicsPipeline::ElementType* old_pipeline = cached->pipelineMaterial_->pipeline_->apiPipeline.get();
			subpassChanged_ = false;
			passChanged_ = false;
			if (++nodeId == spmodel.nodes.size())
			{
				nodeId = 0;
				advanceModelEffect(eff, pass, spass, group);
			}
			else
			{
				cached = &spmodel.nodes[nodeId];
			}
			pipelineChanged_ = (old_pipeline == cached->pipelineMaterial_->pipeline_->apiPipeline.get());
		}

		void advanceModelEffect(RendermanEffect& eff, RendermanPass& pass, RendermanSubpass& spass, RendermanSubpassGroup& group)
		{
			if (++subpassModelId == group.getNumSubpassGroupModels())
			{
				subpassModelId = 0;
				advanceSubpassGroup(eff, pass, spass);
			}
			else
			{
				cached = &group.toSubpassGroupModel(subpassModelId).toRendermanNode(0);
			}
		}
		void advanceSubpass(RendermanEffect& eff, RendermanPass& pass)
		{
			subpassChanged_ = true;
			if (++subpassId == pass.subpasses.size())
			{
				subpassId = 0;
				advancePass(eff);
			}
			else
			{
				cached = &pass.toRendermanSubpass(subpassId).toSubpassGroup(subpassGroupId).toSubpassGroupModel(0).toRendermanNode(0);
			}
		}

		void advanceSubpassGroup(RendermanEffect& effect, RendermanPass& pass, RendermanSubpass& subpass)
		{
			subpassGroupChanged_ = true;
			if (++subpassGroupId == subpass.groups.size())
			{
				subpassGroupId = 0;
				advanceSubpass(effect, pass);
			}
			else
			{
				debug_assertion(subpass.toSubpassGroup(subpassGroupId).getNumSubpassGroupModels() != 0 &&
				                subpass.toSubpassGroup(subpassGroupId).toSubpassGroupModel(0).getNumRendermanNodes() != 0,
				                "Subpassgroup must have atleast one model and a model node");

				cached = &subpass.toSubpassGroup(subpassGroupId).toSubpassGroupModel(0).toRendermanNode(0);
			}
		}

		void advancePass(RendermanEffect& eff)
		{
			passChanged_ = true;
			if (++passId == eff.passes.size())
			{
				passId = 0;
				advanceEffect();
			}
			cached = &eff.toRendermanPass(passId).toRendermanSubpass(0).toSubpassGroup(0).toSubpassGroupModel(0).toRendermanNode(0);
		}
		void advanceEffect()
		{
			if (++effectId == mgr.renderObjects().effects.size())
			{
				cached = NULL;
			}
			else
			{
				cached = &mgr.renderObjects().effects[effectId].toRendermanPass(0).toRendermanSubpass(0).toSubpassGroup(0)
				         .toSubpassGroupModel(0).toRendermanNode(0);
			}
		}
	public:
		/// <summary>Return true if we moved to a new subpass during the last call to operator ++</summary>
		bool passChanged() const { return passChanged_; }

		/// <summary>Return true if we moved to a new subpass during the last call to operator ++</summary>
		bool subpassChanged() const { return subpassChanged_; }

		/// <summary>Return true if the current node is being rendered with a different pipeline than the previous</summary>
		bool pipelineChanged() const { return pipelineChanged_; }

		bool operator==(RendermanNodeIterator& rhs) const
		{
			return (effectId == rhs.effectId) &&
			       (passId == rhs.passId) &&
			       (subpassId == rhs.subpassId) &&
			       (subpassModelId == rhs.subpassModelId) &&
			       (nodeId == rhs.nodeId);
		}

		bool operator!=(RendermanNodeIterator& rhs) const
		{
			return (effectId != rhs.effectId) ||
			       (passId != rhs.passId) ||
			       (subpassId != rhs.subpassId) ||
			       (subpassModelId != rhs.subpassModelId) ||
			       (nodeId != rhs.nodeId);
		}

		RendermanNode& operator*()
		{
			return *cached;
		}

		RendermanNode* operator->()
		{
			return cached;
		}

		RendermanNodeIterator operator++(int)
		{
			RendermanNodeIterator cpy(*this);
			++(*this);
			return cpy;
		}

		RendermanNodeIterator& operator++()
		{
			auto& effect = mgr.toEffect(effectId);
			auto& pass = effect.toRendermanPass(passId);
			auto& subpass = pass.toRendermanSubpass(subpassId);
			auto& subpassGroup = subpass.toSubpassGroup(subpassGroupId);
			auto& spmodel = subpassGroup.toSubpassGroupModel(subpassModelId);
			advanceNode(effect, pass, subpass, subpassGroup, spmodel);
			return *this;
		}

	};

	/// <summary>This class is a dummy container that provides begin() and end() methods to iterate through all nodes
	/// of the RenderManager. Extremely useful to use in C++11 range based for: for (auto& node :
	/// renderManager.renderables())</summary>
	struct RenderManagerNodeIteratorAdapter
	{

		/// <summary>Returns an iterator pointing to the first RendermanNode element</summary>
		/// <returns>RendermanNodeIterator</returns>
		RendermanNodeIterator begin() const
		{
			return RendermanNodeIterator(mgr, true);
		}

		/// <summary>Returns an iterator pointing to the first RendermanNode element</summary>
		/// <returns>RendermanNodeIterator</returns>
		RendermanNodeIterator end() const
		{
			return RendermanNodeIterator(mgr, false);
		}
	private:
		friend class RenderManager;
		RenderManager& mgr;
		RenderManagerNodeIteratorAdapter(RenderManager& mgr) : mgr(mgr) {}
	};

private:
	typedef std::deque<RendermanModel> RendermanModelStorage;
	// effect / pass / subpass
	GraphicsContext context;
	RendermanStructure renderStructure;
	RendermanModelStorage modelStorage; //STORAGE: Deque, so that we can insert elements without invalidating pointers.

	std::map<assets::Mesh*, DynamicArray<AttributeLayout>*> meshAttributeLayout; //points to finalPipeAttributeLayouts
public:
	/// <summary>Constructor. Creates an empty rendermanager. In order to use it, you need to addEffect() and addModel() to
	/// populate it, then buildRenderObjects(), then createAutomaticSemantics(), generate</summary>
	RenderManager() {}

	/// <summary>This method provides a class that functions as a "virtual" node container. Its sole purpose is
	/// providin begin() and end() methods that iterate through all nodes of the RenderManager. Extremely useful to
	/// use in C++11 range based for: for (auto& node : renderManager.renderables())</summary>
	RenderManagerNodeIteratorAdapter renderables()
	{
		return RenderManagerNodeIteratorAdapter(*this);
	}

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Effect object with index 'effect'</summary>
	/// <param name="effect">The index of the effect to navigate to. The index is the order with which it was added to
	/// the RenderManager.</param>
	/// <returns>The RendermanEffect object with index 'effect'.</returns>
	RendermanEffect& toEffect(uint32 effect) { return renderStructure.effects[effect]; }

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Pass object with index 'pass' in effect with
	/// index 'effect'.</summary>
	/// <param name="effect">The index of the effect the object belongs to. The index is the order with which it was
	/// added to the RenderManager.</param>
	/// <param name="pass">The index of the pass within 'effect' that the object belongs to. The index is the order of
	/// the pass in the effect.</param>
	/// <returns>The RendermanPass object with index 'pass' in effect 'effect'.</returns>
	RendermanPass& toPass(uint32 effect, uint32 pass) {  return toEffect(effect).toRendermanPass(pass); }

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Subpass object #'subpass' in pass #'pass' in
	/// effect #'effect'.</summary>
	/// <param name="effect">The index of the effect the object belongs to. The index is the order with which it was
	/// added to the RenderManager.</param>
	/// <param name="pass">The index of the pass within 'effect' that the object belongs to. The index is the order of
	/// the pass in the effect.</param>
	/// <param name="subpass">The index of the subpass within 'pass' that the object belongs to. The index is the
	/// order of the subpass in the pass.</param>
	/// <returns>The RendermanSubpass object with index 'subpass' in pass 'pass' of effect 'effect'.</returns>
	RendermanSubpass& toSubpass(uint32 effect, uint32 pass, uint32 subpass)
	{
		return toPass(effect, pass).toRendermanSubpass(subpass);
	}

	RendermanSubpassGroup& toSubpassGroup(uint32 effect, uint32 pass, uint32 subpass, uint32 subpassGroup)
	{
		return toSubpass(effect, pass, subpass).toSubpassGroup(subpassGroup);
	}

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Pipeline object #'pipeline' in subpass object
	/// #'subpass' in pass #'pass' in effect #'effect'.</summary>
	/// <param name="effect">The index of the effect the object belongs to. The index is the order with which it was
	/// added to the RenderManager.</param>
	/// <param name="pass">The index of the pass within 'effect' that the object belongs to. The index is the order of
	/// the pass in the effect.</param>
	/// <param name="subpass">The index of the subpass within 'pass' that the object belongs to. The index is the
	/// order of the subpass in the pass.</param>
	/// <param name="pipeline">The index of the pipeline within 'subpass'. The index is the order of the pipeline in
	/// the pass.</param>
	/// <param name="subpassGroup">The index of the group within 'subpass' that the object belongs to. The index is the
	/// order of the subpassGroup in the subPass.</param>
	/// <returns>The RendermanPipeline object with index 'pipeline' in subpass 'subpass' in pass 'pass' of effect
	/// 'effect'.</returns>
	RendermanPipeline& toPipeline(uint32 effect, uint32 pass, uint32 subpass, uint32 subpassGroup, uint32 pipeline)
	{
		return toSubpassGroup(effect, pass, subpass, subpassGroup).toRendermanPipeline(pipeline);
	}

	/// <summary>Navigate the structure of the Rendermanager. Goes to a SubpassModel object. A SubpassModel object is the
	/// data held for a Model when it is added to a specific Subpass.</summary>
	/// <param name="effect">The index of the effect the object belongs to. The index is the order with which it was
	/// added to the RenderManager.</param>
	/// <param name="pass">The index of the pass within 'effect' that the object belongs to. The index is the order of
	/// the pass in the effect.</param>
	/// <param name="subpass">The index of the subpass within 'pass' that the object belongs to. The index is the
	/// order of the subpass in the pass.</param>
	/// <param name="subpassGroup">The index of the subpassGroup within 'subpass' that the object belongs to. The index is the
	/// order of the subpassGroup in the subpass.</param>
	/// <param name="model">The index of the model within 'subpass'. The index is the order in which this model was
	/// added to Subpass.</param>
	/// <returns>The RendermanPipeline object with index 'pipeline' in subpass 'subpass' in pass 'pass' of effect
	/// 'effect'.</returns>
	RendermanSubpassGroupModel& toSubpassGroupModel(uint32 effect, uint32 pass, uint32 subpass,
	    uint32 subpassGroup, uint32 model)
	{
		return toSubpassGroup(effect, pass, subpass, subpassGroup).toSubpassGroupModel(model);
	}

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Model object with index 'model'.</summary>
	/// <param name="model">The index of the model. The index is the order with which it was added to the
	/// RenderManager.</param>
	/// <returns>The RendermanModel object with index 'model' in the RenderManager'.</returns>
	RendermanModel& toModel(uint32 model) { return modelStorage[model]; }

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Mesh object #'mesh' of the model #'model'.
	/// </summary>
	/// <param name="model">The index of the rendermodel this mesh belongs to. The index is the order with which it
	/// was added to the RenderManager.</param>
	/// <param name="mesh">The index of the rendermesh in the model. The index of the mesh is the same as it was in
	/// the pvr::assets::Model file.</param>
	/// <returns>The RendermanMesh object with index 'mesh' in the model #'model'.</returns>
	RendermanMesh& toRendermanMesh(uint32 model, uint32 mesh) { return modelStorage[model].meshes[mesh]; }

	/// <summary>Navigate the structure of the Rendermanager. Access a Mesh through its effect (as opposed to through its
	/// model object. The purpose of this function is to find the object while navigating a subpass.</summary>
	/// <param name="effect">The index of the effect in the RenderManager.</param>
	/// <param name="pass">The index of the pass in the Effect.</param>
	/// <param name="subpass">The index of the subpass in the Pass.</param>
	/// <param name="subpassGroup">The index of the subpassGroup in the Subpass.</param>
	/// <param name="model">The index of the model in the Subpass. Caution- this is not the same as the index of the
	/// model in the RenderManager.</param>
	/// <param name="mesh">The index of the mesh in the model. This is the same as the index of the pvr::assets::Mesh
	/// in the pvr::assets::Model.</param>
	/// <returns>The selected RendermanMesh item.</returns>
	RendermanMesh& toRendermanMeshByEffect(uint32 effect, uint32 pass, uint32 subpass, uint32 subpassGroup,
	                                       uint32 model, uint32 mesh)
	{
		return renderStructure.effects[effect].passes[pass].subpasses[subpass].groups[subpassGroup]
		       .allModels[model]->meshes[mesh];
	}

	/// <summary>Get a reference to the entire render structure of the RenderManager. Raw.</summary>
	/// <returns>A reference to the data structure.</returns>
	RendermanStructure& renderObjects() { return renderStructure; }

	/// <summary>Get a reference to the storage of Models in the RenderManager. Raw.</summary>
	/// <returns>A reference to the Models.</returns>
	RendermanModelStorage& renderModels() { return modelStorage; }

	/// <summary>Get the context that this RenderManager uses.</summary>
	/// <returns>The context that this RenderManager uses.</returns>
	GraphicsContext& getContext() { return context; }

	/// <summary>Get the context that this RenderManager uses.</summary>
	/// <returns>The context that this RenderManager uses.</returns>
	const GraphicsContext& getContext() const { return context; }

	/// <summary>Add a model for rendering. This method is a shortcut for adding a model to ALL renderpasses, ALL
	/// subpasses.</summary>
	/// <param name="model">A pvr::assets::Model handle to add for rendering. Default 0.</param>
	/// <param name="effect">The Effect to whose subpasses the model will be added.</param>
	/// <returns>The order of the model within the RenderManager (the index to use for getModel).</returns>
	int32 addModelForAllPasses(const assets::ModelHandle& model, uint16 effect = 0)
	{
		int32 index = -1;
		for (std::size_t pass = 0; pass != renderStructure.effects[effect].passes.size(); ++pass)
		{
			index = addModelForAllSubpasses(model, (uint16)pass, effect);
		}
		return index;
	}

	/// <summary>Add a model for rendering. This method is a shortcut for adding a model to ALL subpasses of a
	/// specific renderpass.</summary>
	/// <param name="model">A pvr::assets::Model handle to add for rendering.</param>
	/// <param name="effect">The Effect to which the pass to render to belongs. Default 0.</param>
	/// <param name="pass">The Pass of the Effect to whose subpasses the model will be added.</param>
	/// <returns>The order of the model within the RenderManager (the index to use for getModel).</returns>
	int32 addModelForAllSubpasses(const assets::ModelHandle& model, uint16 pass, uint16 effect = 0)
	{
		int32 index = -1;
		for (uint32 subpass = 0; subpass != renderStructure.effects[effect].passes[pass].subpasses.size(); ++subpass)
		{
			index = addModelForAllSubpassGroups(model, pass, (uint16)subpass, effect);
		}
		return index;
	}

	int32 addModelForAllSubpassGroups(const assets::ModelHandle& model, uint16 pass, uint16 subpass, uint16 effect = 0)
	{
		int32 index = -1;
		for (uint32 subpassGroup = 0; subpassGroup < toSubpass(effect, pass, subpass).getNumSubpassGroups();
		     ++subpassGroup)
		{
			index = addModelForSubpassGroup(model, pass, subpass, subpassGroup, effect);
		}
		return index;
	}

	/// <summary>Add a model for rendering. Adds a model for rendering to a specific subpass.</summary>
	/// <param name="model">A pvr::assets::Model handle to add for rendering.</param>
	/// <param name="effect">The Effect to which the subpass to render to belongs. Default 0.</param>
	/// <param name="pass">The Pass of the Effect to whose subpass the model will be added.</param>
	/// <param name="subpassGroup">The SubpassGroup to which this model will be added.</param>
	/// <param name="subpass">The Subpass to which this model will be rendered.</param>
	/// <returns>The order of the model within the RenderManager (the index to use for getModel).</returns>efault 0.
	int32 addModelForSubpassGroup(const assets::ModelHandle& model, uint16 pass, uint16 subpass, uint16 subpassGroup,
	                              uint16 effect = 0)
	{
		struct apimodelcomparator
		{
			const assets::ModelHandle& model;
			apimodelcomparator(const assets::ModelHandle& model) : model(model) {}

			bool operator()(const RendermanModel& rhs)
			{
				return rhs.assetModel == model;
			}
		};
		int32 index;
		auto it = std::find_if(modelStorage.begin(), modelStorage.end(), apimodelcomparator(model));

		RendermanModel* apimodel = NULL;
		// check if the model is already processed
		// if not then make a ney entry else return an index to it.
		if (it == modelStorage.end())
		{
			index = (int32)modelStorage.size();
			modelStorage.push_back(RendermanModel());
			apimodel = &modelStorage.back();
			apimodel->mgr_ = this;
			apimodel->assetModel = model;
			apimodel->meshes.resize(model->getNumMeshes());
			apimodel->materials.resize(model->getNumMaterials());
			for (uint32 meshid = 0; meshid < model->getNumMeshes(); ++meshid)
			{
				apimodel->meshes[meshid].assetMesh = assets::getMeshHandle(model, meshid);
				apimodel->meshes[meshid].renderModel_ = apimodel;
				apimodel->meshes[meshid].assetMeshId = meshid;
			}
			for (uint32 materialid = 0; materialid < model->getNumMaterials(); ++materialid)
			{
				apimodel->materials[materialid].assetMaterial = assets::getMaterialHandle(model, materialid);
				apimodel->materials[materialid].renderModel_ = apimodel;
				apimodel->materials[materialid].assetMaterialId = materialid;
			}
		}
		else
		{
			index = (int32)(it - modelStorage.begin());
			apimodel = &*it;
		}

		renderStructure.effects[effect].passes[pass].subpasses[subpass].groups[subpassGroup].allModels.push_back(apimodel);
		return index;
	}

	/// <summary>Add an effect to the RenderManager. Must be called before models are added to this effect.</summary>
	/// <param name="effect">A new effect to be added</param>
	/// <param name="context">GraphicsContext who will own the resources</param>
	/// <param name="assetLoader">AssetLoader</param>
	/// <returns>The order of the Effect within the RenderManager (the index to use for toEffect()). Return -1 if
	/// error.</returns>
	uint32 addEffect(const assets::effect::Effect& effect, GraphicsContext& context, utils::AssetLoadingDelegate& assetLoader)
	{
		//Validate
		if (context.isNull())
		{
			assertion(false, "RenderManager: Invalid Context");
			Log("RenderManager: Invalid Context");
			return (uint32) - 1;
		}
		pvr::utils::effect::EffectApi effectapi;
		effectapi.construct(context, assetLoader);
		if (!effectapi->init(effect)) //validate
		{
			Log("RenderManager: Failed to create effect %s", effect.name.c_str());
			return (uint32) - 1;
		}

		this->context = context;

		renderStructure.effects.resize(renderStructure.effects.size() + 1);
		auto& new_effect = renderStructure.effects.back();
		new_effect.effect = effectapi;
		new_effect.manager_ = this;

		new_effect.passes.resize(effectapi->getNumPasses());

		for (uint32 passId = 0; passId < effectapi->getNumPasses(); ++passId)
		{
			RendermanPass& pass = new_effect.passes[passId];
			pass.subpasses.resize(effectapi->getPass(passId).subpasses.size());
			pass.fbo = effectapi->getPass(passId).fbos;

			for (uint32 subpassId = 0; subpassId < effectapi->getPass(passId).subpasses.size(); ++subpassId)
			{
				pass.subpasses[subpassId].groups.resize(effectapi->getPass(passId).subpasses[subpassId].groups.size());
			}

		}

		return uint32(renderStructure.effects.size() - 1);
	}

	/// <summary>Generate the RenderManager, create the structure, add all rendering effects, create the API objects, and
	/// in general, cook everything. Call AFTER any calls to addEffect(...) and addModel...(...). Call BEFORE any
	/// calls to createAutomaticSemantics(...), update semantics etc.</summary>
	/// <returns>True if successful.</returns>
	bool buildRenderObjects();


	/// <summary>Create rendering commands for all objects of all effects,passes,subpasses etc... added to the
	/// RenderManager. Will iterate the entire render structure generating any necessary binding/drawing commands into
	/// <paramref name="cbuff."/></summary>
	/// <param name="cbuff">A command buffer to record the commands into</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="beginEndRenderPass">If set to true, record a beginRenderPass() at the beginning and
	/// endRenderPass() at the end of this</param>
	/// <param name="recordUpdateUniforms">If set to false, skip the generation of any update uniforms commands
	/// </param>
	/// <remarks>If finer granularity of rendering commands is required, navigate the RenderStructure's objects
	/// (potentially using the RenderNodeIterator (call renderables()) and record rendering commands from them.
	/// </remarks>
	void recordAllRenderingCommands(
	  api::CommandBuffer& cbuff, uint16 swapIdx, bool beginEndRenderPass = true, bool recordUpdateUniforms = true);

	/// <summary>Return number of effects this render manager owns</summary>
	/// <returns>Number of effects</returns>
	size_t getNumberOfEffects()const { return renderStructure.effects.size(); }

	/// <summary>For each object of this RenderManager, generates a list of all the semantics that it requires (as
	/// defined in its Effect's pipeline objecT). Then, searches any assets (model, mesh, material, node etc.) for
	/// these semantics, and creates connections between them so that when updateAutomaticSemantics is called, the
	/// updated values are updated in the semantics so that they can be read (with setUniformPtr, or updating buffers
	/// etc.).</summary>
	void createAutomaticSemantics()
	{
		for (RendermanEffect& effect : renderStructure.effects)
		{
			for (RendermanPass& pass : effect.passes)
			{
				for (RendermanSubpass& subpass : pass.subpasses)
				{
					for (RendermanSubpassGroup& subpassGroup : subpass.groups)
					{
						for (RendermanSubpassGroupModel& subpassModel : subpassGroup.subpassGroupModels)
						{
							subpassModel.createAutomaticSemantics();
						}
						for (RendermanPipeline& pipe : subpassGroup.pipelines)
						{
							pipe.createAutomaticModelSemantics();
						}
					}
				}
			}
		}
	}


	/// <summary>Iterates all the nodes semantics per-effect, per-pass, per-subpass, per-model, per-node, and updates their
	/// values to their new, updated values. Needs to have called createAutomaticSemantics before.</summary>
	/// <param name="swapidx">swapchain index</param>
	void updateAutomaticSemantics(uint32 swapidx)
	{
		for (RendermanEffect& effect : renderStructure.effects)
		{
			effect.updateAutomaticSemantics(swapidx);
		}
	}

};

/////////////////////////// INLINE FUNCTIONS /////////////////////////////////

// This command is used to update uniform semantics directly into the TypedMem objects
inline bool recordUpdateUniformSemanticToExternalMemory(api::CommandBufferBase cbuff, uint32 uniformLocation, TypedMem& value_ptr)
{
	switch (value_ptr.dataType())
	{
	case types::GpuDatatypes::vec2: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::vec2*)value_ptr.raw()); break;
	case types::GpuDatatypes::vec3: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::vec3*)value_ptr.raw()); break;
	case types::GpuDatatypes::vec4: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::vec4*)value_ptr.raw()); break;
	case types::GpuDatatypes::float32: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (float32*)value_ptr.raw()); break;
	case types::GpuDatatypes::integer: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (int32*)value_ptr.raw()); break;
	case types::GpuDatatypes::ivec2: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::ivec2*)value_ptr.raw()); break;
	case types::GpuDatatypes::ivec3: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::ivec3*)value_ptr.raw()); break;
	case types::GpuDatatypes::ivec4: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::ivec4*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat2x2: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::mat2x2*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat2x3: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::mat2x3*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat2x4: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::mat2x4*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat3x2: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::mat3x2*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat3x3: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::mat3x3*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat3x4: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::mat3x4*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat4x2: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::mat4x2*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat4x3: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::mat4x3*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat4x4: cbuff->setUniformPtr(uniformLocation, value_ptr.arrayElements(), (glm::mat4x4*)value_ptr.raw()); break;
	default:
		debug_assertion(false, "recordUpdateUniformSemanticToExternalMemory: data type unrecognised");
	}
	return true;
}

inline void RendermanEffect::endBufferUpdates(uint32 swapChainIndex)
{
	if (isUpdating[swapChainIndex])
	{
		for (auto& buffer : bufferDefinitions)
		{
			auto& structuredbuf = buffer.buffer;
			uint32 idx = structuredbuf.isMultiBuffered() ? swapChainIndex : 0;
			auto& apibuf = *structuredbuf.getConnectedBuffer(idx);
			if (apibuf.isMapped())
			{
				apibuf.unmap();
			}
		}
	}
	isUpdating[swapChainIndex] = false;
}


//                                      RendermanPipeline inline definition
inline RendermanSubpassGroup& RendermanPipeline::backToSubpassGroup() { return *subpassGroup_; }

inline RendermanSubpass& RendermanPipeline::backToSubpass() { return backToSubpassGroup().backToSubpass(); }

inline RendermanEffect& RendermanPipeline::backToRendermanEffect() { return backToSubpass().backToRendermanEffect(); }

inline void RendermanPipeline::recordUpdateAllUniformSemantics(api::CommandBufferBase cbuff)
{
	for (auto& sem : uniformSemantics)
	{
		recordUpdateUniformSemanticToExternalMemory(cbuff, sem.second.uniformLocation, sem.second.memory);
	}
	for (auto& sem : backToRendermanEffect().uniformSemantics)
	{
		recordUpdateUniformSemanticToExternalMemory(cbuff, sem.second.uniformLocation, sem.second.memory);
	}
}

inline void RendermanPipeline::recordUpdateAllUniformModelSemantics(api::CommandBufferBase cbuff)
{
	for (auto& sem : uniformSemantics)
	{
		recordUpdateUniformSemanticToExternalMemory(cbuff, sem.second.uniformLocation, sem.second.memory);
	}
}

inline void RendermanPipeline::recordUpdateAllUniformEffectSemantics(api::CommandBufferBase cbuff)
{
	for (auto& sem : backToRendermanEffect().uniformSemantics)
	{
		recordUpdateUniformSemanticToExternalMemory(cbuff, sem.second.uniformLocation, sem.second.memory);
	}
}

inline void RendermanPipeline::recordUpdateAllUniformNodeSemantics(api::CommandBufferBase cbuff, RendermanNode& node)
{
	for (auto& sem : node.uniformSemantics)
	{
		recordUpdateUniformSemanticToExternalMemory(cbuff, sem.second.uniformLocation, sem.second.memory);
	}
}

inline bool RendermanPipeline::recordUpdateUniformCommandsModelSemantic(api::CommandBufferBase cbuff, const StringHash& semantic)
{
	auto it = uniformSemantics.find(semantic);
	if (it == uniformSemantics.end() || it->second.uniformLocation == -1)
	{
		return false;
	}
	return recordUpdateUniformSemanticToExternalMemory(cbuff, it->second.uniformLocation, it->second.memory);
}

inline bool RendermanPipeline::recordUpdateUniformCommandsEffectSemantic(api::CommandBufferBase cbuff, const StringHash& semantic)
{
	auto& cont = backToRendermanEffect().uniformSemantics;
	auto it = cont.find(semantic);
	if (it == cont.end() || it->second.uniformLocation == -1)
	{
		return false;
	}
	return recordUpdateUniformSemanticToExternalMemory(cbuff, it->second.uniformLocation, it->second.memory);
}

inline bool RendermanPipeline::recordUpdateUniformCommandsNodeSemantic(
  api::CommandBufferBase cbuff, const StringHash& semantic, RendermanNode& node)
{
	auto& cont = node.uniformSemantics;
	auto it = cont.find(semantic);
	if (it == cont.end() || it->second.uniformLocation == -1)
	{
		return false;
	}
	return recordUpdateUniformSemanticToExternalMemory(cbuff, it->second.uniformLocation, it->second.memory);
}

inline bool RendermanPipeline::updateBufferEntryModelSemantic(
  const StringHash& semantic, const FreeValue& value, uint32 swapid, uint32 dynamicClientId)
{
	auto it = bufferEntrySemantics.find(semantic);
	if (it == bufferEntrySemantics.end())
	{
		Log(Log.Debug, "RendermanPipeline::updateBufferEntryModelSemantic - Semantic '%s' not found", semantic.c_str());
		return false;
	}
	auto& sem = it->second;
	sem.buffer->map(swapid);
	sem.buffer->setArrayValue(sem.entryIndex, dynamicClientId, value);
	sem.buffer->unmap(swapid);
	return true;
}

inline bool RendermanPipeline::updateBufferEntryModelSemantics(const StringHash* semantics,
    const FreeValue* value, uint32 numSemantics, uint32 swapid, uint32 dynamicClientId)
{
	for (uint32 i = 0; i < numSemantics; ++i)
	{
		if (!updateBufferEntryModelSemantic(semantics[i], value[i], swapid, dynamicClientId)) { return false; }
	}
	return true;
}



inline bool RendermanPipeline::updateBufferEntryEffectSemantic(
  const StringHash& semantic, const FreeValue& value, uint32 swapid, uint32 dynamicClientId)
{
	auto& cont = backToRendermanEffect().bufferEntrySemantics;
	auto it = cont.find(semantic);
	if (it == cont.end())
	{
		return false;
	}

	auto& sem = it->second;
	sem.buffer->map(swapid);
	sem.buffer->setArrayValue(sem.entryIndex, dynamicClientId, value);
	sem.buffer->unmap(swapid);
	return true;
}

inline bool RendermanPipeline::updateBufferEntryNodeSemantic(
  const StringHash& semantic, const FreeValue& value, uint32 swapid, RendermanNode& node)
{
	auto& effect = backToRendermanEffect();
	auto it = bufferEntrySemantics.find(semantic);
	if (it == bufferEntrySemantics.end())
	{
		auto& cont = effect.bufferEntrySemantics;
		it = cont.find(semantic);
		if (it == cont.end())
		{
			return false;
		}
	}
	auto& sem = it->second;
	bool multibuffered = it->second.buffer->getMultibufferCount() > 0;
	uint32 swapIdMultibuffered = multibuffered ? swapid : 0;

	if (!effect.isUpdating[swapid])
	{
		sem.buffer->map(swapIdMultibuffered);
	}
	else
	{
		if (!sem.buffer->getConnectedBuffer(swapIdMultibuffered)->isMapped())
		{
			sem.buffer->map(swapIdMultibuffered);
		}
	}

	uint32 dynamicClientId = 0;

	if (sem.dynamicOffsetNodeId >= 0 && node.dynamicClientId[sem.setId].size() > 0)
	{
		dynamicClientId = node.dynamicClientId[sem.setId][sem.dynamicOffsetNodeId];
	}

	sem.buffer->setArrayValue(sem.entryIndex, dynamicClientId, value);

	if (!effect.isUpdating[swapid])
	{
		sem.buffer->unmap(swapIdMultibuffered);
	}
	return true;
}

inline bool RendermanPipeline::updateBufferEntryEffectSemantics(
  const StringHash* semantics, const FreeValue* value, uint32 numSemantics, uint32 swapid, uint32 dynamicClientId)
{
	std::vector<utils::StructuredBufferView*> mapedBuffer(numSemantics);
	for (uint32 i = 0; i < numSemantics; ++i)
	{
		auto& cont = backToRendermanEffect().bufferEntrySemantics;
		auto it = cont.find(semantics[i]);
		if (it == cont.end()) { continue; }
		auto& sem = it->second;
		if (!sem.buffer->isMapped(swapid)) { sem.buffer->map(swapid); mapedBuffer.push_back(sem.buffer); }
		sem.buffer->setArrayValue(sem.entryIndex, dynamicClientId, value[i]);
	}

	for (auto& walk : mapedBuffer)
	{
		walk->unmap(swapid);
	}
	return true;
}

inline bool RendermanPipeline::updateBufferEntrySemantic(
  const StringHash& semantic, const FreeValue& value, uint32 swapid, uint32 dynamicClientId)
{
	auto& effect = backToRendermanEffect();
	auto it = bufferEntrySemantics.find(semantic);
	if (it == bufferEntrySemantics.end())
	{
		auto& cont = effect.bufferEntrySemantics;
		it = cont.find(semantic);
		if (it == cont.end())
		{
			return false;
		}
	}
	auto& sem = it->second;
	bool multibuffered = it->second.buffer->getMultibufferCount() > 0;
	uint32 swapIdMultibuffered = multibuffered ? swapid : 0;

	if (!effect.isUpdating[swapid])
	{
		sem.buffer->map(swapIdMultibuffered);
	}
	else
	{
		if (!sem.buffer->getConnectedBuffer(swapIdMultibuffered)->isMapped())
		{
			sem.buffer->map(swapIdMultibuffered);
		}
	}

	sem.buffer->setArrayValue(sem.entryIndex, dynamicClientId, value);

	if (!effect.isUpdating[swapid])
	{
		sem.buffer->unmap(swapIdMultibuffered);
	}
	return true;
}

inline bool RendermanPipeline::updateUniformModelSemantic(const StringHash& semantic, const TypedMem& value)
{
	auto it = uniformSemantics.find(semantic);
	if (it == uniformSemantics.end())
	{
		return false;
	}
	auto& sem = it->second;
	debug_assertion(value.isDataCompatible(it->second.memory), "updateUniformModelSemantic: Semantic not found in pipeline");
	if (!value.isDataCompatible(it->second.memory)) { return false; }
	sem.memory = value;
	return true;
}

inline bool RendermanPipeline::updateUniformEffectSemantic(const StringHash& semantic, const TypedMem& value)
{
	auto& cont =  subpassGroup_->backToSubpass().backToRendermanPass().backToEffect().uniformSemantics;
	auto it = cont.find(semantic);
	if (it == cont.end())
	{
		debug_assertion(false, strings::createFormatted("updateUniformModelSemantic: Semantic [%s] not found in pipeline", semantic.c_str()));
		return false;
	}
	auto& sem = it->second;
	debug_assertion(value.isDataCompatible(it->second.memory), strings::createFormatted(
	                  "updateUniformModelSemantic: Semantic value passed for semantic [%s] type incompatible with uniform type found.       "
	                  "Passed: Datatype id [%d], ArrayElements [%d]   Required: Passed: Datatype id [%d], ArrayElements [%d]"
	                  , semantic.c_str(), value.dataType(), value.arrayElements(), sem.memory.dataType(), sem.memory.arrayElements()));
	if (!value.isDataCompatible(it->second.memory)) { return false; }
	sem.memory = value;
	return true;
}

inline bool RendermanPipeline::updateUniformNodeSemantic(
  const StringHash& semantic, const TypedMem& value, RendermanNode& node)
{
	auto it = node.uniformSemantics.find(semantic);
	if (it == node.uniformSemantics.end())
	{
		debug_assertion(false, strings::createFormatted("updateUniformNodeSemantic: Semantic [%s] not found in pipeline for node id [%d]", semantic.c_str(), node.assetNodeId));
		return false;
	}
	auto& sem = it->second;
	debug_assertion(value.isDataCompatible(it->second.memory), strings::createFormatted(
	                  "updateUniformNodeSemantic: Semantic value passed for semantic [%s] type incompatible with uniform type found in node [%d].       "
	                  "Passed: Datatype id [%d], ArrayElements [%d]   Required: Passed: Datatype id [%d], ArrayElements [%d]"
	                  , semantic.c_str(), node.assetNodeId, value.dataType(), value.arrayElements(), sem.memory.dataType(), sem.memory.arrayElements()));
	if (!value.isDataCompatible(it->second.memory)) { return false; }
	sem.memory = value;
	return true;
}

inline bool RendermanPipeline::createAutomaticModelSemantics(uint32 useMainModelId)
{
	{
		automaticModelBufferEntrySemantics.clear();

		auto& model = backToRendermanEffect().manager_->renderModels()[useMainModelId];

		auto& cont = backToRendermanEffect().bufferEntrySemantics;

		for (auto& reqsem : cont)
		{
			ModelSemanticSetter setter = model.getModelSemanticSetter(reqsem.first);
			if (setter == NULL)
			{
				Log(Log.Information, "Automatic Model semantic [%s] not found.", reqsem.first.c_str());
			}
			else
			{
				Log(Log.Information, "Automatic Model semantic [%s] found! Creating automatic connection with model [%d]", reqsem.first.c_str(), useMainModelId);
				automaticModelBufferEntrySemantics.resize(automaticModelBufferEntrySemantics.size() + 1);
				AutomaticModelBufferEntrySemantic& autosem = automaticModelBufferEntrySemantics.back();
				autosem.model = &model;
				autosem.buffer = reqsem.second.buffer;
				autosem.entryIndex = reqsem.second.entryIndex;
				autosem.semanticSetFunc = setter;
				autosem.semantic = &reqsem.first;
			}
		}
	}
	{
		automaticModelUniformSemantics.clear();

		auto& model = backToRendermanEffect().manager_->renderModels()[useMainModelId];

		auto& cont = uniformSemantics;

		for (auto& reqsem : cont)
		{
			ModelSemanticSetter setter = model.getModelSemanticSetter(reqsem.first);
			if (setter == NULL)
			{
				Log(Log.Information, "Automatic Model semantic [%s] not found.", reqsem.first.c_str());
			}
			else
			{
				Log(Log.Information, "Automatic Model semantic [%s] found! Creating automatic connection with model [%d]", reqsem.first.c_str(), useMainModelId);
				automaticModelUniformSemantics.resize(automaticModelUniformSemantics.size() + 1);
				AutomaticModelUniformSemantic& autosem = automaticModelUniformSemantics.back();
				autosem.model = &model;
				autosem.semanticSetFunc = setter;
				autosem.semantic = &reqsem.first;
				autosem.memory = &reqsem.second.memory;
			}
		}
	}
	return true;
}

inline bool RendermanPipeline::updateAutomaticModelSemantics(uint32 swapidx)
{
	static bool shown_multi_warning = false;
	TypedMem val;
	for (auto& sem : automaticModelBufferEntrySemantics)
	{
		uint32 tmpswapidx = sem.calcSwapId(swapidx);
		bool wasUpdating = this->backToRendermanEffect().isUpdating[swapidx];
		bool mustMap = !sem.buffer->getConnectedBuffer(tmpswapidx)->isMapped();
		if (mustMap)
		{
			sem.buffer->map(tmpswapidx);
		}

		sem.semanticSetFunc(val, *sem.model);
		sem.buffer->setArrayValue(sem.entryIndex, 0, val);

		if (mustMap && !wasUpdating)
		{
			sem.buffer->unmap(tmpswapidx);
			if (!shown_multi_warning)
			{
				Log(Log.Warning, "RenderManager: Performance alert - Pipelines are updating without first calling BeginBufferUpdates on the RenderEffect."
				    " This means that buffers will be mapped and unmapped repeatedly for every node. This must usually be avoided.");
				shown_multi_warning = true;
			}
		}
	}
	for (auto& sem : automaticModelUniformSemantics)
	{
		sem.semanticSetFunc(val, *sem.model);
		(const TypedMem&)(*sem.memory) = val; // const is to avoid the "allocate" call in the TypedMem
	}
	return true;
}

inline void RendermanNode::updateAutomaticSemantics(uint32 swapidx)
{
	static bool shown_multi_warning = false;
	TypedMem val; //Keep it here to avoid tons of different memory allocations...
	for (auto& sem : automaticEntrySemantics)
	{
		uint32 tmpswapidx = sem.calcSwapId(swapidx);
		bool wasUpdating = this->toRendermanPipeline().backToRendermanEffect().isUpdating[swapidx];
		bool mustMap = !sem.buffer->getConnectedBuffer(tmpswapidx)->isMapped();
		if (mustMap)
		{
			sem.buffer->map(tmpswapidx);
		}

		uint32 dynamicClientId = 0;

		if (sem.dynamicOffsetNodeId >= 0 && this->dynamicClientId[sem.setId].data() != NULL)
		{
			dynamicClientId = this->dynamicClientId[sem.setId][sem.dynamicOffsetNodeId];
		}

		sem.semanticSetFunc(val, *this);

		sem.buffer->setArrayValue(sem.entryIndex, dynamicClientId, val);

		if (!wasUpdating && mustMap)
		{
			sem.buffer->unmap(tmpswapidx);
			if (!shown_multi_warning)
			{
				Log(Log.Warning, "RenderManager: Performance alert - Nodes are updating without first calling BeginBufferUpdates on the RenderEffect."
				    " This means that buffers will be mapped and unmapped repeatedly for every node. This must usually be avoided.");
				shown_multi_warning = true;
			}
		}
	}
	for (auto& sem : automaticUniformSemantics)
	{
		sem.semanticSetFunc(val, *this);
		(const TypedMem&)(*sem.memory) = val; // const is to avoid the "allocate" call in the TypedMem
	}
}

inline void RendermanNode::createAutomaticSemantics()
{
	automaticEntrySemantics.clear();
	for (auto& reqsem : this->toRendermanPipeline().bufferEntrySemantics)
	{
		NodeSemanticSetter setter = getNodeSemanticSetter(reqsem.first);
		if (setter == NULL)
		{
			Log(Log.Information, "Renderman: Automatic node semantic [%s] not found.", reqsem.first.c_str());
		}
		else
		{
			Log(Log.Information, "Renderman: Automatic node semantic [%s] found! Creating automatic connection:", reqsem.first.c_str());
			automaticEntrySemantics.resize(automaticEntrySemantics.size() + 1);
			AutomaticNodeBufferEntrySemantic& autosem = automaticEntrySemantics.back();
			autosem.buffer = reqsem.second.buffer;
			autosem.dynamicOffsetNodeId = reqsem.second.dynamicOffsetNodeId;
			autosem.entryIndex = reqsem.second.entryIndex;
			autosem.setId = reqsem.second.setId;
			autosem.semanticSetFunc = setter;
			autosem.semantic = &reqsem.first;
		}
	}
	for (auto& reqsem : uniformSemantics)
	{
		NodeSemanticSetter setter = getNodeSemanticSetter(reqsem.first);
		if (setter == NULL)
		{
			Log(Log.Information, "Automatic node semantic [%s] not found.", reqsem.first.c_str());
		}
		else
		{
			Log(Log.Information, "Automatic node semantic [%s] found! Creating automatic connection:", reqsem.first.c_str());
			automaticUniformSemantics.resize(automaticUniformSemantics.size() + 1);
			AutomaticNodeUniformSemantic& autosem = automaticUniformSemantics.back();
			autosem.semanticSetFunc = setter;
			autosem.semantic = &reqsem.first;
			autosem.memory = &reqsem.second.memory;
		}
	}
}

inline void RendermanSubpassGroupModel::updateFrame(float32 frame)
{
	renderModel_->assetModel->setCurrentFrame(frame);
	//model->assetModel->updateModelSemantics();
}

//                                      RendermanSubpassMaterial inline definition
inline const RendermanModel& RendermanSubpassMaterial::backToModel() const { return *backToSubpassGroupModel().renderModel_; }
inline RendermanModel& RendermanSubpassMaterial::backToModel() { return *backToSubpassGroupModel().renderModel_; }
inline RendermanSubpassGroup& RendermanSubpassMaterial::backToSubpassGroup() { return *backToSubpassGroupModel().renderSubpassGroup_; }
inline const RendermanSubpassGroup& RendermanSubpassMaterial::backToSubpassGroup() const { return *backToSubpassGroupModel().renderSubpassGroup_; }

//                                      RendermanSubpassMesh inline definition
inline const RendermanModel& RendermanSubpassMesh::backToModel() const { return *backToSubpassGroupModel().renderModel_; }
inline RendermanModel& RendermanSubpassMesh::backToModel() { return *backToSubpassGroupModel().renderModel_; }
inline const RendermanSubpassGroup& RendermanSubpassMesh::backToSubpassGroup() const { return *backToSubpassGroupModel().renderSubpassGroup_; }
inline RendermanSubpassGroup& RendermanSubpassMesh::backToSubpassGroup() { return *backToSubpassGroupModel().renderSubpassGroup_; }

//                                      RendermanSubpassModel inline definition
inline RendermanModel& RendermanSubpassGroupModel::backToModel() { return *renderModel_; }
inline RenderManager& RendermanSubpassGroupModel::backToRenderManager() { return *renderModel_->mgr_; }
inline RendermanSubpassGroup& RendermanSubpassGroupModel::backToRendermanSubpassGroup() { return *renderSubpassGroup_; }
inline RendermanSubpass& RendermanSubpassGroupModel::backToRendermanSubpass() { return renderSubpassGroup_->backToSubpass(); }
inline RendermanPass& RendermanSubpassGroupModel::backToRendermanPass() { return renderSubpassGroup_->backToSubpass().backToRendermanPass(); }
inline RendermanEffect& RendermanSubpassGroupModel::backToRendermanEffect() { return renderSubpassGroup_->backToSubpass().backToRendermanEffect(); }

//                                      RendermanMaterial inline definition
inline const RendermanModel& RendermanMaterial::backToRendermanModel() const { return *renderModel_; }
inline const RendermanModel& RendermanMaterial::backToRendermanModel() { return *renderModel_; }
inline const RenderManager& RendermanMaterial::backToRenderManager()const { return *backToRendermanModel().mgr_; }
inline RenderManager& RendermanMaterial::backToRenderManager() { return *backToRendermanModel().mgr_; }

//                                      RendermanMesh inline definition
inline const RenderManager& RendermanMesh::backToRenderManager() const { return *backToRendermanModel().mgr_; }
inline RenderManager& RendermanMesh::backToRenderManager() { return *backToRendermanModel().mgr_; }

//                                      RendermanMaterialSubpassPipeline inline definition
inline const RendermanPipeline& RendermanMaterialSubpassPipeline::toPipeline() const  { return *pipeline_; }
inline RendermanPipeline& RendermanMaterialSubpassPipeline::toPipeline() { return *pipeline_; }

//                                      RendermanSubpass inline definition
inline const RendermanEffect& RendermanSubpass::backToRendermanEffect() const { return *renderingPass_->renderEffect_; }
inline RendermanEffect& RendermanSubpass::backToRendermanEffect() { return *renderingPass_->renderEffect_; }
inline const RenderManager& RendermanSubpass::backToRenderManager() const { return *backToRendermanEffect().manager_; }
inline RenderManager& RendermanSubpass::backToRenderManager() { return *backToRendermanEffect().manager_; }

//                                      RendermanEffect inline definition
inline const RenderManager& RendermanEffect::backToRenderManager()const { return *manager_; }
inline RenderManager& RendermanEffect::backToRenderManager() { return *manager_; }

inline const RendermanSubpass& RendermanSubpassGroup::backToSubpass()const { return *subpass_; }
inline RendermanSubpass& RendermanSubpassGroup::backToSubpass() { return *subpass_; }

}
}