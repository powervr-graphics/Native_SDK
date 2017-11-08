/*!*********************************************************************************************************************
\File         VulkanNavigation2D.cpp
\Title        Vulkan Navigation 2D
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        The 2D navigation example demonstrates the entire process of creating a navigational map from raw XML data.
***********************************************************************************************************************/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "../NavDataProcess.h"

const pvr::StringHash SpriteFileNames[BuildingType::None] =
{
	pvr::StringHash("shop.pvr"), pvr::StringHash("bar.pvr"), pvr::StringHash("cafe.pvr"),
	pvr::StringHash("fastfood.pvr"), pvr::StringHash("pub.pvr"), pvr::StringHash("college.pvr"),
	pvr::StringHash("library.pvr"), pvr::StringHash("university.pvr"), pvr::StringHash("ATM.pvr"),
	pvr::StringHash("bank.pvr"), pvr::StringHash("restaurant.pvr"), pvr::StringHash("doctors.pvr"),
	pvr::StringHash("dentist.pvr"), pvr::StringHash("hospital.pvr"), pvr::StringHash("pharmacy.pvr"),
	pvr::StringHash("cinema.pvr"), pvr::StringHash("casino.pvr"), pvr::StringHash("theatre.pvr"),
	pvr::StringHash("fire.pvr"), pvr::StringHash("courthouse.pvr"), pvr::StringHash("police.pvr"),
	pvr::StringHash("postoffice.pvr"), pvr::StringHash("toilets.pvr"), pvr::StringHash("worship.pvr"),
	pvr::StringHash("petrol.pvr"), pvr::StringHash("parking.pvr"), pvr::StringHash("other.pvr"),
	pvr::StringHash("postbox.pvr"), pvr::StringHash("vets.pvr"), pvr::StringHash("embassy.pvr"),
	pvr::StringHash("hairdresser.pvr"), pvr::StringHash("butcher.pvr"), pvr::StringHash("optician.pvr"),
	pvr::StringHash("florist.pvr"),
};

enum class MapColors
{
	Clear, RoadArea, Motorway, Trunk, Primary, Secondary,
	Service, Other, Parking, Building, Outline, Total
};

namespace SetBinding {
enum SetBinding
{
	UBOStatic = 0,
	UBODynamic = 1
};
}

struct Icon
{
	pvr::ui::Image image;
};

struct Label
{
	pvr::ui::Text text;
};

struct AmenityIconGroup
{
	pvr::ui::PixelGroup group;
	Icon icon;
	IconData iconData;
};

struct AmenityLabelGroup
{
	pvr::ui::PixelGroup group;
	Label label;
	IconData iconData;
};

enum class CameraMode
{
	Auto,
	Manual
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvrvk::Surface surface;
	pvrvk::Device device;
	pvrvk::Swapchain swapchain;
	pvrvk::Queue queue;

	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;

	pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	struct Ubo
	{
		pvrvk::DescriptorSetLayout layout;
		pvr::utils::StructuredBufferView bufferView;
		pvrvk::Buffer buffer;
		pvrvk::DescriptorSet sets[static_cast<uint32_t>(4)];
	};

	Ubo uboDynamic;
	Ubo uboMvp;

	// Pipelines
	pvrvk::GraphicsPipeline roadPipe;
	pvrvk::GraphicsPipeline fillPipe;

	// Descriptor set for texture
	pvrvk::PipelineLayout pipeLayout;

	// Frame and primary command buffers
	pvr::Multi<pvrvk::Framebuffer> framebuffer;
	pvr::Multi<pvrvk::CommandBuffer> commandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> uiRendererCmdBuffers;

	// Texture atlas meta data.
	pvr::TextureHeader texAtlasHeader;
	// Array of UV offsets into the texture atlas.
	pvrvk::Rect2Df atlasOffsets[BuildingType::None];
	// Raw texture atlas containing all sprites.
	pvrvk::ImageView imageAtlas;

	// Font texture data
	pvrvk::ImageView fontImage;
	pvr::Texture fontTexture;
	pvrvk::Sampler fontSampler;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;
};

struct Plane
{
	glm::vec3 normal;
	float distance;

	Plane(glm::vec4 n)
	{
		float invLen = 1.0f / glm::length(glm::vec3(n));
		normal = glm::vec3(n) * invLen;
		normal.y = -normal.y; // Negate normal Y for Vulkan
		distance = n.w * invLen;
	}

	Plane() : normal(glm::vec3()), distance(0.0f) {}
};

struct PerSwapTileResources
{
	pvrvk::SecondaryCommandBuffer uicbuff[LOD::Count];
	pvrvk::SecondaryCommandBuffer secCbo;

	bool tileWasVisible;
	bool uiWasVisible;
	pvr::RefCountedResource<pvr::ui::UIRenderer> renderer;
	pvr::ui::Font font;
	pvr::ui::PixelGroup tileGroup[LOD::Count];
	pvr::ui::PixelGroup cameraRotateGroup[LOD::Count];
	std::vector<Label> labels[LOD::Count];
	std::vector<AmenityIconGroup> amenityIcons[LOD::Count];
	std::vector<AmenityLabelGroup> amenityLabels[LOD::Count];
	pvr::ui::Image spriteImages[BuildingType::None];
	PerSwapTileResources()
	{
		tileWasVisible = false;
		uiWasVisible = false;
	}
};

struct TileRenderingResources
{
	pvrvk::Buffer vbo;
	pvrvk::Buffer ibo;
	uint32_t numSpriteInstances;
	uint32_t numSprites;

	PerSwapTileResources swapResources[pvrvk::FrameworkCaps::MaxSwapChains];

	TileRenderingResources()
	{}

	void reset()
	{
		vbo.reset();
		ibo.reset();
		for (uint32_t i = 0; i < pvrvk::FrameworkCaps::MaxSwapChains; i++)
		{
			swapResources[i].tileWasVisible = false;
			swapResources[i].uiWasVisible = false;
			for (uint32_t j = 0; j < LOD::Count; ++j)
			{
				swapResources[i].cameraRotateGroup[j].reset();
				swapResources[i].labels[j].clear();
				swapResources[i].amenityIcons[j].clear();
				swapResources[i].amenityLabels[j].clear();
				swapResources[i].tileGroup[j].reset();
			}
			swapResources[i].font.reset();
			swapResources[i].renderer.reset();
		}
	}
};

// Alpha, luminance texture.
const char* MapFile = "map.osm";
const char* FontFile = "font.pvr";
const float scales[LOD::Count] = { 10.0f, 7.0f, 5.0f, 3.0f, 2.0f };
const float routescales[LOD::Count] = { 11.0f, 10.0f, 7.0f, 5.0f, 2.0f };

