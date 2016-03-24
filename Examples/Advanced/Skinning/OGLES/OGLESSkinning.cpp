/*!*********************************************************************************************************************
\File         OGLESSkinning.cpp
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to perform skinning combined with Dot3 (normal-mapped) lighting
***********************************************************************************************************************/
#include "PVRApi/PVRApi.h"
#include "PVRShell/PVRShell.h"
#include "PVRUIRenderer/UIRenderer.h"
using namespace pvr::types;
// shader uniforms
namespace SkinnedUniforms {
enum Enum
{
	ViewProj, LightPos, BoneCount, BoneMatrices, BoneMatricesIT, Count
};
const char* Names[] = { "ViewProjMatrix", "LightPos", "BoneCount", "BoneMatrixArray[0]", "BoneMatrixArrayIT[0]" };
}

namespace SkinnedAttributes {
enum Enum
{
	Position, Normal, Tangent, Bitangent, Texcoord, BoneWeight, BoneIndex, Count
};

// Definition of the vertex attributes our meshes are using
pvr::utils::VertexBindings_Name AttributesBindings[] =
{
	{ "POSITION", "inVertex" },
	{ "NORMAL", "inNormal" },
	{ "TANGENT", "inTangent" },
	{ "BINORMAL", "inBiNormal" },
	{ "UV0", "inTexCoord" },
	{ "BONEWEIGHT", "inBoneWeight" },
	{ "BONEINDEX", "inBoneIndex" },
};
}
// shader uniforms
namespace DefaultUniforms {
enum Enum
{
	WorldMatrix, MVPMatrix, MWITMatrix, LightPos, Count
};
const char* Names[] = { "ModelMatrix", "MVPMatrix", "ModelWorldIT3x3", "LightPos" };
}

namespace DefaultAttributes {
enum Enum
{
	Position, Normal, Texcoord, Count
};

pvr::utils::VertexBindings_Name Bindings[] =
{
	{ "POSITION", "inVertex" },
	{ "NORMAL", "inNormal" },
	{ "UV0", "inTexCoord" },
};
}

namespace Configuration {
// Filenames of our shaders
const char SkinnedFragmentFilename[]	= "SkinnedFragShader.fsh";
const char SkinnedVertexFilename[]		= "SkinnedVertShader.vsh";
const char DefaultFragmentFilename[]	= "DefaultFragShader.fsh";
const char DefaultVertexFilename[]		= "DefaultVertShader.vsh";

// POD scene files
const char SceneFile[]					= "Robot.pod";
}

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class OGLESSkinning : public pvr::Shell
{
	//Put all API managed objects in a struct so that we can one-line free them...
	struct ApiObjects
	{
		pvr::api::ParentableGraphicsPipeline skinnedPipeline;
		pvr::api::GraphicsPipeline defaultPipeline;
		// Print3D class used to display text
		pvr::ui::UIRenderer uiRenderer;

		pvr::api::AssetStore assetManager;
		pvr::api::Sampler samplerTrilinear;
		pvr::api::Sampler samplerBilinear;

		// OpenGL handles for shaders and VBOs
		std::vector<pvr::api::Buffer> vbos;
		std::vector<pvr::api::Buffer> ibos;
		std::vector<pvr::api::DescriptorSet> descriptorSets;

		pvr::GraphicsContext context;
		pvr::api::CommandBuffer commandBuffer;
		pvr::api::Fbo fboOnScreen;
	};
	std::auto_ptr<ApiObjects> apiObj;
	// 3D Model
	pvr::assets::ModelHandle scene;

	bool isPaused;

	// Group shader programs and their uniform uniformLocations together
	pvr::int32 skinnedUniformLocations[SkinnedUniforms::Count];
	// Group shader programs and their uniform uniformLocations together
	pvr::int32 defaultUniformLocations[DefaultUniforms::Count];

	// Variables to handle the animation in a time-based manner
	float currentFrame;

	struct BoneMatrixMem
	{
		glm::mat4x4 boneWorld[8];
		glm::mat3x3 boneWorldIT[8];
	};
	struct UniformsNonSkinned
	{
		glm::mat4 mvp;
		glm::mat4 modelWorld;
		glm::mat3 modelWorldIT;
	};
	struct Uniforms
	{
		std::vector<std::vector<BoneMatrixMem>/**/> perSkinnedMesh;
		std::vector<UniformsNonSkinned> perNonSkinnedMesh;
		glm::mat4 viewProj;
		glm::vec3 lightPos;
	} uniforms;
	glm::mat4 proj;

public:
	OGLESSkinning() : currentFrame(0), isPaused(false) {}

	pvr::Result::Enum initApplication();
	pvr::Result::Enum initView();
	pvr::Result::Enum releaseView();
	pvr::Result::Enum quitApplication();
	pvr::Result::Enum renderFrame();

	bool createDescriptors(pvr::api::DescriptorSetLayout& defaultLayout, pvr::api::DescriptorSetLayout& skinnedLayout);
	bool createPipelines(const pvr::api::DescriptorSetLayout& defaultLayout, const pvr::api::DescriptorSetLayout& skinnedLayout);
	bool loadBuffers();
	void updateBonesAndMatrices();
	void updateAnimation();
	void updateCameraAndLight();
	void recordCommandBuffer();
	void eventMappedInput(pvr::SimplifiedInput::Enum action);
};

