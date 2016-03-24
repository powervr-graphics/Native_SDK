/*!*********************************************************************************************************************
\File         VulkanSkinning.cpp
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to perform skinning combined with Dot3 (normal-mapped) lighting
***********************************************************************************************************************/
#include "PVRApi/PVRApi.h"
#include "PVRShell/PVRShell.h"
#include "PVRUIRenderer/UIRenderer.h"
#include "PVRCore/StringFunctions.h"
using namespace pvr;
using namespace pvr::types;
const pvr::uint32 MaxBoneSize = 8;
// shader uniforms
namespace BoneMatrixElements {
enum
{
	BoneWorld0 = 0,
	BoneWorldIT0 = BoneWorld0 + MaxBoneSize,
	BoneCount = BoneWorldIT0 + MaxBoneSize,
};
}

namespace NonSkinnedUboElements {
enum
{
	ModelWorld,
	ModelWorldIT
};
}

namespace SkinnedAttributes {
enum Enum
{
	Position, Normal, Tangent, Bitangent, Texcoord, BoneWeight, BoneIndex, Count
};

// Definition of the vertex attributes our meshes are using
pvr::utils::VertexBindings AttributesBindings[] =
{
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "TANGENT", 2 },
	{ "BINORMAL", 3 },
	{ "UV0", 4 },
	{ "BONEWEIGHT", 5 },
	{ "BONEINDEX", 6 },
};
}

namespace DefaultAttributes {
enum Enum
{
	Position, Normal, Texcoord, Count
};

pvr::utils::VertexBindings Bindings[] =
{
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
};
}

namespace Configuration {
// Filenames of our shaders
const char SkinnedFragmentFilename[] = "SkinnedFragShader_vk.spv";
const char SkinnedVertexFilename[] = "SkinnedVertShader_vk.spv";
const char DefaultFragmentFilename[] = "DefaultFragShader_vk.spv";
const char DefaultVertexFilename[] = "DefaultVertShader_vk.spv";

// POD scene files
const char SceneFile[] = "Robot.pod";
}

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanSkinning : public pvr::Shell
{
	//Put all API managed objects in a struct so that we can one-line free them...
	struct DeviceResource
	{
		pvr::api::GraphicsPipeline skinnedPipeline;
		pvr::api::GraphicsPipeline defaultPipeline;
		// Print3D class used to display text
		pvr::ui::UIRenderer uiRenderer;

		pvr::api::AssetStore assetManager;
		pvr::api::Sampler samplerTrilinear;
		pvr::api::Sampler samplerBilinear;

		struct Ubo
		{
			pvr::api::DescriptorSet			 descSet;
			pvr::utils::StructuredMemoryView bufferView;
		};
		typedef std::vector<Ubo> MultiBufferUbo;

		// OpenGL handles for shaders and VBOs
		std::vector<pvr::api::Buffer> vbos;
		std::vector<pvr::api::Buffer> ibos;
		std::vector<pvr::api::DescriptorSet> texDescriptorSets;
		pvr::api::DescriptorSetLayout skinnedTexDescLayout, nonSkinnedTexDescLayout;
		pvr::api::DescriptorSetLayout uboLayoutDynamic, uboLayoutStatic;
		MultiBufferUbo uboDynamicSkinning, uboDynamicNonSkinning;
		MultiBufferUbo	uboStatic;

		pvr::GraphicsContext context;
		std::vector<pvr::api::CommandBuffer>		  cmdBuffer;
		std::vector<pvr::api::SecondaryCommandBuffer> secondaryCmdBufferSkinned;
		std::vector<pvr::api::SecondaryCommandBuffer> secondaryCmdBufferDefault;
		std::vector<pvr::api::SecondaryCommandBuffer> secondaryCmdBufferUIRenderer;
		pvr::Multi<pvr::api::Fbo> fboOnScreen;
	};
	std::auto_ptr<DeviceResource> deviceResource;

	// 3D Model
	pvr::assets::ModelHandle scene;

	bool isPaused;

	// Variables to handle the animation in a time-based manner
	float currentFrame;
	glm::mat4 proj;

public:
	VulkanSkinning() : isPaused(false), currentFrame(0) {}

	pvr::Result::Enum initApplication();
	pvr::Result::Enum initView();
	pvr::Result::Enum releaseView();
	pvr::Result::Enum quitApplication();
	pvr::Result::Enum renderFrame();

	bool createTextureDescriptors();
	bool createPipelines();
	bool loadBuffers();
	void updateBonesAndMatrices();
	void updateAnimation();
	void updateCameraAndLight();
	void recordCommandBuffer();
	void eventMappedInput(pvr::SimplifiedInput::Enum action);
	bool createUboDescriptors();
};

