/*!*********************************************************************************************************************
\File         VulkanIntroducingPVRApi.cpp
\Title        Introducing the PowerVR Framework
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to use the PVRApi library together with loading models from POD files and rendering them with effects from PFX files.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"

using namespace pvr;

pvr::utils::VertexBindings Attributes[] =
{
	{"POSITION", 0},
	{"NORMAL", 1},
	{"UV0", 2}
};

/*!*********************************************************************************************************************
 Content file names
***********************************************************************************************************************/
const char VertShaderFileName[] = "VertShader_vk.spv";
const char FragShaderFileName[] = "FragShader_vk.spv";
const char SceneFileName[]		= "GnomeToy.pod"; // POD scene files

/*!*********************************************************************************************************************
 Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class VulkanIntroducingPVRApi : public pvr::Shell
{
	// 3D Model
	pvr::assets::ModelHandle scene;

	// Projection and Model View matrices
	glm::mat4 projMtx, viewMtx;

	// Variables to handle the animation in a time-based manner
	float frame;

	typedef std::pair<pvr::int32, pvr::api::DescriptorSet> MaterialDescSet;
	struct DeviceResources
	{
		// The Vertex buffer object handle array.
		std::vector<api::Buffer> vbos;
		std::vector<api::Buffer> ibos;
		Multi<api::Fbo> fboOnScreen;
		std::vector<api::CommandBuffer> commandBuffer;
		std::vector<api::SecondaryCommandBuffer> uiRendererCommandBuffer;
		std::vector<MaterialDescSet> texDescSet;
		std::vector<api::DescriptorSet> uboDescSet1, uboDescSet2;
		std::vector<utils::StructuredMemoryView> ubo1;
		std::vector<utils::StructuredMemoryView> ubo2;
		api::Sampler samplerTrilinear;
		api::DescriptorSetLayout texDescSetLayout;
		api::DescriptorSetLayout uboDescSetLayoutDynamic, uboDescSetLayoutStatic;
		api::PipelineLayout pipelineLayout;
		api::GraphicsPipeline pipeline;
	};

	std::auto_ptr<DeviceResources> deviceResource;
	pvr::ui::UIRenderer uiRenderer;
	api::AssetStore assetManager;
	GraphicsContext context;
	struct DrawPass
	{
		std::vector<glm::mat4> worldViewProj;
		std::vector<glm::mat4> worldViewIT;
		std::vector<glm::vec3> dirLight;
		glm::mat4 scale;
	};
	DrawPass drawPass;

public:
	virtual pvr::Result::Enum initApplication();
	virtual pvr::Result::Enum initView();
	virtual pvr::Result::Enum releaseView();
	virtual pvr::Result::Enum quitApplication();
	virtual pvr::Result::Enum renderFrame();

	bool createDescriptorSet();
	void recordCommandBuffer();
	void createPipeline();
};

struct DescripotSetComp
{
	pvr::int32 id;
	DescripotSetComp(pvr::int32 id) : id(id) {}
	bool operator()(std::pair<pvr::int32, pvr::api::DescriptorSet> const& pair)	{	return pair.first == id;	}
};


/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
		context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result::Enum VulkanIntroducingPVRApi::initApplication()
{
	// Load the scene
	assetManager.init(*this);
	pvr::Result::Enum rslt = pvr::Result::Success;
	if ((scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(SceneFileName)))).isNull())
	{
		this->setExitMessage("ERROR: Couldn't load the %s file\n", SceneFileName);
		return rslt;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (scene->getNumCameras() == 0)
	{
		this->setExitMessage("ERROR: The scene does not contain a camera\n");
		return pvr::Result::InvalidData;
	}

	// Ensure that all meshes use an indexed triangle list
	for (uint32 i = 0; i < scene->getNumMeshes(); ++i)
	{
		if (scene->getMesh(i).getPrimitiveType() != types::PrimitiveTopology::TriangleList ||
		        scene->getMesh(i).getFaces().getDataSize() == 0)
		{
			this->setExitMessage("ERROR: The meshes in the scene should use an indexed triangle list\n");
			return pvr::Result::InvalidData;
		}
	}
	// Initialize variables used for the animation
	frame = 0;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
				If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result::Enum VulkanIntroducingPVRApi::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change  in the rendering context.
				Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result::Enum VulkanIntroducingPVRApi::initView()
{
	context = getGraphicsContext();
	deviceResource.reset(new DeviceResources());
	deviceResource->fboOnScreen = context->createOnScreenFboSet();
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, deviceResource->vbos, deviceResource->ibos);

	// We check the scene contains at least one light
	if (scene->getNumLights() == 0)
	{
		pvr::Log("The scene does not contain a light\n");
		return pvr::Result::InvalidData;
	}

	if (uiRenderer.init(context, deviceResource->fboOnScreen[0]->getRenderPass(), 0) != pvr::Result::Success)
	{
		setExitMessage("Failed top initialize the UIRenderer");
		return pvr::Result::NotInitialized;
	}

	uiRenderer.getDefaultTitle()->setText("IntroducingPVRApi").commitUpdates();


	createPipeline();
	createDescriptorSet();
	recordCommandBuffer();
	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		projMtx = math::perspective(getApiType(), scene->getCamera(0).getFOV(), (float)this->getHeight() / (float)this->getWidth(),
		                            scene->getCamera(0).getNear(), scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		projMtx = math::perspective(getApiType(), scene->getCamera(0).getFOV(), (float)this->getWidth() / (float)this->getHeight(),
		                            scene->getCamera(0).getNear(), scene->getCamera(0).getFar());
	}


	// update the light direction ubo only once.
	glm::vec3 lightDir3;
	scene->getLightDirection(0, lightDir3);
	lightDir3 = glm::normalize(lightDir3);
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		deviceResource->ubo2[i].map(types::MapBufferFlags::Write);
		deviceResource->ubo2[i].setValue(0, glm::vec4(lightDir3, 1.f));
		deviceResource->ubo2[i].unmap();
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result::Enum VulkanIntroducingPVRApi::releaseView()
{
	assetManager.releaseAll();
	deviceResource.reset();
	uiRenderer.release();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result::Enum VulkanIntroducingPVRApi::renderFrame()
{
	//	Calculates the frame number to animate in a time-based manner.
	//	get the time in milliseconds.
	frame += (float)getFrameTime() / 30.f; // design-time target fps for animation

	if (frame >= scene->getNumFrames() - 1)	{	frame = 0;	}

	// Sets the scene animation to this frame
	scene->setCurrentFrame(frame);

	//	We can build the world view matrix from the camera position, target and an up vector.
	//	A scene is composed of nodes. There are 3 types of nodes:
	//	- MeshNodes :
	//		references a mesh in the getMesh().
	//		These nodes are at the beginning of of the Nodes array.
	//		And there are nNumMeshNode number of them.
	//		This way the .pod format can instantiate several times the same mesh
	//		with different attributes.
	//	- lights
	//	- cameras
	//	To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.

	pvr::float32 fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;
	scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	// update the ubo
	{
		// only update the current swapchain ubo
		utils::StructuredMemoryView& memView = deviceResource->ubo1[getPlatformContext().getSwapChainIndex()];
		memView.map(types::MapBufferFlags::Write);
		//TODO this need to be in a static ubo
		glm::mat4 tempMtx;
		for (pvr::uint32 i = 0; i < scene->getNumMeshNodes(); ++i)
		{
			tempMtx = viewMtx * scene->getWorldMatrix(i);
			memView.setArrayValue(memView.getIndex("MVP"), i, projMtx * tempMtx);
			memView.setArrayValue(memView.getIndex("WorldViewItMtx"), i, glm::inverseTranspose(tempMtx));
		}
		memView.unmap();
	}
	deviceResource->commandBuffer[getPlatformContext().getSwapChainIndex()]->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Pre-record the rendering commands
***********************************************************************************************************************/
void VulkanIntroducingPVRApi::recordCommandBuffer()
{
	deviceResource->commandBuffer.resize(getPlatformContext().getSwapChainLength());
	deviceResource->uiRendererCommandBuffer.resize(getPlatformContext().getSwapChainLength());
	for (uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		deviceResource->commandBuffer[i] = context->createCommandBufferOnDefaultPool();
		deviceResource->uiRendererCommandBuffer[i] = context->createSecondaryCommandBufferOnDefaultPool();
		api::CommandBuffer& commandBuffer = deviceResource->commandBuffer[i];
		api::SecondaryCommandBuffer& uiRendererCmd = deviceResource->uiRendererCommandBuffer[i];
		// uiRenderer
		context->createSecondaryCommandBufferOnDefaultPool();
		uiRenderer.beginRendering(uiRendererCmd);
		uiRenderer.getSdkLogo()->render();
		uiRenderer.getDefaultTitle()->render();
		uiRenderer.endRendering();

		commandBuffer->beginRecording();
		commandBuffer->beginRenderPass(deviceResource->fboOnScreen[i], pvr::Rectanglei(0, 0, getWidth(), getHeight()), false, glm::vec4(0.00, 0.70, 0.67, 1.0f));
		commandBuffer->bindPipeline(deviceResource->pipeline);
		// A scene is composed of nodes. There are 3 types of nodes:
		// - MeshNodes :
		// references a mesh in the getMesh().
		// These nodes are at the beginning of of the Nodes array.
		// And there are nNumMeshNode number of them.
		// This way the .pod format can instantiate several times the same mesh
		// with different attributes.
		// - lights
		// - cameras
		// To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
		uint32 offset = 0;
		api::DescriptorSet descSet[3];
		descSet[1] = deviceResource->uboDescSet1[i];
		descSet[2] = deviceResource->uboDescSet2[i];
		for (int j = 0; j < (int)scene->getNumMeshNodes(); ++j)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(j);
			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(pNode->getObjectId());
			pvr::int32 matId = pNode->getMaterialIndex();
			auto found = std::find_if(deviceResource->texDescSet.begin(), deviceResource->texDescSet.end(), DescripotSetComp(matId));
			descSet[0] = found->second;
			offset =  deviceResource->ubo1[i].getAlignedElementArrayOffset(j);
			commandBuffer->bindDescriptorSets(types::PipelineBindPoint::Graphics, deviceResource->pipelineLayout, 0, descSet, 3, &offset, 1);
			commandBuffer->bindVertexBuffer(deviceResource->vbos[pNode->getObjectId()], 0, 0);
			commandBuffer->bindIndexBuffer(deviceResource->ibos[pNode->getObjectId()], 0, pMesh->getFaces().getDataType());

			//Now that the model-view matrix is set and the materials ready,
			//call another function to actually draw the mesh.
			commandBuffer->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}
		commandBuffer->enqueueSecondaryCmds(uiRendererCmd);
		commandBuffer->endRenderPass();
		commandBuffer->endRecording();
	}
}