/*!*********************************************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class VulkanNavigation2D : public pvr::Shell
{
	std::unique_ptr<NavDataProcess> _OSMdata;

	// Graphics resources - buffers, samplers, descriptors.
	std::auto_ptr<DeviceResources> _deviceResources;
	std::vector<std::vector<TileRenderingResources>> _tileRenderingResources;

	uint16_t _currentScaleLevel;
	uint32_t _numSwapchains;
	uint32_t _frameId;

	glm::mat4 _mapMVPMtx;
	glm::vec4 _clearColor;

	// Road types colors
	glm::vec4 _roadAreaColor;
	glm::vec4 _motorwayColor;
	glm::vec4 _trunkRoadColor;
	glm::vec4 _primaryRoadColor;
	glm::vec4 _secondaryRoadColor;
	glm::vec4 _serviceRoadColor;
	glm::vec4 _otherRoadColor;

	// other map object colors
	glm::vec4 _parkingColor;
	glm::vec4 _buildingColor;
	glm::vec4 _outlineColor;

	// Transformation variables
	glm::vec2 _translation;
	float _scale;
	glm::mat4 _projMtx;
	glm::mat4 _mapProjMtx;
	float _rotation;

	std::vector<Plane> _clipPlanes;

	// Map tile dimensions
	uint32_t _numRows;
	uint32_t _numCols;

	float _totalRouteDistance;
	float _weight;
	float _keyFrameTime;

	CameraMode _cameraMode;

	bool _uiRendererChanged[pvrvk::FrameworkCaps::MaxSwapChains];

	glm::dvec2 _mapWorldDim;

	float _timePassed;
	bool _increaseScale;
	bool _scaleChange;
	bool _updateRotation;
	bool _turning;
	uint16_t _previousScaleLevel;
	uint32_t _routeIndex;
	float _animTime;
	float _rotateTime;
	float _rotateAnimTime;
        float _screenWidth, _screenHeight;
public:
	VulkanNavigation2D() : _totalRouteDistance(0.0f), _weight(0.0f),
		_projMtx(1.0), _rotation(0.0f), _cameraMode(CameraMode::Auto)
	{}

	// PVR shell functions
	pvr::Result initApplication() override;
	pvr::Result quitApplication() override;
	pvr::Result initView() override;
	pvr::Result releaseView() override;
	pvr::Result renderFrame() override;

	bool initializeRenderers(TileRenderingResources* begin, TileRenderingResources* end, Tile& tile);
	bool createDescriptorSets();
	void createBuffers();
	bool createUBOs();
	bool loadTexture(pvrvk::CommandBuffer& uploadCmd, std::vector<pvr::utils::ImageUploadResults>& uploadresults);
	void setColors();
	void initRoute();
	void updateCommandBuffer(pvrvk::CommandBuffer& cbo, uint32_t index);
	void recordUiRendererCommandBuffer(uint32_t swapchainIndex);
	void updateLabels(uint32_t col, uint32_t row, uint32_t swapchainIndex);
	void updateAmenities(uint32_t col, uint32_t row, uint32_t swapchainIndex);
	void updateGroups(uint32_t col, uint32_t row, uint32_t swapindex);
	void updateAnimation(uint32_t swapindex);
	void calculateClipPlanes();
	bool inFrustum(glm::vec2 min, glm::vec2 max);
	void createUIRendererItems();
	void eventMappedInput(pvr::SimplifiedInput e);
	void resetCameraVariables();
	void updateSubtitleText();
	void handleInput();
	pvrvk::SecondaryCommandBuffer getOrCreateTileUiCommandBuffer(TileRenderingResources& tile, uint32_t swapIdx, uint32_t lod)
	{
		auto& retval = tile.swapResources[swapIdx].uicbuff[lod];
		if (retval.isNull())
		{
			retval = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		}
		return retval;
	}
};

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by the Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it  (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanNavigation2D::initApplication()
{
	// As we are rendering in 2D we have no need for either of the depth of stencil buffers
	setDepthBitsPerPixel(0);
	setStencilBitsPerPixel(0);

	_clipPlanes.resize(4);

	_OSMdata.reset(new NavDataProcess(getAssetStream(MapFile)));
	pvr::Result result = _OSMdata->loadAndProcessData();
	Log(LogLevel::Information, "MAP SIZE IS: [ %d x %d ] TILES", _OSMdata->getNumRows(), _OSMdata->getNumCols());

	_frameId = 0;

	resetCameraVariables();

	return result;
}

void VulkanNavigation2D::handleInput()
{
	const float dt = float(getFrameTime());
	const float transDelta = dt;
	int right = isKeyPressed(pvr::Keys::Right) - isKeyPressed(pvr::Keys::Left);
	int up = isKeyPressed(pvr::Keys::Up) - isKeyPressed(pvr::Keys::Down);
	if (isKeyPressed(pvr::Keys::W))
	{
		_scale *= 1.05f;
	}
	if (isKeyPressed(pvr::Keys::S))
	{
		_scale *= .95f;
		_scale = glm::max(_scale, 0.1f);
	}

	if (isKeyPressed(pvr::Keys::A))
	{
		_rotation += dt * .1f;
	}
	if (isKeyPressed(pvr::Keys::D))
	{
		_rotation -= dt * .1f;
	}

	if (_rotation <= -180) { _rotation += 360; }
	if (_rotation > 180) { _rotation -= 360; }

	float fup = (-transDelta * up / _scale) * glm::cos(glm::pi<float>() * _rotation / 180) + (transDelta * right / _scale) * glm::sin(glm::pi<float>() * _rotation / 180);
	float fright = (-transDelta * up / _scale) * glm::sin(glm::pi<float>() * _rotation / 180) - (transDelta * right / _scale) * glm::cos(glm::pi<float>() * _rotation / 180);

	_translation.x += fright;
	_translation.y += fup;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanNavigation2D::initView()
{
	_deviceResources.reset(new DeviceResources());

	if (!pvr::utils::createInstanceAndSurface(getApplicationName(), getWindow(), getDisplay(), _deviceResources->instance, _deviceResources->surface))
	{
		setExitMessage("Failed to create Vulkan Instance");
		return pvr::Result::UnknownError;
	}
	pvr::utils::QueuePopulateInfo queuePopulate = { VkQueueFlags::e_GRAPHICS_BIT, _deviceResources->surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulate, 1, &queueAccessInfo);
	if (_deviceResources->device.isNull())
	{
		setExitMessage("Failed to create the Vulkan Device");
		return pvr::Result::UnknownError;
	}
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	_deviceResources->swapchain = pvr::utils::createSwapchain(_deviceResources->device, _deviceResources->surface, getDisplayAttributes(), swapchainImageUsage);

	// Create the swapchain
	if (!_deviceResources->swapchain.isValid())
	{
		setExitMessage("Failed to create Swapchain");
		return pvr::Result::UnknownError;
	}
	_numSwapchains = _deviceResources->swapchain->getSwapchainLength();
	if (!pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain, nullptr, _deviceResources->framebuffer))
	{
		setExitMessage("Failed to create OnScreen Framebuffer and RenderPass");
		return pvr::Result::UnknownError;
	}

	if (!createDescriptorSets())
	{
		setExitMessage("Failed to create Descriptor Sets");
		return pvr::Result::UnknownError;
	}

	// Create the commandpool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(queueAccessInfo.familyId, VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	// Create the commandbuffers
	if (!_deviceResources->commandPool->allocateCommandBuffers(_numSwapchains, &_deviceResources->commandBuffers[0]))
	{
		setExitMessage("Failed to allocate CommandBuffers");
		return pvr::Result::UnknownError;
	}

	if (!_deviceResources->commandPool->allocateSecondaryCommandBuffers(_numSwapchains, &_deviceResources->uiRendererCmdBuffers[0]))
	{
		setExitMessage("Failed to allocate CommandBuffers");
		return pvr::Result::UnknownError;
	}

	// load the textures using the main command buffer
	std::vector<pvr::utils::ImageUploadResults> uploadResults;

	_deviceResources->commandBuffers[0]->begin();
	if (!loadTexture(_deviceResources->commandBuffers[0], uploadResults))
	{
		setExitMessage("Failed to load the textures");
		return pvr::Result::UnknownError;
	}
	_deviceResources->commandBuffers[0]->end();

	// submit the main command buffer to complete the texture load
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();

	// reset the command buffer so its ready to be used later
	_deviceResources->commandBuffers[0]->reset(VkCommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);
	uploadResults.clear();

	_numRows = _OSMdata->getNumRows();
	_numCols = _OSMdata->getNumCols();

	Log(LogLevel::Information, "Initialising Tile Data");

	_mapWorldDim = getMapWorldDimensions(*_OSMdata, _numCols, _numRows);

	_OSMdata->initTiles(glm::ivec2(getWidth(), getHeight()));

	_tileRenderingResources.resize(_numCols);
	for (uint32_t i = 0; i < _numCols; ++i)
	{
		_tileRenderingResources[i].resize(_numRows);
	}

	if (!createUBOs())
	{
		setExitMessage("Failed to create the Ubos");
		return pvr::Result::UnknownError;
	}

	for (uint32_t i = 0; i < _numSwapchains; ++i)
	{
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
	}

	// Pipeline parameters
	pvrvk::GraphicsPipelineCreateInfo roadInfo;
	pvrvk::GraphicsPipelineCreateInfo fillInfo;

	// Set parameters shared by all pipelines
	roadInfo.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(float) * 4));
	roadInfo.vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(0, 0, VkFormat::e_R32G32_SFLOAT, 0));
	roadInfo.depthStencil.enableDepthTest(false).enableDepthWrite(false);
	roadInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState(false));
	roadInfo.vertexShader = _deviceResources->device->createShader(getAssetStream("VertShader_vk.vsh.spv")->readToEnd<uint32_t>());
	roadInfo.fragmentShader = _deviceResources->device->createShader(getAssetStream("FragShader_vk.fsh.spv")->readToEnd<uint32_t>());
	roadInfo.inputAssembler.setPrimitiveTopology(VkPrimitiveTopology::e_TRIANGLE_LIST);
	roadInfo.rasterizer.setCullMode(VkCullModeFlags::e_NONE);
	roadInfo.renderPass = _deviceResources->framebuffer[0]->getRenderPass();
	roadInfo.pipelineLayout = _deviceResources->pipeLayout;
	pvr::utils::populateViewportStateCreateInfo(_deviceResources->framebuffer[0], roadInfo.viewport);
	fillInfo = roadInfo;

	roadInfo.vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(1, 0, VkFormat::e_R32G32_SFLOAT, sizeof(float) * 2)); // Set vertex & tex-co-ordinate layout

	roadInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState(
	    true, VkBlendFactor::e_SRC_ALPHA,  VkBlendFactor::e_ONE_MINUS_SRC_ALPHA)); // Blending (src Alpha, 1 - src Alpha)
	roadInfo.vertexShader = _deviceResources->device->createShader(getAssetStream("AA_VertShader_vk.vsh.spv")->readToEnd<uint32_t>());
	roadInfo.fragmentShader = _deviceResources->device->createShader(getAssetStream("AA_FragShader_vk.fsh.spv")->readToEnd<uint32_t>());

	// Create pipeline objects
	_deviceResources->roadPipe = _deviceResources->device->createGraphicsPipeline(roadInfo);
	_deviceResources->fillPipe = _deviceResources->device->createGraphicsPipeline(fillInfo);

	Log(LogLevel::Information, "Remapping item coordinate data");
	remapItemCoordinates(*_OSMdata, _numCols, _numRows, _mapWorldDim);

	Log(LogLevel::Information, "Creating UI renderer items");
	createUIRendererItems();
	setColors();

        _screenWidth = getWidth();
        _screenHeight = getHeight();

        if(isScreenRotated() && isFullScreen())
        {
            std::swap(_screenWidth, _screenHeight);
        }

	_projMtx = pvr::math::ortho(pvr::Api::Vulkan, 0.0, (float)_screenWidth, 0.0f, (float)_screenHeight);
	_mapProjMtx = _tileRenderingResources[0][0].swapResources[0].renderer->getScreenRotation() * _projMtx;

	Log(LogLevel::Information, "Creating per Tile buffers");
	createBuffers();

	Log(LogLevel::Information, "Converting Route");
	initRoute();

	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->framebuffer[0]->getRenderPass(), 0,
	                                       _deviceResources->commandPool, _deviceResources->queue))
	{
		setExitMessage("Error: Failed to initialize the UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Navigation2D");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	updateSubtitleText();

	for (uint32_t i = 0; i < _numSwapchains; ++i)
	{
		recordUiRendererCommandBuffer(i);
	}

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanNavigation2D::renderFrame()
{
	handleInput();
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	updateAnimation(swapchainIndex);
	float r = glm::radians(_rotation);

	_mapMVPMtx = _mapProjMtx
	             * glm::translate(glm::vec3(_translation.x + _screenWidth * .5 /*center the map*/, _translation.y + _screenHeight * .5/*center the map*/, 0.0f))// final transform
	             * glm::translate(glm::vec3(-_translation.x, -_translation.y, 0.0f))// undo the translation
	             * glm::rotate(r, glm::vec3(0.0f, 0.0f, 1.0f))// rotate
	             * glm::scale(glm::vec3(_scale, _scale, 1.0f))// scale the focus area
	             * glm::translate(glm::vec3(_translation.x, _translation.y, 0.0f)); // translate the camera to the center of the current focus area

	{
		void* memory;
		_deviceResources->uboMvp.buffer->getDeviceMemory()->map(&memory,
		    _deviceResources->uboMvp.bufferView.getDynamicSliceOffset(swapchainIndex),
		    _deviceResources->uboMvp.bufferView.getDynamicSliceSize());
		_deviceResources->uboMvp.bufferView.pointToMappedMemory(memory, swapchainIndex);
		_deviceResources->uboMvp.bufferView.getElement(0, 0, swapchainIndex).setValue(_mapMVPMtx);
		_deviceResources->uboMvp.buffer->getDeviceMemory()->unmap();
	}

	calculateClipPlanes();

	// Update commands
	updateCommandBuffer(_deviceResources->commandBuffers[swapchainIndex], swapchainIndex);

	// SUBMIT
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numSignalSemaphores = 1;
	VkPipelineStageFlags waitStage = VkPipelineStageFlags::e_ALL_GRAPHICS_BIT;
	submitInfo.waitDestStages = &waitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[swapchainIndex]);

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
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	presentInfo.numWaitSemaphores = 1;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanNavigation2D::releaseView()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->perFrameAcquireFence[i]->wait();
		_deviceResources->perFrameAcquireFence[i]->reset();

		_deviceResources->perFrameCommandBufferFence[i]->wait();
		_deviceResources->perFrameCommandBufferFence[i]->reset();
	}

	// Reset context and associated resources.
	_deviceResources->device->waitIdle();

	// Clean up tile rendering resource data.
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_tileRenderingResources[i].clear();
	}
	_tileRenderingResources.clear();

	_OSMdata->releaseTileData();
	_OSMdata.reset();

	_deviceResources.reset(0);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanNavigation2D::quitApplication()
{
	return pvr::Result::Success;
}

