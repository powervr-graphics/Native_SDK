/*!****************************************************************************************************************
\file         PVRApi/RenderManager.h
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        The RenderManager class. Provides basic engine rendering functionality. 
			See class documentation for basic use.
*******************************************************************************************************************/

#pragma once
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRApi/EffectApi_2.h"
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

struct RendermanSubpassModel;
struct RendermanPipeline;
struct RendermanSubpassMaterial;
struct RendermanSubpass;
struct RendermanPass;
struct RendermanEffect;
struct RendermanNode;

/*!
   \brief This struct is used to store buffers such as ubo and ssbo
 */
struct RendermanBufferDefinition
{
	StringHash name; //!< buffer name
	StructuredMemoryView buffer;
	types::BufferViewTypes allSupportedBindings;
	types::VariableScope scope; //!< buffer scope
	uint16 numBuffers;
	uint32 numDynamicClients;
	RendermanBufferDefinition() : numBuffers(1), numDynamicClients(0) {}
};


/*!
  \brief This struct is used to store a material.
  \note (Exists only to avoid duplication between different textures.)
*/
struct RendermanMaterial
{
	RendermanModel* renderModel_;//!< the model this material belongs to
	std::map<StringHash, api::TextureView> textures;//!< material textures
	assets::MaterialHandle assetMaterial;
	uint32 assetMaterialId;//!< material id

	/*!
	   \brief Return RendermanModel which own this object (const).
	   \return RendermanModel
	 */
	const RendermanModel& backToRendermanModel() const;

	/*!
	   \brief Return RendermanModel which own this object
	   \return RendermanModel
	 */
	const RendermanModel& backToRendermanModel();

	/*!
	   \brief Return RenderManager which own this object (const)
	   \return Return RenderManager
	 */
	const RenderManager& backToRenderManager()const;

	/*!
	   \brief Return RenderManager which own this object
	   \return Return RenderManager
	 */
	RenderManager& backToRenderManager();
};


/*!
   \brief Part of RendermanStructure. This class is used to store VBOs/IBOs.
          Unique per mesh
          RendermanNodes inside passes/subpasses reference these items by pointer.
 */
struct RendermanMesh
{
	RendermanModel* renderModel_;
	assets::MeshHandle assetMesh;
	uint32 assetMeshId;
	DynamicArray<api::Buffer> vbos; // ONLY ONE - OPTIMISED FOR ALL PIPELINES
	api::Buffer ibo;				// ONLY ONE - OPTIMISED FOR ALL PIPELINES
	types::IndexType indexType;

	/*!
	   \brief Return RendermanModel which owns this object (const).
	   \return RendermanModel
	 */
	const RendermanModel& backToRendermanModel()const { return *renderModel_; }

	/*!
	   \brief Return RendermanModel which owns this object.
	   \return RendermanModel
	 */
	RendermanModel& backToRendermanModel() { return *renderModel_; }

	/*!
	   \brief Return RenderManager which own this object (const)
	   \return Return RenderManager
	 */
	const RenderManager& backToRenderManager()const;

	/*!
	   \brief Return RenderManager which own this object
	   \return Return RenderManager
	 */
	RenderManager& backToRenderManager();
};


typedef bool(*ModelSemanticSetter)(TypedMem& mem, const RendermanModel& model);


/*!
   \brief Part of RendermanStructure. This class is used to store RendermanMeshes.
          Unique per model
          RendermanModelEffects inside passes/subpasses reference these items by pointer.
 */
struct RendermanModel
{
	RenderManager* mgr_;//!< render manager
	assets::ModelHandle assetModel; //!< handle to the model
	std::deque<RendermanMesh> meshes;//!< renderable meshes
	std::deque<RendermanMaterial> materials;//< materials

	/*!
	   \brief Get model semantic data
	   \param semantic Semantic
	   \param memory Data returned
	   \return Return true if found
	 */
	bool getModelSemantic(const StringHash& semantic, TypedMem& memory) const;

	/*!
	   \brief Return model semantic setter
	   \param semantic Semantic
	 */
	ModelSemanticSetter getModelSemanticSetter(const StringHash& semantic) const;

	/*!
	   \brief Return RenderManager which own this object (const)
	   \return Return RenderManager
	 */
	const RenderManager& backToRenderManager()const { return *mgr_; }

	/*!
	   \brief Return RenderManager which own this object
	   \return Return RenderManager
	 */
	RenderManager& backToRenderManager() { return *mgr_; }

	/*!
	   \brief Get renderman mesh object belongs to this model
	   \param mesh Mesh index
	   \return RendermanMesh
	 */
	RendermanMesh& toRendermanMesh(uint32 mesh)
	{
		debug_assertion(mesh < meshes.size(), "Mesh index out of bound");
		return meshes[mesh];
	}

	/*!
	   \brief Get renderman mesh object belongs to this model (const)
	   \param mesh Mesh index
	   \return RendermanMesh
	 */
	const RendermanMesh& toRendermanMesh(uint32 mesh) const
	{
		debug_assertion(mesh < meshes.size(), "Mesh index out of bound");
		return meshes[mesh];
	}

	/*!
	   \brief Get renderman material object belongs to this model
	   \param material Material index
	   \return RendermanMaterial
	 */
	RendermanMaterial& toRendermanMaterial(uint32 material)
	{
		debug_assertion(material < materials.size(), "material index out of bound");
		return materials[material];
	}

	/*!
	   \brief Get renderman material object belongs to this model (const)
	   \param material Material index
	   \return RendermanMaterial
	 */
	const RendermanMaterial& toRendermanMaterial(uint32 material) const
	{
		debug_assertion(material < materials.size(), "material index out of bound");
		return materials[material];
	}
};

/*!
   \brief Contains information to bind a buffer to a specific pipeline's descriptor sets.
 */
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

	/*!
	   \brief Get RendermanPipeline object (const)
	   \return RendermanPipeline
	 */
	const RendermanPipeline& toPipeline() const;

	/*!
	   \brief Get RendermanPipeline object
	   \return RendermanPipeline
	 */
	RendermanPipeline& toPipeline();

	/*!
	   \brief Return the RendermanSubpassMaterial object which owns this object (const)
	   \return RendermanSubpassMaterial
	 */
	const RendermanSubpassMaterial& backToSubpassMaterial() const { return *materialSubpass_; }

	/*!
	   \brief Return the RendermanSubpassMaterial object which owns this object
	   \return RendermanSubpassMaterial
	 */
	RendermanSubpassMaterial& backToSubpassMaterial() { return *materialSubpass_; }
};

//Part of RendermanStructure. This class contains the Material's instances that are used by a pipeline
//The reason is that a pipeline is selected BOTH by material AND by mesh, making it possible for one
//material in one subpass to be used by different pipelines.
struct RendermanSubpassMaterial
{
	std::vector<RendermanMaterialSubpassPipeline> materialSubpassPipelines;
	RendermanSubpassModel* modelSubpass_;
	RendermanMaterial* material;

	/*!
	   \brief Return RendermanMaterialSubpassPipeline object
	   \param index RendermanMaterialSubpassPipeline index
	   \return RendermanMaterialSubpassPipeline
	 */
	RendermanMaterialSubpassPipeline& toMaterialSubpassPipeline(uint32 index)
	{
		debug_assertion(index < materialSubpassPipelines.size(), "Material subpass pipeline index out of bound");
		return materialSubpassPipelines[index];
	}

