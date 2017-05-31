/*!*********************************************************************************************************************
\File         VulkanIntroducingPVRApi.cpp
\Title        Introducing the PowerVR Framework
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to use the PVRApi library together with loading models from POD files and rendering them with effects from PFX files.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"
#include "PVREngineUtils/AssetStore.h"

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
const char SceneFileName[] = "GnomeToy.pod"; // POD scene files

/*!*********************************************************************************************************************
 Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class VulkanIntroducingPVRApi : public pvr::Shell
{
	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and Model View matrices
	glm::mat4 _projMtx;
	glm::mat4 _viewMtx;

	// Variables to handle the animation in a time-based manner
	float _frame;

	typedef std::pair<pvr::int32, pvr::api::DescriptorSet> MaterialDescSet;
	struct DeviceResources
	{
		// The Vertex buffer object handle array.
<<<<<<< HEAD
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
=======
		std::vector<pvr::api::Buffer> vbos;
		std::vector<pvr::api::Buffer> ibos;

		// the fbo used in the demo
		pvr::api::FboSet fboOnScreen;

		// main command buffer used to store rendering commands
		pvr::Multi<pvr::api::CommandBuffer> commandBuffers;

		// descriptor sets
		std::vector<MaterialDescSet> texDescSets;
		pvr::Multi<pvr::api::DescriptorSet> matrixUboDescSets;
		pvr::api::DescriptorSet lightUboDescSet;

		// structured memory views
		pvr::utils::StructuredMemoryView matrixMemoryView;
		pvr::utils::StructuredMemoryView lightMemoryView;

		// samplers
		pvr::api::Sampler samplerTrilinear;

		// descriptor set layouts
		pvr::api::DescriptorSetLayout texDescSetLayout;
		pvr::api::DescriptorSetLayout uboDescSetLayoutDynamic, uboDescSetLayoutStatic;

		// pipeline layout
		pvr::api::PipelineLayout pipelineLayout;

		// graphics pipeline
		pvr::api::GraphicsPipeline pipeline;

		// ui renderer
		pvr::ui::UIRenderer uiRenderer;

		pvr::utils::AssetStore assetManager;
		pvr::GraphicsContext context;
>>>>>>> 1776432f... 4.3
	};
	std::auto_ptr<DeviceResources> _deviceResources;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

<<<<<<< HEAD
	bool createDescriptorSet();
	void recordCommandBuffer();
=======
	void createBuffers();
	bool createDescriptorSets();
	void recordCommandBuffers();
>>>>>>> 1776432f... 4.3
	void createPipeline();
	void createDescriptorSetLayouts();
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
<<<<<<< HEAD
	// Load the scene
	deviceResource.reset(new DeviceResources());
	deviceResource->assetManager.init(*this);
	pvr::Result rslt = pvr::Result::Success;
	if ((scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(SceneFileName)))).isNull())
=======
	_deviceResources.reset(new DeviceResources());
	_deviceResources->assetManager.init(*this);

	// Load the _scene
	if ((_scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(SceneFileName)))).isNull())
>>>>>>> 1776432f... 4.3
	{
		this->setExitMessage("ERROR: Couldn't load the %s file\n", SceneFileName);
		return pvr::Result::NotInitialized;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (_scene->getNumCameras() == 0)
	{
		this->setExitMessage("ERROR: The _scene does not contain a camera\n");
		return pvr::Result::InvalidData;
	}

	// Ensure that all meshes use an indexed triangle list
	for (pvr::uint32 i = 0; i < _scene->getNumMeshes(); ++i)
	{
<<<<<<< HEAD
		if (scene->getMesh(i).getPrimitiveType() != types::PrimitiveTopology::TriangleList ||
		    scene->getMesh(i).getFaces().getDataSize() == 0)
=======
		if (_scene->getMesh(i).getPrimitiveType() != pvr::types::PrimitiveTopology::TriangleList ||
		    _scene->getMesh(i).getFaces().getDataSize() == 0)
>>>>>>> 1776432f... 4.3
		{
			this->setExitMessage("ERROR: The meshes in the _scene should use an indexed triangle list\n");
			return pvr::Result::InvalidData;
		}
	}

	// Initialize variables used for the animation
	_frame = 0;

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
<<<<<<< HEAD
	deviceResource->context = getGraphicsContext();
	deviceResource->fboOnScreen = deviceResource->context->createOnScreenFboSet();
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, deviceResource->vbos, deviceResource->ibos);
=======
	_deviceResources->context = getGraphicsContext();
	_deviceResources->fboOnScreen = _deviceResources->context->createOnScreenFboSet();
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *_scene, _deviceResources->vbos, _deviceResources->ibos);
>>>>>>> 1776432f... 4.3

	// We check the _scene contains at least one light
	if (_scene->getNumLights() == 0)
	{
		pvr::Log("The _scene does not contain a light\n");
		return pvr::Result::InvalidData;
	}

<<<<<<< HEAD
	if (deviceResource->uiRenderer.init(deviceResource->fboOnScreen[0]->getRenderPass(), 0) != pvr::Result::Success)
=======
	if (_deviceResources->uiRenderer.init(_deviceResources->fboOnScreen[0]->getRenderPass(), 0) != pvr::Result::Success)
>>>>>>> 1776432f... 4.3
	{
		setExitMessage("Failed top initialize the UIRenderer");
		return pvr::Result::NotInitialized;
	}

<<<<<<< HEAD
	deviceResource->uiRenderer.getDefaultTitle()->setText("IntroducingPVRApi").commitUpdates();
=======
	_deviceResources->uiRenderer.getDefaultTitle()->setText("IntroducingPVRApi").commitUpdates();

	// create demo buffers
	createBuffers();
>>>>>>> 1776432f... 4.3

	// create the descriptor set layouts and pipeline layouts
	createDescriptorSetLayouts();

	// create the descriptor sets
	createDescriptorSets();

	// create demo graphics pipeline
	createPipeline();

	// record the rendering commands
	recordCommandBuffers();

	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		_projMtx = pvr::math::perspective(getApiType(), _scene->getCamera(0).getFOV(), (float)this->getHeight() / (float)this->getWidth(),
		                                  _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		_projMtx = pvr::math::perspective(getApiType(), _scene->getCamera(0).getFOV(), (float)this->getWidth() / (float)this->getHeight(),
		                                  _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}


	// update the light direction ubo only once.
	glm::vec3 lightDir3;
	_scene->getLightDirection(0, lightDir3);
	lightDir3 = glm::normalize(lightDir3);
<<<<<<< HEAD
	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		deviceResource->ubo2.map(i, types::MapBufferFlags::Write);
		deviceResource->ubo2.setValue(0, glm::vec4(lightDir3, 1.f));
		deviceResource->ubo2.unmap(i);
	}
=======

	_deviceResources->lightMemoryView.map(0, pvr::types::MapBufferFlags::Write);
	_deviceResources->lightMemoryView.setValue("LightPos", glm::vec4(lightDir3, 1.f));
	_deviceResources->lightMemoryView.unmap(0);

>>>>>>> 1776432f... 4.3
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRApi::releaseView()
{
<<<<<<< HEAD
	deviceResource.reset();
=======
	_deviceResources.reset();
>>>>>>> 1776432f... 4.3
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every _frame.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRApi::renderFrame()
{
	//	Calculates the _frame number to animate in a time-based manner.
	//	get the time in milliseconds.
	_frame += (float)getFrameTime() / 30.f; // design-time target fps for animation

	if (_frame >= _scene->getNumFrames() - 1)	{	_frame = 0;	}

	// Sets the _scene animation to this _frame
	_scene->setCurrentFrame(_frame);

	//	We can build the world view matrix from the camera position, target and an up vector.
	//	A _scene is composed of nodes. There are 3 types of nodes:
	//	- MeshNodes :
	//		references a mesh in the getMesh().
	//		These nodes are at the beginning of of the Nodes array.
	//		And there are nNumMeshNode number of them.
	//		This way the .pod format can instantiate several times the same mesh
	//		with different attributes.
	//	- lights
	//	- cameras
	//	To draw a _scene, you must go through all the MeshNodes and draw the referenced meshes.
	pvr::float32 fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;
	_scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	_viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	// update the matrix uniform buffer
	{
		// only update the current swapchain ubo
<<<<<<< HEAD
		utils::StructuredMemoryView& memView = deviceResource->ubo1;
		memView.mapMultipleArrayElements(getSwapChainIndex(), 0, scene->getNumMeshNodes(), types::MapBufferFlags::Write);
=======
		_deviceResources->matrixMemoryView.mapMultipleArrayElements(getSwapChainIndex(), 0, _scene->getNumMeshNodes(), pvr::types::MapBufferFlags::Write);
>>>>>>> 1776432f... 4.3
		glm::mat4 tempMtx;
		for (pvr::uint32 i = 0; i < _scene->getNumMeshNodes(); ++i)
		{
			tempMtx = _viewMtx * _scene->getWorldMatrix(i);
			_deviceResources->matrixMemoryView.setArrayValue("MVP", i, _projMtx * tempMtx);
			_deviceResources->matrixMemoryView.setArrayValue("WorldViewItMtx", i, glm::inverseTranspose(tempMtx));
		}
<<<<<<< HEAD
		memView.unmap(getSwapChainIndex());
	}
	deviceResource->commandBuffer[getSwapChainIndex()]->submit();
=======
		_deviceResources->matrixMemoryView.unmap(getSwapChainIndex());
	}

	// submit the per swap chain command buffer
	_deviceResources->commandBuffers[getSwapChainIndex()]->submit();
>>>>>>> 1776432f... 4.3
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Pre-record the rendering commands
***********************************************************************************************************************/
void VulkanIntroducingPVRApi::recordCommandBuffers()
{
<<<<<<< HEAD
	deviceResource->commandBuffer.resize(getPlatformContext().getSwapChainLength());
	for (uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		deviceResource->commandBuffer[i] = deviceResource->context->createCommandBufferOnDefaultPool();
		api::CommandBuffer& commandBuffer = deviceResource->commandBuffer[i];

		commandBuffer->beginRecording();
		commandBuffer->beginRenderPass(deviceResource->fboOnScreen[i], pvr::Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(0.00, 0.70, 0.67, 1.0f));
		commandBuffer->bindPipeline(deviceResource->pipeline);
=======
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		// create a new command buffer
		_deviceResources->commandBuffers.add(_deviceResources->context->createCommandBufferOnDefaultPool());

		// begin recording commands
		_deviceResources->commandBuffers[i]->beginRecording();

		// begin the renderpass
		_deviceResources->commandBuffers[i]->beginRenderPass(_deviceResources->fboOnScreen[i],
		    pvr::Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(0.00, 0.70, 0.67, 1.0f));

		// bind the graphics pipeline
		_deviceResources->commandBuffers[i]->bindPipeline(_deviceResources->pipeline);

>>>>>>> 1776432f... 4.3
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
		pvr::uint32 offset = 0;
		pvr::api::DescriptorSet descriptorSets[3];
		descriptorSets[1] = _deviceResources->matrixUboDescSets[i];
		descriptorSets[2] = _deviceResources->lightUboDescSet;
		for (pvr::uint32 j = 0; j < _scene->getNumMeshNodes(); ++j)
		{
			// get the current mesh node
			const pvr::assets::Model::Node* pNode = &_scene->getMeshNode(j);

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &_scene->getMesh(pNode->getObjectId());

			// get the material id
			pvr::int32 matId = pNode->getMaterialIndex();
<<<<<<< HEAD
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
=======

			// find the texture descriptor set which matches the current material
			auto found = std::find_if(_deviceResources->texDescSets.begin(), _deviceResources->texDescSets.end(), DescripotSetComp(matId));
			descriptorSets[0] = found->second;

			// get the matrix buffer array offset
			offset =  _deviceResources->matrixMemoryView.getAlignedElementArrayOffset(j);
>>>>>>> 1776432f... 4.3

			// bind the descriptor sets
			_deviceResources->commandBuffers[i]->bindDescriptorSets(pvr::types::PipelineBindPoint::Graphics, _deviceResources->pipelineLayout, 0, descriptorSets, 3, &offset, 1);

<<<<<<< HEAD
	pipeDesc.vertexShader.setShader(deviceResource->context->createShader(*vertSource, types::ShaderType::VertexShader));
	pipeDesc.fragmentShader.setShader(deviceResource->context->createShader(*fragSource, types::ShaderType::FragmentShader));
=======
			// bind the vbo and ibos for the current mesh node
			_deviceResources->commandBuffers[i]->bindVertexBuffer(_deviceResources->vbos[pNode->getObjectId()], 0, 0);
			_deviceResources->commandBuffers[i]->bindIndexBuffer(_deviceResources->ibos[pNode->getObjectId()], 0, pMesh->getFaces().getDataType());
>>>>>>> 1776432f... 4.3

			// draw
			_deviceResources->commandBuffers[i]->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}

		// add ui effects using ui renderer
		_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBuffers[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->commandBuffers[i]->endRenderPass();
		_deviceResources->commandBuffers[i]->endRecording();
	}
}

/*!*********************************************************************************************************************
\brief	Creates the descriptor set layouts used throughout the demo.
***********************************************************************************************************************/
void VulkanIntroducingPVRApi::createDescriptorSetLayouts()
{
	// create the texture descriptor set layout and pipeline layout
	{
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
<<<<<<< HEAD
		descSetInfo.setBinding(0, types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		deviceResource->texDescSetLayout = deviceResource->context->createDescriptorSetLayout(descSetInfo);
=======
		descSetInfo.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		_deviceResources->texDescSetLayout = _deviceResources->context->createDescriptorSetLayout(descSetInfo);
>>>>>>> 1776432f... 4.3
	}

	// create the ubo descriptor set layouts
	{
		// dynamic ubo
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
<<<<<<< HEAD
		descSetInfo.setBinding(0, types::DescriptorType::UniformBufferDynamic, 1, types::ShaderStageFlags::Vertex); /*binding 0*/
		deviceResource->uboDescSetLayoutDynamic = deviceResource->context->createDescriptorSetLayout(descSetInfo);
=======
		descSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1, pvr::types::ShaderStageFlags::Vertex); /*binding 0*/
		_deviceResources->uboDescSetLayoutDynamic = _deviceResources->context->createDescriptorSetLayout(descSetInfo);
>>>>>>> 1776432f... 4.3
	}
	{
		//static ubo
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
<<<<<<< HEAD
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
=======
		descSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBuffer, 1, pvr::types::ShaderStageFlags::Vertex);/*binding 0*/
		_deviceResources->uboDescSetLayoutStatic = _deviceResources->context->createDescriptorSetLayout(descSetInfo);
	}

	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_deviceResources->texDescSetLayout);/* set 0 */
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDescSetLayoutDynamic);/* set 1 */
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDescSetLayoutStatic);/* set 2 */
	_deviceResources->pipelineLayout = _deviceResources->context->createPipelineLayout(pipeLayoutInfo);
}

