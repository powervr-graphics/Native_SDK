/*!*********************************************************************************************************************
\File         VulkanPVRScopeRemote.cpp
\Title        PVRScopeRemote
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to use our example PVRScope graph code.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRScopeComms.h"

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader_vk.fsh.spv";
const char VertShaderSrcFile[] = "VertShader_vk.vsh.spv";

// PVR texture files
const char TextureFile[] = "Marble.pvr";

// POD scene files
const char SceneFile[] = "scene.pod";
enum { MaxSwapChains = 8 };
namespace CounterDefs {
enum Enum
{
	Counter, Counter10, NumCounter
};
}

namespace PipelineConfigs {
enum DescriptorSetId { DescriptorUbo, DescriptorMaterial, DescriptorCount }; // Pipeline Descriptor sets
enum MaterialBindingId { MaterialBindingTex, MaterialBindingData, MaterialBindingCount }; // Material Descritpor set bindings
}

const char* FrameDefs[CounterDefs::NumCounter] = { "Frames", "Frames10" };

/*!*********************************************************************************************************************
\brief Class implementing the PVRShell functions.
***********************************************************************************************************************/
class VulkanPVRScopeRemote : public pvr::Shell
{
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvrvk::Surface surface;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvrvk::Queue queue;

		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;

		pvr::Multi<pvrvk::ImageView> depthStencilImages;

		pvr::Multi<pvrvk::Semaphore> semaphoreAcquire;
		pvr::Multi<pvrvk::Semaphore> semaphoreSubmit;
		pvr::Multi<pvrvk::Fence> perFrameFence;

		pvrvk::GraphicsPipeline	pipeline;
		pvrvk::ImageView texture;
		std::vector<pvrvk::Buffer> vbos;
		std::vector<pvrvk::Buffer> ibos;
		std::vector<pvrvk::CommandBuffer> commandBuffer;

		pvr::utils::StructuredBufferView uboMVPBufferView;
		pvrvk::Buffer uboMVP;
		pvr::utils::StructuredBufferView uboMaterialBufferView;
		pvrvk::Buffer uboMaterial;

		pvrvk::DescriptorSet uboMvpDesc[MaxSwapChains];
		pvrvk::DescriptorSet uboMatDesc;

		pvrvk::DescriptorSetLayout descriptorSetLayout;
		pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

		// 3D Model
		pvr::assets::ModelHandle scene;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
	};
	std::unique_ptr<DeviceResources> _deviceResources;

	uint32_t _frameId;
	glm::mat4 _projectionMtx;
	glm::mat4 _viewMtx;

	struct UboMaterialData
	{
		glm::vec3	albedo;
		float specularExponent;
		float metallicity;
		float reflectivity;
		bool isDirty;
	} _uboMatData;

	// The translation and Rotate parameter of Model
	float _angleY;

	// Data connection to PVRPerfServer
	bool _hasCommunicationError;
	SSPSCommsData*	_spsCommsData;
	SSPSCommsLibraryTypeFloat _commsLibSpecularExponent;
	SSPSCommsLibraryTypeFloat _commsLibMetallicity;
	SSPSCommsLibraryTypeFloat _commsLibReflectivity;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoR;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoG;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoB;
	uint32_t _frameCounter;
	uint32_t _frame10Counter;
	uint32_t _counterReadings[CounterDefs::NumCounter];