	/*!
	   \brief Return RendermanMaterialSubpassPipeline object (const)
	   \param index RendermanMaterialSubpassPipeline index
	   \return RendermanMaterialSubpassPipeline
	 */
	const RendermanMaterialSubpassPipeline& toMaterialSubpassPipeline(uint32 index)const
	{
		debug_assertion(index < materialSubpassPipelines.size(), "Material subpass pipeline index out of bound");
		return materialSubpassPipelines[index];
	}

	/*!
	   \brief Return RendermanSubpassModel object which owns this object (const).
	   \return RendermanSubpassModel
	 */
	RendermanSubpassModel& backToSubpassModel() { return *modelSubpass_; }

	/*!
	   \brief Return RendermanSubpassModel object which owns this object (const).
	   \return RendermanSubpassModel
	 */
	const RendermanSubpassModel& backToSubpassModel()const { return *modelSubpass_; }

	/*!
	   \brief Return RendermanModel which owns this object (const)
	   \return RendermanModel
	 */
	const RendermanModel& backToModel()const;

	/*!
	   \brief Return RendermanModel which owns this object
	   \return RendermanModel
	 */
	RendermanModel& backToModel();

	/*!
	   \brief Return RendermanSubpass which owns this object.
	   \return RendermanSubpass
	 */
	RendermanSubpass& backToSubpass();

	/*!
	   \brief Return RendermanSubpass which owns this object (const).
	   \return RendermanSubpass
	 */
	const RendermanSubpass& backToSubpass()const;

	/*!
	   \brief Return material
	   \return RendermanMaterial
	 */
	RendermanMaterial& toMaterial() { return  *material; }

	/*!
	   \brief Return material (const)
	   \return RendermanMaterial
	 */
	const RendermanMaterial& toMaterial()const { return  *material; }
};

//Part of RendermanStructure. This class is a Mesh's instances as used by a pipeline
//The "usedByPipelines" is only a helper.
struct RendermanSubpassMesh
{
	RendermanSubpassModel* modelSubpass_;
	RendermanMesh* rendermesh_;
	std::set<RendermanPipeline*> usedByPipelines;

	/*!
	   \brief Return RendermanSubpassModel object which owns this object
	   \return RendermanSubpassModel
	 */
	RendermanSubpassModel& backToSubpassModel() { return *modelSubpass_; }

	/*!
	   \brief Return RendermanSubpassModel object which owns this object (const)
	   \return RendermanSubpassModel
	 */
	const RendermanSubpassModel& backToSubpassModel()const { return *modelSubpass_; }

	/*!
	   \brief Return RendermanModel which owns this object
	   \return RendermanModel
	 */
	RendermanModel& backToRendermanModel();

	/*!
	   \brief Return RendermanModel which owns this object (const).
	   \return RendermanModel
	 */
	const RendermanModel& backToRendermanModel()const;

	/*!
	   \brief Return RendermanSubpass which owns this object (const).
	   \return RendermanSubpass
	 */
	const RendermanSubpass& backToRendermanSubpass()const;

	/*!
	   \brief Return RendermanSubpass which owns this object.
	   \return RendermanSubpass
	 */
	RendermanSubpass& backToRendermanSubpass();

	/*!
	   \brief Return RendermanMesh object which owns this object (const)
	   \return RendermanMesh
	 */
	RendermanMesh& backToRendermanMesh() { return *rendermesh_; }

	/*!
	   \brief Return RendermanMesh object which owns this object (const)
	   \return RendermanMesh
	 */
	const RendermanMesh& backToRendermanMesh()const { return *rendermesh_; }
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

	/*!
	   \brief Return if the buffer is multibuffered
	 */
	bool multibuffered()
	{
		return buffer->getMultibufferSize() > 0;
	}

	/*!
	   \brief Calculate the swapchain id
	   \param swapId
	   \return Return 0 if it is not multi buffered, else return swapId
	 */
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

	/*!
	   \brief Return if the buffer is multibuffered
	 */
	bool multibuffered()
	{
		return buffer->getMultibufferSize() > 0;
	}

	/*!
	   \brief Calculate the swapchain id
	   \param swapId
	   \return Return 0 if it is not multi buffered, else return swapId
	 */
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

	/*!
	   \brief Get node semantic
	   \param semantic
	   \param memory Semantic data returned
	   \return Return true if found
	 */
	bool getNodeSemantic(const StringHash& semantic, TypedMem& memory) const;

	/*!
	   \brief Get node semantic raw mem
	   \param semantic
	   \param memory Semantic data returned
	   \return Return true if found
	 */
	bool getNodeSemanticRawMem(const StringHash& semantic, void* memory) const;

	/*!
	   \brief Get node semantic setter
	   \param semantic
	   \return Return node semantic setter
	 */
	NodeSemanticSetter getNodeSemanticSetter(const StringHash& semantic) const;

	/*!
	   \brief Get node semantic raw memsetter
	   \param semantic
	   \param memory
	   \return
	 */
	NodeSemanticSetter getNodeSemanticRawMemSetter(const StringHash& semantic, void* memory) const;

	/*!
	   \brief Update node value semantic
	   \param semantic
	   \param value New value to set
	   \param swapid Swapchain id
	   \return Return true if success
	 */
	bool updateNodeValueSemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid);

	/*!
	   \brief Update node buffer semantic
	   \param semantic
	   \param memory New values to update
	   \param size Size of the memory
	   \param swapid Swapchain id
	   \return Return true on if success
	 */
	bool updateNodeBufferSemantic(const StringHash& semantic, void* memory, uint32 size, uint32 swapid);

	/*!
	   \brief Iterates any semantics that are per-node, and updates their values to their specific per-node values.
	          Needs to have callsed createAutomaticSemantics before.
	   \param swapidx
	 */
	void updateAutomaticSemantics(uint32 swapidx);

	/*!
	   \brief Generates a list of semantics that the pipeline requires, but are changed per-node.
	          Necessary to use updateAutomaticSemantics afterwards.
	 */
	void createAutomaticSemantics();

	/*!
	   \brief Record rendering commands
	   \param cbuff Recording commandbuffer
	   \param swapIdx Swapchain index
	   \param recordBindPipeline record pipeline beinding if true
	   \param recordBindDescriptorSets record descritpor sets binding if true
	   \param recordUpdateUniforms update the unform if true
	   \param recordBindVboIbo record the vertex and index buffer if true
	   \param recordDrawCalls record the draw calls
	 */
	void recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapIdx, bool recordBindPipeline = true,
	  bool* recordBindDescriptorSets = NULL, bool recordUpdateUniforms = true,
	  bool recordBindVboIbo = true, bool recordDrawCalls = true);

	/*!
	   \brief Return the renderman pipeline
	 */
	RendermanPipeline& toRendermanPipeline() { return *pipelineMaterial_->pipeline_; }

	/*!
	   \brief Return the renderman pipeline (const)
	 */
	const RendermanPipeline& toRendermanPipeline() const { return *pipelineMaterial_->pipeline_; }

	/*!
	   \brief Return the renderman mesh
	 */
	RendermanMesh& toRendermanMesh() { return *subpassMesh_->rendermesh_; }

	/*!
	   \brief Return the renderman mesh (const)
	 */
	const RendermanMesh& toRendermanMesh() const { return *subpassMesh_->rendermesh_; }
};

/*!
   \brief   Part of RendermanStructure. This class stores RendermanNodes and RendermanMaterialEffects
            The list of nodes here references the list of materials.
            It references the Models in the original RendermanModelStore list.
 */