/*!*********************************************************************************************************************
\brief	Creates the graphics pipeline used in the demo.
***********************************************************************************************************************/
void VulkanIntroducingPVRApi::createPipeline()
{
	pvr::api::GraphicsPipelineCreateParam pipeDesc;
	pvr::types::BlendingConfig colorBlendAttachment;
	colorBlendAttachment.blendEnable = false;

	pipeDesc.colorBlend.setAttachmentState(0, colorBlendAttachment);
	pipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);
	pipeDesc.rasterizer.setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);
	pvr::utils::createInputAssemblyFromMesh(_scene->getMesh(0), Attributes, 3, pipeDesc);

	pvr::Stream::ptr_type vertSource =  getAssetStream(VertShaderFileName);
	pvr::Stream::ptr_type fragSource =  getAssetStream(FragShaderFileName);

	pipeDesc.vertexShader.setShader(_deviceResources->context->createShader(*vertSource, pvr::types::ShaderType::VertexShader));
	pipeDesc.fragmentShader.setShader(_deviceResources->context->createShader(*fragSource, pvr::types::ShaderType::FragmentShader));

	pipeDesc.renderPass = _deviceResources->fboOnScreen[0]->getRenderPass();
>>>>>>> 1776432f... 4.3
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pipeDesc.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::Less);
	pipeDesc.depthStencil.setDepthWrite(true);
	pipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);
	pipeDesc.subPass = 0;
