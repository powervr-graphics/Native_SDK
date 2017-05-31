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
	};
	std::auto_ptr<DeviceResources> _deviceResources;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createBuffers();
	bool createDescriptorSets();
	void recordCommandBuffers();
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
	_deviceResources.reset(new DeviceResources());
	_deviceResources->assetManager.init(*this);

	// Load the _scene
	if ((_scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(SceneFileName)))).isNull())
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
		if (_scene->getMesh(i).getPrimitiveType() != pvr::types::PrimitiveTopology::TriangleList ||
		    _scene->getMesh(i).getFaces().getDataSize() == 0)
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
	_deviceResources->context = getGraphicsContext();
	_deviceResources->fboOnScreen = _deviceResources->context->createOnScreenFboSet();
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *_scene, _deviceResources->vbos, _deviceResources->ibos);

	// We check the _scene contains at least one light
	if (_scene->getNumLights() == 0)
	{
		pvr::Log("The _scene does not contain a light\n");
		return pvr::Result::InvalidData;
	}

	if (_deviceResources->uiRenderer.init(_deviceResources->fboOnScreen[0]->getRenderPass(), 0) != pvr::Result::Success)
	{
		setExitMessage("Failed top initialize the UIRenderer");
		return pvr::Result::NotInitialized;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("IntroducingPVRApi").commitUpdates();

	// create demo buffers
	createBuffers();

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

	_deviceResources->lightMemoryView.map(0, pvr::types::MapBufferFlags::Write);
	_deviceResources->lightMemoryView.setValue("LightPos", glm::vec4(lightDir3, 1.f));
	_deviceResources->lightMemoryView.unmap(0);

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRApi::releaseView()
{
	_deviceResources.reset();
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
		_deviceResources->matrixMemoryView.mapMultipleArrayElements(getSwapChainIndex(), 0, _scene->getNumMeshNodes(), pvr::types::MapBufferFlags::Write);
		glm::mat4 tempMtx;
		for (pvr::uint32 i = 0; i < _scene->getNumMeshNodes(); ++i)
		{
			tempMtx = _viewMtx * _scene->getWorldMatrix(i);
			_deviceResources->matrixMemoryView.setArrayValue("MVP", i, _projMtx * tempMtx);
			_deviceResources->matrixMemoryView.setArrayValue("WorldViewItMtx", i, glm::inverseTranspose(tempMtx));
		}
		_deviceResources->matrixMemoryView.unmap(getSwapChainIndex());
	}

	// submit the per swap chain command buffer
	_deviceResources->commandBuffers[getSwapChainIndex()]->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Pre-record the rendering commands
***********************************************************************************************************************/
void VulkanIntroducingPVRApi::recordCommandBuffers()
{
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

			// find the texture descriptor set which matches the current material
			auto found = std::find_if(_deviceResources->texDescSets.begin(), _deviceResources->texDescSets.end(), DescripotSetComp(matId));
			descriptorSets[0] = found->second;

			// get the matrix buffer array offset
			offset =  _deviceResources->matrixMemoryView.getAlignedElementArrayOffset(j);

			// bind the descriptor sets
			_deviceResources->commandBuffers[i]->bindDescriptorSets(pvr::types::PipelineBindPoint::Graphics, _deviceResources->pipelineLayout, 0, descriptorSets, 3, &offset, 1);

			// bind the vbo and ibos for the current mesh node
			_deviceResources->commandBuffers[i]->bindVertexBuffer(_deviceResources->vbos[pNode->getObjectId()], 0, 0);
			_deviceResources->commandBuffers[i]->bindIndexBuffer(_deviceResources->ibos[pNode->getObjectId()], 0, pMesh->getFaces().getDataType());

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
		descSetInfo.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		_deviceResources->texDescSetLayout = _deviceResources->context->createDescriptorSetLayout(descSetInfo);
	}

	// create the ubo descriptor set layouts
	{
		// dynamic ubo
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
		descSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1, pvr::types::ShaderStageFlags::Vertex); /*binding 0*/
		_deviceResources->uboDescSetLayoutDynamic = _deviceResources->context->createDescriptorSetLayout(descSetInfo);
	}
	{
		//static ubo
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
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
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pipeDesc.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::Less);
	pipeDesc.depthStencil.setDepthWrite(true);
	pipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);
	pipeDesc.subPass = 0;

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
	_deviceResources->samplerTrilinear = _deviceResources->context->createSampler(samplerInfo);

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
		if (!_deviceResources->assetManager.getTextureWithCaching(getGraphicsContext(), _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName(),
		    &(diffuseMap), NULL))
		{
			setExitMessage("ERROR: Failed to load texture %s", _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str());
			return false;
		}

		descSetInfo.setCombinedImageSampler(0, diffuseMap, _deviceResources->samplerTrilinear);

		MaterialDescSet matDescSet = std::make_pair(i, _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->texDescSetLayout));
		matDescSet.second->update(descSetInfo);
		_deviceResources->texDescSets.push_back(matDescSet);
		++i;
	}

	_deviceResources->lightUboDescSet = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboDescSetLayoutStatic);
	pvr::api::DescriptorSetUpdate descWrite;
	descWrite.setUbo(0, _deviceResources->lightMemoryView.getConnectedBuffer(0));
	if (!_deviceResources->lightUboDescSet->update(descWrite))
	{
		return false;
	}

	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		_deviceResources->matrixUboDescSets.add(_deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboDescSetLayoutDynamic));
		pvr::api::DescriptorSetUpdate descWrite;
		descWrite.setDynamicUbo(0, _deviceResources->matrixMemoryView.getConnectedBuffer(i));
		if (!_deviceResources->matrixUboDescSets[i]->update(descWrite))
		{
			return false;
		}
	}

	return true;
}

/*!*********************************************************************************************************************
\brief	This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.
\return Return an auto ptr to the demo supplied by the user
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanIntroducingPVRApi()); }
