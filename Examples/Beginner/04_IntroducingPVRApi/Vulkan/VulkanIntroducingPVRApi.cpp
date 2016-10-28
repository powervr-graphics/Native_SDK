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
		api::FboSet fboOnScreen;
		std::vector<api::CommandBuffer> commandBuffer;
		std::vector<MaterialDescSet> texDescSet;
		std::vector<api::DescriptorSet> uboDescSet1, uboDescSet2;
		utils::StructuredMemoryView ubo1;
		utils::StructuredMemoryView ubo2;
		api::Sampler samplerTrilinear;
		api::DescriptorSetLayout texDescSetLayout;
		api::DescriptorSetLayout uboDescSetLayoutDynamic, uboDescSetLayoutStatic;
		api::PipelineLayout pipelineLayout;
		api::GraphicsPipeline pipeline;
		pvr::ui::UIRenderer uiRenderer;
		api::AssetStore assetManager;
		GraphicsContext context;
	};
	std::auto_ptr<DeviceResources> deviceResource;
	struct DrawPass
	{
		std::vector<glm::mat4> worldViewProj;
		std::vector<glm::mat4> worldViewIT;
		std::vector<glm::vec3> dirLight;
		glm::mat4 scale;
	};
	DrawPass drawPass;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

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
pvr::Result VulkanIntroducingPVRApi::initApplication()
{
	// Load the scene
	deviceResource.reset(new DeviceResources());
	deviceResource->assetManager.init(*this);
	pvr::Result rslt = pvr::Result::Success;
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
pvr::Result VulkanIntroducingPVRApi::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change  in the rendering context.
				Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRApi::initView()
{
	deviceResource->context = getGraphicsContext();
	deviceResource->fboOnScreen = deviceResource->context->createOnScreenFboSet();
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, deviceResource->vbos, deviceResource->ibos);

	// We check the scene contains at least one light
	if (scene->getNumLights() == 0)
	{
		pvr::Log("The scene does not contain a light\n");
		return pvr::Result::InvalidData;
	}

	if (deviceResource->uiRenderer.init(deviceResource->fboOnScreen[0]->getRenderPass(), 0) != pvr::Result::Success)
	{
		setExitMessage("Failed top initialize the UIRenderer");
		return pvr::Result::NotInitialized;
	}

	deviceResource->uiRenderer.getDefaultTitle()->setText("IntroducingPVRApi").commitUpdates();

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
	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		deviceResource->ubo2.map(i, types::MapBufferFlags::Write);
		deviceResource->ubo2.setValue(0, glm::vec4(lightDir3, 1.f));
		deviceResource->ubo2.unmap(i);
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRApi::releaseView()
{
	deviceResource.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRApi::renderFrame()
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
		utils::StructuredMemoryView& memView = deviceResource->ubo1;
		memView.mapMultipleArrayElements(getSwapChainIndex(), 0, scene->getNumMeshNodes(), types::MapBufferFlags::Write);
		glm::mat4 tempMtx;
		for (pvr::uint32 i = 0; i < scene->getNumMeshNodes(); ++i)
		{
			tempMtx = viewMtx * scene->getWorldMatrix(i);
			memView.setArrayValue(memView.getIndex("MVP"), i, projMtx * tempMtx);
			memView.setArrayValue(memView.getIndex("WorldViewItMtx"), i, glm::inverseTranspose(tempMtx));
		}
		memView.unmap(getSwapChainIndex());
	}
	deviceResource->commandBuffer[getSwapChainIndex()]->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Pre-record the rendering commands
***********************************************************************************************************************/
void VulkanIntroducingPVRApi::recordCommandBuffer()
{
	deviceResource->commandBuffer.resize(getPlatformContext().getSwapChainLength());
	for (uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		deviceResource->commandBuffer[i] = deviceResource->context->createCommandBufferOnDefaultPool();
		api::CommandBuffer& commandBuffer = deviceResource->commandBuffer[i];

		commandBuffer->beginRecording();
		commandBuffer->beginRenderPass(deviceResource->fboOnScreen[i], pvr::Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(0.00, 0.70, 0.67, 1.0f));
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
			offset =  deviceResource->ubo1.getAlignedElementArrayOffset(j);
			commandBuffer->bindDescriptorSets(types::PipelineBindPoint::Graphics, deviceResource->pipelineLayout, 0, descSet, 3, &offset, 1);
			commandBuffer->bindVertexBuffer(deviceResource->vbos[pNode->getObjectId()], 0, 0);
			commandBuffer->bindIndexBuffer(deviceResource->ibos[pNode->getObjectId()], 0, pMesh->getFaces().getDataType());

			//Now that the model-view matrix is set and the materials ready,
			//call another function to actually draw the mesh.
			commandBuffer->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}
		deviceResource->uiRenderer.beginRendering(commandBuffer);
		deviceResource->uiRenderer.getDefaultTitle()->render();
		deviceResource->uiRenderer.getSdkLogo()->render();
		deviceResource->uiRenderer.endRendering();
		commandBuffer->endRenderPass();
		commandBuffer->endRecording();
	}
}