public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void recordCommandBuffer(uint32_t swapchain);
	bool createPipeline();
	void loadVbos();
	void drawMesh(int i32NodeIndex, pvrvk::CommandBuffer& command);
	bool createDescriptorSet(std::vector<pvr::utils::ImageUploadResults>& uploadResults);
	void updateUbo(uint32_t swapchain);
};

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool VulkanPVRScopeRemote::createPipeline()
{
	pvrvk::Device& device = _deviceResources->device;
	//Mapping of mesh semantic names to shader variables
	pvr::utils::VertexBindings_Name vertexBindings[] =
	{
		{ "POSITION", "inVertex" },
		{ "NORMAL", "inNormal" },
		{ "UV0", "inTexCoord" }
	};

	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	pvrvk::GraphicsPipelineCreateInfo pipeDesc;
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo; pipeLayoutInfo
	.setDescSetLayout(PipelineConfigs::DescriptorUbo, device->createDescriptorSetLayout(
	                    pvrvk::DescriptorSetLayoutCreateInfo().setBinding(0,
	                        VkDescriptorType::e_UNIFORM_BUFFER, 1, VkShaderStageFlags::e_VERTEX_BIT)))

	.setDescSetLayout(PipelineConfigs::DescriptorMaterial, device->createDescriptorSetLayout(
	                    pvrvk::DescriptorSetLayoutCreateInfo()
	                    .setBinding(PipelineConfigs::MaterialBindingTex, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
	                                1, VkShaderStageFlags::e_FRAGMENT_BIT)
	                    .setBinding(PipelineConfigs::MaterialBindingData, VkDescriptorType::e_UNIFORM_BUFFER, 1,
	                                VkShaderStageFlags::e_FRAGMENT_BIT)));

	pipeDesc.pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	if (!pipeDesc.pipelineLayout.isValid())
	{
		setExitMessage("Failed to create the pipeline layout");
		return false;
	}

	/* Load and compile the shaders from files. */
	pipeDesc.vertexShader.setShader(device->createShader(getAssetStream(VertShaderSrcFile)->readToEnd<uint32_t>()));
	pipeDesc.fragmentShader.setShader(device->createShader(getAssetStream(FragShaderSrcFile)->readToEnd<uint32_t>()));
	pvr::utils::populateViewportStateCreateInfo(_deviceResources->onScreenFramebuffer[0], pipeDesc.viewport);
	pipeDesc.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
	pipeDesc.depthStencil.enableDepthTest(true);
	pipeDesc.depthStencil.setDepthCompareFunc(VkCompareOp::e_LESS);
	pipeDesc.depthStencil.enableDepthWrite(true);
	pipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	pipeDesc.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pvr::utils::populateInputAssemblyFromMesh(_deviceResources->scene->getMesh(0), vertexBindings, 3,
	    pipeDesc.vertexInput, pipeDesc.inputAssembler);
	_deviceResources->pipeline = device->createGraphicsPipeline(pipeDesc);
	if (!_deviceResources->pipeline.isValid()) { setExitMessage("Failed to create the Pipeline "); return false; }
	return true;
}