/*!*********************************************************************************************************************
\brief	handle the input event
\param	action input actions to handle
***********************************************************************************************************************/
void OGLESSkinning::eventMappedInput(pvr::SimplifiedInput::Enum action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3:
		isPaused = !isPaused; break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}
}

/*!*********************************************************************************************************************
\brief	Loads the textures required for this training course
\param[out]	outSkinnedLayout A std::string describing the error on failure
\return		Return true if no error occurred
***********************************************************************************************************************/
bool OGLESSkinning::createDescriptors(pvr::api::DescriptorSetLayout& outDefaultLayout, pvr::api::DescriptorSetLayout& outSkinnedLayout)
{
	// Texture IDs
	std::vector<pvr::api::TextureView> textures;
	for (int i = 0; i < scene->getNumTextures(); ++i)
	{
		const pvr::assets::Model::Texture& tex = scene->getTexture(i);
		if (!apiObj->assetManager.getTextureWithCaching(getGraphicsContext(), tex.getName(), NULL, NULL))
		{
			setExitMessage("Could not load the texture %s", tex.getName().c_str());
			return false;
		}
	}

	// create descriptor set layout
	pvr::api::DescriptorSetLayoutCreateParam defaultLayoutDesc;
	defaultLayoutDesc.setBinding(0, DescriptorType::CombinedImageSampler, ShaderStageFlags::Fragment);
	outDefaultLayout = apiObj->context->createDescriptorSetLayout(defaultLayoutDesc);

	//-- create descriptor set layout
	pvr::api::DescriptorSetLayoutCreateParam skinnedLayoutDesc;
	skinnedLayoutDesc.setBinding(0, DescriptorType::CombinedImageSampler, ShaderStageFlags::Fragment)
	.setBinding(1, DescriptorType::CombinedImageSampler, ShaderStageFlags::Fragment);

	outSkinnedLayout = apiObj->context->createDescriptorSetLayout(skinnedLayoutDesc);

	pvr::assets::SamplerCreateParam desc;
	desc.magnificationFilter = SamplerFilter::Linear;
	desc.minificationFilter = SamplerFilter::Linear;
	desc.mipMappingFilter = SamplerFilter::Linear;
	apiObj->samplerTrilinear = apiObj->context->createSampler(desc);

	desc.mipMappingFilter = SamplerFilter::None;
	apiObj->samplerBilinear = apiObj->context->createSampler(desc);
	apiObj->descriptorSets.resize(scene->getNumMaterials());

	for (pvr::uint32 i = 0; i < scene->getNumMaterials(); ++i)
	{
		//We are using one texture atlas for this model, regardless of the amount of meshes.
		pvr::api::DescriptorSetUpdate descSetWrite; const pvr::assets::Material& mat = scene->getMaterial(i);
		if (mat.getDiffuseTextureIndex() == -1)
		{
			setExitMessage("This demo does not support non-textured materials.");
			return false;
		}
		const pvr::StringHash& textureName = scene->getTexture(mat.getDiffuseTextureIndex()).getName();
		pvr::api::TextureView tex;
		apiObj->assetManager.getTextureWithCaching(getGraphicsContext(), textureName, &tex, NULL);
		descSetWrite.setCombinedImageSampler(0, tex, apiObj->samplerTrilinear);
		if (mat.getBumpMapTextureIndex() != -1)
		{
			const pvr::StringHash& bumpTextureName = scene->getTexture(mat.getBumpMapTextureIndex()).getName();
			apiObj->assetManager.getTextureWithCaching(getGraphicsContext(), bumpTextureName, &tex, NULL);
			descSetWrite.setCombinedImageSampler(1, tex, apiObj->samplerBilinear);
			apiObj->descriptorSets[i] = apiObj->context->createDescriptorSetOnDefaultPool(outSkinnedLayout);
		}
		else
		{
			apiObj->descriptorSets[i] = apiObj->context->createDescriptorSetOnDefaultPool(outDefaultLayout);
		}
		apiObj->descriptorSets[i]->update(descSetWrite);
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool OGLESSkinning::createPipelines(const pvr::api::DescriptorSetLayout& defaultLayout, const pvr::api::DescriptorSetLayout& skinnedLayout)
{
	pvr::api::logApiError("CreatePipelines"); //Clear any errors we may have.

	// Create the pipelines. CAUTION: We are assuming two specific effects for this demo:
	// 1) Bumpmapped, Skinned
	// 2) Non-Bumpmapped, Not Skinned
	// THESE MUST OBVIOUSLY BE COMPATIBLE WITH THE VERTEX ATTRIBUTES WE GAVE DEFINED IN Configuration::SkinnedVertexAttributes, DefaultVertexAttributes
	// Obviously, in order to render "arbitrary" meshes, you would need a suite of shaders
	// and some kind of heuristic to pick among them.
	int skinnedMeshId = -1, nonSkinnedMeshId = -1;
	for (int i = 0; i < scene->getNumMeshes() && (skinnedMeshId == -1 || nonSkinnedMeshId == -1); ++i)
	{
		bool skinned = (scene->getMesh(i).getVertexAttributeIndex("BONEWEIGHT") != -1);
		if (skinned && skinnedMeshId == -1) { skinnedMeshId = i; }
		if (!skinned && nonSkinnedMeshId == -1) { nonSkinnedMeshId = i; }
	}

	//Create skinned pipeline
	if (skinnedMeshId != -1)
	{
		pvr::api::GraphicsPipelineCreateParam skinnedDesc;
		pvr::api::PipelineLayoutCreateParam skinnedPipeLayoutCreateParam;
		skinnedPipeLayoutCreateParam.addDescSetLayout(skinnedLayout);
		skinnedDesc.pipelineLayout = apiObj->context->createPipelineLayout(skinnedPipeLayoutCreateParam);
		skinnedDesc.depthStencil.setDepthTestEnable(true);
		skinnedDesc.colorBlend.addAttachmentState(pvr::api::pipelineCreation::ColorBlendAttachmentState());
		//Depth test TRUE is default, depth func LESS is default
		pvr::assets::ShaderFile vertShaderFile(Configuration::SkinnedVertexFilename, *this);
		pvr::assets::ShaderFile fragShaderFile(Configuration::SkinnedFragmentFilename, *this);

		pvr::Stream::ptr_type skinVertShader = vertShaderFile.getBestStreamForApi(getApiType());
		pvr::Stream::ptr_type skinFragShader = fragShaderFile.getBestStreamForApi(getApiType());
		skinnedDesc.vertexShader.setShader(apiObj->context->createShader(*skinVertShader, ShaderType::VertexShader));
		skinnedDesc.fragmentShader.setShader(apiObj->context->createShader(*skinFragShader, ShaderType::FragmentShader));

		pvr::utils::createInputAssemblyFromMesh(scene->getMesh(skinnedMeshId), SkinnedAttributes::AttributesBindings, SkinnedAttributes::Count, skinnedDesc);
		//create the skin pipeline, it is mutable after creation
		apiObj->skinnedPipeline = apiObj->context->createParentableGraphicsPipeline(skinnedDesc);
		// Store the location of uniforms for later use
		apiObj->skinnedPipeline->getUniformLocation(SkinnedUniforms::Names, SkinnedUniforms::Count, skinnedUniformLocations);
	}

	//Create skinned pipeline
	if (nonSkinnedMeshId != -1)
	{
		pvr::api::GraphicsPipelineCreateParam defaultDesc;
		pvr::api::PipelineLayoutCreateParam defaultPipeLayoutCreateParam;
		defaultPipeLayoutCreateParam.addDescSetLayout(defaultLayout);
		defaultDesc.pipelineLayout = apiObj->context->createPipelineLayout(defaultPipeLayoutCreateParam);
		defaultDesc.colorBlend.addAttachmentState(pvr::api::pipelineCreation::ColorBlendAttachmentState());
		defaultDesc.depthStencil.setDepthTestEnable(true);
		pvr::assets::ShaderFile vertShaderFile(Configuration::DefaultVertexFilename, *this);
		pvr::assets::ShaderFile fragShaderFile(Configuration::DefaultFragmentFilename, *this);

		pvr::Stream::ptr_type defaultVertShader = vertShaderFile.getBestStreamForApi(getApiType());
		pvr::Stream::ptr_type defaultFragShader = fragShaderFile.getBestStreamForApi(getApiType());
		defaultDesc.vertexShader.setShader(apiObj->context->createShader(*defaultVertShader, ShaderType::VertexShader));
		defaultDesc.fragmentShader.setShader(apiObj->context->createShader(*defaultFragShader, ShaderType::FragmentShader));

		pvr::utils::createInputAssemblyFromMesh(scene->getMesh(nonSkinnedMeshId), DefaultAttributes::Bindings, DefaultAttributes::Count, defaultDesc);
		//create the skin pipeline, it is mutable after creation
		apiObj->defaultPipeline = apiObj->context->createParentableGraphicsPipeline(defaultDesc);

		// Store the location of uniforms for later use
		apiObj->defaultPipeline->getUniformLocation(DefaultUniforms::Names, DefaultUniforms::Count, defaultUniformLocations);
	}

	std::string error;
	if (pvr::api::checkApiError(&error) != pvr::Result::Success)
	{
		pvr::Log(pvr::Log.Critical, "CreatePipelines finished with error \"%s\". This will ultimately prove fatal...", pvr::Log.Error, error.c_str());
		return false;
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result::Enum OGLESSkinning::initApplication()
{
	pvr::assets::PODReader podReader(getAssetStream(Configuration::SceneFile));
	if ((scene = pvr::assets::Model::createWithReader(podReader)).isNull())
	{
		setExitMessage("Error: Could not create the scene file %s.", Configuration::SceneFile);
		return pvr::Result::NoData;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (scene->getNumCameras() == 0)
	{
		setExitMessage("Error: The scene does not contain a camera.");
		return pvr::Result::NoData;
	}

	// Check the scene contains at least one light
	if (scene->getNumLights() == 0)
	{
		setExitMessage("Error: The scene does not contain a light.");
		return pvr::Result::NoData;
	}

	uniforms.perSkinnedMesh.clear();

	uniforms.perNonSkinnedMesh.clear();
	for (size_t meshNodeID = 0; meshNodeID < scene->getNumMeshNodes(); ++meshNodeID)
	{
		using namespace pvr::assets;
		const Node& node = scene->getNode(meshNodeID);
		const Mesh& mesh = scene->getMesh(node.getObjectId());
		if (mesh.getMeshInfo().isSkinned)
		{
			uniforms.perSkinnedMesh.push_back(std::vector<BoneMatrixMem>());
			uniforms.perSkinnedMesh.back().resize(mesh.getNumBoneBatches());
		}
		else
		{
			uniforms.perNonSkinnedMesh.push_back(UniformsNonSkinned());
		}
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by Shell once per run, just before exiting the program.
		If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result::Enum OGLESSkinning::quitApplication()
{
	scene.reset();
	uniforms.perNonSkinnedMesh.clear();
	uniforms.perSkinnedMesh.clear();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result::Enum OGLESSkinning::initView()
{
	apiObj.reset(new ApiObjects);
	apiObj->assetManager.init(*this);

	currentFrame = 0.;
	apiObj->context = getGraphicsContext();
	apiObj->commandBuffer = apiObj->context->createCommandBufferOnDefaultPool();

	// automatically populate the VBOs and IBOs from the Model.
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, apiObj->vbos, apiObj->ibos);
	pvr::api::DescriptorSetLayout defaultLayout;
	pvr::api::DescriptorSetLayout skinnedLayout;

	if (!createDescriptors(defaultLayout, skinnedLayout))
	{
		setExitMessage("Failed to create Descriptor Sets. Probable cause: file not found. Please examine logs for details.");
		return pvr::Result::NotFound;
	}

	if (!createPipelines(defaultLayout, skinnedLayout))
	{
		setExitMessage("Failed to create Descriptor Sets. Probable cause: unexpected device configuration, or bug. Please examine logs for details.");
		return pvr::Result::NotFound;
	}

	apiObj->fboOnScreen = apiObj->context->createOnScreenFbo(0);

	pvr::Result::Enum result = pvr::Result::Success;
	result = apiObj->uiRenderer.init(getGraphicsContext(), apiObj->fboOnScreen->getRenderPass(), 0);
	if (result != pvr::Result::Success) { return result; }

	apiObj->uiRenderer.getDefaultTitle()->setText("Skinning");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	apiObj->uiRenderer.getDefaultDescription()->setText("Skinning with Normal Mapped Per Pixel Lighting");
	apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
	apiObj->uiRenderer.getDefaultControls()->setText("Any Action Key : Pause");
	apiObj->uiRenderer.getDefaultControls()->commitUpdates();
	recordCommandBuffer();

	pvr::assets::Camera const& camera = scene->getCamera(0);

	if (isScreenRotated() && isFullScreen())
	{
		proj = pvr::math::perspectiveFov(apiObj->context->getApiType(), camera.getFOV(), getHeight(), getWidth(), camera.getNear(), camera.getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		proj = pvr::math::perspectiveFov(apiObj->context->getApiType(), camera.getFOV(), getWidth(), getHeight(), camera.getNear(), camera.getFar());
	}
	return result;
}


/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result::Enum OGLESSkinning::releaseView() {	apiObj.reset(0); return pvr::Result::Success; }

/*!*********************************************************************************************************************
\brief	update the animation
***********************************************************************************************************************/
void OGLESSkinning::updateAnimation()
{
	//Calculates the frame number to animate in a time-based manner.
	//Uses the shell function this->getTime() to get the time in milliseconds.

	float fDelta = getFrameTime();
	if (fDelta > 0.0001f)
	{
		if (!isPaused)	{	currentFrame += fDelta / scene->getFPS();	}

		// Wrap the Frame number back to Start
		while (currentFrame >= scene->getNumFrames() - 1) { currentFrame -= (scene->getNumFrames() - 1); }
	}
	// Set the scene animation to the current frame
	scene->setCurrentFrame(currentFrame);
}

/*!*********************************************************************************************************************
\brief	update the camera and light position
***********************************************************************************************************************/
void OGLESSkinning::updateCameraAndLight()
{
	glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
	glm::mat4x4 view;
	float fov, nearClip, farClip;

	scene->getCameraProperties(0, fov, from, to, up, nearClip, farClip); // vTo is calculated from the rotation
	fov *= 0.78f;

	//We can build the model view matrix from the camera position, target and an up vector.
	//For this we use glm::lookAt().
	view = glm::lookAt(from, to, up);

	// Update Light Position and related VGP Program constant
	uniforms.lightPos = glm::vec3(scene->getWorldMatrix(scene->getNodeIdFromLightNodeId(0))[3]);

	// Set up the View * Projection Matrix
	uniforms.viewProj = proj * view;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result::Enum OGLESSkinning::renderFrame()
{
	//Update the scene animation;
	updateAnimation();

	//Get a new worldview camera and light position
	updateCameraAndLight();

	//Update all the bones matrices
	updateBonesAndMatrices();
	apiObj->commandBuffer->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief update the bone array matrices
***********************************************************************************************************************/
void OGLESSkinning::updateBonesAndMatrices()
{
	pvr::uint32 skinnedNode = 0;
	pvr::uint32 nonSkinnedNode = 0;

	for (size_t meshNode = 0; meshNode < scene->getNumMeshNodes(); ++meshNode)
	{
		const pvr::assets::Model::Node& node = scene->getNode(meshNode);
		const pvr::assets::Mesh& mesh = scene->getMesh(node.getObjectId());
		if (mesh.getMeshInfo().isSkinned)
		{
			for (pvr::uint32 batch = 0; batch < mesh.getNumBoneBatches(); ++batch)
			{
				int i32Count = mesh.getBatchBoneCount(batch);

				for (int i = 0; i < i32Count; ++i)
				{
					// Get the Node of the bone
					int i32BoneNodeID = mesh.getBatchBone(batch, i);

					auto& perBatch = uniforms.perSkinnedMesh[skinnedNode][batch];
					// Generates the world matrix for the given bone in this batch.
					perBatch.boneWorld[i] = scene->getBoneWorldMatrix(meshNode, i32BoneNodeID);

					// Calculate the inverse transpose of the 3x3 rotation/scale part for correct lighting
					perBatch.boneWorldIT[i] = glm::inverseTranspose(glm::mat3x3(perBatch.boneWorld[i]));
				}
			}
			skinnedNode++;
		}
		else
		{
			UniformsNonSkinned& mem = uniforms.perNonSkinnedMesh[nonSkinnedNode];
			mem.mvp = uniforms.viewProj * scene->getWorldMatrix(meshNode); //ViewProj updated in the updateCamerasAndLights...
			mem.modelWorld = scene->getWorldMatrix(meshNode);
			mem.modelWorldIT = glm::inverseTranspose(glm::mat3(mem.modelWorld));
			nonSkinnedNode++;
		}
	}
}

/*!*********************************************************************************************************************
\brief	pre-record the rendering commands
***********************************************************************************************************************/
void OGLESSkinning::recordCommandBuffer()
{
	apiObj->commandBuffer->beginRecording();
	////"STICKY" STUFF - These will be set, executed, and forgotten...
	// Set the sampler2D uniforms to corresponding texture units
	apiObj->commandBuffer->bindPipeline(apiObj->skinnedPipeline);
	apiObj->commandBuffer->setUniform<pvr::int32>(apiObj->skinnedPipeline->getUniformLocation("sTexture"), 0);
	apiObj->commandBuffer->setUniform<pvr::int32>(apiObj->skinnedPipeline->getUniformLocation("sNormalMap"), 1);
	apiObj->commandBuffer->bindPipeline(apiObj->defaultPipeline);
	apiObj->commandBuffer->setUniform<pvr::int32>(apiObj->defaultPipeline->getUniformLocation("sTexture"), 0);
	apiObj->commandBuffer->endRecording();
	apiObj->commandBuffer->submit();
	apiObj->commandBuffer->clear();

	//In order to minimize state changes, we will group together all skinned with all non-skinned nodes.
	//In fact, secondary command buffers are ideal to help us separate them - we will put all drawing commands
	//for non-skinned meshes in a secondary command buffer, and all skinned meshes in another secondary command buffer.
	//In a multi-thread environment, this could be done, for example, in different threads.
	pvr::api::SecondaryCommandBuffer	cbuffSkinned = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
	pvr::api::SecondaryCommandBuffer cbuffDefault = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();

	cbuffSkinned->beginRecording(apiObj->fboOnScreen, 0);
	cbuffDefault->beginRecording(apiObj->fboOnScreen, 0);
	//// Since we group them, the pipelines need only be bound in the beginning.
	cbuffSkinned->bindPipeline(apiObj->skinnedPipeline);
	cbuffDefault->bindPipeline(apiObj->defaultPipeline);

	//// Set up the common stuffbasics...
	cbuffSkinned->setUniformPtr<glm::vec3>(skinnedUniformLocations[SkinnedUniforms::LightPos], 1, &uniforms.lightPos);
	cbuffSkinned->setUniformPtr<glm::mat4>(skinnedUniformLocations[SkinnedUniforms::ViewProj], 1, &uniforms.viewProj);
	cbuffDefault->setUniformPtr<glm::vec3>(defaultUniformLocations[DefaultUniforms::LightPos], 1, &uniforms.lightPos);

	int currentNonSkinnedMesh = 0; //We need that to know where we store/update the MVP of this mesh.
	int currentSkinnedMesh = 0; //We need that to know where we store/update the MVP of this mesh.
	//WE SUPPORT: Bumpmapped + skinned, or NonBumpmapped, nonskinned. We will only detect the first .
	for (size_t meshNodeID = 0; meshNodeID < scene->getNumMeshNodes(); ++meshNodeID)
	{
		const pvr::assets::Model::Node& node = scene->getNode(meshNodeID);
		const pvr::assets::Mesh& mesh = scene->getMesh(node.getObjectId());
		const pvr::assets::Material& mat = scene->getMaterial(node.getMaterialIndex());

		bool isSkinned = mesh.getMeshInfo().isSkinned;

		pvr::api::SecondaryCommandBuffer& secBuff = (isSkinned ? cbuffSkinned : cbuffDefault);
		pvr::api::GraphicsPipeline pipe = (isSkinned ? apiObj->skinnedPipeline : apiObj->defaultPipeline);

		secBuff->bindDescriptorSet(pipe->getPipelineLayout(), 0, apiObj->descriptorSets[node.getMaterialIndex()], 0);

		//Construct the per-Mesh part
		secBuff->bindVertexBuffer(apiObj->vbos[node.getObjectId()], 0, 0);
		secBuff->bindIndexBuffer(apiObj->ibos[node.getObjectId()], 0, mesh.getFaces().getDataType());

		if (isSkinned)
		{
			for (pvr::uint32 i32Batch = 0; i32Batch < mesh.getNumBoneBatches(); ++i32Batch)
			{
				// Set the number of bones that will influence each vertex in the mesh
				secBuff->setUniform<pvr::int32>(skinnedUniformLocations[SkinnedUniforms::BoneCount], mesh.getVertexAttributeByName("BONEINDEX")->getN());
				secBuff->setUniformPtr<glm::mat4>(skinnedUniformLocations[SkinnedUniforms::BoneMatrices], mesh.getBatchBoneCount(i32Batch), uniforms.perSkinnedMesh[currentSkinnedMesh][i32Batch].boneWorld);
				secBuff->setUniformPtr<glm::mat3>(skinnedUniformLocations[SkinnedUniforms::BoneMatricesIT], mesh.getBatchBoneCount(i32Batch), uniforms.perSkinnedMesh[currentSkinnedMesh][i32Batch].boneWorldIT);

				// As we are using bone batching we don't want to draw all the faces contained within the Model, we only want
				// to draw the ones that are in the current batch. To do this we pass to the drawMesh function the offset
				// to the start of the current batch of triangles (Mesh.getBatchFaceOffset(i32Batch)) and the
				// total number of triangles to draw (Mesh.getNumFaces(i32Batch) * 3)

				// Draw the mesh
				pvr::uint32 offset = mesh.getBatchFaceOffset(i32Batch) *  sizeof(unsigned short) * 3;
				secBuff->drawIndexed(0, mesh.getNumFaces(i32Batch) * 3, offset, 0, 1);
			}
			currentSkinnedMesh++;
		}
		else
		{
			cbuffDefault->setUniformPtr<glm::mat4>(defaultUniformLocations[DefaultUniforms::WorldMatrix], 1, &uniforms.perNonSkinnedMesh[currentNonSkinnedMesh].modelWorld);
			cbuffDefault->setUniformPtr<glm::mat4>(defaultUniformLocations[DefaultUniforms::MVPMatrix], 1, &uniforms.perNonSkinnedMesh[currentNonSkinnedMesh].mvp);
			cbuffDefault->setUniformPtr<glm::mat3>(defaultUniformLocations[DefaultUniforms::MWITMatrix], 1, &uniforms.perNonSkinnedMesh[currentNonSkinnedMesh].modelWorldIT);
			secBuff->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
			currentNonSkinnedMesh++;
		}
	}
	cbuffSkinned->endRecording(); cbuffDefault->endRecording();

	///// PART 3 :  UIRenderer
	pvr::api::SecondaryCommandBuffer uicmd = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
	apiObj->uiRenderer.beginRendering(uicmd);
	apiObj->uiRenderer.getDefaultDescription()->render();
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->uiRenderer.getDefaultControls()->render();
	apiObj->uiRenderer.endRendering();

	///// PART 4 :  Put it all together
	apiObj->commandBuffer->beginRecording();
	// Clear the color and depth buffer automatically.
	apiObj->commandBuffer->beginRenderPass(apiObj->fboOnScreen, pvr::Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(.2f, .3f, .4f, 1.f));

	apiObj->commandBuffer->enqueueSecondaryCmds(cbuffSkinned);
	apiObj->commandBuffer->enqueueSecondaryCmds(cbuffDefault);
	apiObj->commandBuffer->enqueueSecondaryCmds(uicmd);

	///// PART 4 : End the RenderePass
	apiObj->commandBuffer->endRenderPass();
	apiObj->commandBuffer->endRecording();
}

/*!*********************************************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its Shell object defining the behaviour
		of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new OGLESSkinning()); }