/*!*********************************************************************************************************************
\brief	handle the input event
\param	action input actions to handle
***********************************************************************************************************************/
void VulkanSkinning::eventMappedInput(pvr::SimplifiedInput::Enum action)
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
bool VulkanSkinning::createTextureDescriptors()
{
	// Texture IDs
	std::vector<pvr::api::TextureView> textures;
	for (uint32 i = 0; i < scene->getNumTextures(); ++i)
	{
		const pvr::assets::Model::Texture& tex = scene->getTexture(i);
		if (!deviceResource->assetManager.getTextureWithCaching(getGraphicsContext(), tex.getName(), NULL, NULL))
		{
			setExitMessage("Could not load the texture %s", tex.getName().c_str());
			return false;
		}
	}

	pvr::assets::SamplerCreateParam desc;
	desc.magnificationFilter = SamplerFilter::Linear;
	desc.minificationFilter = SamplerFilter::Linear;
	desc.mipMappingFilter = SamplerFilter::Linear;
	deviceResource->samplerTrilinear = deviceResource->context->createSampler(desc);

	desc.mipMappingFilter = SamplerFilter::None;
	deviceResource->samplerBilinear = deviceResource->context->createSampler(desc);
	deviceResource->texDescriptorSets.resize(scene->getNumMaterials());

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
		deviceResource->assetManager.getTextureWithCaching(getGraphicsContext(), textureName, &tex, NULL);
		descSetWrite.setCombinedImageSampler(0, tex, deviceResource->samplerTrilinear);// Albedo Binding 0
		if (mat.getBumpMapTextureIndex() != -1)
		{
			const pvr::StringHash& bumpTextureName = scene->getTexture(mat.getBumpMapTextureIndex()).getName();
			deviceResource->assetManager.getTextureWithCaching(getGraphicsContext(), bumpTextureName, &tex, NULL);
			descSetWrite.setCombinedImageSampler(1, tex, deviceResource->samplerBilinear);// Bumpmap Binding 1
			deviceResource->texDescriptorSets[i] = deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->skinnedTexDescLayout);
		}
		else
		{
			deviceResource->texDescriptorSets[i] = deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->nonSkinnedTexDescLayout);
		}
		deviceResource->texDescriptorSets[i]->update(descSetWrite);
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool VulkanSkinning::createPipelines()
{
	// Create the pipelines. CAUTION: We are assuming two specific effects for this demo:
	// 1) Bumpmapped, Skinned
	// 2) Non-Bumpmapped, Not Skinned
	// THESE MUST OBVIOUSLY BE COMPATIBLE WITH THE VERTEX ATTRIBUTES WE GAVE DEFINED IN Configuration::SkinnedVertexAttributes, DefaultVertexAttributes
	// Obviously, in order to render "arbitrary" meshes, you would need a suite of shaders
	// and some kind of heuristic to pick among them.
	int skinnedMeshId = -1, nonSkinnedMeshId = -1;
	for (uint32 i = 0; i < scene->getNumMeshes() && (skinnedMeshId == -1 || nonSkinnedMeshId == -1); ++i)
	{
		bool skinned = (scene->getMesh(i).getVertexAttributeIndex("BONEWEIGHT") != -1);
		if (skinned && skinnedMeshId == -1) { skinnedMeshId = i; }
		if (!skinned && nonSkinnedMeshId == -1) { nonSkinnedMeshId = i; }
	}

	// ubo descriptor setlayout
	{
		// dynamic
		pvr::api::DescriptorSetLayoutCreateParam descLayoutParam;
		descLayoutParam.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1, pvr::types::ShaderStageFlags::Vertex);
		deviceResource->uboLayoutDynamic = deviceResource->context->createDescriptorSetLayout(descLayoutParam);

		//static
		descLayoutParam.setBinding(0, pvr::types::DescriptorType::UniformBuffer, 1, pvr::types::ShaderStageFlags::Vertex);
		deviceResource->uboLayoutStatic = deviceResource->context->createDescriptorSetLayout(descLayoutParam);
	}

	// skinning texture descriptor setlayout
	{
		pvr::api::DescriptorSetLayoutCreateParam descLayoutParam;
		// diffuse map binding = 0
		// normal map binding  = 1
		descLayoutParam.
		setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment)
		.setBinding(1, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		deviceResource->skinnedTexDescLayout = deviceResource->context->createDescriptorSetLayout(descLayoutParam);
	}

	// default texture descriptor setlayout
	{
		pvr::api::DescriptorSetLayoutCreateParam descLayoutParam;
		// diffuse map binding = 0
		descLayoutParam.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		deviceResource->nonSkinnedTexDescLayout = deviceResource->context->createDescriptorSetLayout(descLayoutParam);
	}

	//Create skinned descriptorsetlayout & pipeline
	if (skinnedMeshId != -1)
	{
		pvr::api::GraphicsPipelineCreateParam skinnedDesc;
		pvr::api::PipelineLayoutCreateParam skinnedPipeLayoutCreateParam;

		skinnedPipeLayoutCreateParam
		.addDescSetLayout(deviceResource->skinnedTexDescLayout) // texture		0
		.addDescSetLayout(deviceResource->uboLayoutDynamic)		// perframe ubo 1
		.addDescSetLayout(deviceResource->uboLayoutStatic);		// static ubo	2
		pvr::api::pipelineCreation::ColorBlendAttachmentState colorAttachmentState;
		skinnedDesc.colorBlend.addAttachmentState(colorAttachmentState);
		skinnedDesc.pipelineLayout = deviceResource->context->createPipelineLayout(skinnedPipeLayoutCreateParam);
		skinnedDesc.depthStencil.setDepthTestEnable(true);

		skinnedDesc.renderPass = deviceResource->fboOnScreen[0]->getRenderPass();
		skinnedDesc.subPass = 0;
		pvr::Stream::ptr_type skinVertShader = getAssetStream(Configuration::SkinnedVertexFilename);
		pvr::Stream::ptr_type skinFragShader = getAssetStream(Configuration::SkinnedFragmentFilename);
		skinnedDesc.vertexShader.setShader(deviceResource->context->createShader(*skinVertShader, ShaderType::VertexShader));
		skinnedDesc.fragmentShader.setShader(deviceResource->context->createShader(*skinFragShader, ShaderType::FragmentShader));

		pvr::utils::createInputAssemblyFromMesh(scene->getMesh(skinnedMeshId), SkinnedAttributes::AttributesBindings, SkinnedAttributes::Count, skinnedDesc);
		//create the skin pipeline, it is mutable after creation
		deviceResource->skinnedPipeline = deviceResource->context->createGraphicsPipeline(skinnedDesc);
	}

	//Create non skinned descriptorsetlayout & pipeline
	if (nonSkinnedMeshId != -1)
	{
		pvr::api::GraphicsPipelineCreateParam defaultDesc;
		pvr::api::PipelineLayoutCreateParam defaultPipeLayoutCreateParam;
		defaultPipeLayoutCreateParam
		.addDescSetLayout(deviceResource->nonSkinnedTexDescLayout)// texture		0
		.addDescSetLayout(deviceResource->uboLayoutDynamic)			// perframe ubo 1
		.addDescSetLayout(deviceResource->uboLayoutStatic);			// static ubo	2


		pvr::api::pipelineCreation::ColorBlendAttachmentState colorAttachementState;
		// Simple Pipeline
		defaultDesc.colorBlend.addAttachmentState(colorAttachementState);
		defaultDesc.rasterizer.setCullFace(pvr::types::Face::Back);
		defaultDesc.renderPass = deviceResource->fboOnScreen[0]->getRenderPass();
		defaultDesc.pipelineLayout = deviceResource->context->createPipelineLayout(defaultPipeLayoutCreateParam);
		defaultDesc.depthStencil.setDepthTestEnable(true);
		defaultDesc.subPass = 0;
		pvr::assets::ShaderFile vertShaderFile(Configuration::DefaultVertexFilename, *this);
		pvr::assets::ShaderFile fragShaderFile(Configuration::DefaultFragmentFilename, *this);

		pvr::Stream::ptr_type defaultVertShader = getAssetStream(Configuration::DefaultVertexFilename);
		pvr::Stream::ptr_type defaultFragShader = getAssetStream(Configuration::DefaultFragmentFilename);
		defaultDesc.vertexShader.setShader(deviceResource->context->createShader(*defaultVertShader, ShaderType::VertexShader));
		defaultDesc.fragmentShader.setShader(deviceResource->context->createShader(*defaultFragShader, ShaderType::FragmentShader));

		pvr::utils::createInputAssemblyFromMesh(scene->getMesh(nonSkinnedMeshId), DefaultAttributes::Bindings, DefaultAttributes::Count, defaultDesc);
		//create the skin pipeline, it is mutable after creation
		deviceResource->defaultPipeline = deviceResource->context->createGraphicsPipeline(defaultDesc);
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result::Enum VulkanSkinning::initApplication()
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
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by Shell once per run, just before exiting the program.
		If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result::Enum VulkanSkinning::quitApplication()
{
	scene.reset();
	deviceResource.reset();
	return pvr::Result::Success;
}

bool VulkanSkinning::createUboDescriptors()
{
	const pvr::uint32 swapChainLength = deviceResource->context->getPlatformContext().getSwapChainLength();
	deviceResource->uboDynamicSkinning.resize(swapChainLength);
	deviceResource->uboDynamicSkinning.resize(swapChainLength);
	deviceResource->uboDynamicNonSkinning.resize(swapChainLength);
	deviceResource->uboStatic.resize(swapChainLength);

	// calculate the number of skinned and non-skinned elements required
	pvr::uint32 skinnedBatchCount = 0, nonSkinnedMeshCount = 0;
	for (pvr::uint32 i = 0; i < scene->getNumMeshNodes(); ++i)
	{
		const auto& mesh = scene->getMesh(scene->getNode(i).getObjectId());
		if (mesh.getMeshInfo().isSkinned == true)
		{
			skinnedBatchCount += mesh.getNumBoneBatches();
			continue;
		}
		++nonSkinnedMeshCount;
	}

	for (pvr::uint32 i = 0; i < swapChainLength; ++i)
	{
		//skinning dynamic ubo
		{
			pvr::utils::StructuredMemoryView bufferView;
			bufferView.setupDynamic(deviceResource->context, skinnedBatchCount, pvr::BufferViewTypes::UniformBufferDynamic);
			pvr::uint32 offset = 0;

			// boneworld
			for (pvr::uint32 bone = 0; bone < MaxBoneSize; ++bone)
			{
				bufferView.addEntryPacked(pvr::strings::createFormatted("BoneWorld%d", bone), pvr::GpuDatatypes::mat4x4);
			}

			// boneworldIT
			for (pvr::uint32 bone = 0; bone < MaxBoneSize; ++bone)
			{
				bufferView.addEntryPacked(pvr::strings::createFormatted("BoneWorldIT%d", bone), pvr::GpuDatatypes::mat3x4);
			}

			//boneCount
			bufferView.addEntryPacked("boneCount", pvr::GpuDatatypes::integer);

			auto buffer = deviceResource->context->createBuffer(bufferView.getAlignedTotalSize(), BufferBindingUse::UniformBuffer);
			bufferView.connectWithBuffer(deviceResource->context->createBufferView(buffer, 0, bufferView.getAlignedElementSize()), BufferViewTypes::UniformBufferDynamic);

			deviceResource->uboDynamicSkinning[i].bufferView = bufferView;
			deviceResource->uboDynamicSkinning[i].descSet = deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->uboLayoutDynamic);
			pvr::api::DescriptorSetUpdate write;
			write.setDynamicUbo(0, bufferView.getConnectedBuffer());
			deviceResource->uboDynamicSkinning[i].descSet->update(write);
		}
		// non-skinned
		{
			pvr::utils::StructuredMemoryView bufferView;
			bufferView.setupDynamic(deviceResource->context, nonSkinnedMeshCount, pvr::BufferViewTypes::UniformBufferDynamic);
			// model world
			bufferView.addEntryPacked("ModelWorld", pvr::GpuDatatypes::mat4x4);
			// modelworldIT
			bufferView.addEntryPacked("ModelWorldIT", pvr::GpuDatatypes::mat3x4);

			auto buffer = deviceResource->context->createBuffer(bufferView.getAlignedTotalSize(), BufferBindingUse::UniformBuffer);
			bufferView.connectWithBuffer(deviceResource->context->createBufferView(buffer, 0, bufferView.getAlignedElementSize()), BufferViewTypes::UniformBufferDynamic);

			deviceResource->uboDynamicNonSkinning[i].bufferView = bufferView;
			deviceResource->uboDynamicNonSkinning[i].descSet = deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->uboLayoutDynamic);
			pvr::api::DescriptorSetUpdate write;
			write.setDynamicUbo(0, bufferView.getConnectedBuffer());
			deviceResource->uboDynamicNonSkinning[i].descSet->update(write);
		}

		// static ubo.
		{
			pvr::utils::StructuredMemoryView bufferView;
			bufferView.setupDynamic(deviceResource->context, nonSkinnedMeshCount, pvr::BufferViewTypes::UniformBuffer);
			bufferView.addEntryPacked("viewProj", pvr::GpuDatatypes::mat4x4);
			bufferView.addEntryPacked("lightPos", pvr::GpuDatatypes::vec3);

			auto buffer = deviceResource->context->createBuffer(bufferView.getAlignedTotalSize(), BufferBindingUse::UniformBuffer);
			bufferView.connectWithBuffer(deviceResource->context->createBufferView(buffer, 0, bufferView.getAlignedElementSize()),
			                             BufferViewTypes::UniformBuffer);
			deviceResource->uboStatic[i].bufferView = bufferView;

			deviceResource->uboStatic[i].descSet = deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->uboLayoutStatic);
			pvr::api::DescriptorSetUpdate write;
			write.setUbo(0, bufferView.getConnectedBuffer());
			deviceResource->uboStatic[i].descSet->update(write);
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result::Enum VulkanSkinning::initView()
{
	deviceResource.reset(new DeviceResource);
	deviceResource->context = getGraphicsContext();
	deviceResource->assetManager.init(*this);
	deviceResource->fboOnScreen = deviceResource->context->createOnScreenFboSet();
	currentFrame = 0.;

	// automatically populate the VBOs and IBOs from the Model.
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, deviceResource->vbos, deviceResource->ibos);

	if (!createPipelines())
	{
		setExitMessage("Failed to create Descriptor Sets. Probable cause: unexpected device configuration, or bug. Please examine logs for details.");
		return pvr::Result::NotFound;
	}

	if (!createTextureDescriptors())
	{
		setExitMessage("Failed to create Descriptor Sets. Probable cause: file not found. Please examine logs for details.");
		return pvr::Result::NotFound;
	}

	if (!createUboDescriptors())
	{
		setExitMessage("Failed to create uniform buffer and descriptor Sets. Probable cause: unexpected device configuration, or bug. Please examine logs for details.");
		return pvr::Result::NotFound;
	}

	pvr::Result::Enum result = pvr::Result::Success;
	result = deviceResource->uiRenderer.init(getGraphicsContext(), deviceResource->fboOnScreen[0]->getRenderPass(), 0);
	if (result != pvr::Result::Success) { return result; }

	deviceResource->uiRenderer.getDefaultTitle()->setText("Skinning");
	deviceResource->uiRenderer.getDefaultTitle()->commitUpdates();
	deviceResource->uiRenderer.getDefaultDescription()->setText("Skinning with Normal Mapped Per Pixel Lighting");
	deviceResource->uiRenderer.getDefaultDescription()->commitUpdates();
	deviceResource->uiRenderer.getDefaultControls()->setText("Any Action Key : Pause");
	deviceResource->uiRenderer.getDefaultControls()->commitUpdates();
	deviceResource->uiRenderer.getSdkLogo()->setColor(1.0f, 1.0f, 1.0f, 1.f);
	deviceResource->uiRenderer.getSdkLogo()->commitUpdates();
	recordCommandBuffer();

	pvr::assets::Camera const& camera = scene->getCamera(0);

	if (isScreenRotated() && isFullScreen())
	{
		proj = pvr::math::perspective(deviceResource->context->getApiType(), camera.getFOV(),
		                              (pvr::float32)getWidth() / getHeight(), camera.getNear(), camera.getFar(),
		                              glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		proj = pvr::math::perspective(deviceResource->context->getApiType(), camera.getFOV(),
		                              (pvr::float32) getWidth() / getHeight(), camera.getNear(),
		                              camera.getFar());
	}
	return result;
}


/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result::Enum VulkanSkinning::releaseView() { deviceResource.reset(0); return pvr::Result::Success; }

/*!*********************************************************************************************************************
\brief	update the animation
***********************************************************************************************************************/
void VulkanSkinning::updateAnimation()
{
	//Calculates the frame number to animate in a time-based manner.
	//Uses the shell function this->getTime() to get the time in milliseconds.

	float32 fDelta = (float32)getFrameTime();
	if (fDelta > 0.0001f)
	{
		if (!isPaused) { currentFrame += fDelta / scene->getFPS(); }

		// Wrap the Frame number back to Start
		while (currentFrame >= scene->getNumFrames() - 1) { currentFrame -= (scene->getNumFrames() - 1); }
	}
	// Set the scene animation to the current frame
	scene->setCurrentFrame(currentFrame);
}

/*!*********************************************************************************************************************
\brief	update the camera and light position
***********************************************************************************************************************/
void VulkanSkinning::updateCameraAndLight()
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
	const glm::vec3& lightPos = glm::vec3(scene->getWorldMatrix(scene->getNodeIdFromLightNodeId(0))[3]);

	// Set up the View * Projection Matrix
	const glm::mat4& viewProj = proj * view;

	// update the static ubo
	pvr::uint32 swapChainIndex = getSwapChainIndex();
	deviceResource->uboStatic[swapChainIndex].bufferView.map(pvr::types::MapBufferFlags::Write);
	deviceResource->uboStatic[swapChainIndex].bufferView.setValue(0, viewProj).setValue(1, lightPos);
	deviceResource->uboStatic[swapChainIndex].bufferView.unmap();
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result::Enum VulkanSkinning::renderFrame()
{
	//Update the scene animation;
	updateAnimation();
	//Get a new worldview camera and light position
	updateCameraAndLight();
	//Update all the bones matrices
	updateBonesAndMatrices();
	deviceResource->cmdBuffer[deviceResource->context->getPlatformContext().getSwapChainIndex()]->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief update the bone array matrices
***********************************************************************************************************************/
void VulkanSkinning::updateBonesAndMatrices()
{
	pvr::uint32 nonSkinnedNode = 0;
	pvr::uint32 skinnedBatchId = 0;
	pvr::uint32 swapChainIndex = deviceResource->context->getPlatformContext().getSwapChainIndex();
	// update the ubo
	utils::StructuredMemoryView& skinningBuffer = deviceResource->uboDynamicSkinning[swapChainIndex].bufferView;
	utils::StructuredMemoryView& nonSkinningBuffer = deviceResource->uboDynamicNonSkinning[swapChainIndex].bufferView;
	skinningBuffer.map(pvr::types::MapBufferFlags::Write);
	nonSkinningBuffer.map(pvr::types::MapBufferFlags::Write);
	for (size_t meshNode = 0; meshNode < scene->getNumMeshNodes(); ++meshNode)
	{
		const pvr::assets::Model::Node& node = scene->getNode(meshNode);
		const pvr::assets::Mesh& mesh = scene->getMesh(node.getObjectId());
		if (mesh.getMeshInfo().isSkinned)
		{
			pvr::uint32 boneCountPerIndex = mesh.getVertexAttributeByName("BONEINDEX")->getN();
			for (pvr::uint32 batch = 0; batch < mesh.getNumBoneBatches(); ++batch)
			{
				int i32Count = mesh.getBatchBoneCount(batch);
				for (int i = 0; i < i32Count; ++i)
				{
					// Get the Node of the bone
					int i32BoneNodeID = mesh.getBatchBone(batch, i);
					const glm::mat4& boneWorld = scene->getBoneWorldMatrix(meshNode, i32BoneNodeID);
					// Generates the world matrix for the given bone in this batch.
					skinningBuffer
					.setArrayValue(BoneMatrixElements::BoneWorld0 + i, skinnedBatchId, boneWorld)
					// Calculate the inverse transpose of the 3x3 rotation/scale part for correct lighting
					.setArrayValue(BoneMatrixElements::BoneWorldIT0 + i, skinnedBatchId, glm::mat3x4(glm::inverseTranspose(glm::mat3(boneWorld))));
				}
				skinningBuffer.setArrayValue(BoneMatrixElements::BoneCount, skinnedBatchId, boneCountPerIndex);
				++skinnedBatchId;
			}
		}
		else
		{
			const glm::mat4& modelWorld = scene->getWorldMatrix(meshNode);
			//mem.mvp = uniforms.viewProj * scene->getWorldMatrix(meshNode); //ViewProj updated in the updateCamerasAndLights...
			nonSkinningBuffer
			.setArrayValue(NonSkinnedUboElements::ModelWorld, nonSkinnedNode, modelWorld)
			.setArrayValue(NonSkinnedUboElements::ModelWorldIT, nonSkinnedNode, glm::mat3x4(glm::inverseTranspose(glm::mat3(modelWorld))));
			++nonSkinnedNode;
		}
	}
	skinningBuffer.unmap();
	nonSkinningBuffer.unmap();
}

/*!*********************************************************************************************************************
\brief	pre-record the rendering commands
***********************************************************************************************************************/
void VulkanSkinning::recordCommandBuffer()
{
	deviceResource->cmdBuffer.resize(deviceResource->context->getPlatformContext().getSwapChainLength());
	deviceResource->secondaryCmdBufferSkinned.resize(deviceResource->context->getPlatformContext().getSwapChainLength());
	deviceResource->secondaryCmdBufferDefault.resize(deviceResource->context->getPlatformContext().getSwapChainLength());
	deviceResource->secondaryCmdBufferUIRenderer.resize(deviceResource->context->getPlatformContext().getSwapChainLength());
	for (pvr::uint32 i = 0; i < deviceResource->context->getPlatformContext().getSwapChainLength(); ++i)
	{
		//In order to minimize state changes, we will group together all skinned with all non-skinned nodes.
		//In fact, secondary command buffers are ideal to help us separate them - we will put all drawing commands
		//for non-skinned meshes in a secondary command buffer, and all skinned meshes in another secondary command buffer.
		//In a multi-thread environment, this could be done, for example, in different threads.
		pvr::api::SecondaryCommandBuffer cbuffSkinned = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
		pvr::api::SecondaryCommandBuffer cbuffDefault = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
		deviceResource->secondaryCmdBufferSkinned[i] = cbuffSkinned;
		deviceResource->secondaryCmdBufferDefault[i] = cbuffDefault;


		cbuffSkinned->beginRecording(deviceResource->fboOnScreen[i]);
		cbuffDefault->beginRecording(deviceResource->fboOnScreen[i]);
		//// Since we group them, the pipelines need only be bound in the beginning.
		cbuffSkinned->bindPipeline(deviceResource->skinnedPipeline);
		cbuffDefault->bindPipeline(deviceResource->defaultPipeline);

		//// Set up the common stuffbasics...
		int currentNonSkinnedMesh = 0; //We need that to know where we store/update the MVP of this mesh.
		pvr::uint32 skinnedBatchOffset = 0;
		for (size_t meshNodeID = 0; meshNodeID < scene->getNumMeshNodes(); ++meshNodeID)
		{
			const pvr::assets::Model::Node& node = scene->getNode(meshNodeID);
			const pvr::assets::Mesh& mesh = scene->getMesh(node.getObjectId());
			bool isSkinned = mesh.getMeshInfo().isSkinned;

			pvr::api::SecondaryCommandBuffer& secBuff = (isSkinned ? cbuffSkinned : cbuffDefault);
			pvr::api::GraphicsPipeline pipe = (isSkinned ? deviceResource->skinnedPipeline : deviceResource->defaultPipeline);

			// bind the texture to set 0
			secBuff->bindDescriptorSet(pipe->getPipelineLayout(), 0, deviceResource->texDescriptorSets[node.getMaterialIndex()], 0, 0);
			// bind the static ubo to set 2
			secBuff->bindDescriptorSet(pipe->getPipelineLayout(), 2, deviceResource->uboStatic[i].descSet);
			//Construct the per-Mesh part
			secBuff->bindVertexBuffer(deviceResource->vbos[node.getObjectId()], 0, 0);
			secBuff->bindIndexBuffer(deviceResource->ibos[node.getObjectId()], 0, mesh.getFaces().getDataType());
			if (isSkinned)
			{
				for (pvr::uint32 batch = 0; batch < mesh.getNumBoneBatches(); ++batch)
				{
					const pvr::uint32 dynamicOffset = deviceResource->uboDynamicSkinning[i].bufferView.getAlignedElementArrayOffset(skinnedBatchOffset);
					// Set the number of bones that will influence each vertex in the mesh
					// dynamic ubo set 1
					secBuff->bindDescriptorSet(pipe->getPipelineLayout(), 1, deviceResource->uboDynamicSkinning[i].descSet, &dynamicOffset, 1);

					// As we are using bone batching we don't want to draw all the faces contained within the Model, we only want
					// to draw the ones that are in the current batch. To do this we pass to the drawMesh function the offset
					// to the start of the current batch of triangles (Mesh.getBatchFaceOffset(i32Batch)) and the
					// total number of triangles to draw (Mesh.getNumFaces(i32Batch) * 3)
					// Draw the mesh
					secBuff->drawIndexed(mesh.getBatchFaceOffset(batch) * 3, mesh.getNumFaces(batch) * 3, 0, 0, 1);
					++skinnedBatchOffset;
				}
			}
			else
			{
				const pvr::uint32 dynamicOffset = deviceResource->uboDynamicNonSkinning[i].bufferView.getAlignedElementArrayOffset(currentNonSkinnedMesh);;
				// dynamic ubo set 1
				secBuff->bindDescriptorSet(pipe->getPipelineLayout(), 1, deviceResource->uboDynamicNonSkinning[i].descSet, &dynamicOffset, 1);
				secBuff->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
				++currentNonSkinnedMesh;
			}
		}

		cbuffSkinned->endRecording();
		cbuffDefault->endRecording();

		/// PART 3 :  UIRenderer
		pvr::api::SecondaryCommandBuffer uicmd = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
		deviceResource->secondaryCmdBufferUIRenderer[i] = uicmd;
		deviceResource->uiRenderer.beginRendering(uicmd);
		deviceResource->uiRenderer.getDefaultDescription()->render();
		deviceResource->uiRenderer.getDefaultTitle()->render();
		deviceResource->uiRenderer.getSdkLogo()->render();
		deviceResource->uiRenderer.getDefaultControls()->render();
		deviceResource->uiRenderer.endRendering();

		///// PART 4 :  Put it all together
		deviceResource->cmdBuffer[i] = deviceResource->context->createCommandBufferOnDefaultPool();
		deviceResource->cmdBuffer[i]->beginRecording();
		// Clear the color and depth buffer automatically.
		deviceResource->cmdBuffer[i]->beginRenderPass(deviceResource->fboOnScreen[i], pvr::Rectanglei(0, 0,
		    getWidth(), getHeight()), false, glm::vec4(.2f, .3f, .4f, 1.f));
		deviceResource->cmdBuffer[i]->enqueueSecondaryCmds(cbuffSkinned);
		deviceResource->cmdBuffer[i]->enqueueSecondaryCmds(cbuffDefault);
		deviceResource->cmdBuffer[i]->enqueueSecondaryCmds(uicmd);

		///// PART 4 : End the RenderePass
		deviceResource->cmdBuffer[i]->endRenderPass();
		deviceResource->cmdBuffer[i]->endRecording();
	}
}

/*!*********************************************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its Shell object defining the behaviour
		of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanSkinning()); }