/*!*********************************************************************************************************************
\brief	Loads the mesh data required for this training course into vertex buffer objects
***********************************************************************************************************************/
void VulkanPVRScopeRemote::loadVbos()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	//	Load vertex data of all meshes in the scene into VBOs
	//	The meshes have been exported with the "Interleave Vectors" option,
	//	so all data is interleaved in the buffer at pMesh->pInterleaved.
	//	Interleaving data improves the memory access pattern and cache efficiency,
	//	thus it can be read faster by the hardware.
	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_deviceResources->scene, _deviceResources->vbos, _deviceResources->ibos);
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanPVRScopeRemote::initApplication()
{
	_frameId = 0;
	_deviceResources.reset(new DeviceResources());
	// Load the scene
	if (!pvr::assets::helper::loadModel(*this, SceneFile, _deviceResources->scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}
	// We want a data connection to PVRPerfServer
	{
		_spsCommsData = pplInitialise("PVRScopeRemote", 14);
		_hasCommunicationError = false;
		if (_spsCommsData)
		{
			// Demonstrate that there is a good chance of the initial data being
			// lost - the connection is normally completed asynchronously.
			pplSendMark(_spsCommsData, "lost", static_cast<uint32_t>(strlen("lost")));

			// This is entirely optional. Wait for the connection to succeed, it will
			// timeout if e.g. PVRPerfServer is not running.
			int isConnected;
			pplWaitForConnection(_spsCommsData, &isConnected, 1, 200);
		}
	}
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	_uboMatData.specularExponent = 5.f;            // Width of the specular highlights (using low exponent for a brushed metal look)
	_uboMatData.albedo = glm::vec3(1.f, .77f, .33f); // Overall color
	_uboMatData.metallicity = 1.f;                 // Is the color of the specular white (nonmetallic), or coloured by the object(metallic)
	_uboMatData.reflectivity = .8f;                // Percentage of contribution of diffuse / specular
	_uboMatData.isDirty = true;
	_frameCounter = 0;
	_frame10Counter = 0;

	// set angle of rotation
	_angleY = 0.0f;

	//	Remotely editable library items
	if (_spsCommsData)
	{
		std::vector<SSPSCommsLibraryItem> communicableItems;

		// Editable: Specular Exponent
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibSpecularExponent.fCurrent = _uboMatData.specularExponent;
		_commsLibSpecularExponent.fMin = 1.1f;
		_commsLibSpecularExponent.fMax = 300.0f;
		communicableItems.back().pszName = "Specular Exponent";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibSpecularExponent;
		communicableItems.back().nDataLength = sizeof(_commsLibSpecularExponent);

		communicableItems.push_back(SSPSCommsLibraryItem());
		// Editable: Metallicity
		_commsLibMetallicity.fCurrent = _uboMatData.metallicity;
		_commsLibMetallicity.fMin = 0.0f;
		_commsLibMetallicity.fMax = 1.0f;
		communicableItems.back().pszName = "Metallicity";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibMetallicity;
		communicableItems.back().nDataLength = sizeof(_commsLibMetallicity);

		// Editable: Reflectivity
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibReflectivity.fCurrent = _uboMatData.reflectivity;
		_commsLibReflectivity.fMin = 0.;
		_commsLibReflectivity.fMax = 1.;
		communicableItems.back().pszName = "Reflectivity";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibReflectivity;
		communicableItems.back().nDataLength = sizeof(_commsLibReflectivity);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoR.fCurrent = _uboMatData.albedo.r;
		_commsLibAlbedoR.fMin = 0.0f;
		_commsLibAlbedoR.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo R";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoR;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoR);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoG.fCurrent = _uboMatData.albedo.g;
		_commsLibAlbedoG.fMin = 0.0f;
		_commsLibAlbedoG.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo G";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoG;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoG);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoB.fCurrent = _uboMatData.albedo.b;
		_commsLibAlbedoB.fMin = 0.0f;
		_commsLibAlbedoB.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo B";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoB;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoB);

		// Ok, submit our library
		if (!pplLibraryCreate(_spsCommsData, communicableItems.data(), static_cast<uint32_t>(communicableItems.size())))
		{
			Log(LogLevel::Debug, "PVRScopeRemote: pplLibraryCreate() failed\n");
		}

		// User defined counters
		SSPSCommsCounterDef counterDefines[CounterDefs::NumCounter];
		for (uint32_t i = 0; i < CounterDefs::NumCounter; ++i)
		{
			counterDefines[i].pszName = FrameDefs[i];
			counterDefines[i].nNameLength = static_cast<uint32_t>(strlen(FrameDefs[i]));
		}

		if (!pplCountersCreate(_spsCommsData, counterDefines, CounterDefs::NumCounter))
		{
			Log(LogLevel::Debug, "PVRScopeRemote: pplCountersCreate() failed\n");
		}
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by Shell once per run, just before exiting the program.
		If the rendering context is lost, QuitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanPVRScopeRemote::quitApplication()
{
	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, __FUNCTION__,
		                          static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

		// Close the data connection to PVRPerfServer
		for (uint32_t i = 0; i < 40; ++i)
		{
			char buf[128];
			const int nLen = sprintf(buf, "test %u", i);
			_hasCommunicationError |= !pplSendMark(_spsCommsData, buf, nLen);
		}
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		pplShutdown(_spsCommsData);
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanPVRScopeRemote::initView()
{
	//--------------------
	// Create Vk instance
	bool instanceResult = pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(), this->getDisplay(), _deviceResources->instance, _deviceResources->surface);
	if (!instanceResult || !_deviceResources->instance.isValid())
	{
		setExitMessage("Failed to create the instance.\n");
		return pvr::Result::InitializationError;
	}

	pvr::utils::QueuePopulateInfo queuePopulateInfo = { VkQueueFlags::e_GRAPHICS_BIT, _deviceResources->surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0),
	                           &queuePopulateInfo, 1, &queueAccessInfo);

	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	if (!_deviceResources->device.isValid())
	{
		return pvr::Result::UnknownError;
	}

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// create the swapchain
	bool swapchainResult = pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device,
	                       _deviceResources->surface, getDisplayAttributes(), _deviceResources->swapchain,
	                       _deviceResources->depthStencilImages, swapchainImageUsage,
	                       VkImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT);

	if (!swapchainResult || !_deviceResources->swapchain.isValid())
	{
		return pvr::Result::InitializationError;
	}

	//Create the Commandpool and Descriptorpool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(_deviceResources->queue->getQueueFamilyId(),
	                                VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(
	                                     pvrvk::DescriptorPoolCreateInfo()
	                                     .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 16));
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	_deviceResources->commandBuffer.resize(swapchainLength);
	_deviceResources->semaphoreAcquire.resize(swapchainLength);
	_deviceResources->semaphoreSubmit.resize(swapchainLength);
	_deviceResources->perFrameFence.resize(swapchainLength);
	for (uint32_t i = 0; i < swapchainLength; ++i)
	{
		_deviceResources->commandBuffer[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->semaphoreAcquire[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreSubmit[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
	}

	if (!pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain, &_deviceResources->depthStencilImages[0], _deviceResources->onScreenFramebuffer))
	{
		return pvr::Result::UnknownError;
	}

	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	//	Initialize VBO data
	loadVbos();

	if (!createPipeline()) {  return pvr::Result::NotInitialized; }

	std::vector<pvr::utils::ImageUploadResults> imageUploads;
	// create the pipeline
	if (!createDescriptorSet(imageUploads)) { return pvr::Result::NotInitialized; }

	//	Initialize the UI Renderer
	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
	                                       _deviceResources->commandPool, _deviceResources->queue))
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	// create the pvrscope connection pass and fail text
	_deviceResources->uiRenderer.getDefaultTitle()->setText("PVRScopeRemote");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	_deviceResources->uiRenderer.getDefaultDescription()->setScale(glm::vec2(.5, .5));
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Use PVRTune to remotely control the parameters of this application.");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	// Calculate the projection and view matrices
	// Is the screen rotated?
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	_viewMtx = glm::lookAt(glm::vec3(0.f, 0.f, 75.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	_projectionMtx = pvr::math::perspectiveFov(pvr::Api::Vulkan, glm::pi<float>() / 6, (float)getWidth(),
	                 (float)getHeight(), _deviceResources->scene->getCamera(0).getNear(),
	                 _deviceResources->scene->getCamera(0).getFar(), isRotated ? glm::pi<float>() * .5f : 0.0f);

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i) {	recordCommandBuffer(i);	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanPVRScopeRemote::releaseView()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);
	// Release UIRenderer
	_deviceResources->device->waitIdle();
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanPVRScopeRemote::renderFrame()
{
	_deviceResources->perFrameFence[_frameId]->wait();
	_deviceResources->perFrameFence[_frameId]->reset();

	pvrvk::Semaphore& semaphoreAcquire = _deviceResources->semaphoreAcquire[_frameId];
	pvrvk::Semaphore& semaphoreSubmit = _deviceResources->semaphoreSubmit[_frameId];

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), semaphoreAcquire);
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);
	bool currCommunicationErr = _hasCommunicationError;
	if (_spsCommsData)
	{
		// mark every N frames
		if (!(_frameCounter % 100))
		{
			char buf[128];
			const int nLen = sprintf(buf, "frame %u", _frameCounter);
			_hasCommunicationError |= !pplSendMark(_spsCommsData, buf, nLen);
		}

		// Check for dirty items
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "dirty",
		                          static_cast<uint32_t>(strlen("dirty")), _frameCounter);
		{
			uint32_t nItem, nNewDataLen;
			const char* pData;
			bool recompile = false;
			while (pplLibraryDirtyGetFirst(_spsCommsData, &nItem, &nNewDataLen, &pData))
			{
				Log(LogLevel::Debug, "dirty item %u %u 0x%08x\n", nItem, nNewDataLen, pData);
				switch (nItem)
				{
				case 0:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_uboMatData.specularExponent = psData->fCurrent;
						_uboMatData.isDirty = true;
						Log(LogLevel::Information, "Setting Specular Exponent to value [%6.2f]", _uboMatData.specularExponent);
					}
					break;
				case 1:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_uboMatData.metallicity = psData->fCurrent;
						_uboMatData.isDirty = true;
						Log(LogLevel::Information, "Setting Metallicity to value [%3.2f]", _uboMatData.metallicity);
					}
					break;
				case 2:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_uboMatData.reflectivity = psData->fCurrent;
						_uboMatData.isDirty = true;
						Log(LogLevel::Information, "Setting Reflectivity to value [%3.2f]", _uboMatData.reflectivity);
					}
					break;
				case 3:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_uboMatData.albedo.r = psData->fCurrent;
						_uboMatData.isDirty = true;
						Log(LogLevel::Information, "Setting Albedo Red channel to value [%3.2f]", _uboMatData.albedo.r);
					}
					break;
				case 4:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_uboMatData.albedo.g = psData->fCurrent;
						_uboMatData.isDirty = true;
						Log(LogLevel::Information, "Setting Albedo Green channel to value [%3.2f]", _uboMatData.albedo.g);
					}
					break;
				case 5:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_uboMatData.albedo.b = psData->fCurrent;
						_uboMatData.isDirty = true;
						Log(LogLevel::Information, "Setting Albedo Blue channel to value [%3.2f]", _uboMatData.albedo.b);
					}
					break;
				}
			}

			if (recompile)
			{
				Log(LogLevel::Error, "*** Could not recompile the shaders passed from PVRScopeCommunication ****");
			}
		}
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
	}

	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "draw",
		                          static_cast<uint32_t>(strlen("draw")), _frameCounter);
	}

	updateUbo(swapchainIndex);

	// Set eye position in model space
	// Now that the uniforms are set, call another function to actually draw the mesh.
	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "UIRenderer",
		                          static_cast<uint32_t>(strlen("UIRenderer")), _frameCounter);
	}

	if (_hasCommunicationError)
	{
		_deviceResources->uiRenderer.getDefaultControls()->setText("Communication Error:\nPVRScopeComms failed\n"
		    "Is PVRPerfServer connected?");
		_deviceResources->uiRenderer.getDefaultControls()->setColor(glm::vec4(.8f, .3f, .3f, 1.0f));
		_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
		_hasCommunicationError = false;
	}
	else
	{
		_deviceResources->uiRenderer.getDefaultControls()->setText("PVRScope Communication established.");
		_deviceResources->uiRenderer.getDefaultControls()->setColor(glm::vec4(1.f));
		_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	}

	if (_spsCommsData) { _hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData); }

	// send counters
	_counterReadings[CounterDefs::Counter] = _frameCounter;
	_counterReadings[CounterDefs::Counter10] = _frame10Counter;
	if (_spsCommsData) { _hasCommunicationError |= !pplCountersUpdate(_spsCommsData, _counterReadings); }

	// update some counters
	++_frameCounter;
	if (0 == (_frameCounter / 10) % 10) { _frame10Counter += 10; }

	// SUBMIT
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffer[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &semaphoreAcquire;
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &semaphoreSubmit;
	submitInfo.numSignalSemaphores = 1;
	VkPipelineStageFlags waitStages = VkPipelineStageFlags::e_ALL_GRAPHICS_BIT;
	submitInfo.waitDestStages = &waitStages;

	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameFence[_frameId]);

	if (this->shouldTakeScreenshot())
	{
		if (_deviceResources->swapchain->supportsUsage(VkImageUsageFlags::e_TRANSFER_SRC_BIT))
		{
			pvr::utils::takeScreenshot(_deviceResources->swapchain, swapchainIndex, _deviceResources->commandPool, _deviceResources->queue, this->getScreenshotFileName());
		}
		else
		{
			Log(LogLevel::Warning, "Could not take screenshot as the swapchain does not support TRANSFER_SRC_BIT");
		}
	}

	// PRESENT
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.numSwapchains = 1;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &semaphoreSubmit;
	_deviceResources->queue->present(presentInfo);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Draws a assets::Mesh after the model view matrix has been set and the material prepared.