void VulkanIntroducingPVRApi::createPipeline()
{
	pvr::api::GraphicsPipelineCreateParam pipeDesc;
	pvr::api::pipelineCreation::ColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.blendEnable = false;

	pipeDesc.colorBlend.addAttachmentState(colorBlendAttachment);
	pipeDesc.rasterizer.setCullFace(types::Face::Back).setFrontFaceWinding(types::PolygonWindingOrder::FrontFaceCCW);
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pvr::utils::createInputAssemblyFromMesh(scene->getMesh(0), Attributes, 3, pipeDesc);

	Stream::ptr_type vertSource =  getAssetStream(VertShaderFileName);
	Stream::ptr_type fragSource =  getAssetStream(FragShaderFileName);

	pipeDesc.vertexShader.setShader(context->createShader(*vertSource, types::ShaderType::VertexShader));
	pipeDesc.fragmentShader.setShader(context->createShader(*fragSource, types::ShaderType::FragmentShader));

	// create the texture descriptor set layout and pipeline layout
	{
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
		descSetInfo.setBinding(0, types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		deviceResource->texDescSetLayout = context->createDescriptorSetLayout(descSetInfo);
	}
	// create the ubo descriptor setlayout
	{
		// dynamic ubo
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
		descSetInfo.setBinding(0, types::DescriptorType::UniformBufferDynamic, 1, types::ShaderStageFlags::Vertex); /*binding 0*/
		deviceResource->uboDescSetLayoutDynamic = context->createDescriptorSetLayout(descSetInfo);

		//static ubo
		descSetInfo.setBinding(0, types::DescriptorType::UniformBuffer, 1, types::ShaderStageFlags::Vertex);/*binding 0*/
		deviceResource->uboDescSetLayoutStatic = context->createDescriptorSetLayout(descSetInfo);
	}

	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo
	.addDescSetLayout(deviceResource->texDescSetLayout)/* set 0 */
	.addDescSetLayout(deviceResource->uboDescSetLayoutDynamic)/* set 1 */
	.addDescSetLayout(deviceResource->uboDescSetLayoutStatic);/* set 2 */
	pipeDesc.pipelineLayout = deviceResource->pipelineLayout = context->createPipelineLayout(pipeLayoutInfo);
	pipeDesc.renderPass = deviceResource->fboOnScreen[0]->getRenderPass();
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);
	pipeDesc.subPass = 0;
	deviceResource->pipeline = context->createGraphicsPipeline(pipeDesc);
}