<<<<<<< HEAD
	deviceResource->pipeline = deviceResource->context->createGraphicsPipeline(pipeDesc);
=======

	pipeDesc.pipelineLayout = _deviceResources->pipelineLayout;

	_deviceResources->pipeline = _deviceResources->context->createGraphicsPipeline(pipeDesc);
}

/*!*********************************************************************************************************************
\brief	Creates the buffers used throughout the demo.
***********************************************************************************************************************/
void VulkanIntroducingPVRApi::createBuffers()
{
	_deviceResources->matrixMemoryView.addEntryPacked("MVP", pvr::types::GpuDatatypes::mat4x4);
	_deviceResources->matrixMemoryView.addEntryPacked("WorldViewItMtx", pvr::types::GpuDatatypes::mat4x4);
	_deviceResources->matrixMemoryView.finalize(_deviceResources->context, _scene->getNumMeshNodes(), pvr::types::BufferBindingUse::UniformBuffer, true, false);
	_deviceResources->matrixMemoryView.createConnectedBuffers(getPlatformContext().getSwapChainLength(), _deviceResources->context);

	_deviceResources->lightMemoryView.addEntryPacked("LightPos", pvr::types::GpuDatatypes::vec4);
	_deviceResources->lightMemoryView.finalize(_deviceResources->context, 1, pvr::types::BufferBindingUse::UniformBuffer);
	_deviceResources->lightMemoryView.createConnectedBuffer(0, _deviceResources->context);
>>>>>>> 1776432f... 4.3
}