void VulkanIntroducingPVRApi::createPipeline()
{
	pvr::api::GraphicsPipelineCreateParam pipeDesc;
	pvr::types::BlendingConfig colorBlendAttachment;
	colorBlendAttachment.blendEnable = false;

	pipeDesc.colorBlend.setAttachmentState(0, colorBlendAttachment);
	pipeDesc.rasterizer.setCullFace(types::Face::Back);
	pipeDesc.rasterizer.setFrontFaceWinding(types::PolygonWindingOrder::FrontFaceCCW);
	pvr::utils::createInputAssemblyFromMesh(scene->getMesh(0), Attributes, 3, pipeDesc);

	Stream::ptr_type vertSource =  getAssetStream(VertShaderFileName);
	Stream::ptr_type fragSource =  getAssetStream(FragShaderFileName);

	pipeDesc.vertexShader.setShader(deviceResource->context->createShader(*vertSource, types::ShaderType::VertexShader));
	pipeDesc.fragmentShader.setShader(deviceResource->context->createShader(*fragSource, types::ShaderType::FragmentShader));

	// create the texture descriptor set layout and pipeline layout
	{
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
		descSetInfo.setBinding(0, types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		deviceResource->texDescSetLayout = deviceResource->context->createDescriptorSetLayout(descSetInfo);
	}
	// create the ubo descriptor setlayout
	{
		// dynamic ubo
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
		descSetInfo.setBinding(0, types::DescriptorType::UniformBufferDynamic, 1, types::ShaderStageFlags::Vertex); /*binding 0*/
		deviceResource->uboDescSetLayoutDynamic = deviceResource->context->createDescriptorSetLayout(descSetInfo);
	}
	{
		//static ubo
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
		descSetInfo.setBinding(0, types::DescriptorType::UniformBuffer, 1, types::ShaderStageFlags::Vertex);/*binding 0*/
		deviceResource->uboDescSetLayoutStatic = deviceResource->context->createDescriptorSetLayout(descSetInfo);
	}

	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo
	.addDescSetLayout(deviceResource->texDescSetLayout)/* set 0 */
	.addDescSetLayout(deviceResource->uboDescSetLayoutDynamic)/* set 1 */
	.addDescSetLayout(deviceResource->uboDescSetLayoutStatic);/* set 2 */
	pipeDesc.pipelineLayout = deviceResource->pipelineLayout = deviceResource->context->createPipelineLayout(pipeLayoutInfo);
	pipeDesc.renderPass = deviceResource->fboOnScreen[0]->getRenderPass();
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pipeDesc.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::Less);
	pipeDesc.depthStencil.setDepthWrite(true);
	pipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);
	pipeDesc.subPass = 0;
	deviceResource->pipeline = deviceResource->context->createGraphicsPipeline(pipeDesc);
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
	deviceResource->samplerTrilinear = deviceResource->context->createSampler(samplerInfo);

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
		if (!deviceResource->assetManager.getTextureWithCaching(getGraphicsContext(), scene->getTexture(material.getDiffuseTextureIndex()).getName(),
		    &(diffuseMap), NULL))
		{
			setExitMessage("ERROR: Failed to load texture %s", scene->getTexture(material.getDiffuseTextureIndex()).getName().c_str());
			return false;
		}

		descSetInfo.setCombinedImageSampler(0, diffuseMap, deviceResource->samplerTrilinear);

		MaterialDescSet matDescSet = std::make_pair(i, deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->texDescSetLayout));
		matDescSet.second->update(descSetInfo);
		deviceResource->texDescSet.push_back(matDescSet);
		++i;
	}

	// create the ubo
	deviceResource->uboDescSet1.resize(getPlatformContext().getSwapChainLength());
	deviceResource->uboDescSet2.resize(getPlatformContext().getSwapChainLength());

	utils::StructuredMemoryView& memView = deviceResource->ubo1;
	memView.addEntryPacked("MVP", types::GpuDatatypes::mat4x4);
	memView.addEntryPacked("WorldViewItMtx", types::GpuDatatypes::mat4x4);
	memView.setupArray(deviceResource->context, scene->getNumMeshNodes(), types::BufferViewTypes::UniformBufferDynamic);

	utils::StructuredMemoryView& memView2 = deviceResource->ubo2;
	memView2.addEntryPacked("LightPos", types::GpuDatatypes::vec4);
	memView2.setupArray(deviceResource->context, 1, types::BufferViewTypes::UniformBuffer);

	for (uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		{
			auto buffer = deviceResource->context->createBuffer(memView.getAlignedTotalSize(), types::BufferBindingUse::UniformBuffer, true);
			memView.connectWithBuffer(i, deviceResource->context->createBufferView(buffer, 0, memView.getUnalignedElementSize()), types::BufferViewTypes::UniformBufferDynamic);

			deviceResource->uboDescSet1[i] = deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->uboDescSetLayoutDynamic);
			api::DescriptorSetUpdate descWrite;
			descWrite.setDynamicUbo(0, memView.getConnectedBuffer(i));
			if (!deviceResource->uboDescSet1[i]->update(descWrite)) { return false; }
		}

		{
			auto buffer = deviceResource->context->createBuffer(memView2.getAlignedTotalSize(), types::BufferBindingUse::UniformBuffer, true);
			memView2.connectWithBuffer(i, deviceResource->context->createBufferView(buffer, 0, memView2.getUnalignedElementSize()), types::BufferViewTypes::UniformBufferDynamic);
			deviceResource->uboDescSet2[i] = deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->uboDescSetLayoutStatic);
			api::DescriptorSetUpdate descWrite;
			descWrite.setUbo(0, memView2.getConnectedBuffer(i));
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