struct RendermanSubpassModel
{
	RendermanModel* renderModel_;
	RendermanSubpass* renderSubpass_;
	std::deque<RendermanSubpassMesh> subpassMeshes; //STORAGE: Deque, so that we can insert elements without invalidating pointers.
	std::deque<RendermanSubpassMaterial> materialeffects;
	std::deque<RendermanNode> nodes;

	/*!
	   \brief Update the current frame
	   \param frame
	 */
	void updateFrame(float32 frame);

	/*!
	   \brief record rendering commands
	   \param cbuff Recoding commandbuffer
	   \param swapIdx Swapchain index
	   \param recordUpdateUniforms update the uniforms if true
	 */
	void recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapIdx, bool recordUpdateUniforms = true);

	/*!
	   \brief return the model this object belongs to.
	 */
	RendermanModel& backToRendermanModel();

	/*!
	   \brief return the manager this object belongs to
	 */
	RenderManager& backToRenderManager();

	/*!
	   \brief Return the subpass this object belongs to.
	 */
	RendermanSubpass& backToRendermanSubpass();

	/*!
	   \brief Return the pass this object belongs to
	 */
	RendermanPass& backToRendermanPass();

	/*!
	   \brief Return the effect this object belongs to.
	 */
	RendermanEffect& backToRendermanEffect();
};

//Part of RendermanStructure. This class is a cooked EffectPipeline, exactly mirroring the PFX pipelines.
//It is affected on creation time by the meshes that use it (for the Vertex Input configuration)
//but after that it is used for rendering directly when traversing the scene.
struct RendermanPipeline
{
	RendermanSubpass* subpass_;
	std::vector<RendermanSubpassMaterial*> subpassMaterials;
	api::GraphicsPipeline apiPipeline;
	api::effect::PipelineDef* pipelineInfo;

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

	/*!
	   \brief Return the subpass this object belongs to.
	 */
	RendermanSubpass& toRendermanSubpass();

	/*!
	   \brief Return the pass this object belongs to.
	 */
	RendermanPass& toRendermanPass();

	/*!
	   \brief Return the effect this object belongs to.
	 */
	RendermanEffect& toRendermanEffect();

	/*!
	   \brief Record and update all the uniform semantics
	   \param cbuff Recording commandbuffer
	 */
	void recordUpdateAllUniformSemantics(api::CommandBufferBase cbuff);

	/*!
	   \brief Record and update all uniform effect semantics
	   \param cbuff Recording comandbuffer
	 */
	void recordUpdateAllUniformEffectSemantics(api::CommandBufferBase cbuff);

	/*!
	   \brief Record updateAllUniformModelSemantics
	   \param cbuff
	 */
	void recordUpdateAllUniformModelSemantics(api::CommandBufferBase cbuff);

	/*!
	   \brief Record update all uniform node semantics
	   \param cbuff Recording commandbuffer
	   \param node Node to update
	 */
	void recordUpdateAllUniformNodeSemantics(api::CommandBufferBase cbuff, RendermanNode& node);

	/*!
	   \brief Record update uniform commands model semantic
	   \param cbuff Recording commandbuffer
	   \param semantic Model semantic to update
	   \return true on success
	 */
	bool recordUpdateUniformCommandsModelSemantic(api::CommandBufferBase cbuff, const StringHash& semantic);

	/*!
	   \brief Record update uniform commands effect semantic
	   \param cbuff Recording commandbuffer
	   \param semantic effect semantic to update
	   \return Return true on success
	 */
	bool recordUpdateUniformCommandsEffectSemantic(api::CommandBufferBase cbuff, const StringHash& semantic);

	/*!
	   \brief RecordUpdateUniformCommandsNodeSemantic
	   \param cbuff
	   \param semantic
	   \param node
	   \return  Return true on success
	 */
	bool recordUpdateUniformCommandsNodeSemantic(api::CommandBufferBase cbuff, const StringHash& semantic, RendermanNode& node);

	/*!
	   \brief Update uniform model semantic
	   \param semantic Model semantic to update
	   \param value New value
	   \return  Return true on success
	 */
	bool updateUniformModelSemantic(const StringHash& semantic, const TypedMem& value);

	/*!
	   \brief Update uniform effect semantic
	   \param semantic Effect semantic to update
	   \param value New value
	   \return  Return true on success
	 */
	bool updateUniformEffectSemantic(const StringHash& semantic, const TypedMem& value);

	/*!
	   \brief Update uniform node semantic
	   \param semantic Node semantic to update
	   \param value New value
	   \param node Node to update
	   \return  Return true on success
	 */
	bool updateUniformNodeSemantic(const StringHash& semantic, const TypedMem& value, RendermanNode& node);

	/*!
	   \brief Update buffer entry model semantic
	   \param semantic Model semantic to update
	   \param value New value
	   \param swapid swapchain id
	   \param dynamicClientId
	   \return  Return true on success
	 */
	bool updateBufferEntryModelSemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid, uint32 dynamicClientId = 0);

	/*!
	   \brief Update buffer entry effect semantic
	   \param semantic Effect semantic to update
	   \param value New value
	   \param swapid swapchain id
	   \param dynamicClientId
	   \return  Return true on success
	 */
	bool updateBufferEntryEffectSemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid, uint32 dynamicClientId = 0);

	/*!
	   \brief Update buffer entry semantic
	   \param semantic Sematic to update
	   \param value New value
	   \param swapid Swapchain id
	   \param dynamicClientId
	   \return  Return true on success
	 */
	bool updateBufferEntrySemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid, uint32 dynamicClientId = 0);

	/*!
	   \brief Update buffer entry node semantic
	   \param semantic Node semantic to update
	   \param value New value
	   \param swapid Swapchain id
	   \param node Node to update
	   \return
	 */
	bool updateBufferEntryNodeSemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid, RendermanNode& node);

	/*!
	   \brief Update buffer entry effect semantics
	   \param semantics Array of semantic to update
	   \param value Array of new values
	   \param numSemantics Number of Semantics update
	   \param swapid Swapchain id
	   \param dynamicClientId
	   \return
	 */
	bool updateBufferEntryEffectSemantics(const StringHash* semantics, const FreeValue* value, uint32 numSemantics,
	                                      uint32 swapid, uint32 dynamicClientId = 0);
	/*!
	   \brief Create automatic model semantics
	   \param useMainModelId
	   \return  Return true on success
	 */
	bool createAutomaticModelSemantics(uint32 useMainModelId = 0);

	/*!
	   \brief Update automatic model semantics
	   \param swapIdx
	   \return  Return true on success
	 */
	bool updateAutomaticModelSemantics(uint32 swapIdx);
};


//Part of RendermanStructure. This class contains the different pipelines, exactly mirroring the PFX subpass
//with the different models. Contained in the Passes
struct RendermanSubpass
{
	RendermanPass* renderingPass_;
	std::deque<RendermanPipeline> pipelines;
	std::deque<RendermanSubpassModel> subpassModels;
	std::deque<RendermanModel*> allModels;

	/*!
	   \brief Return the RendermanPass which this object belongs to (const).
	   \return RendermanPass
	 */
	const RendermanPass& backToRendermanPass()const { return *renderingPass_; }

	/*!
	   \brief Return the RendermanPass which this object belongs to.
	   \return RendermanPass
	 */
	RendermanPass& backToRendermanPass() {  return *renderingPass_; }

	/*!
	   \brief Return the RendermanEffect which this object belongs to (const).
	   \return RendermanEffect
	 */
	const RendermanEffect& backToRendermanEffect()const;