/*!*********************************************************************************************************************
\brief	Create combined texture and sampler descriptor set for the materials in the _scene
\return	Return true on success
***********************************************************************************************************************/
bool VulkanIntroducingPVRApi::createDescriptorSets()
{
	// create the sampler object
	pvr::assets::SamplerCreateParam samplerInfo;
	samplerInfo.minificationFilter = samplerInfo.magnificationFilter = samplerInfo.mipMappingFilter = pvr::types::SamplerFilter::Linear;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = pvr::types::SamplerWrap::Repeat;
<<<<<<< HEAD
	deviceResource->samplerTrilinear = deviceResource->context->createSampler(samplerInfo);
=======
	_deviceResources->samplerTrilinear = _deviceResources->context->createSampler(samplerInfo);
>>>>>>> 1776432f... 4.3

	if (!_deviceResources->samplerTrilinear.isValid())
	{
		pvr::Log("Failed to create Sampler Object");
		return false;
	}

	pvr::uint32 i = 0;
	while (i < _scene->getNumMaterials() && _scene->getMaterial(i).defaultSemantics().getDiffuseTextureIndex() != -1)
	{
		pvr::api::DescriptorSetUpdate descSetInfo;
		pvr::api::TextureView diffuseMap;
		const pvr::assets::Model::Material& material = _scene->getMaterial(i);

		// Load the diffuse texture map
<<<<<<< HEAD
		if (!deviceResource->assetManager.getTextureWithCaching(getGraphicsContext(), scene->getTexture(material.getDiffuseTextureIndex()).getName(),
=======
		if (!_deviceResources->assetManager.getTextureWithCaching(getGraphicsContext(), _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName(),
>>>>>>> 1776432f... 4.3
		    &(diffuseMap), NULL))
		{
			setExitMessage("ERROR: Failed to load texture %s", _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str());
			return false;
		}

		descSetInfo.setCombinedImageSampler(0, diffuseMap, _deviceResources->samplerTrilinear);

<<<<<<< HEAD
		MaterialDescSet matDescSet = std::make_pair(i, deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->texDescSetLayout));
=======
		MaterialDescSet matDescSet = std::make_pair(i, _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->texDescSetLayout));
>>>>>>> 1776432f... 4.3
		matDescSet.second->update(descSetInfo);
		_deviceResources->texDescSets.push_back(matDescSet);
		++i;
	}

<<<<<<< HEAD
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
=======
	_deviceResources->lightUboDescSet = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboDescSetLayoutStatic);
	pvr::api::DescriptorSetUpdate descWrite;
	descWrite.setUbo(0, _deviceResources->lightMemoryView.getConnectedBuffer(0));
	if (!_deviceResources->lightUboDescSet->update(descWrite))
	{
		return false;
	}