/*!*********************************************************************************************************************
\brief	Create combined texture and sampler descriptor set for the materials in the scene
\return	Return true on success
***********************************************************************************************************************/
bool VulkanIntroducingPVRApi::createDescriptorSet()
{
	// create the sampler object
	pvr::assets::SamplerCreateParam samplerInfo;
	samplerInfo.minificationFilter = samplerInfo.magnificationFilter = samplerInfo.mipMappingFilter = pvr::types::SamplerFilter::Linear;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = pvr::types::SamplerWrap::Repeat;
	deviceResource->samplerTrilinear = context->createSampler(samplerInfo);

	if (!deviceResource->samplerTrilinear.isValid())
	{
		pvr::Log("Failed to create Sampler Object");
		return false;
	}

	pvr::uint32 i = 0;
	while (i < scene->getNumMaterials() && scene->getMaterial(i).getDiffuseTextureIndex() != -1)
	{
		pvr::api::DescriptorSetUpdate descSetInfo;
		pvr::api::TextureView diffuseMap;
		const pvr::assets::Model::Material& material = scene->getMaterial(i);

		// Load the diffuse texture map
		if (!assetManager.getTextureWithCaching(getGraphicsContext(), scene->getTexture(material.getDiffuseTextureIndex()).getName(),
		                                        &(diffuseMap), NULL))
		{
			setExitMessage("ERROR: Failed to load texture %s", scene->getTexture(material.getDiffuseTextureIndex()).getName().c_str());
			return false;
		}

		descSetInfo.setCombinedImageSampler(0, diffuseMap, deviceResource->samplerTrilinear);

		MaterialDescSet matDescSet = std::make_pair(i, context->createDescriptorSetOnDefaultPool(deviceResource->texDescSetLayout));
		matDescSet.second->update(descSetInfo);
		deviceResource->texDescSet.push_back(matDescSet);
		++i;
	}

	// create the ubo
	deviceResource->ubo1.resize(getPlatformContext().getSwapChainLength());
	deviceResource->ubo2.resize(getPlatformContext().getSwapChainLength());
	deviceResource->uboDescSet1.resize(getPlatformContext().getSwapChainLength());
	deviceResource->uboDescSet2.resize(getPlatformContext().getSwapChainLength());
	for (uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		{
			utils::StructuredMemoryView memView;
			memView.setupArray(context, scene->getNumMeshNodes(), BufferViewTypes::UniformBufferDynamic);
			memView.addEntryPacked("MVP", GpuDatatypes::mat4x4);
			memView.addEntryPacked("WorldViewItMtx", GpuDatatypes::mat4x4);
			auto buffer = context->createBuffer(memView.getAlignedTotalSize(), types::BufferBindingUse::UniformBuffer);
			memView.connectWithBuffer(context->createBufferView(buffer, 0, memView.getAlignedElementSize()),
			                          pvr::BufferViewTypes::UniformBufferDynamic);
			deviceResource->ubo1[i] = memView;
			deviceResource->uboDescSet1[i] = context->createDescriptorSetOnDefaultPool(deviceResource->uboDescSetLayoutDynamic);
			api::DescriptorSetUpdate descWrite;
			descWrite.setDynamicUbo(0, memView.getConnectedBuffer());
			if (!deviceResource->uboDescSet1[i]->update(descWrite)) { return false; }
		}

		{
			utils::StructuredMemoryView memView;
			memView.setupArray(context, 1, BufferViewTypes::UniformBuffer);
			memView.addEntryPacked("LightPos", GpuDatatypes::vec4);
			auto buffer = context->createBuffer(memView.getAlignedTotalSize(), types::BufferBindingUse::UniformBuffer);
			memView.connectWithBuffer(context->createBufferView(buffer, 0, memView.getAlignedElementSize()), pvr::BufferViewTypes::UniformBufferDynamic);
			deviceResource->ubo2[i] = memView;
			deviceResource->uboDescSet2[i] = context->createDescriptorSetOnDefaultPool(deviceResource->uboDescSetLayoutStatic);
			api::DescriptorSetUpdate descWrite;
			descWrite.setUbo(0, memView.getConnectedBuffer());
			if (!deviceResource->uboDescSet2[i]->update(descWrite)) { return false; }
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\brief	This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.
\return Return an auto ptr to the demo supplied by the user
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanIntroducingPVRApi()); }