void VulkanNavigation2D::updateSubtitleText()
{
	if (_cameraMode == CameraMode::Auto)
	{
		_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("Automatic Camera Mode"));
	}
	else
	{
		_deviceResources->uiRenderer.getDefaultDescription()->setText("Manual Camera Model use up/down/left/right to control the camera");
	}
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_uiRendererChanged[i] = true;
	}
}

void VulkanNavigation2D::resetCameraVariables()
{
	_weight = 0.0f;
	_currentScaleLevel = LOD::L4;
	_previousScaleLevel = _currentScaleLevel;
	_scale = scales[_currentScaleLevel];
	_rotation = 0.0f;
	_keyFrameTime = 0.0f;

	_timePassed = 0.0f;
	_routeIndex = 0;
	_animTime = 0.0f;
	_updateRotation = true;
	_rotateTime = 0.0f;
	_rotateAnimTime = 0.0f;
	_turning = false;
	_increaseScale = false;
	_scaleChange = false;
	_translation = _OSMdata->getRouteData()[_routeIndex].point;
}

/*!********************************************************************************************
\brief  Handles user input and updates live variables accordingly.
***********************************************************************************************/
void VulkanNavigation2D::eventMappedInput(pvr::SimplifiedInput e)
{
	const float transDelta = float(getFrameTime());

	switch (e)
	{
	case pvr::SimplifiedInput::ActionClose:
		this->exitShell();
		break;
	case pvr::SimplifiedInput::Action1:
		if (_cameraMode == CameraMode::Auto)
		{
			_cameraMode = CameraMode::Manual;
		}
		else
		{
			_cameraMode = CameraMode::Auto;
		}
		resetCameraVariables();
		updateSubtitleText();
		_deviceResources->device->waitIdle();
		break;
// IF THE PLATFORM IS DESKTOP, THE FOLLOWING ARE NOT USED AS KEYBOARD INPUT IS DIRECTLY USED
#ifndef PVR_PLATFORM_IS_DESKTOP
	// zoom in
	case pvr::SimplifiedInput::Action2:
		if (_cameraMode == CameraMode::Manual)
		{
			_scale *= 1.05f;
			_scale = glm::min(_scale, 10.0f);
		}
		break;
	// zoom out
	case pvr::SimplifiedInput::Action3:
		if (_cameraMode == CameraMode::Manual)
		{
			_scale *= .95f;
			_scale = glm::max(_scale, .01f);
		}
		break;
	case pvr::SimplifiedInput::Up:
		if (_cameraMode == CameraMode::Manual)
		{
			float fup = (-transDelta * 1.0f / _scale);
			_translation.y += fup;
		}
		break;
	case pvr::SimplifiedInput::Down:
		if (_cameraMode == CameraMode::Manual)
		{
			float fup = -(-transDelta * 1.0f / _scale);
			_translation.y += fup;
		}
		break;
	case pvr::SimplifiedInput::Left:
		if (_cameraMode == CameraMode::Manual)
		{
			float fright = (transDelta * 1.0f / _scale);
			_translation.x += fright;
		}
		break;
	case pvr::SimplifiedInput::Right:
		if (_cameraMode == CameraMode::Manual)
		{
			float fright = -(transDelta * 1.0f / _scale);
			_translation.x += fright;
		}
		break;
#endif
	}
}