>>>>>>> 1776432f... 4.3

	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		_deviceResources->matrixUboDescSets.add(_deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboDescSetLayoutDynamic));
		pvr::api::DescriptorSetUpdate descWrite;
		descWrite.setDynamicUbo(0, _deviceResources->matrixMemoryView.getConnectedBuffer(i));
		if (!_deviceResources->matrixUboDescSets[i]->update(descWrite))
		{
<<<<<<< HEAD
			auto buffer = deviceResource->context->createBuffer(memView2.getAlignedTotalSize(), types::BufferBindingUse::UniformBuffer, true);
			memView2.connectWithBuffer(i, deviceResource->context->createBufferView(buffer, 0, memView2.getUnalignedElementSize()), types::BufferViewTypes::UniformBufferDynamic);
			deviceResource->uboDescSet2[i] = deviceResource->context->createDescriptorSetOnDefaultPool(deviceResource->uboDescSetLayoutStatic);
			api::DescriptorSetUpdate descWrite;
			descWrite.setUbo(0, memView2.getConnectedBuffer(i));
			if (!deviceResource->uboDescSet2[i]->update(descWrite)) { return false; }
=======
			return false;
>>>>>>> 1776432f... 4.3
		}
	}

	return true;
}

/*!*********************************************************************************************************************
\brief	This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.
\return Return an auto ptr to the demo supplied by the user
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanIntroducingPVRApi()); }