\param	nodeIndex Node index of the mesh to draw
***********************************************************************************************************************/
void VulkanPVRScopeRemote::drawMesh(int nodeIndex, pvrvk::CommandBuffer& command)
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	const int32_t meshIndex = _deviceResources->scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _deviceResources->scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	command->bindVertexBuffer(_deviceResources->vbos[meshIndex], 0, 0);

	//	The geometry can be exported in 4 ways:
	//	- Indexed Triangle list
	//	- Non-Indexed Triangle list
	//	- Indexed Triangle strips
	//	- Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (_deviceResources->ibos[meshIndex].isValid())
		{
			// Indexed Triangle list
			command->bindIndexBuffer(_deviceResources->ibos[meshIndex], 0, VkIndexType::e_UINT16);
			command->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			command->draw(0, mesh.getNumFaces(), 0, 1);
		}
	}
	else
	{
		for (int32_t i = 0; i < (int32_t)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (_deviceResources->ibos[meshIndex].isValid())
			{
				command->bindIndexBuffer(_deviceResources->ibos[meshIndex], 0, VkIndexType::e_UINT16);

				// Indexed Triangle strips
				command->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				command->draw(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

bool VulkanPVRScopeRemote::createDescriptorSet(std::vector<pvr::utils::ImageUploadResults>& uploadResults)
{
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	// create the MVP ubo
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("MVP", pvr::GpuDatatypes::mat4x4);
		desc.addElement("MVIT", pvr::GpuDatatypes::mat3x3);

		_deviceResources->uboMVPBufferView.initDynamic(desc, swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));

		_deviceResources->uboMVP = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->uboMVPBufferView.getSize(),
		                           VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	pvrvk::WriteDescriptorSet descSetWrites[pvrvk::FrameworkCaps::MaxSwapChains + 2];
	uint32_t i = 0;
	for (; i < swapchainLength; ++i)
	{
		_deviceResources->uboMvpDesc[i] = _deviceResources->descriptorPool->allocateDescriptorSet(
		                                    _deviceResources->pipeline->getPipelineLayout()->getDescriptorSetLayout(0));

		descSetWrites[i]
		.set(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->uboMvpDesc[i], 0)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboMVP, 0, _deviceResources->uboMVPBufferView.getDynamicSliceSize()));
	}
	_deviceResources->commandBuffer[0]->begin();
	//--- create the material descriptor
	uploadResults.push_back(pvr::utils::loadAndUploadImage(_deviceResources->device, TextureFile, true,
	                        _deviceResources->commandBuffer[0], *this));
	if (uploadResults.back().getImageView().isNull())
	{
		setExitMessage("ERROR: Failed to load texture.");
		return false;
	}
	_deviceResources->commandBuffer[0]->end();
	// submit the texture upload commands
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffer[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);

	_deviceResources->texture = uploadResults.back().getImageView();

	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = VkFilter::e_LINEAR;
	samplerInfo.mipMapMode = VkSamplerMipmapMode::e_LINEAR;
	samplerInfo.magFilter = VkFilter::e_LINEAR;
	pvrvk::Sampler trilinearSampler = _deviceResources->device->createSampler(samplerInfo);

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("AlbdeoModulation", pvr::GpuDatatypes::vec3);
		desc.addElement("SpecularExponent", pvr::GpuDatatypes::Float);
		desc.addElement("Metallicity", pvr::GpuDatatypes::Float);
		desc.addElement("Reflectivity", pvr::GpuDatatypes::Float);

		_deviceResources->uboMaterialBufferView.init(desc);
		_deviceResources->uboMaterial = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->uboMaterialBufferView.getSize(),
		                                VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	_deviceResources->uboMatDesc = _deviceResources->descriptorPool->allocateDescriptorSet(
	                                 _deviceResources->pipeline->getPipelineLayout()->getDescriptorSetLayout(PipelineConfigs::DescriptorMaterial));

	descSetWrites[i++]
	.set(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->uboMatDesc, PipelineConfigs::MaterialBindingTex)
	.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->texture, trilinearSampler, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	descSetWrites[i]
	.set(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->uboMatDesc, PipelineConfigs::MaterialBindingData)
	.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboMaterial, 0, _deviceResources->uboMaterialBufferView.getSize()));

	_deviceResources->device->updateDescriptorSets(descSetWrites, swapchainLength + 2, nullptr, 0);
	_deviceResources->queue->waitIdle();// make sure the queue submission is done.
	return true;
}