bool VulkanNavigation2D::initializeRenderers(TileRenderingResources* begin, TileRenderingResources* end, Tile& tile)
{
	for (uint32_t swapIndex = 0; swapIndex < _deviceResources->swapchain->getSwapchainLength(); swapIndex++)
	{
		// determine the number of sprites for the current tile
		uint32_t numSprites = 0;
		uint32_t numSpriteInstances = 0;
		for (uint32_t lod = 0; lod < LOD::Count; ++lod)
		{
			numSprites += static_cast<uint32_t>(tile.labels[lod].size());
			numSprites += static_cast<uint32_t>(tile.icons[lod].size());
			numSprites += static_cast<uint32_t>(tile.amenityLabels[lod].size());
		}

		for (uint32_t lod = 0; lod < LOD::Count; ++lod)
		{
			numSpriteInstances += static_cast<uint32_t>(tile.icons[lod].size()) * 2 * 2; // each amenity icon is part of a group (sprite + group) and is part of translation and camera groups
			numSpriteInstances += static_cast<uint32_t>(tile.amenityLabels[lod].size()) * 2 * 2; // each amenity label is part of a group (sprite + group) and is part of translation and camera groups
			numSpriteInstances += static_cast<uint32_t>(tile.labels[lod].size()) * 2; // each road label is part of translation and camera groups and is a sprite
		}

		begin->numSprites = numSprites;
		begin->numSpriteInstances = numSpriteInstances;

		if (begin->numSpriteInstances > 0 && begin->numSprites > 0)
		{
			begin->swapResources[swapIndex].renderer.construct();
			auto& renderer = *begin->swapResources[swapIndex].renderer;

			if (!renderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->framebuffer[0]->getRenderPass(), 0,
			                   _deviceResources->commandPool, _deviceResources->queue, false, false, false, numSpriteInstances, numSprites))
			{
				Log(LogLevel::Critical, "Cannot initialise UI Renderer\n");
				return false;
			}
			begin->swapResources[swapIndex].font = begin->swapResources[swapIndex].renderer->createFont(_deviceResources->fontImage, _deviceResources->fontTexture, _deviceResources->fontSampler);

			for (uint32_t lod = 0; lod < LOD::Count; ++lod)
			{
				for (uint32_t iconIndex = 0; iconIndex < tile.icons[lod].size(); iconIndex++)
				{
					for (uint32_t i = 0; i < BuildingType::None; ++i)
					{
						if (tile.icons[lod][iconIndex].buildingType == BuildingType::Shop + i)
						{
							begin->swapResources[swapIndex].spriteImages[i] = begin->swapResources[swapIndex].renderer->createImageFromAtlas(_deviceResources->imageAtlas,
							    _deviceResources->atlasOffsets[i]);
						}
					}
				}
			}
			for (auto it = begin + 1; it < end; ++it)
			{
				it->swapResources[swapIndex].font = begin->swapResources[swapIndex].font;
				it->swapResources[swapIndex].renderer = begin->swapResources[swapIndex].renderer;
				for (uint32_t lod = 0; lod < LOD::Count; ++lod)
				{
					for (uint32_t iconIndex = 0; iconIndex < tile.icons[lod].size(); iconIndex++)
					{
						for (uint32_t i = 0; i < BuildingType::None; ++i)
						{
							if (tile.icons[lod][iconIndex].buildingType == BuildingType::Shop + i)
							{
								it->swapResources[swapIndex].spriteImages[i] = begin->swapResources[swapIndex].spriteImages[i];
							}
						}
					}
				}
			}
		}
	}
	return true;
}

bool VulkanNavigation2D::createDescriptorSets()
{
	// In general it is a good idea for performance reasons to a) separate static data from dynamic data in layouts, and
	// b) separate the objects in frequency of update
	// STATIC UBO LAYOUT
	pvrvk::DescriptorSetLayoutCreateInfo staticUboLayoutDesc;
	staticUboLayoutDesc.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER, 1, VkShaderStageFlags::e_VERTEX_BIT);
	_deviceResources->uboMvp.layout = _deviceResources->device->createDescriptorSetLayout(staticUboLayoutDesc);

	if (!_deviceResources->uboMvp.layout.isValid())
	{
		Log(LogLevel::Critical, "Failed to create static UBO descriptor set.");
		return false;
	}

	// DYNAMIC UBO LAYOUT
	pvrvk::DescriptorSetLayoutCreateInfo dynamicUboLayoutDesc;
	dynamicUboLayoutDesc.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->uboDynamic.layout = _deviceResources->device->createDescriptorSetLayout(dynamicUboLayoutDesc);

	if (!_deviceResources->uboDynamic.layout.isValid())
	{
		Log(LogLevel::Critical, "Failed to create dynamic UBO descriptor set layout.");
		return false;
	}

	// create the pipeline layout
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboMvp.layout); // Set 0
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDynamic.layout); // Set 1
	_deviceResources->pipeLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	return true;
}

/*!*********************************************************************************************************************
\brief  Create static and dynamic UBOs. Static UBO used to hold transform matrix and is updated once per frame.
Dynamic UBO is used to hold color data for map elements and is only updated once during initialisation.
***********************************************************************************************************************/
bool VulkanNavigation2D::createUBOs()
{
	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo().configureBasic());
	// Static UBO params
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("transform", pvr::GpuDatatypes::mat4x4);

		_deviceResources->uboMvp.bufferView.initDynamic(desc, _numSwapchains, pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
		_deviceResources->uboMvp.buffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->uboMvp.bufferView.getSize(),
		                                  VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
	}

	pvrvk::WriteDescriptorSet writeDescSet[pvrvk::FrameworkCaps::MaxSwapChains + 1];
	for (uint32_t i = 0; i < _numSwapchains; ++i)
	{
		// Static buffer creation
		_deviceResources->uboMvp.sets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboMvp.layout);

		writeDescSet[i].set(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->uboMvp.sets[i], 0)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboMvp.buffer,
		               _deviceResources->uboMvp.bufferView.getDynamicSliceOffset(i), _deviceResources->uboMvp.bufferView.getDynamicSliceSize()));
	}

	// Dynamic UBO params
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("myColor", pvr::GpuDatatypes::vec4);

		_deviceResources->uboDynamic.bufferView.initDynamic(desc, static_cast<uint32_t>(MapColors::Total), pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));

		// Dynamic buffer creation
		_deviceResources->uboDynamic.buffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->uboDynamic.bufferView.getSize(),
		                                      VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

		_deviceResources->uboDynamic.sets[0] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboDynamic.layout);
		writeDescSet[_numSwapchains].set(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->uboDynamic.sets[0], 0)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboDynamic.buffer,
		               0, _deviceResources->uboDynamic.bufferView.getDynamicSliceSize()));
	}
	_deviceResources->device->updateDescriptorSets(writeDescSet, _numSwapchains + 1, nullptr, 0);

	return true;
}

/*!*********************************************************************************************************************
\return Return true if no error occurred, false if the sampler descriptor set is not valid.
\brief  Load a texture from file using PVR Asset Store, create a trilinear sampler, create a description set.
***********************************************************************************************************************/
bool VulkanNavigation2D::loadTexture(pvrvk::CommandBuffer& uploadCmd,
                                     std::vector<pvr::utils::ImageUploadResults>& outUploadResults)
{
	// Load font texture
	pvr::utils::ImageUploadResults upload = pvr::utils::loadAndUploadImage(_deviceResources->device, FontFile, true, uploadCmd, *this,
	                                        VkImageUsageFlags::e_SAMPLED_BIT, &_deviceResources->fontTexture);
	if (upload.getImageView().isNull() || upload.getImageView()->getImage().isNull())
	{
		return false;
	}
	_deviceResources->fontImage = upload.getImageView();
	outUploadResults.push_back(upload);

	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = VkFilter::e_LINEAR;
	samplerInfo.minFilter = VkFilter::e_LINEAR;
	samplerInfo.mipMapMode = VkSamplerMipmapMode::e_LINEAR;
	samplerInfo.wrapModeU = VkSamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.wrapModeV = VkSamplerAddressMode::e_CLAMP_TO_EDGE;

	_deviceResources->fontSampler = _deviceResources->device->createSampler(samplerInfo);

	// Load & generate texture atlas for icons.
	// load the textures from the disk
	pvrvk::Image images[ARRAY_SIZE(SpriteFileNames)];
	for (uint32_t i = 0; i < ARRAY_SIZE(SpriteFileNames); ++i)
	{
		upload = pvr::utils::loadAndUploadImage(_deviceResources->device, SpriteFileNames[i].c_str(),
		                                        true, uploadCmd, *this, VkImageUsageFlags::e_SAMPLED_BIT | VkImageUsageFlags::e_TRANSFER_SRC_BIT);

		if (upload.getImageView().isNull())
		{
			Log(LogLevel::Error, "failed to load texture %s", SpriteFileNames[i].c_str());
			return false;
		}
		outUploadResults.push_back(upload);
		images[i] =  upload.getImageView()->getImage();
	}

	if (!pvr::utils::generateTextureAtlas(_deviceResources->device, images, _deviceResources->atlasOffsets, ARRAY_SIZE(SpriteFileNames),
	                                      &_deviceResources->imageAtlas, nullptr, uploadCmd))
	{
		return false;
	}
	return true;
}