	/*!
	   \brief Return the RendermanEffect which this object belongs to.
	   \return RendermanEffect
	 */
	RendermanEffect& backToRendermanEffect();

	/*!
	   \brief Return the RenderManager which this object belongs to (const).
	   \return RenderManager
	 */
	const RenderManager& backToRenderManager()const;


	/*!
	   \brief Return the RenderManager which this object belongs to.
	   \return RenderManager
	 */
	RenderManager& backToRenderManager();

	/*!
	     \brief Record rendering commands for this subpass
	     \param cbuff Recording Commandbuffer
	     \param swapIdx Swapchain index
	     \param recordUpdateUniforms
	   */
	void recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapIdx, bool recordUpdateUniforms = true);

	/*!
	   \brief Record rendering commands for this subpass with or without begin/ end rendpass
	   \param cbuff
	   \param swapIdx
	   \param beginWithNextSubpassCommand
	   \param recordUpdateUniforms
	 */
	void recordRenderingCommands(api::CommandBuffer& cbuff, uint16 swapIdx, bool beginWithNextSubpassCommand,
	                             bool recordUpdateUniforms = true);

	/*!
	\brief Generates a list of semantics that the pipeline requires, but are changed per-node.
	Necessary to use updateAutomaticSemantics afterwards.
	*/
	void createAutomaticSemantics()
	{
		for (RendermanSubpassModel& subpassModel : subpassModels)
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


	/*!
	\brief Iterates all the nodes semantics per-pipeline and per-model, per-node,
	and updates their values to their specific per-node values.
	Needs to have called createAutomaticSemantics before.
	\param swapidx swapchain index
	*/
	void updateAutomaticSemantics(uint32 swapidx)
	{
		for (RendermanSubpassModel& subpassModel : subpassModels)
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


//Part of RendermanStructure. This class contains the different subpasses, exactly mirroring the PFX pass.
struct RendermanPass
{
	api::FboSet fbo;
	RendermanEffect* renderEffect_;
	std::deque<RendermanSubpass> subpasses;

	/*!
	   \brief Record rendering commands for this pass
	   \param cbuff
	   \param swapIdx
	   \param beginEnderRendermanPass Begin/ End renderpass. If the loadop is "clear", the first Model's clear colour will be used.
	   \param recordUpdateUniforms
	 */
	void recordRenderingCommands(api::CommandBuffer& cbuff, uint16 swapIdx, bool beginEnderRendermanPass, bool recordUpdateUniforms = true);

	/*!
	   \brief Record rendering commands for this pass with begin and renderpass call
	   \param cbuff Recording commandbuffer
	   \param swapIdx Swapchain index
	   \param clearColor Renderpass begin clear color
	   \param recordUpdateUniforms
	 */
	void recordRenderingCommandsWithClearColor(api::CommandBuffer& cbuff, uint16 swapIdx, const glm::vec4& clearColor = glm::vec4(.0f, 0.0f, 0.0f, 1.0f), bool recordUpdateUniforms = true)
	{
		recordRenderingCommands_(cbuff, swapIdx, recordUpdateUniforms, &clearColor);
	}

	/*!
	\brief Generates a list of semantics that the pipeline requires, but are changed per-node.
	Necessary to use updateAutomaticSemantics afterwards.
	*/
	void createAutomaticSemantics()
	{
		for (RendermanSubpass& subpass : subpasses)
		{
			for (RendermanSubpassModel& subpassModel : subpass.subpassModels)
			{
				for (RendermanNode& node : subpassModel.nodes)
				{
					node.createAutomaticSemantics();
				}
			}
			for (RendermanPipeline& pipe : subpass.pipelines)
			{
				pipe.createAutomaticModelSemantics();
			}
		}
	}


	/*!
	\brief Iterates all the nodes semantics per-effect, per-pass, per-subpass, per-model, per-node,
	and updates their values to their specific per-node values.
	Needs to have called createAutomaticSemantics before.
	\param swapidx swapchain index
	*/
	void updateAutomaticSemantics(uint32 swapidx)
	{
		for (RendermanSubpass& subpass : subpasses)
		{
			subpass.updateAutomaticSemantics(swapidx);
		}
	}


	/*!
	     \brief Return subpass (const)
	     \param subpass Subpass index
	     \return RendermanSubpass
	   */
	const RendermanSubpass& toSubpass(uint16 subpass)const
	{
		assertion(subpass < subpasses.size(), "Subpass index out of bound");
		return subpasses[subpass];
	}

	/*!
	   \brief Return subpass
	   \param subpass Subpass index
	   \return RendermanSubpass
	 */
	RendermanSubpass& toSubpass(uint16 subpass)
	{
		assertion(subpass < subpasses.size(), "Subpass index out of bound");
		return subpasses[subpass];
	}
private:
	void recordRenderingCommands_(api::CommandBuffer& cbuff, uint16 swapIdx, bool recordUpdateUniforms, const glm::vec4* clearColor);
};

/*!
   \brief Part of RendermanStructure. This class contains the different passes, exactly mirroring the PFX effect.
          Contains the original EffectApi.
 */
struct RendermanEffect
{
	RenderManager* manager_;
	std::deque<RendermanPass> passes;
	std::deque<RendermanBufferDefinition> bufferDefinitions;

	std::map<StringHash, StructuredMemoryView*> bufferSemantics;
	std::map<StringHash, BufferEntrySemantic> bufferEntrySemantics;
	std::map<StringHash, UniformSemantic> uniformSemantics;
	bool isUpdating[4];
	api::effect::EffectApi effect;

	/*!
	   \brief ctor
	 */
	RendermanEffect() { memset(isUpdating, 0, sizeof(isUpdating)); }

	/*!
	   \brief Return the RenderManager which owns this object (const)
	   \return RenderManager
	 */
	const RenderManager& backToRenderManager() const;

	/*!
	   \brief Return the RenderManager which owns this object
	   \return RenderManager
	 */
	RenderManager& backToRenderManager();

	/*!
	     \brief Begin buffer updates
	     \param swapChainIndex Swapchain index
	   */
	void beginBufferUpdates(uint32 swapChainIndex) { isUpdating[swapChainIndex] = true; }

	/*!
	   \brief End buffer updates
	   \param swapChainIndex Swapchain index
	 */
	void endBufferUpdates(uint32 swapChainIndex);

	/*!
	   \brief Record rendering commands without begin/end renderpass
	   \param cbuff Recording commandbuffer
	   \param swapIdx Swapchain index
	   \param recordUpdateUniforms Update the unforms if true
	 */
	void recordRenderingCommandsNoBeginEndRenderpass(api::CommandBuffer& cbuff, uint16 swapIdx, bool recordUpdateUniforms = true);

	/*!
	   \brief Record rendering commands
	   \param cbuff Recording commandbuffer
	   \param swapIdx Swapchain index
	   \param beginEnderRendermanPasses Begin and end renderpass id true
	   \param recordUpdateUniforms Update the uniforms inf true
	 */
	void recordRenderingCommands(api::CommandBuffer& cbuff, uint16 swapIdx, bool beginEnderRendermanPasses, bool recordUpdateUniforms = true);

	/*!
	\brief Generates a list of semantics that the pipeline requires, but are changed per-node.
	Necessary to use updateAutomaticSemantics afterwards.
	*/
	void createAutomaticSemantics()
	{
		for (RendermanPass& pass : passes)
		{
			pass.createAutomaticSemantics();
		}
	}


	/*!
	\brief Iterates all the nodes semantics per-effect, per-pass, per-subpass, per-model, per-node,
	and updates their values to their specific per-node values.
	Needs to have called createAutomaticSemantics before.
	\param swapidx swapchain index
	*/
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

	/*!
      \brief Return renderman pass object
      \param toPass Pass index
      \return RendermanPass
    */
	RendermanPass& toPass(uint16 toPass) { return passes[toPass]; }

	/*!
	   \brief Return renderman pass object (const)
	   \param toPass Pass index
	   \return RendermanPass
	 */
	const RendermanPass& toPass(uint16 toPass)const { return passes[toPass]; }
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



/*!*************************************************************************************************
\brief The RenderManager is a rendering automation class, with class responsibilities such as:
- Putting together PFX files (Effects) with POD models (Models) to render
- Creating Graphics Pipelines, Descriptor Sets, VBOs, IBOs, UBOs, etc.
- Creating and configuring render - to - texture targets
- Automatically generate command buffers for rendering
- Automatically update textures/uniforms/buffers in the rendering api with info provided by the the model (textures, matrices etc)
\description Basic use:
Create a RenderManager object
Add Effects to it (usually one) : addEffect(...)
Add Models to specific parts of the Effect (Normally, a model is added to be rendered by a specific
subpass (addModelToSubpass), but shortcut methods are provided to add it to entire renderpasses, or even all renderpasses)
Cook the RenderManager : buildRenderObjects(...)
Get rendering commands in command buffers : recordRenderingCommands(...)
(For complete automation) : createAutomaticSemantics(...)
For each frame:
   updateAutomaticSemantics(...)
   submitCommandBuffer(..)
Semantics are "slots" where pieces of information can be put to renders.
For example, a "DIFFUSETEXTURE" semantic may exist where a texture must be added in order to function
as the diffuse texture for a shader, or a "MVP" semantic may exist where a matrix must be uploaded
for the vertex transformation. Automatic semantics are "connections" where this information will be
automatically retrieved from the Model (the scene object).
It is important to realize that semantics exist on different parts of the object, and are expected to be
updated with different rates.
A) Effect - things like, for example, the clear color, that may be common among all objects that use an effect.
B) Model - similar to the effect, it is common for an entire model. Might be the Projection matrix or an ambient colour.
C) Node - things that are specific to an object. Commonly exist on dynamic buffers. Things like the MVP matrix or Textures.
   PFX Bonebatch scope items also end up in nodes (as one node is generated per bonebatch).

Some things to look out for:
The final "renderable" is the Node. Each nodes carries enough information (either directly or through
pointers to "outer" object to render itself. One Node is created for each bonebatch of a node of a model
There are many different intermediate objects to avoid duplications, and different parts of storage
items around the objects.
The distinct phases that can be discerned are:
- Setup (adding Effect(s) and Model(s))
- Object generation (buildRenderObjects())
- Command generation (recordCommandBuffers())
- Memory updates (updateSemantics, updateAutomaticSemantics)
***************************************************************************************************/
class RenderManager
{
	friend class RenderManagerNodeIterator;
public:
	typedef RendermanNode Renderable;

	/*!*************************************************************************************************
	\brief A special and complex iterator class that is used to iterate through ALL renderable objects
	(nodes) of the entire render manager. Unidirectional, sequential. Additionally provides methods
	to know when (from the previous element), the pass, subpass or pipeline has changed. The effect
	of iterating with this class is identical to iterating for each pass, each subpass, each
	subpassmodel, each node.
	***************************************************************************************************/
	struct RendermanNodeIterator
	{
		friend class RenderManager;
	private:
		RenderManager& mgr;
		RendermanNode* cached;
		uint32 nodeId;
		uint32 subpassModelId;
		uint32 subpassId;
		uint32 passId;
		uint32 effectId;
		bool passChanged_;
		bool subpassChanged_;
		bool pipelineChanged_;
		api::GraphicsPipeline::ElementType* pipeline;

		RendermanNodeIterator(RenderManager& mgr, bool begin) : mgr(mgr), nodeId(0), subpassModelId(0), subpassId(0), passId(0), pipeline(NULL)
		{
			effectId = begin ? 0 : (uint32)mgr.renderObjects().effects.size();
			cached = begin ? &mgr.toSubpassModel(0, 0, 0, 0).nodes[0] : NULL;
		}

		void advanceNode(RendermanEffect& eff, RendermanPass& pass, RendermanSubpass& spass, RendermanSubpassModel& spmodel)
		{
			api::GraphicsPipeline::ElementType* old_pipeline = cached->pipelineMaterial_->pipeline_->apiPipeline.get();
			subpassChanged_ = false;
			passChanged_ = false;
			if (++nodeId == spmodel.nodes.size())
			{
				nodeId = 0;
				advanceModeleffect(eff, pass, spass);
			}
			else
			{
				cached = &spmodel.nodes[nodeId];
			}
			pipelineChanged_ = (old_pipeline == cached->pipelineMaterial_->pipeline_->apiPipeline.get());
		}

		void advanceModeleffect(RendermanEffect& eff, RendermanPass& pass, RendermanSubpass& spass)
		{
			if (++subpassModelId == spass.subpassModels.size())
			{
				subpassModelId = 0;
				advanceSubpass(eff, pass);
			}
			else
			{
				cached = &spass.subpassModels[subpassModelId].nodes[0];
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
				cached = &pass.subpasses[subpassId].subpassModels[0].nodes[0];
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
			cached = &eff.passes[passId].subpasses[0].subpassModels[0].nodes[0];
		}
		void advanceEffect()
		{
			if (++effectId == mgr.renderObjects().effects.size())
			{
				cached = NULL;
			}
			else
			{
				cached = &mgr.renderObjects().effects[effectId].passes[0].subpasses[0].subpassModels[0].nodes[0];
			}
		}
	public:
		/*!
		   \brief Return true if the pass has been changed
		 */
		bool passChanged()const { return passChanged_; }

		/*!
		   \brief Return true if the subpass has been changed
		 */
		bool subpassChanged()const { return subpassChanged_; }

		/*!
		   \brief Return true if the pipeline has been changed
		 */
		bool pipelineChanged()const { return pipelineChanged_; }

		/*!
		   \brief operator ==
		   \param rhs
		   \return Return true if equal
		 */
		bool operator==(RendermanNodeIterator& rhs) const
		{
			return (effectId == rhs.effectId) &&
			       (passId == rhs.passId) &&
			       (subpassId == rhs.subpassId) &&
			       (subpassModelId == rhs.subpassModelId) &&
			       (nodeId == rhs.nodeId);
		}

		/*!
		   \brief operator !=
		   \param rhs
		   \return Return true if not equal
		 */
		bool operator!=(RendermanNodeIterator& rhs) const
		{
			return (effectId != rhs.effectId) ||
			       (passId != rhs.passId) ||
			       (subpassId != rhs.subpassId) ||
			       (subpassModelId != rhs.subpassModelId) ||
			       (nodeId != rhs.nodeId);
		}

		/*!
		   \brief Dereference operator *
		   \return Reference to this
		 */
		RendermanNode& operator*()
		{
			return *cached;
		}

		/*!
		   \brief Dereference operator ->
		   \return Pointer to this
		 */
		RendermanNode* operator->()
		{
			return cached;
		}

		/*!
		   \brief Post increment operator
		   \return  Return a copy of this
		 */
		RendermanNodeIterator operator++(int)
		{
			RendermanNodeIterator cpy(*this);
			++(*this);
			return cpy;
		}

		/*!
		   \brief Pre increpement operator.
		   \return Return this
		 */
		RendermanNodeIterator& operator++()
		{
			auto& effect = mgr.toEffect(effectId);
			auto& pass = mgr.toPass(effectId, passId);
			auto& subpass = mgr.toSubpass(effectId, passId, subpassId);
			auto& spmodel = subpass.subpassModels[subpassModelId];
			advanceNode(effect, pass, subpass, spmodel);
			return *this;
		}

	};

	/*!*************************************************************************************************
	\brief This class is a dummy container that provides begin() and end() methods to iterate through
	all nodes of the RenderManager. Extremely useful to use in C++11 range based for:
	for (auto& node : renderManager.getAllRenderables())
	***************************************************************************************************/
	struct RenderManagerNodeIteratorAdapter
	{

        /*!
           \brief Returns an iterator pointing to the first element to the RendermanNode (const).
           \return RendermanNodeIterator
         */
		RendermanNodeIterator begin() const
		{
			return RendermanNodeIterator(mgr, true);
		}

        /*!
           \brief Returns an iterator pointing to the first element to the RendermanNode.
           \return RendermanNodeIterator
         */
		RendermanNodeIterator end() const
		{
			return RendermanNodeIterator(mgr, false);
		}
	private:
		friend class RenderManager;
		RenderManager& mgr;
		RenderManagerNodeIteratorAdapter(RenderManager& mgr) : mgr(mgr) {}
	};

	typedef std::deque<RendermanModel> RendermanModelStorage;
private:
	// effect / pass / subpass
	GraphicsContext context;
	RendermanStructure renderStructure;
	RendermanModelStorage modelStorage; //STORAGE: Deque, so that we can insert elements without invalidating pointers.

	std::map<assets::Mesh*, DynamicArray<AttributeLayout>*> meshAttributeLayout; //points to finalPipeAttributeLayouts
public:
	/*!*************************************************************************************************
	\brief Constructor. Creates an empty rendermanager.
	***************************************************************************************************/
	RenderManager() {}

	/*!*************************************************************************************************
	\brief This method provides a class that functions as a "virtual" node container. Its sole purpose
	is providin begin() and end() methods that iterate through all nodes of the RenderManager.
	Extremely useful to use in C++11 range based for:	for (auto& node : renderManager.renderables())
	***************************************************************************************************/
	RenderManagerNodeIteratorAdapter renderables()
	{
		return RenderManagerNodeIteratorAdapter(*this);
	}

	/*!*************************************************************************************************
	\brief Navigate the structure of the Rendermanager. Goes to the Effect object with index 'effect'
	\param effect The index of the effect to navigate to. The index is the order with which it was added to the RenderManager.
	\return The RendermanEffect object with index 'effect'.
	***************************************************************************************************/
	RendermanEffect& toEffect(uint32 effect) { return renderStructure.effects[effect]; }

	/*!*************************************************************************************************
	\brief Navigate the structure of the Rendermanager. Goes to the Pass object with index 'pass' in
	effect with index 'effect'.
	\param effect The index of the effect the object belongs to. The index is the order with which it was added to the RenderManager.
	\param pass The index of the pass within 'effect' that the object belongs to. The index is the order of the pass in the effect.
	\return The RendermanPass object with index 'pass' in effect 'effect'.
	***************************************************************************************************/
	RendermanPass& toPass(uint32 effect, uint32 pass) { return renderStructure.effects[effect].passes[pass]; }

	/*!*************************************************************************************************
	\brief Navigate the structure of the Rendermanager. Goes to the Subpass object #'subpass' in pass #'pass'
	in effect #'effect'.
	\param effect The index of the effect the object belongs to. The index is the order with which it was added to the RenderManager.
	\param pass The index of the pass within 'effect' that the object belongs to. The index is the order of the pass in the effect.
	\param subpass The index of the subpass within 'pass' that the object belongs to. The index is the order of the subpass in the pass.
	\return The RendermanSubpass object with index 'subpass' in pass 'pass' of effect 'effect'.
	***************************************************************************************************/
	RendermanSubpass& toSubpass(uint32 effect, uint32 pass, uint32 subpass)
	{
		return renderStructure.effects[effect].passes[pass].subpasses[subpass];
	}

	/*!*************************************************************************************************
	\brief Navigate the structure of the Rendermanager. Goes to the Pipeline object #'pipeline' in
	subpass object #'subpass' in pass #'pass' in effect #'effect'.
	\param effect The index of the effect the object belongs to. The index is the order with which it was added to the RenderManager.
	\param pass The index of the pass within 'effect' that the object belongs to. The index is the order of the pass in the effect.
	\param subpass The index of the subpass within 'pass' that the object belongs to. The index is the order of the subpass in the pass.
	\param pipeline The index of the pipeline within 'subpass'. The index is the order of the pipeline in the pass.
	\return The RendermanPipeline object with index 'pipeline' in subpass 'subpass' in pass 'pass' of effect 'effect'.
	***************************************************************************************************/
	RendermanPipeline& toPipeline(uint32 effect, uint32 pass, uint32 subpass, uint32 pipeline)
	{
		return renderStructure.effects[effect].passes[pass].subpasses[subpass].pipelines[pipeline];
	}

	/*!*************************************************************************************************
	\brief Navigate the structure of the Rendermanager. Goes to a SubpassModel object. A SubpassModel
	object is the data held for a Model when it is added to a specific Subpass.
	\param effect The index of the effect the object belongs to. The index is the order with which it was added to the RenderManager.
	\param pass The index of the pass within 'effect' that the object belongs to. The index is the order of the pass in the effect.
	\param subpass The index of the subpass within 'pass' that the object belongs to. The index is the order of the subpass in the pass.
	\param model The index of the model within 'subpass'. The index is the order in which this model was added to Subpass.
	\return The RendermanPipeline object with index 'pipeline' in subpass 'subpass' in pass 'pass' of effect 'effect'.
	***************************************************************************************************/
	RendermanSubpassModel& toSubpassModel(uint32 effect, uint32 pass, uint32 subpass, uint32 model)
	{
		return renderStructure.effects[effect].passes[pass].subpasses[subpass].subpassModels[model];
	}

	/*!*************************************************************************************************
	\brief Navigate the structure of the Rendermanager. Goes to the Model object with index 'model'.
	\param model The index of the model. The index is the order with which it was added to the RenderManager.
	\return The RendermanModel object with index 'model' in the RenderManager'.
	***************************************************************************************************/
	RendermanModel& toModel(uint32 model) { return modelStorage[model]; }

	/*!*************************************************************************************************
	\brief Navigate the structure of the Rendermanager. Goes to the Mesh object #'mesh' of the model #'model'.
	\param model The index of the rendermodel this mesh belongs to. The index is the order with which it was added to the RenderManager.
	\param mesh The index of the rendermesh in the model. The index of the mesh is the same as it was in the pvr::assets::Model file.
	\return The RendermanMesh object with index 'mesh' in the model #'model'.
	***************************************************************************************************/
	RendermanMesh& toRendermanMesh(uint32 model, uint32 mesh) { return modelStorage[model].meshes[mesh]; }

	/*!*************************************************************************************************
	\brief Navigate the structure of the Rendermanager. Access a Mesh through its effect (as opposed to
	through its model object. The purpose of this function is to find the object while navigating a subpass.
	\param effect The index of the effect in the RenderManager.
	\param pass The index of the pass in the Effect.
	\param subpass The index of the subpass in the Pass.
	\param model The index of the model in the Subpass. Caution- this is not the same as the index of the model in the RenderManager.
	\param mesh The index of the mesh in the model. This is the same as the index of the pvr::assets::Mesh in the pvr::assets::Model.
	\return The selected RendermanMesh item.
	***************************************************************************************************/
	RendermanMesh& toRendermanMeshByEffect(uint32 effect, uint32 pass, uint32 subpass, uint32 model, uint32 mesh)
	{
		return renderStructure.effects[effect].passes[pass].subpasses[subpass].allModels[model]->meshes[mesh];
	}

	/*!*************************************************************************************************
	\brief Get a reference to the entire render structure of the RenderManager. Raw.
	\return A reference to the data structure.
	***************************************************************************************************/
	RendermanStructure& renderObjects() { return renderStructure; }

	/*!*************************************************************************************************
	\brief Get a reference to the storage of Models in the RenderManager. Raw.
	\return A reference to the Models.
	***************************************************************************************************/
	RendermanModelStorage& renderModels() { return modelStorage; }

	/*!*************************************************************************************************
	\brief Get the context that this RenderManager uses.
	\return The context that this RenderManager uses.
	***************************************************************************************************/
	GraphicsContext& getContext() { return context; }

	/*!*************************************************************************************************
	\brief Get the context that this RenderManager uses.
	\return The context that this RenderManager uses.
	***************************************************************************************************/
	const GraphicsContext& getContext() const { return context; }

	/*!*************************************************************************************************
	\brief Add a model for rendering. This method is a shortcut for adding a model to ALL renderpasses,
	ALL subpasses.
	\param model A pvr::assets::Model handle to add for rendering. Default 0.
	\param effect The Effect to whose subpasses the model will be added.
	\return The order of the model within the RenderManager (the index to use for getModel).
	***************************************************************************************************/
	int32 addModelForAllPasses(const assets::ModelHandle& model, uint16 effect = 0)
	{
		int32 index = -1;
		for (std::size_t pass = 0; pass != renderStructure.effects[effect].passes.size(); ++pass)
		{
			index = addModelForAllSubpasses(model, (uint16)pass, effect);
		}
		return index;
	}

	/*!*************************************************************************************************
	\brief Add a model for rendering. This method is a shortcut for adding a model to ALL subpasses of a
	specific renderpass.
	\param model A pvr::assets::Model handle to add for rendering.
	\param effect The Effect to which the pass to render to belongs. Default 0.
	\param pass The Pass of the Effect to whose subpasses the model will be added.
	\return The order of the model within the RenderManager (the index to use for getModel).
	***************************************************************************************************/
	int32 addModelForAllSubpasses(const assets::ModelHandle& model, uint16 pass, uint16 effect = 0)
	{
		int32 index = -1;
		for (uint32 subpass = 0; subpass != renderStructure.effects[effect].passes[pass].subpasses.size(); ++subpass)
		{
			index = addModelForSubpass(model, pass, (uint16)subpass, effect);
		}
		return index;
	}

	/*!*************************************************************************************************
	\brief Add a model for rendering. Adds a model for rendering to a specific subpass.
	\param model A pvr::assets::Model handle to add for rendering.
	\param effect The Effect to which the subpass to render to belongs. Default 0.
	\param pass The Pass of the Effect to whose subpass the model will be added.
	\param subpass The Subpass to which this model will be rendered.
	\return The order of the model within the RenderManager (the index to use for getModel).
	***************************************************************************************************/
	int32 addModelForSubpass(const assets::ModelHandle& model, uint16 pass, uint16 subpass, uint16 effect = 0)
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

		renderStructure.effects[effect].passes[pass].subpasses[subpass].allModels.push_back(apimodel);
		return index;
	}

	/*!*************************************************************************************************
	\brief Add an effect to the RenderManager. Must be called before models are added to this effect.
	  \param effect A new effect to be added
	  \param context GraphicsContext who will own the resources
	  \param assetLoader  AssetLoader
	  \return The order of the Effect within the RenderManager (the index to use for toEffect()).
	          Return -1 if error.
	***************************************************************************************************/
	uint32 addEffect(const assets::effect::Effect& effect, GraphicsContext& context, api::AssetLoadingDelegate& assetLoader)
	{
		//Validate
		if (context.isNull())
		{
			assertion(false, "RenderManager: Invalid Context");
			Log("RenderManager: Invalid Context");
			return (uint32) - 1;
		}
		pvr::api::effect::EffectApi effectapi;
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
			new_effect.passes[passId].subpasses.resize(effectapi->getPass(passId).subpasses.size());
			new_effect.passes[passId].fbo = effectapi->getPass(passId).fbos;
		}
		return uint32(renderStructure.effects.size() - 1);
	}

	/*!*************************************************************************************************
	\brief Generate the RenderManager, create the structure, add all rendering effects, create the API objects,
	and in general, cook everything.
	Call AFTER any calls to addEffect(...) and addModel...(...).
	Call BEFORE any calls to createAutomaticSemantics(...), update semantics etc.
	\return True if successful.
	***************************************************************************************************/
	bool buildRenderObjects();

	/*!
	\brief Create rendering commands for all objects added to the RenderManager. Will iterate the entire
	render structure recording any necessary binding/drawing commands into cbuff.
	If finer granularity of rendering commands is required, navigate the RenderStructure's objects
	and record rendering commands from them.
	   \param cbuff Recording Commandbuffer
	   \param swapIdx Swapchain index
	   \param recordBeginEndRenderpass Record with begin and end renderpass command
	   \param recordUpdateUniforms
	 */
	void recordAllRenderingCommands(
	  api::CommandBuffer& cbuff, uint16 swapIdx, bool recordBeginEndRenderpass = true,
	  bool recordUpdateUniforms = true);

	/*!
	   \brief Return number of effects this render manager owns
	   \return Number of effects
	 */
	size_t getNumberOfEffects()const { return renderStructure.effects.size(); }

	/*!
	   \brief Generates a list of semantics that the pipeline requires, but are changed per-node.
	          Necessary to use updateAutomaticSemantics afterwards.
	 */
	void createAutomaticSemantics()
	{
		for (RendermanEffect& effect : renderStructure.effects)
		{
			for (RendermanPass& pass : effect.passes)
			{
				for (RendermanSubpass& subpass : pass.subpasses)
				{
					for (RendermanSubpassModel& subpassModel : subpass.subpassModels)
					{
						for (RendermanNode& node : subpassModel.nodes)
						{
							node.createAutomaticSemantics();
						}
					}
					for (RendermanPipeline& pipe : subpass.pipelines)
					{
						pipe.createAutomaticModelSemantics();
					}
				}
			}
		}
	}


	/*!
	   \brief Iterates all the nodes semantics per-effect, per-pass, per-subpass, per-model, per-node,
	          and updates their values to their specific per-node values.
	          Needs to have called createAutomaticSemantics before.
	   \param swapidx swapchain index
	 */
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
	case types::GpuDatatypes::vec2: cbuff->setUniformPtr<glm::vec2>(uniformLocation, value_ptr.arrayElements(), (glm::vec2*)value_ptr.raw()); break;
	case types::GpuDatatypes::vec3: cbuff->setUniformPtr<glm::vec3>(uniformLocation, value_ptr.arrayElements(), (glm::vec3*)value_ptr.raw()); break;
	case types::GpuDatatypes::vec4: cbuff->setUniformPtr<glm::vec4>(uniformLocation, value_ptr.arrayElements(), (glm::vec4*)value_ptr.raw()); break;
	case types::GpuDatatypes::float32: cbuff->setUniformPtr<float32>(uniformLocation, value_ptr.arrayElements(), (float32*)value_ptr.raw()); break;
	case types::GpuDatatypes::integer: cbuff->setUniformPtr<int32>(uniformLocation, value_ptr.arrayElements(), (int32*)value_ptr.raw()); break;
	case types::GpuDatatypes::ivec2: cbuff->setUniformPtr<glm::ivec2>(uniformLocation, value_ptr.arrayElements(), (glm::ivec2*)value_ptr.raw()); break;
	case types::GpuDatatypes::ivec3: cbuff->setUniformPtr<glm::ivec3>(uniformLocation, value_ptr.arrayElements(), (glm::ivec3*)value_ptr.raw()); break;
	case types::GpuDatatypes::ivec4: cbuff->setUniformPtr<glm::ivec4>(uniformLocation, value_ptr.arrayElements(), (glm::ivec4*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat2x2: cbuff->setUniformPtr<glm::mat2x2>(uniformLocation, value_ptr.arrayElements(), (glm::mat2x2*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat2x3: cbuff->setUniformPtr<glm::mat2x3>(uniformLocation, value_ptr.arrayElements(), (glm::mat2x3*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat2x4: cbuff->setUniformPtr<glm::mat2x4>(uniformLocation, value_ptr.arrayElements(), (glm::mat2x4*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat3x2: cbuff->setUniformPtr<glm::mat3x2>(uniformLocation, value_ptr.arrayElements(), (glm::mat3x2*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat3x3: cbuff->setUniformPtr<glm::mat3x3>(uniformLocation, value_ptr.arrayElements(), (glm::mat3x3*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat3x4: cbuff->setUniformPtr<glm::mat3x4>(uniformLocation, value_ptr.arrayElements(), (glm::mat3x4*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat4x2: cbuff->setUniformPtr<glm::mat4x2>(uniformLocation, value_ptr.arrayElements(), (glm::mat4x2*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat4x3: cbuff->setUniformPtr<glm::mat4x3>(uniformLocation, value_ptr.arrayElements(), (glm::mat4x3*)value_ptr.raw()); break;
	case types::GpuDatatypes::mat4x4: cbuff->setUniformPtr<glm::mat4x4>(uniformLocation, value_ptr.arrayElements(), (glm::mat4x4*)value_ptr.raw()); break;
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
inline RendermanSubpass& RendermanPipeline::toRendermanSubpass() { return *subpass_; }

inline RendermanPass& RendermanPipeline::toRendermanPass() { return *subpass_->renderingPass_; }

inline RendermanEffect& RendermanPipeline::toRendermanEffect() { return *subpass_->renderingPass_->renderEffect_; }

inline void RendermanPipeline::recordUpdateAllUniformSemantics(api::CommandBufferBase cbuff)
{
	for (auto& sem : uniformSemantics)
	{
		recordUpdateUniformSemanticToExternalMemory(cbuff, sem.second.uniformLocation, sem.second.memory);
	}
	for (auto& sem : subpass_->renderingPass_->renderEffect_->uniformSemantics)
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
	for (auto& sem : subpass_->renderingPass_->renderEffect_->uniformSemantics)
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
	auto& cont = subpass_->renderingPass_->renderEffect_->uniformSemantics;
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
		return false;
	}
	auto& sem = it->second;
	sem.buffer->map(swapid);
	sem.buffer->setArrayValue(sem.entryIndex, dynamicClientId, value);
	sem.buffer->unmap(swapid);
	return true;
}

inline bool RendermanPipeline::updateBufferEntryEffectSemantic(
  const StringHash& semantic, const FreeValue& value, uint32 swapid, uint32 dynamicClientId)
{
	auto& cont = subpass_->renderingPass_->renderEffect_->bufferEntrySemantics;
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
	auto& effect = toRendermanEffect();
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
	bool multibuffered = it->second.buffer->getMultibufferSize() > 0;
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

	if (sem.dynamicOffsetNodeId >= 0)
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
		auto& cont = subpass_->renderingPass_->renderEffect_->bufferEntrySemantics;
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
	auto& effect = toRendermanEffect();
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
	bool multibuffered = it->second.buffer->getMultibufferSize() > 0;
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
	auto& cont = subpass_->renderingPass_->renderEffect_->uniformSemantics;
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

		auto& model = toRendermanEffect().manager_->renderModels()[useMainModelId];

		auto& cont = subpass_->renderingPass_->renderEffect_->bufferEntrySemantics;

		for (auto& reqsem : cont)
		{
			ModelSemanticSetter setter = model.getModelSemanticSetter(reqsem.first);
			if (setter == NULL)
			{
				Log(Log.Information, "Automatic Model semantic [%s] not found.", reqsem.first.c_str());
			}
			else
			{
				Log(Log.Information, "Automatic Model semantic [%s] found! Creating automatic connection with model [%d]:", reqsem.first.c_str(), useMainModelId);
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

		auto& model = toRendermanEffect().manager_->renderModels()[useMainModelId];

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
				Log(Log.Information, "Automatic Model semantic [%s] found! Creating automatic connection with model [%d]:", reqsem.first.c_str(), useMainModelId);
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
		bool wasUpdating = this->toRendermanEffect().isUpdating[swapidx];
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
		bool wasUpdating = this->toRendermanPipeline().toRendermanEffect().isUpdating[swapidx];
		bool mustMap = !sem.buffer->getConnectedBuffer(tmpswapidx)->isMapped();
		if (mustMap)
		{
			sem.buffer->map(tmpswapidx);
		}

		uint32 dynamicClientId = 0;

		if (sem.dynamicOffsetNodeId >= 0)
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

inline void RendermanSubpassModel::updateFrame(float32 frame)
{
	renderModel_->assetModel->setCurrentFrame(frame);
	//model->assetModel->updateModelSemantics();
}

//                                      RendermanSubpassMaterial inline definition
inline const RendermanModel& RendermanSubpassMaterial::backToModel() const { return *backToSubpassModel().renderModel_; }
inline RendermanModel& RendermanSubpassMaterial::backToModel() { return *backToSubpassModel().renderModel_; }
inline RendermanSubpass& RendermanSubpassMaterial::backToSubpass() { return *backToSubpassModel().renderSubpass_; }
inline const RendermanSubpass& RendermanSubpassMaterial::backToSubpass() const { return *backToSubpassModel().renderSubpass_; }

//                                      RendermanSubpassMesh inline definition
inline const RendermanModel& RendermanSubpassMesh::backToRendermanModel() const { return *backToSubpassModel().renderModel_; }
inline RendermanModel& RendermanSubpassMesh::backToRendermanModel() { return *backToSubpassModel().renderModel_; }
inline const RendermanSubpass& RendermanSubpassMesh::backToRendermanSubpass() const { return *backToSubpassModel().renderSubpass_; }
inline RendermanSubpass& RendermanSubpassMesh::backToRendermanSubpass() { return *backToSubpassModel().renderSubpass_; }

//                                      RendermanSubpassModel inline definition
inline RendermanModel& RendermanSubpassModel::backToRendermanModel() { return *renderModel_; }
inline RenderManager& RendermanSubpassModel::backToRenderManager() { return *renderModel_->mgr_; }
inline RendermanSubpass& RendermanSubpassModel::backToRendermanSubpass() { return *renderSubpass_; }
inline RendermanPass& RendermanSubpassModel::backToRendermanPass() { return *renderSubpass_->renderingPass_; }
inline RendermanEffect& RendermanSubpassModel::backToRendermanEffect() { return *renderSubpass_->renderingPass_->renderEffect_; }

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
}
}