void VulkanPVRScopeRemote::updateUbo(uint32_t swapchain)
{
	// Rotate and Translation the model matrix
	const glm::mat4 modelMtx = glm::rotate(_angleY, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.6f))
	                           * _deviceResources->scene->getWorldMatrix(0);
	_angleY += (2 * glm::pi<float>() * getFrameTime() / 1000) / 10;

	// Set model view projection matrix
	const glm::mat4 mvMatrix = _viewMtx * modelMtx;

	{
		void* memory;
		_deviceResources->uboMVP->getDeviceMemory()->map(&memory, _deviceResources->uboMVPBufferView.getDynamicSliceOffset(swapchain), _deviceResources->uboMVPBufferView.getDynamicSliceSize());
		_deviceResources->uboMVPBufferView.pointToMappedMemory(memory, swapchain);
		_deviceResources->uboMVPBufferView.getElement(0, 0, swapchain).setValue(_projectionMtx * mvMatrix);
		_deviceResources->uboMVPBufferView.getElement(1, 0, swapchain).setValue(glm::mat3x4(glm::inverseTranspose(glm::mat3(mvMatrix))));
		_deviceResources->uboMVP->getDeviceMemory()->unmap();
	}

	if (_uboMatData.isDirty)
	{
		_deviceResources->device->waitIdle();
		void* memory;
		_deviceResources->uboMaterial->getDeviceMemory()->map(&memory);
		_deviceResources->uboMaterialBufferView.pointToMappedMemory(memory);
		_deviceResources->uboMaterialBufferView.getElementByName("AlbdeoModulation").setValue(glm::vec4(_uboMatData.albedo, 0.0f));
		_deviceResources->uboMaterialBufferView.getElementByName("SpecularExponent").setValue(_uboMatData.specularExponent);
		_deviceResources->uboMaterialBufferView.getElementByName("Metallicity").setValue(_uboMatData.metallicity);
		_deviceResources->uboMaterialBufferView.getElementByName("Reflectivity").setValue(_uboMatData.reflectivity);
		_deviceResources->uboMaterial->getDeviceMemory()->unmap();
		_uboMatData.isDirty = false;
	}
}