/*!*********************************************************************************************************************
\brief  Setup colors used for drawing the map. Fill dynamic UBO with data.
***********************************************************************************************************************/
void VulkanNavigation2D::setColors()
{
	// Set colors
	_clearColor = glm::vec4(0.6863f, 0.9333f, 0.9333f, 1.0f);

	_roadAreaColor = glm::vec4(0.9960f, 0.9960f, 0.9960f, 1.0f);
	// Roads
	_motorwayColor = glm::vec4(0.9098f, 0.5725f, 0.6352f, 1.0f);
	_trunkRoadColor = glm::vec4(0.9725f, 0.6980f, 0.6117f, 1.0f);
	_primaryRoadColor = glm::vec4(0.9882f, 0.8392f, 0.6431f, 1.0f);
	_secondaryRoadColor = glm::vec4(1.0f, 1.0f, 0.5019f, 1.0f);
	_serviceRoadColor = glm::vec4(0.996f, 0.996f, 0.996f, 1.0f);
	_otherRoadColor = glm::vec4(0.996f, 0.996f, 0.996f, 1.0f);

	_buildingColor = glm::vec4(1.0f, 0.7411f, 0.3568f, 1.0f);
	_parkingColor = glm::vec4(0.9412f, 0.902f, 0.549f, 1.0f);
	_outlineColor = glm::vec4(0.4392f, 0.5412f, 0.5647f, 1.0f);

	void* memory;
	_deviceResources->uboDynamic.buffer->getDeviceMemory()->map(&memory);
	_deviceResources->uboDynamic.bufferView.pointToMappedMemory(memory);

	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Clear)).setValue(_clearColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Building)).setValue(_buildingColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Motorway)).setValue(_motorwayColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Other)).setValue(_otherRoadColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Outline)).setValue(_outlineColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Parking)).setValue(_parkingColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Primary)).setValue(_primaryRoadColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::RoadArea)).setValue(_roadAreaColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Secondary)).setValue(_secondaryRoadColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Service)).setValue(_serviceRoadColor);
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, static_cast<uint32_t>(MapColors::Trunk)).setValue(_trunkRoadColor);

	_deviceResources->uboDynamic.buffer->getDeviceMemory()->unmap();
}

void VulkanNavigation2D::initRoute()
{
	convertRoute(_mapWorldDim, _numCols, _numRows, *_OSMdata, _weight, _rotation, _totalRouteDistance);
	if (_cameraMode == CameraMode::Auto)
	{
		// Initial weighting for first iteration of the animation
		_weight = _OSMdata->getRouteData()[0].distanceToNext / _totalRouteDistance;
		_keyFrameTime = 0.0f;
		_rotation = _OSMdata->getRouteData()[0].rotation;
	}
}

/*!*********************************************************************************************************************
\brief  Creates vertex and index buffers and records the secondary command buffers for each tile.
***********************************************************************************************************************/
void VulkanNavigation2D::createBuffers()
{
	for (uint32_t col = 0; col < _OSMdata->getTiles().size(); ++col)
	{
		auto& tileCol = _OSMdata->getTiles()[col];
		for (uint32_t row = 0; row < tileCol.size(); ++row)
		{
			Tile& tile = tileCol[row];

			// Create vertices for tile
			for (auto nodeIterator = tile.nodes.begin(); nodeIterator != tile.nodes.end(); ++nodeIterator)
			{
				nodeIterator->second.index = static_cast<uint32_t>(tile.vertices.size());

				Tile::VertexData vertData(glm::vec2(remap(nodeIterator->second.coords, _OSMdata->getTiles()[0][0].min,
				                                    _OSMdata->getTiles()[_numCols - 1][_numRows - 1].max, -_mapWorldDim * .5, _mapWorldDim * .5)),
				                          nodeIterator->second.texCoords);
				tile.vertices.push_back(vertData);
			}

			// Add car parking to indices
			uint32_t parkingNum = generateIndices(tile, tile.parkingWays);

			// Add buildings to indices
			uint32_t buildNum = generateIndices(tile, tile.buildWays);

			// Add inner ways to indices
			uint32_t innerNum = generateIndices(tile, tile.innerWays);

			// Add road area ways to indices
			uint32_t areaNum = generateIndices(tile, tile.areaWays);

			// Add roads to indices
			uint32_t serviceRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Service);
			uint32_t otherRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Other);
			uint32_t secondaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Secondary);
			uint32_t primaryRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Primary);
			uint32_t trunkRoadNum = generateIndices(tile, tile.roadWays, RoadTypes::Trunk);
			uint32_t motorwayNum = generateIndices(tile, tile.roadWays, RoadTypes::Motorway);

			if (tile.vertices.size())
			{
				auto& tileRes = _tileRenderingResources[col][row];
				// Create vertex and index buffers
				// Interleaved vertex buffer (vertex position + texCoord)
				tileRes.vbo = pvr::utils::createBuffer(_deviceResources->device,static_cast<uint32_t>(tile.vertices.size() * sizeof(tile.vertices[0])),
				              VkBufferUsageFlags::e_VERTEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT); // Vertex data
				tileRes.ibo = pvr::utils::createBuffer(_deviceResources->device,static_cast<uint32_t>(tile.indices.size() * sizeof(tile.indices[0])),
				              VkBufferUsageFlags::e_INDEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT); // indices

				pvr::utils::updateBuffer(_deviceResources->device, tileRes.vbo,
				                         tile.vertices.data(), 0, static_cast<uint32_t>(tile.vertices.size() * sizeof(tile.vertices[0])), true); // Vertex data
				pvr::utils::updateBuffer(_deviceResources->device, tileRes.ibo,
				                         tile.indices.data(), 0, static_cast<uint32_t>(tile.indices.size() * sizeof(tile.indices[0])), true); // indices
				uint32_t uboOffset = 0;

				// Secondary commands
				for (uint32_t i = 0; i < _numSwapchains; ++i)
				{
					uint32_t offset = 0;
					tileRes.swapResources[i].secCbo = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
					tileRes.swapResources[i].secCbo->begin(_deviceResources->framebuffer[i]);

					// Bind the vertex and index buffers for the tile
					tileRes.swapResources[i].secCbo->bindVertexBuffer(tileRes.vbo, 0, 0);
					tileRes.swapResources[i].secCbo->bindIndexBuffer(tileRes.ibo, 0, VkIndexType::e_UINT32);

					tileRes.swapResources[i].secCbo->bindPipeline(_deviceResources->fillPipe);
					tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
					    _deviceResources->fillPipe->getPipelineLayout(), SetBinding::UBOStatic, _deviceResources->uboMvp.sets[i]);

					// Draw the car parking
					if (parkingNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::Parking));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
						    _deviceResources->fillPipe->getPipelineLayout(), SetBinding::UBODynamic,
						    _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(0, parkingNum);
						offset += parkingNum;
					}

					// Draw the buildings
					if (buildNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::Building));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
						    _deviceResources->fillPipe->getPipelineLayout(), SetBinding::UBODynamic,
						    _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(offset, buildNum);
						offset += buildNum;
					}

					// Draw the insides of car parking and buildings for polygons with holes
					if (innerNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::Clear));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->fillPipe->getPipelineLayout(), SetBinding::UBODynamic, _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(offset, innerNum);
						offset += innerNum;
					}

					// Draw the road areas
					if (areaNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::RoadArea));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->fillPipe->getPipelineLayout(), SetBinding::UBODynamic, _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(offset, areaNum);
						offset += areaNum;
					}

					tileRes.swapResources[i].secCbo->bindPipeline(_deviceResources->roadPipe);
					tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->roadPipe->getPipelineLayout(), SetBinding::UBOStatic, _deviceResources->uboMvp.sets[i]);

					/**** Draw the roads ****/

					// Service Roads
					if (serviceRoadNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::Service));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->roadPipe->getPipelineLayout(), SetBinding::UBODynamic, _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(offset, serviceRoadNum);
						offset += serviceRoadNum;
					}
					// Other (any other roads)
					if (otherRoadNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::Other));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->roadPipe->getPipelineLayout(), SetBinding::UBODynamic, _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(offset, otherRoadNum);
						offset += otherRoadNum;
					}
					// Secondary Roads
					if (secondaryRoadNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::Secondary));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->roadPipe->getPipelineLayout(), SetBinding::UBODynamic, _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(offset, secondaryRoadNum);
						offset += secondaryRoadNum;
					}
					// Primary Roads
					if (primaryRoadNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::Primary));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->roadPipe->getPipelineLayout(), SetBinding::UBODynamic, _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(offset, primaryRoadNum);
						offset += primaryRoadNum;
					}
					// Trunk Roads
					if (trunkRoadNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::Trunk));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->roadPipe->getPipelineLayout(), SetBinding::UBODynamic, _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(offset, trunkRoadNum);
						offset += trunkRoadNum;
					}
					// Motorways
					if (motorwayNum > 0)
					{
						uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(static_cast<uint32_t>(MapColors::Motorway));
						tileRes.swapResources[i].secCbo->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->roadPipe->getPipelineLayout(), SetBinding::UBODynamic, _deviceResources->uboDynamic.sets[0], &uboOffset, 1);
						tileRes.swapResources[i].secCbo->drawIndexed(offset, motorwayNum);
						offset += motorwayNum;
					}

					tileRes.swapResources[i].secCbo->end();
				}
			}
		}
	}
}

/*!*********************************************************************************************************************
\brief  Update animation using pre-computed path for the camera to follow.
***********************************************************************************************************************/
void VulkanNavigation2D::updateAnimation(uint32_t swapindex)
{
	static const float scaleAnimTime = 350.0f;
	static const float rotationScaler = 50.0f;
	static const float scaleGracePeriod = 8000.0f;
	static const float baseSpeed = 18.f;
	static float r1 = 0.0f;
	static float r2 = 0.0f;

	float dt = float(getFrameTime());
	_timePassed += dt;
	if (_cameraMode == CameraMode::Auto)
	{
		if (!_turning)
		{
			if (_keyFrameTime > 0.0f)
			{
				// Interpolate between two positions.
				_translation = glm::mix(_OSMdata->getRouteData()[_routeIndex].point,
				                        _OSMdata->getRouteData()[_routeIndex + 1].point, _animTime / _keyFrameTime);
			}
			else
			{
				_translation = _OSMdata->getRouteData()[_routeIndex].point;
			}
			_animTime += dt / _scale;
		}
		if (_OSMdata->getRouteData().size() > 2)
		{
			if (_animTime >= _keyFrameTime)
			{
				_turning = true;
				if (_updateRotation)
				{
					r1 = _OSMdata->getRouteData()[_routeIndex].rotation;
					r2 = _OSMdata->getRouteData()[_routeIndex + 1].rotation;

					float angleDiff = glm::abs(r1 - r2);

					if (angleDiff > 180.0f)
					{
						if (r1 > r2)
						{
							r2 += 360.0f;
						}
						else
						{
							r2 -= 360.0f;
						}
					}

					float diff = (r2 > r1) ? r2 - r1 : r1 - r2;
					// Calculate the time to animate the _rotation based on angle.
					_rotateTime = glm::abs(rotationScaler * (diff / (2.0f * glm::pi<float>())));
					_updateRotation = false;
				}

				if (_rotateTime > dt)
				{
					_rotation = glm::mix(r1, r2, _rotateAnimTime / _rotateTime);
				}
				_rotateAnimTime += dt;

				if (_rotateAnimTime >= _rotateTime)
				{
					_rotation = r2;
					_updateRotation = true;
					_turning = false;
					_rotateAnimTime = 0.0f;
				}
			}

			if (_animTime >= _keyFrameTime && !_turning)
			{
				_animTime = 0.0f;

				// Iterate through the route
				if (++_routeIndex == _OSMdata->getRouteData().size() - 1)
				{
					_rotation = _OSMdata->getRouteData()[0].rotation;
					_routeIndex = 0;
				}

				// get new weighting for this part of the route.
				_weight = _OSMdata->getRouteData()[_routeIndex].distanceToNext / _totalRouteDistance;
				_keyFrameTime = (float(_OSMdata->getRouteData().size()) * baseSpeed * glm::sqrt(_totalRouteDistance)) * _weight;
			}
		}
		else
		{
			Log(LogLevel::Debug, "Could not find multiple routes in the map data");
		}
	}

	// Check for _scale changes
	if (_cameraMode == CameraMode::Manual)
	{
		_currentScaleLevel = LOD::L4;
		for (int32_t i = LOD::L4; i >= 0; --i)
		{
			if (_scale > scales[_currentScaleLevel])
			{
				_currentScaleLevel = i;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		if (_timePassed >= scaleGracePeriod)
		{
			_previousScaleLevel = _currentScaleLevel;
			if (_increaseScale)
			{
				if (++_currentScaleLevel == LOD::L4)
				{
					_increaseScale = false;
				}
			}
			else
			{
				if (--_currentScaleLevel == LOD::L1)
				{
					_increaseScale = true;
				}
			}

			_timePassed = 0.0f;
			_scaleChange = _previousScaleLevel != _currentScaleLevel;
		}

		if (_scaleChange)
		{
			if (_timePassed >= scaleAnimTime)
			{
				_scaleChange = false;
			}
			// interpolate
			_scale = glm::mix(routescales[_previousScaleLevel] * 1.5f, routescales[_currentScaleLevel] * 1.5f, _timePassed / scaleAnimTime);
		}
	}
}

bool skipAmenityLabel(AmenityLabelData& labelData, Label& label, glm::dvec3& extent)
{
	// Check if labels overlap.
	// Almost half extent (dividing by 1.95 to leave some padding between text) of the scaled text.
	float halfExtent_x = label.text->getScaledDimension().x / 1.95f;

	// Check if this and the previous text (in the same LOD level) overlap, if they do skip this text.
	float distance = (float)glm::distance(labelData.coords, glm::dvec2(extent));
	if (distance < (extent.z + halfExtent_x) && glm::abs(extent.z - halfExtent_x) < distance)
	{
		label.text.reset();
		return true;
	}

	// Update with fresh data - position (stored in x, y components) and half extent (stored in z component).
	extent = glm::vec3(labelData.coords, halfExtent_x);

	return false;
}

bool skipLabel(LabelData& labelData, Label& label, glm::dvec3& extent)
{
	// Check if labels overlap.
	// Almost half extent (dividing by 1.95 to leave some padding between text) of the scaled text.
	float halfExtent_x = label.text->getScaledDimension().x / 1.95f;

	// Check if this text crosses the tile boundary or the text overruns the end of the road segment.
	if (labelData.distToBoundary < halfExtent_x)
	{
		label.text.reset();
		return true;
	}

	// Check if the text overruns the end of the road segment.
	if (labelData.distToEndOfSegment < halfExtent_x)
	{
		label.text.reset();
		return true;
	}

	// Check if this and the previous text (in the same LOD level) overlap, if they do skip this text.
	float distance = (float)glm::distance(labelData.coords, glm::dvec2(extent));
	if (distance < (extent.z + halfExtent_x) && glm::abs(extent.z - halfExtent_x) < distance)
	{
		label.text.reset();
		return true;
	}

	// Update with fresh data - position (stored in x, y components) and half extent (stored in z component).
	extent = glm::vec3(labelData.coords, halfExtent_x);

	return false;
}

/*!*********************************************************************************************************************
\brief  Record the primary command buffer.
***********************************************************************************************************************/
void VulkanNavigation2D::createUIRendererItems()
{
	for (uint32_t col = 0; col < _numCols; col++)
	{
		for (uint32_t row = 0; row < _numRows; row++)
		{
			auto& tile = _OSMdata->getTiles()[col][row];
			initializeRenderers(&_tileRenderingResources[col][row], &_tileRenderingResources[col][std::min(row + 1, _numRows - 1)], tile);
		}
	}

	for (uint32_t swapIndex = 0; swapIndex < _deviceResources->swapchain->getSwapchainLength(); ++swapIndex)
	{
		for (uint32_t col = 0; col < _numCols; ++col)
		{
			for (uint32_t row = 0; row < _numRows; ++row)
			{
				auto& tile = _OSMdata->getTiles()[col][row];
				auto& tileRes = _tileRenderingResources[col][row];
				for (uint32_t lod = 0; lod < LOD::Count; ++lod)
				{
					glm::dvec3 extent(0, 0, 0);
					if (!tile.icons[lod].empty() || !tile.labels[lod].empty() || !tile.amenityLabels[lod].empty())
					{
						tileRes.swapResources[swapIndex].tileGroup[lod] = tileRes.swapResources[swapIndex].renderer->createPixelGroup();
						auto& group = tileRes.swapResources[swapIndex].tileGroup[lod];
						auto& camGroup = tileRes.swapResources[swapIndex].cameraRotateGroup[lod] = tileRes.swapResources[swapIndex].renderer->createPixelGroup();
						group->setAnchor(pvr::ui::Anchor::Center, 0.f, 0.f);

						for (auto && icon : tile.icons[lod])
						{
							// create the amenity group
							tileRes.swapResources[swapIndex].amenityIcons[lod].push_back(AmenityIconGroup());
							auto& tileResAmenityIcon = tileRes.swapResources[swapIndex].amenityIcons[lod].back();

							tileResAmenityIcon.iconData = icon;
							tileResAmenityIcon.group = tileRes.swapResources[swapIndex].renderer->createPixelGroup();

							// create the image - or at least take a copy that we'll work with from now on
							tileResAmenityIcon.icon.image = tileRes.swapResources[swapIndex].spriteImages[icon.buildingType];
							tileResAmenityIcon.icon.image->setAnchor(pvr::ui::Anchor::Center, 0.f, 0.f);

							// flip the icon
							tileResAmenityIcon.icon.image->setRotation(glm::pi<float>());
							tileResAmenityIcon.icon.image->commitUpdates();

							// add the amenity icon to the group
							tileResAmenityIcon.group->add(tileResAmenityIcon.icon.image);
							tileResAmenityIcon.group->setAnchor(pvr::ui::Anchor::Center, 0.f, 0.f);
							tileResAmenityIcon.group->commitUpdates();

							group->add(tileResAmenityIcon.group);
						}

						for (auto && amenityLabel : tile.amenityLabels[lod])
						{
							tileRes.swapResources[swapIndex].amenityLabels[lod].push_back(AmenityLabelGroup());
							auto& tileResAmenityLabel = tileRes.swapResources[swapIndex].amenityLabels[lod].back();

							tileResAmenityLabel.iconData = amenityLabel.iconData;

							tileResAmenityLabel.group = tileRes.swapResources[swapIndex].renderer->createPixelGroup();

							tileResAmenityLabel.label.text = tileRes.swapResources[swapIndex].renderer->createText(amenityLabel.name, tileRes.swapResources[swapIndex].font);
							debug_assertion(tileResAmenityLabel.label.text.isValid(), "Amenity label must be a valid UIRenderer Text Element");
							tileResAmenityLabel.label.text->setColor(0.f, 0.f, 0.f, 1.f);
							tileResAmenityLabel.label.text->setAlphaRenderingMode(true);

							float txtScale = 1.0f / (scales[lod + 1] * 12.0f);

							tileResAmenityLabel.label.text->setScale(txtScale, txtScale);
							tileResAmenityLabel.label.text->setPixelOffset(-glm::abs(tileResAmenityLabel.iconData.coords - amenityLabel.coords));
							tileResAmenityLabel.label.text->commitUpdates();

							if (skipAmenityLabel(amenityLabel, tileResAmenityLabel.label, extent))
							{
								continue;
							}

							// add the label to its corresponding amenity group
							tileResAmenityLabel.group->add(tileResAmenityLabel.label.text);
							tileResAmenityLabel.group->commitUpdates();

							group->add(tileResAmenityLabel.group);
						}

						for (auto && label : tile.labels[lod])
						{
							tileRes.swapResources[swapIndex].labels[lod].push_back(Label());
							auto& tileResLabel = tileRes.swapResources[swapIndex].labels[lod].back();

							tileResLabel.text = tileRes.swapResources[swapIndex].renderer->createText(label.name, tileRes.swapResources[swapIndex].font);
							debug_assertion(tileResLabel.text.isValid(), "Label must be a valid UIRenderer Text Element");

							tileResLabel.text->setColor(0.f, 0.f, 0.f, 1.f);
							tileResLabel.text->setAlphaRenderingMode(true);

							float txtScale = label.scale * 2.0f;

							tileResLabel.text->setScale(txtScale, txtScale);
							tileResLabel.text->setPixelOffset(label.coords);
							tileResLabel.text->commitUpdates();

							if (skipLabel(label, tileResLabel, extent))
							{
								continue;
							}

							group->add(tileResLabel.text);
						}

						group->commitUpdates();
						camGroup->add(group);
						camGroup->commitUpdates();

						auto cb = getOrCreateTileUiCommandBuffer(tileRes, swapIndex, lod);
						tileRes.swapResources[swapIndex].renderer->beginRendering(cb);
						camGroup->render();
						tileRes.swapResources[swapIndex].renderer->endRendering();
					}
				}
			}
		}
	}
}

void VulkanNavigation2D::recordUiRendererCommandBuffer(uint32_t swapchainIndex)
{
	_deviceResources->uiRendererCmdBuffers[swapchainIndex]->begin(_deviceResources->framebuffer[swapchainIndex], 0,
	    VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	_deviceResources->uiRenderer.beginRendering(_deviceResources->uiRendererCmdBuffers[swapchainIndex]);
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->uiRendererCmdBuffers[swapchainIndex]->end();
}

static std::vector<TileRenderingResources*> renderqueue;

/*!*********************************************************************************************************************
\brief  Find the tiles that need to be rendered.
***********************************************************************************************************************/
void VulkanNavigation2D::updateCommandBuffer(pvrvk::CommandBuffer& cbo, uint32_t swapchainIndex)
{
	renderqueue.clear();

	static uint16_t prevLod[pvrvk::FrameworkCaps::MaxSwapChains] = { LOD::Count };

	bool hasChanges = false;

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		if (_currentScaleLevel != prevLod[i])
		{
			hasChanges = true;
			prevLod[swapchainIndex] = _currentScaleLevel;
		}
	}

	if (_uiRendererChanged[swapchainIndex])
	{
		recordUiRendererCommandBuffer(swapchainIndex);
	}

	for (uint32_t i = 0; i < _numCols; ++i)
	{
		for (uint32_t j = 0; j < _numRows; ++j)
		{
			auto& tile = _tileRenderingResources[i][j];
			if (inFrustum(_OSMdata->getTiles()[i][j].screenMin, _OSMdata->getTiles()[i][j].screenMax))
			{
				if (!tile.swapResources[swapchainIndex].tileWasVisible)
				{
					hasChanges = true;
					tile.swapResources[swapchainIndex].tileWasVisible = true;
				}

				// Add the current tile to the list of tiles to render
				renderqueue.push_back(&tile);

				// Update text elements
				updateLabels(i, j, swapchainIndex);

				// Update icons (points of interest)
				updateAmenities(i, j, swapchainIndex);

				// Update groups
				updateGroups(i, j, swapchainIndex);
			}
			else
			{
				if (tile.swapResources[swapchainIndex].tileWasVisible)
				{
					hasChanges = true;
					tile.swapResources[swapchainIndex].tileWasVisible = false;
				}
			}
		}
	}

	if (hasChanges || _uiRendererChanged[swapchainIndex])
	{
		const pvrvk::ClearValue clearValues[] =
		{
			pvrvk::ClearValue(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a),
			pvrvk::ClearValue::createDefaultDepthStencilClearValue()
		};

		_deviceResources->commandBuffers[swapchainIndex]->begin();
		_deviceResources->commandBuffers[swapchainIndex]->beginRenderPass(_deviceResources->framebuffer[swapchainIndex],
		    pvrvk::Rect2Di(0, 0, getWidth(), getHeight()), false, clearValues, ARRAY_SIZE(clearValues));

		for (auto && tile : renderqueue)
		{
			if (tile->swapResources[swapchainIndex].secCbo.isValid())
			{
				cbo->executeCommands(tile->swapResources[swapchainIndex].secCbo);
			}

			for (uint16_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
			{
				if (tile->swapResources[swapchainIndex].uicbuff[lod].isValid())
				{
					cbo->executeCommands(tile->swapResources[swapchainIndex].uicbuff[lod]);
				}
			}
		}

		_deviceResources->commandBuffers[swapchainIndex]->executeCommands(_deviceResources->uiRendererCmdBuffers[swapchainIndex]);

		_deviceResources->commandBuffers[swapchainIndex]->endRenderPass();
		_deviceResources->commandBuffers[swapchainIndex]->end();

		_uiRendererChanged[swapchainIndex] = false;
	}
}

/*!*********************************************************************************************************************
\brief  Capture frustum planes from the current View Projection matrix
***********************************************************************************************************************/
void VulkanNavigation2D::calculateClipPlanes()
{
	glm::vec4 rowX = glm::vec4(_mapMVPMtx[0][0], _mapMVPMtx[1][0], _mapMVPMtx[2][0], _mapMVPMtx[3][0]);
	glm::vec4 rowY = glm::vec4(_mapMVPMtx[0][1], _mapMVPMtx[1][1], _mapMVPMtx[2][1], _mapMVPMtx[3][1]);
	glm::vec4 rowW = glm::vec4(_mapMVPMtx[0][3], _mapMVPMtx[1][3], _mapMVPMtx[2][3], _mapMVPMtx[3][3]);

	_clipPlanes[0] = Plane(rowW - rowX); // Right
	_clipPlanes[1] = Plane(rowW + rowX); // Left
	_clipPlanes[2] = Plane(rowW - rowY); // Top
	_clipPlanes[3] = Plane(rowW + rowY); // Bottom
}

/*!*********************************************************************************************************************
\param min The minimum co-ordinates of the bounding box.
\param max The maximum co-ordinates of the bounding box.
\return boolean True if inside the view frustum, false if outside.
\brief  Tests whether a 2D bounding box is intersected or enclosed by a view frustum.
Only the near, far, left and right planes of the view frustum are taken into consideration to optimize the intersection test.
***********************************************************************************************************************/
bool VulkanNavigation2D::inFrustum(glm::vec2 min, glm::vec2 max)
{
	//Test the axis-aligned bounding box against each frustum plane,
	//cull if all points are outside of one the view frustum planes.
	for (uint32_t i = 0; i < _clipPlanes.size(); ++i)
	{
		uint32_t pointsOut = 0;

		// Test the points against the plane
		if ((_clipPlanes[i].normal.x * min.x - _clipPlanes[i].normal.y * min.y + _clipPlanes[i].distance) < 0.0f)
		{
			pointsOut++;
		}
		if ((_clipPlanes[i].normal.x * max.x - _clipPlanes[i].normal.y * min.y + _clipPlanes[i].distance) < 0.0f)
		{
			pointsOut++;
		}
		if ((_clipPlanes[i].normal.x * max.x - _clipPlanes[i].normal.y * max.y + _clipPlanes[i].distance) < 0.0f)
		{
			pointsOut++;
		}
		if ((_clipPlanes[i].normal.x * min.x - _clipPlanes[i].normal.y * max.y + _clipPlanes[i].distance) < 0.0f)
		{
			pointsOut++;
		}

		//If all four corners are outside of the plane then it is not visible.
		if (pointsOut == 4)
		{
			return false;
		}
	}
	return true;
}

void VulkanNavigation2D::updateGroups(uint32_t col, uint32_t row, uint32_t swapindex)
{
	const glm::vec2 pixelOffset = _translation * _scale;
	TileRenderingResources& tileRes = _tileRenderingResources[col][row];

	for (uint32_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
	{
		if (tileRes.swapResources[swapindex].tileGroup[lod].isValid())
		{
			tileRes.swapResources[swapindex].tileGroup[lod]->setAnchor(pvr::ui::Anchor::Center, 0, 0);
			tileRes.swapResources[swapindex].tileGroup[lod]->setPixelOffset(pixelOffset.x, pixelOffset.y);
			tileRes.swapResources[swapindex].tileGroup[lod]->setScale(_scale, _scale);
			tileRes.swapResources[swapindex].tileGroup[lod]->commitUpdates();
		}
		if (tileRes.swapResources[swapindex].cameraRotateGroup[lod].isValid())
		{
			tileRes.swapResources[swapindex].cameraRotateGroup[lod]->setRotation(glm::radians(_rotation));
			tileRes.swapResources[swapindex].cameraRotateGroup[lod]->setAnchor(pvr::ui::Anchor::Center, 0, 0);
			tileRes.swapResources[swapindex].cameraRotateGroup[lod]->commitUpdates();
		}
	}
}

/*!*********************************************************************************************************************
\param col  Column index for tile.
\param row  Row index for tile.
\brief Update the renderable text (dependant on LOD level) using the pre-processed data (position, _scale, _rotation, std::string) and UIRenderer.
***********************************************************************************************************************/
void VulkanNavigation2D::updateLabels(uint32_t col, uint32_t row, uint32_t swapchainIndex)
{
	Tile& tile = _OSMdata->getTiles()[col][row];
	TileRenderingResources& tileRes = _tileRenderingResources[col][row];

	for (uint32_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
	{
		for (uint32_t labelIdx = 0; labelIdx < tile.labels[lod].size(); ++labelIdx)
		{
			auto& tileResLabelLod = tileRes.swapResources[swapchainIndex].labels[lod];

			if (tileResLabelLod.empty())
			{
				continue;
			}

			auto& tileLabel = tile.labels[lod][labelIdx];
			auto& tileResLabel = tileRes.swapResources[swapchainIndex].labels[lod][labelIdx];
			if (tileResLabel.text.isNull())
			{
				continue;
			}

			glm::dvec2 offset(0, 0);

			float txtScale = tileLabel.scale * 2.0f;

			// Make sure road text is displayed upright (between 90 deg and -90 deg), otherwise flip it.
			float total_angle = tileLabel.rotation + _rotation; // Use that to calculate if the text is upright
			float angle = tileLabel.rotation;

			// check whether the label needs flipping
			// we add a small buffer onto the total angles to reduce the chance of parts of roads being flipped whilst other parts are not
			if (total_angle + 0.2f < -glm::degrees(glm::half_pi<float>()))
			{
				angle += glm::degrees(glm::pi<float>());
			}
			else if (total_angle - 0.2f > glm::degrees(glm::half_pi<float>()))
			{
				angle -= glm::degrees(glm::pi<float>());
			}

			float aabbHeight = tileResLabel.text->getBoundingBox().getSize().y;

			offset.y += tileLabel.scale * aabbHeight * 0.6f; // CENTRE THE TEXT ON THE ROAD

			// rotate the label to align with the road rotation
			tileResLabel.text->setRotation(glm::radians(angle));
			tileResLabel.text->setScale(txtScale, txtScale);
			tileResLabel.text->commitUpdates();
		}
	}
}

/*!*********************************************************************************************************************
\param col  Column index for tile.
\param row  Row index for tile.
\brief Update renderable icon, dependant on LOD level (for buildings such as; cafe, pub, library etc.) using the pre-processed data (position, type) and UIRenderer.
***********************************************************************************************************************/
void VulkanNavigation2D::updateAmenities(uint32_t col, uint32_t row, uint32_t swapchainIndex)
{
	TileRenderingResources& tileRes = _tileRenderingResources[col][row];

	for (uint32_t lod = _currentScaleLevel; lod < LOD::Count; ++lod)
	{
		for (uint32_t amenityIconIndex = 0; amenityIconIndex < tileRes.swapResources[swapchainIndex].amenityIcons[lod].size(); ++amenityIconIndex)
		{
			AmenityIconGroup& amenityIcon = tileRes.swapResources[swapchainIndex].amenityIcons[lod][amenityIconIndex];
			debug_assertion(amenityIcon.icon.image.isValid(), "Amenity Icon must be a valid UIRenderer Icon");

			float iconScale = (1.0f / (_scale * 20.0f));
			iconScale = glm::clamp(iconScale, amenityIcon.iconData.scale, amenityIcon.iconData.scale * 2.0f);

			amenityIcon.icon.image->setScale(glm::vec2(iconScale, iconScale));
			amenityIcon.icon.image->commitUpdates();

			// reverse the rotation applied by the camera rotation group
			amenityIcon.group->setRotation(glm::radians(-_rotation));
			amenityIcon.group->setPixelOffset(static_cast<float>(amenityIcon.iconData.coords.x), static_cast<float>(amenityIcon.iconData.coords.y));
			amenityIcon.group->commitUpdates();
		}

		for (uint32_t amenityLabelIndex = 0; amenityLabelIndex < tileRes.swapResources[swapchainIndex].amenityLabels[lod].size(); ++amenityLabelIndex)
		{
			AmenityLabelGroup& amenityLabel = tileRes.swapResources[swapchainIndex].amenityLabels[lod][amenityLabelIndex];
			if (amenityLabel.label.text.isNull())
			{
				continue;
			}

			float txtScale = 1.0f / (_scale * 15.0f);

			amenityLabel.label.text->setScale(txtScale, txtScale);
			// move the label below the icon based on the size of the label
			amenityLabel.label.text->setPixelOffset(0.0f, -2.2f * amenityLabel.label.text->getBoundingBox().getHalfExtent().y * txtScale);
			amenityLabel.label.text->commitUpdates();

			// reverse the rotation applied by the camera rotation group
			amenityLabel.group->setRotation(glm::radians(-_rotation));
			amenityLabel.group->setPixelOffset(static_cast<float>(amenityLabel.iconData.coords.x), static_cast<float>(amenityLabel.iconData.coords.y));
			amenityLabel.group->commitUpdates();
		}
	}
}
/*!*********************************************************************************************************************
\return auto ptr of the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its PVRShell object defining
the behaviour of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanNavigation2D()); }