/*!*********************************************************************************************************************
\brief	pre-record the rendering the commands
***********************************************************************************************************************/
void VulkanPVRScopeRemote::recordCommandBuffer(uint32_t swapchain)
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	_deviceResources->commandBuffer[swapchain]->begin();
	const pvrvk::ClearValue clearValues[2] =
	{
		pvrvk::ClearValue(0.00, 0.70, 0.67, 1.0f),
		pvrvk::ClearValue(1.f, 0u)
	};
	_deviceResources->commandBuffer[swapchain]->beginRenderPass(_deviceResources->onScreenFramebuffer[swapchain],
	    pvrvk::Rect2Di(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

	// Use shader program
	_deviceResources->commandBuffer[swapchain]->bindPipeline(_deviceResources->pipeline);

	// Bind texture
	_deviceResources->commandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->pipeline->getPipelineLayout(), 0, _deviceResources->uboMvpDesc[swapchain], 0);
	_deviceResources->commandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->pipeline->getPipelineLayout(), 1, _deviceResources->uboMatDesc, 0);

	drawMesh(0, _deviceResources->commandBuffer[swapchain]);

	_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBuffer[swapchain]);
	// Displays the demo name using the tools. For a detailed explanation, see the example
	// IntroUIRenderer
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->commandBuffer[swapchain]->endRenderPass();
	_deviceResources->commandBuffer[swapchain]->end();
}

/*!*********************************************************************************************************************
\return	Return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanPVRScopeRemote()); }
