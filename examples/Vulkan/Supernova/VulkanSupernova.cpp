/*!
\brief Shows how to use thre PVRSuperResolution library for the sharpening and upscaling tehcniques.
\file VulkanSupernova.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRCore/cameras/TPSCamera.h"
#include "PVRAssets/fileio/GltfReader.h"
#include "PVRSuperResolution/SuperResolution.h"

namespace SceneElements {

/// <summary>Scene element rotation value.</summary>
const float RotateY = glm::pi<float>() / 150.0f;

/// <summary>Scene light direction.</summary>
const glm::vec4 LightDir(0.24f, 0.685f, -0.685f, 0.0f);

/// <summary>Helper value for the scene element rotation.</summary>
float angleY = 0.0f;

/// <summary>POD scene file.</summary>
const char SceneFile[] = "Satyr.pod";

/// <summary>Diffuse texture file.</summary>
std::string StatueTexFile = "Marble";

/// <summary>Normal texture file.</summary>
std::string StatueNormalMapTexFile = "MarbleNormalMap";

/// <summary>Camera "view from" parameter (constant in the scene).</summary>
glm::vec3 _cameraFrom;

/// <summary>Camera "view to" parameter (constant in the scene).</summary>
glm::vec3 _cameraTo;

/// <summary>Camera up direction parameter (constant in the scene).</summary>
glm::vec3 _cameraUp;

/// <summary>Camera field of view parameter (constant in the scene).</summary>
float _cameraFov;

/// <summary>Camera look at matrix (constant in the scene).</summary>
glm::mat4 _cameraLookAt;
} // namespace SceneElements

/// <summary>Antialiasing techniques.</summary>
namespace UIText {
const char NoAntialiasing[] = "No Anti Aliasing";
const char SupernovaV1Mode1X[] = "Supernova V1 Mode 1X";
const char SupernovaV1Mode2X[] = "Supernova V1 Mode 2X";
const char MentisSpatialUpscaler[] = "Mentis spatial upscaler";
} // namespace UIText

/// <summary>Shader Source Files.</summary>
namespace ShaderFiles {
const char AttributelessVertexShaderFile[] = "AttributelessVertexShader.vsh.spv";
const char VertexShaderFile[] = "VertShader.vsh.spv";
const char FragmentShaderFile[] = "FragShader.fsh.spv";
const char ImageOutputFragShader[] = "ImageOutputFragShader.fsh.spv";
} // namespace ShaderFiles

/// <summary>Shader Source Files.</summary>
namespace BufferEntryNames {
const char* const MVPMatrix = "MVPMatrix";
const char* const LightDirModel = "LightDirModel";
} // namespace BufferEntryNames

/// <summary>Vertex Attribute Bindings.</summary>
const pvr::utils::VertexBindings VertexAttribBindings[] = {
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
	{ "TANGENT", 3 },
};

/// <summary>Selected Anti Aliasing techniques.</summary>
enum class SupernovaTechnique
{
	SUPERNOVA_V1_MODE_1X = 0,
	SUPERNOVA_V1_MODE_2X,

	SUPERNOVA_SIZE
};

/// <summary>Values are used for calculation in vertex shaders of no antialiasing, FXAA and MSAA.</summary>
struct SceneInformationBuffer
{
	/// <summary>Model view projection matrix.</summary>
	glm::mat4 modelViewProjectionMatrix;

	/// <summary>Light direction.</summary>
	glm::vec3 lightDirModel;
};

struct DeviceResources
{
	/// <summary>Encapsulation of a Vulkan instance.</summary>
	pvrvk::Instance instance;

	/// <summary>Callbacks and messengers for debug messages.</summary>
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;

	/// <summary>Encapsulation of a Vulkan logical device.</summary>
	pvrvk::Device device;

	/// <summary>Queue where to submit commands.</summary>
	pvrvk::Queue graphicsQueue;

	/// <summary>Descriptor pool to allocate the descriptor sets.</summary>
	pvrvk::DescriptorPool descriptorPool;

	/// <summary>Command ppol to allocate command buffers.</summary>
	pvrvk::CommandPool commandPool;

	/// <summary>Encapsulation of a Vulkan swapchain.</summary>
	pvrvk::Swapchain swapchain;

	/// <summary>vma allocator, only used to build the swapchain.</summary>
	pvr::utils::vma::Allocator vmaAllocator;

	/// <summary>Pipeline cache used to build the pipelines.</summary>
	pvrvk::PipelineCache pipelineCache;

	/// <summary>Trilinear sampler used for the offscreen scene rendering pass.</summary>
	pvrvk::Sampler samplerTrilinear;

	/// <summary>Trilinear sampler used for the image output mode (when _isImageOutputMode is true).</summary>
	pvrvk::Sampler samplerNearest;

	/// <summary>Helper command buffer used for initial resource loading.</summary>
	pvrvk::CommandBuffer utilityCommandBuffer;

	/// <summary>UIRenderer used to display text.</summary>
	pvr::ui::UIRenderer arrayUIRenderer[static_cast<int>(SupernovaTechnique::SUPERNOVA_SIZE)];

	/// <summary>Albedo image view for the scene model.</summary>
	pvrvk::ImageView albedoImageView;

	/// <summary>Normal image view for the scene model.</summary>
	pvrvk::ImageView normalMapImageView;

	/// <summary>Vector with the semaphores used at the beginning of each frame to acquire the next swap chain image index (so there is one element in this vector per swap chain image).</summary>
	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;

	/// <summary>Vector with the semaphores used to signal the command buffer with all the commands for the current frame (one element in this vector per swap chain image).</summary>
	std::vector<pvrvk::Semaphore> graphicsSemaphores;

	/// <summary>Vector with the fences used in the command buffer submit done each frame which contains all the commands for the current frame. It is waited for at the beginning of each frame, after after the next swap chain image is acquired with _imageAcquiredSemaphores (one element in this vector per swap chain image).</summary>
	std::vector<pvrvk::Fence> perFrameResourcesFences;

	/// <summary>Vector with the fences used in the command buffer submit done each frame for the Mentis upscaler, when a blit is performed from the upscaled image in offscreenDepthAttachmentImageViewFullSize to the swapchain image.</summary>
	std::vector<pvrvk::Fence> vectorMentisBlitCommandBufferFence;

	/// <summary>Vertex buffer object with the scene element geometry.</summary>
	std::vector<pvrvk::Buffer> sceneVertexBuffer;

	/// <summary>Index buffer object for the scene element geometry.</summary>
	std::vector<pvrvk::Buffer> sceneIndexBuffer;

	/// <summary>Descriptor set layout for the vertex shader (uniform buffer object information) in render passes where the scene is rendered.</summary>
	pvrvk::DescriptorSetLayout sceneVertexDescriptorSetLayout;

	/// <summary>Descriptor set layout for the fragment shader (textures to sample from) in render passes where the scene is rendered.</summary>
	pvrvk::DescriptorSetLayout sceneFragmentDescriptorSetLayout;

	/// <summary>Descriptor sets for render passes where the scene is rendered.</summary>
	std::vector<pvrvk::DescriptorSet> sceneFragmentDescriptorSets;

	/// <summary>Descriptor sets for render passes where the scene is rendered.</summary>
	std::vector<pvrvk::DescriptorSet> sceneVertexDescriptorSets;

	/// <summary>Pipeline layout used in the render passes where the scene is rendered.</summary>
	pvrvk::PipelineLayout scenePipelineLayout;

	/// <summary>Buffer used by the structured buffer view.</summary>
	pvrvk::Buffer sceneUniformBuffer;

	/// <summary>Structured buffer view for the uniform buffer values used in the render passes where the scene is rendered.</summary>
	pvr::utils::StructuredBufferView sceneStructuredBufferView;

	/// <summary>Vector of depth images from the swapchain.</summary>
	std::vector<pvrvk::ImageView> depthImages;

	/// <summary>Vector with the color images used as color attachment in the offscreenFramebufferFullSize framebuffer, to draw the scene offscreen at the same size as the swap chain images.</summary>
	std::vector<pvrvk::Image> offscreenColorAttachmentImageFullSize;

	/// <summary>Vector with the color image views used as color attachment in the offscreenFramebufferFullSize framebuffer, to draw the scene offscreen at the same size as the swap chain images.</summary>
	std::vector<pvrvk::ImageView> offscreenColorAttachmentImageViewFullSize;

	/// <summary>Vector with the depth images used as color attachment in the offscreenFramebufferFullSize framebuffer, to draw the scene offscreen at the same size as the swap chain images.</summary>
	std::vector<pvrvk::Image> offscreenDepthAttachmentImageFullSize;

	/// <summary>Vector with the depth image views used as color attachment in the offscreenFramebufferFullSize framebuffer, to draw the scene offscreen at the same size as the swap chain images.</summary>
	std::vector<pvrvk::ImageView> offscreenDepthAttachmentImageViewFullSize;

	/// <summary>Vector with the color images used as color attachment in the offscreenFramebufferHalfSize framebuffer, to draw the scene offscreen at half the size of the swap chain images.</summary>
	std::vector<pvrvk::Image> offscreenColorAttachmentImageHalfSize;

	/// <summary>Vector with the color images used as color attachment in the offscreenFramebufferHalfSize framebuffer, to draw the scene offscreen at half the size of the swap chain images.</summary>
	std::vector<pvrvk::ImageView> offscreenColorAttachmentImageViewHalfSize;

	/// <summary>Vector with the color images used as color attachment in the offscreenFramebufferHalfSize framebuffer, to draw the scene offscreen at half the size of the swap chain images.</summary>
	std::vector<pvrvk::Image> offscreenDepthAttachmentImageHalfSize;

	/// <summary>Vector with the color images used as color attachment in the offscreenFramebufferHalfSize framebuffer, to draw the scene offscreen at half the size of the swap chain images.</summary>
	std::vector<pvrvk::ImageView> offscreenDepthAttachmentImageViewHalfSize;

	/// <summary>Framebuffer to render the scene geometry to an offscreen color attachment with same size as the swapchain images.</summary>
	std::vector<pvrvk::Framebuffer> offscreenFramebufferFullSize;

	/// <summary>Framebuffer to render the scene geometry to an offscreen color attachment with half the size as the swapchain images.</summary>
	std::vector<pvrvk::Framebuffer> offscreenFramebufferHalfSize;

	/// <summary>Render pass used when rendering the scene geometry offscreen.</summary>
	pvrvk::RenderPass offScreenGeometryRenderPass;

	/// <summary>Graphics pipeline used when rendering the scene geometry to an offscreen color attachment with same size as the swapchain images.</summary>
	pvrvk::GraphicsPipeline offscreenPipelineFullSize;

	/// <summary>Graphics pipeline used when rendering the scene geometry to an offscreen color attachment with half the size of the swapchain images.</summary>
	pvrvk::GraphicsPipeline offscreenPipelineHalfSize;

	/// <summary>Command buffer where to record the PVRSuperResolution library changes for the sharpening technique.</summary>
	std::vector<pvrvk::CommandBuffer> supernovaV1Mode1XCommandBuffer;

	/// <summary>Command buffer where to record the PVRSuperResolution library changes for the upscaling technique.</summary>
	std::vector<pvrvk::CommandBuffer> supernovaV1Mode2XCommandBuffer;
	
	/// <summary>PVRSuperResolution instance to implement the sharpener technique.</summary>
	pvr::SuperResolution* supernovaV1Mode1X = nullptr;

	/// <summary>PVRSuperResolution instance to implement the spatial upscaler technique.</summary>
	pvr::SuperResolution* supernovaV1Mode2X = nullptr;

	~DeviceResources()
	{
		if (device) { device->waitIdle(); }
		uint32_t l = swapchain->getSwapchainLength();
		for (uint32_t i = 0; i < l; ++i)
		{
			if (perFrameResourcesFences[i]) { perFrameResourcesFences[i]->wait(); }
		}

		for (uint32_t i = 0; i < l; ++i)
		{
			if (vectorMentisBlitCommandBufferFence[i]) { vectorMentisBlitCommandBufferFence[i]->wait(); }
		}

		if (supernovaV1Mode1X != nullptr)
		{
			delete supernovaV1Mode1X;
			supernovaV1Mode1X = nullptr;
		}

		if (supernovaV1Mode2X != nullptr)
		{
			delete supernovaV1Mode2X;
			supernovaV1Mode2X = nullptr;
		}
	}

	/// <summary>Texture with the image provided as command parameter in case _isTextureOutputMode is true.</summary>
	pvrvk::ImageView _textureModeImageView = VK_NULL_HANDLE;
};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanSupernova : public pvr::Shell
{
public:
	/// <summary>Default constructor.</summary>
	VulkanSupernova() {}

	/// <summary>This event represents application start. When implementing, return a suitable error code to signify failure. If pvr::Result::Success
	/// is not returned, the Shell will detect that, clean up, and exit. It will be fired once, on start, before any other callback and before
	/// Graphics Context aquisition.It is suitable to do per - run initialisation, load assets files and similar tasks. A context does not exist yet,
	/// hence if the user tries to create API objects, they will fail and the behaviour is undefined.</summary>
	/// <returns>When implementing, return a suitable error code to signify failure. If pvr::Result::Success is not
	/// returned, the Shell will detect that, clean up, and exit.</returns>
	virtual pvr::Result initApplication();

	/// <summary> If pvr::Result::Success is not returned, the Shell will detect that, clean up, and exit. This function will be fired once after
	/// every time the main Graphics Context (the one the Application Window is using) is initialized. This is usually once per application run,
	/// but in some cases (context lost) it may be called more than once. If the context is lost, the releaseView() callback will be fired,
	/// and if it is reaquired this function will be called again. This callback is suitable to do all do-once tasks that require a graphics context,
	/// such as creating an On-Screen Framebuffer, and for simple applications creating the graphics objects.</summary>
	/// <returns>Return a suitable error code to signify failure. If pvr::Result::Success is not
	/// returned , the Shell will detect that, clean up, and exit.</returns>
	virtual pvr::Result initView();

	/// <summary>This function will be fired once before the main Graphics Context is lost. The user should use this callback as his main callback to
	/// release all API objects as they will be invalid afterwards. In simple applications where all objects are created in initView, it should release
	/// all objects acquired in initView. This callback will be called when the application is exiting, but not only then - losing (and later re-acquiring)
	/// the Graphics Context will lead to this callback being fired, followed by an initView callback, renderFrame etc.</summary>
	/// <returns>Return a suitable error code to signify failure. If pvr::Result::Success is not
	/// returned, the Shell will detect that, clean up, and exit. If the shell was exiting, this will happen anyway.</returns>
	virtual pvr::Result releaseView();

	/// <summary>This function will be fired once before the application exits, after the Graphics Context is torn down. The user should use this
	/// callback as his main callback to release all objects that need to. The application will exit shortly after this callback is fired.
	/// In effect, the user should release all objects that were acquired during initApplication. Do NOT use this to release API objects - these
	/// should already have been released during releaseView.</summary>
	/// <returns>Return a suitable error code to signify a failure that will be logged.</returns>
	virtual pvr::Result quitApplication();

	/// <summary>This function will be fired once every frame. The user should use this callback as his main callback to start
	/// rendering and per-frame code. This callback is suitable to do all per-frame task. In multithreaded environments, it
	/// should be used to mark the start and signal the end of frames.</summary>
	/// <returns>Return a suitable error code to signify failure. Return pvr::Success to signify
	/// success and allow the Shell to do all actions necessary to render the frame (swap buffers etc.). If
	/// pvr::Result::Success is not returned, the Shell will detect that, clean up, and exit. Return
	/// pvr::Result::ExitRenderFrame to signify a clean, non-error exit for the application. Any other error code will
	/// be logged.</returns>
	virtual pvr::Result renderFrame();

	/// <summary>Override of Shell::eventMappedInput. This event abstracts, maps and unifies several input devices.</summary>
	/// <param name="key">The Simplified Unified Event</param>
	virtual void eventMappedInput(pvr::SimplifiedInput key);

	/// <summary>Utility function to change the current anti aliasing technique selected.</summary>
	void changeCurrentTechnique();

	/// <summary>Build the images and framebuffers that are required by all the techniques (MSAA, FXAA, TAA).</summary>
	void createImagesAndFramebuffers();

	/// <summary>Build the different graphics pipelines used by all the techniques (MSAA, FXAA, TAA).</summary>
	void createGraphicsPipelines();

	/// <summary>Transition the swpachain images from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.</summary>
	/// <param name="commandBuffer">Command buffer to record the layout transitions against.</param>
	void transitionSwapchainImageLayoutsToPresent(pvrvk::CommandBuffer commandBuffer);

	/// <summary>Helper method used to fill the color and depth attachment description for each of the render passes used by each anti aliasing technique,
	/// given all color attachments have the same format and all depth attachments have the same format.</summary>
	/// <param name="numColorAttachments">Number of color attachments to add to the attachment description vector.</param>
	/// <param name="addDepthAttachment">Whether to add or not a depth attachment to the attachment description vector.</param>
	/// <param name="numSamplesPerPixel">Number of samples per pixel for the color and depth attachments added to the attachment description vector.</param>
	/// <param name="vectorAttachmentDescription">Vector with the attachment description to fill.</param>
	void fillAttachmentDescription(
		int numColorAttachments, bool addDepthAttachment, pvrvk::SampleCountFlags numSamplesPerPixel, std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription);

	/// <summary>Helper method to build the render passes for each one of the techniques. Uses a vector to provide the attachment description.
	/// Adds a subpass self-dependency to change the access layout of all color attachments from e_COLOR_ATTACHMENT_WRITE_BIT to e_SHADER_READ_BIT.
	/// <param name="vectorAttachmentDescription">Vector with the attachment description. Depth attachment must be provided as the last element, as
	/// the attachment binding index for color attachments in VkRenderPassCreateInfo::pSubpasses::pColorAttachments is taken from the array index,
	/// interleaving a depth attachment would provide wrong information.
	/// <returns>Return the built render pass.</returns>
	pvrvk::RenderPass createRenderPass(const std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription);

	/// <summary>Creates the render pass used to draw the scene offscreen.</summary>
	void createOffScreenGeometryRenderPass();

	/// <summary>Creates and updates the descriptor sets used for rendering the scene geometry.</summary>
	void createSceneDescriptorSets();

	/// <summary>Creates and updates the descriptor sets used when the _isImageOutputMode mode is enabled.</summary>
	void createImageOutputDescriptorSets();

	/// <summary>Initialization of the PVRSuperResolution techniques implemented in this sample.</summary>
	void initializeSuperResolution();

	/// <summary>Helper function to submit a single command buffer to the graphics queue with a varying amount of semaphore to wait for / signal.</summary>
	/// <param name="vectorSubmitWaitFlags">Vector with the pipeline submit flags for the command buffer submission.</param>
	/// <param name="vectorCommandBufferSemaphoresToWaitFor">Vector with the semaphores to wait for.</param>
	/// <param name="vectorCommandBufferSemaphoresToSignal">Vector with the semaphores to signal.</param>
	/// <param name="fence">Fence to signal once the command buffer has finished execution.</param>
	/// <param name="commandBuffer">Command buffer to submit.</param>
	void submitCommandBuffer(const std::vector<pvrvk::PipelineStageFlags>& vectorSubmitWaitFlags, const std::vector<pvrvk::Semaphore>& vectorCommandBufferSemaphoresToWaitFor, 
		const std::vector<pvrvk::Semaphore>& vectorCommandBufferSemaphoresToSignal, const pvrvk::Fence fence, const pvrvk::CommandBuffer commandBuffer);

	/// <summary>Method where the Vulkan command buffers for the supernova V1 Mode 1X are submitted.</summary>
	void submitSupernovaV1Mode1XCommands();

	/// <summary>Method where the Vulkan command buffers for the supernova V1 Mode 2X are submitted.</summary>
	void submitSupernovaV1Mode2XCommands();

private:
	/// <summary>Initialize primary and secondary command buffers used for each of the techniques.</summary>
	void initializeComandBuffers();

	/// <summary>Helper method to identify whether the application is given an image as command line 
	/// parameter to be used as input for the Supernova algorithms instead of the usual 3D scene.</summary>
	/// <returns>True if the image output mode is enabled, false otherwise.</returns>
	bool isImageOutputMode();

	/// <summary>Load the image in path _imageOutputPath in case _isImageOutputMode is true.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	void loadImageOutputMode(pvrvk::Device device, pvrvk::CommandBuffer utilityCommandBuffer, pvr::utils::vma::Allocator vmaAllocator);

	/// <summary>Creates the textures used for rendering the statue.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	void loadTextures(pvrvk::Device device, pvrvk::CommandBuffer utilityCommandBuffer, pvr::utils::vma::Allocator vmaAllocator);

	/// <summary>Creates the various samplers used throughout the demo.</summary>
	void createSamplers();

	/// <summary>Draws an assets::Mesh after the model view matrix has been set and the material prepared.</summary>
	/// <param name="cmdBuffer">The command buffer to record rendering commands to.</param>
	/// <param name="nodeIndex">Node index of the mesh to draw</param>
	void drawMesh(pvrvk::CommandBuffer cmdBuffer, int nodeIndex);

	/// <summary>Build a graphics pipeline used for drawing the scene geometry.</summary>
	/// <param name="renderpass">Render pass to use in the graphics pipeline.</param>
	/// <param name="pipelineLayout">Pipeline layout to use in the graphics pipeline.</param>
	/// <param name="vertexShader">Vertex shader to use in the pipeline.</param>
	/// <param name="fragmentShader">Fragment shader to use in the pipeline.</param>
	/// <param name="isPostProcessing">Whether the graphivcs pipeline wil be used for postprocessing pass.</param>
	pvrvk::GraphicsPipeline createScenePipeline(const pvrvk::RenderPass renderpass, pvrvk::PipelineLayout pipelineLayout, const char* vertexShader, const char* fragmentShader, 
		bool isPostProcessing, uint32_t viewportDimensionDivisor);

	/// <summary>Creates a structured buffer view with the scene information for rendering the scene geometry.</summary>
	void createSceneDataUniformBuffer();

	/// <summary>Update the structured buffer view DeviceResources::sceneStructuredBufferView with the latest values of the variables used in it.</summary>
	/// <param name="swapchainIndex">Swapchain index to udpate the structued buffer view.</param>
	void updateSceneUniformBuffer(int swapchainIndex);

	/// <summary>Record the UI rendering commands (text with current technique being applied and the IMG logo).</summary>
	/// <param name="cmdBuffer">Command buffer to record to.</param>
	/// <param name="uiRendererIndex">Index for the Supernova UI renderer index to use to record commands.</param>
	void recordUIRendererCommands(pvrvk::CommandBuffer cmdBufferrecord, int uiRendererIndex);

	/// <summary>Record command buffers for the PVRSuperResolution sharpening technique.</summary>
	void recordSupernovaV1Mode1X();

	/// <summary>Record command buffers for the PVRSuperResolution spatial upscaling technique.</summary>
	void recordSupernovaV1Mode2X();

	/// <summary>Handle to the scene loaded.</summary>
	pvr::assets::ModelHandle _scene;

	/// <summary>Model matrix for the scene element.</summary>
	glm::mat4 _modelMatrix = glm::mat4(1.0f);

	/// <summary>World matrix for the scene element.</summary>
	glm::mat4 _worldMatrix = glm::mat4(1.0f);

	/// <summary>View matrix for the scene element.</summary>
	glm::mat4 _viewProjMatrix = glm::mat4(1.0f);

	/// <summary>Number of presentable images in the swap chain.</summary>
	uint32_t _swapchainLength = 0;

	/// <summary>Index of the current swap chain image being used.</summary>
	uint32_t _swapchainIndex = 0;

	/// <summary>Swapchain image index, in interval [0, numSwapChainImages - 1].</summary>
	uint32_t _frameId = 0;

	/// <summary>Helper variable to know the current technique index being used.</summary>
	uint32_t _inputIndex = 0;

	/// <summary>Pointer to struct encapsulating all the resources made with the current logical device.</summary>
	std::unique_ptr<DeviceResources> _deviceResources;

	/// <summary>Current selected antialiasing technique to be changed later with inputs.</summary>
	SupernovaTechnique _currentTechnique = SupernovaTechnique::SUPERNOVA_V1_MODE_1X;

	/// <summary>Variable to map the jitter values read from jitter on each frame, ranging from 0 to frameCount.</summary>
	int _frameOffset = 0;

	/// <summary>Clear values for the color attachment for the offscreen passes.</summary>
	pvrvk::ClearValue _clearValues[2] = { pvrvk::ClearValue(0.7f, 0.8f, 0.9f, 1.0f), pvrvk::ClearValue::createDefaultDepthStencilClearValue() };

	/// <summary>Variable holding all the information needed to draw the scene element for the no antialiasing, MSAA and FXAA techniques.</summary>
	SceneInformationBuffer _sceneInformationBuffer = {};

	/// <summary>Platform-independent command line argument parser.</summary>
	pvr::CommandLine _cmdLine{};

	/// <summary>Flag to know whether the application is given an image as command line parameter to be used as input for the 
	/// Supernova algorithms instead of the usual 3D scene.</summary>
	bool _isImageOutputMode = false;

	/// <summary>In case _isImageOutputMode is true, this member variable stores the path to the texture to output.</summary>
	std::string _imageOutputPath = "";
	
	/// <summary>In case _isImageOutputMode is true, this member variable stores dimensions of the image.</summary>
	glm::ivec2 _imageOutPutDimensions = glm::ivec2(0);

	/// <summary>Whether to show the UI renderer text.</summary>
	bool _showUIRendererText = true;

	/// <summary>In case the Mentis upscaling technique is selected through command line as the current one when running the application, the swapchain images need to be transitioned from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.</summary>
	bool _transitionSwapchainImageLayoutsToPresent = false;
};

pvr::Result VulkanSupernova::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create a Vulkan 1.3 instance and retrieve compatible physical devices
	// NOTE: Vulkan 1.3 is required for device and instance
	pvr::utils::VulkanVersion VulkanVersion(1, 3, 0);

	pvr::utils::InstanceExtensions instanceExtensions;
	instanceExtensions.addExtension(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
	instanceExtensions.addExtension(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
	instanceExtensions.addExtension(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), VulkanVersion, instanceExtensions);

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Choose the physical device
	uint32_t physicalDevice = 0;
	if (_deviceResources->instance->getNumPhysicalDevices() > 1)
	{
		for (uint32_t i = 0; i < _deviceResources->instance->getNumPhysicalDevices(); ++i)
		{
			// Prefer discrete gpu
			if (_deviceResources->instance->getPhysicalDevice(i)->getProperties().getDeviceType() == pvrvk::PhysicalDeviceType::e_DISCRETE_GPU)
			{
				physicalDevice = i;
				break;
			}
		}
	}

	// Create the surface
	pvrvk::Surface surface = pvr::utils::createSurface(
		_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(physicalDevice), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	pvr::utils::QueuePopulateInfo queueCreateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;

	pvr::utils::DeviceExtensions deviceExtensions;
	deviceExtensions.addExtension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
	deviceExtensions.addExtension(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
	deviceExtensions.addExtension(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
	// Some of the extensions are platform dependent
#ifdef _WIN32
	deviceExtensions.addExtension(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
	deviceExtensions.addExtension(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
	deviceExtensions.addExtension(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
#else
	deviceExtensions.addExtension(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
	deviceExtensions.addExtension(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
	deviceExtensions.addExtension(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
#endif

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(physicalDevice), &queueCreateInfo, 1, &queueAccessInfo, deviceExtensions);

	_deviceResources->graphicsQueue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);
	_deviceResources->graphicsQueue->setObjectName("GraphicsQueue");

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;

	// Create memory allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Create the swapchain, framebuffers and main rendering images
	// Note the use of the colour attachment load operation (pvrvk::AttachmentLoadOp::e_DONT_CARE). The final composition pass will be a full screen render
	// so we don't need to clear the attachment prior to rendering to the image as each pixel will get a new value either way
	// The final render is a full screen pass, so no depth is required (see below)...
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;

	// Get current swap index
	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->depthImages.resize(_swapchainLength);

	_deviceResources->imageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->graphicsSemaphores.resize(_swapchainLength);
	_deviceResources->perFrameResourcesFences.resize(_swapchainLength);
	_deviceResources->vectorMentisBlitCommandBufferFence.resize(_swapchainLength);

	// create the command pool and the descriptor pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->graphicsQueue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	// This demo application makes use of quite a large number of Images and Buffers and therefore we're making possible for the descriptor pool to allocate descriptors with various limits.maxDescriptorSet*
	_deviceResources->descriptorPool =
		_deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
														   .setMaxDescriptorSets(static_cast<uint16_t>(80 * _swapchainLength))
														   .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(80 * _swapchainLength))
														   .addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, static_cast<uint16_t>(80 * _swapchainLength))
														   .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, static_cast<uint16_t>(80 * _swapchainLength))
														   .addDescriptorInfo(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, static_cast<uint16_t>(10 * _swapchainLength)));

	_deviceResources->descriptorPool->setObjectName("DescriptorPool");

	// create the utility commandbuffer which will be used for image layout transitions and buffer/image uploads.
	_deviceResources->utilityCommandBuffer = _deviceResources->commandPool->allocateCommandBuffer();
	_deviceResources->utilityCommandBuffer->begin();

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	pvr::utils::createAttachmentImages(_deviceResources->depthImages, _deviceResources->device, _swapchainLength,
		pvr::utils::getSupportedDepthStencilFormat(_deviceResources->device, getDisplayAttributes()), _deviceResources->swapchain->getDimension(),
		pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, pvrvk::SampleCountFlags::e_1_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT, "DepthStencilBufferImages");

	if (_isImageOutputMode)
	{
		loadImageOutputMode(_deviceResources->device, _deviceResources->utilityCommandBuffer, _deviceResources->vmaAllocator);
	}
	else
	{
		bool requiresCommandBufferSubmission = false;
		pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_scene, _deviceResources->sceneVertexBuffer, _deviceResources->sceneIndexBuffer,
			_deviceResources->utilityCommandBuffer, requiresCommandBufferSubmission, _deviceResources->vmaAllocator);

		loadTextures(_deviceResources->device, _deviceResources->utilityCommandBuffer, _deviceResources->vmaAllocator);
	}

	createSamplers();

	if (!_isImageOutputMode)
	{
		createSceneDataUniformBuffer();
	}
	
	createOffScreenGeometryRenderPass();

	if (_isImageOutputMode)
	{
		createImageOutputDescriptorSets();
	}
	else
	{
		createSceneDescriptorSets();
	}
	
	createImagesAndFramebuffers();
	createGraphicsPipelines();

	if (_transitionSwapchainImageLayoutsToPresent)
	{
		transitionSwapchainImageLayoutsToPresent(_deviceResources->utilityCommandBuffer);
	}

	//  Initialize UIRenderer, using one instance for each Supernova technique, will be recorded with the scene offscreen pass
	int indexSupernovaV1Mode1X = static_cast<int>(SupernovaTechnique::SUPERNOVA_V1_MODE_1X);
	_deviceResources->arrayUIRenderer[indexSupernovaV1Mode1X].init(getWidth(), getHeight(), isFullScreen(), _deviceResources->offScreenGeometryRenderPass, 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->graphicsQueue);
	_deviceResources->arrayUIRenderer[indexSupernovaV1Mode1X].getDefaultTitle()->setText("Supernova V1 x1 Mode (sharpener)");
	_deviceResources->arrayUIRenderer[indexSupernovaV1Mode1X].getDefaultTitle()->commitUpdates();

	int indexSupernovaV2Mode1X = static_cast<int>(SupernovaTechnique::SUPERNOVA_V1_MODE_2X);
	_deviceResources->arrayUIRenderer[indexSupernovaV2Mode1X].init(getWidth() / 2, getHeight() / 2, isFullScreen(), _deviceResources->offScreenGeometryRenderPass, 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->graphicsQueue);
	_deviceResources->arrayUIRenderer[indexSupernovaV2Mode1X].getDefaultTitle()->setText("Supernova V1 x2 Mode (upscaler)");
	_deviceResources->arrayUIRenderer[indexSupernovaV2Mode1X].getDefaultTitle()->commitUpdates();

	_deviceResources->utilityCommandBuffer->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->utilityCommandBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->graphicsQueue->submit(&submitInfo, 1);
	_deviceResources->graphicsQueue->waitIdle(); // wait

	pvrvk::SemaphoreCreateInfo semaphoreCreateInfo = {};

#ifdef _WIN32
	semaphoreCreateInfo.setExternalSemaphoreHandleTypeFlagBitslags(pvrvk::ExternalSemaphoreHandleTypeFlagBits::e_OPAQUE_WIN32_BIT);
#else
	semaphoreCreateInfo.setExternalSemaphoreHandleTypeFlagBitslags(pvrvk::ExternalSemaphoreHandleTypeFlagBits::e_OPAQUE_FD_BIT);
#endif

	// create the synchronisation primitives
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->graphicsSemaphores[i] = (_deviceResources->device->createSemaphore());
		_deviceResources->imageAcquiredSemaphores[i] = (_deviceResources->device->createSemaphore());

		_deviceResources->graphicsSemaphores[i]->setObjectName("GraphicsSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = (_deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT));
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));

		_deviceResources->vectorMentisBlitCommandBufferFence[i] = (_deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT));
		_deviceResources->vectorMentisBlitCommandBufferFence[i]->setObjectName("FenceBlit" + std::to_string(i));
	}

	initializeComandBuffers();
	initializeSuperResolution();
	recordSupernovaV1Mode1X();
	recordSupernovaV1Mode2X();

	if (!_isImageOutputMode)
	{
		_scene->getCameraProperties(0, SceneElements::_cameraFov, SceneElements::_cameraFrom, SceneElements::_cameraTo, SceneElements::_cameraUp);
		_worldMatrix = _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
		SceneElements::_cameraLookAt = glm::lookAt(SceneElements::_cameraFrom, SceneElements::_cameraTo, SceneElements::_cameraUp);
	}

	return pvr::Result::Success;
}

void VulkanSupernova::createImagesAndFramebuffers()
{
	pvrvk::ImageCreateInfo colorImageInfo1SPP =
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Extent3D(getWidth(), getHeight(), 1u),
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_INPUT_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT);

	pvrvk::ImageCreateInfo colorImageInfo1SPPOneQuarter =
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Extent3D(getWidth() / 2, getHeight() / 2, 1u),
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_INPUT_ATTACHMENT_BIT);
	
	// Build color attachment images and image views for the offscreen pass
	for (int i = 0; i < _swapchainLength; ++i)
	{
		pvrvk::Image colorImage1SPP = pvr::utils::createImage(_deviceResources->device, colorImageInfo1SPP, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT);
		pvrvk::ImageView colorImageView1SPP = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(colorImage1SPP));
		_deviceResources->offscreenColorAttachmentImageFullSize.push_back(colorImage1SPP);
		_deviceResources->offscreenColorAttachmentImageFullSize.back()->setObjectName("offscreenColorAttachmentImageFullSize[" + std::to_string(i) + "]");
		_deviceResources->offscreenColorAttachmentImageViewFullSize.push_back(colorImageView1SPP);

		pvrvk::Image colorImage1SPPOneQuarter = pvr::utils::createImage(_deviceResources->device, colorImageInfo1SPPOneQuarter, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT);
		pvrvk::ImageView colorImageView1SPPOneQuarter = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(colorImage1SPPOneQuarter));
		_deviceResources->offscreenColorAttachmentImageHalfSize.push_back(colorImage1SPPOneQuarter);
		_deviceResources->offscreenColorAttachmentImageHalfSize.back()->setObjectName("offscreenColorAttachmentImageHalfSize[" + std::to_string(i) + "]");
		_deviceResources->offscreenColorAttachmentImageViewHalfSize.push_back(colorImageView1SPPOneQuarter);
	}

	if (!_isImageOutputMode)
	{
		pvrvk::ImageCreateInfo depthImageInfo1SPP = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _deviceResources->depthImages[0]->getFormat(),
			pvrvk::Extent3D(getWidth(), getHeight(), 1u), pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT);

		pvrvk::ImageCreateInfo depthImageInfo1SPPOneQuarter = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _deviceResources->depthImages[0]->getFormat(),
			pvrvk::Extent3D(getWidth() / 2, getHeight() / 2, 1u), pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT);

		// Build depth attachment images and image views for the offscreen pass
		for (int i = 0; i < _swapchainLength; ++i)
		{
			pvrvk::Image depthImage1SPP = pvr::utils::createImage(
				_deviceResources->device, depthImageInfo1SPP, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
			pvrvk::ImageView depthImageView1SPP = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(depthImage1SPP));
			_deviceResources->offscreenDepthAttachmentImageFullSize.push_back(depthImage1SPP);
			std::string objectName = "offscreenDepthAttachmentImageFullSize[" + std::to_string(i) + "]";
			_deviceResources->offscreenDepthAttachmentImageFullSize.back()->setObjectName(objectName);
			_deviceResources->offscreenDepthAttachmentImageViewFullSize.push_back(depthImageView1SPP);

			pvrvk::Image depthImage1SPPOneQuarter = pvr::utils::createImage(_deviceResources->device, depthImageInfo1SPPOneQuarter, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
				pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
			pvrvk::ImageView depthImageView1SPPOneQuarter = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(depthImage1SPPOneQuarter));
			_deviceResources->offscreenDepthAttachmentImageHalfSize.push_back(depthImage1SPPOneQuarter);
			std::string objectNameOneQuarter = "offscreenDepthAttachmentImageHalfSize[" + std::to_string(i) + "]";
			_deviceResources->offscreenDepthAttachmentImageHalfSize.back()->setObjectName(objectNameOneQuarter);
			_deviceResources->offscreenDepthAttachmentImageViewHalfSize.push_back(depthImageView1SPPOneQuarter);
		}
	}

	// Build framebuffers
	for (int i = 0; i < _swapchainLength; ++i)
	{
		// Build framebuffer for the full size offscreen scene pass
		pvrvk::FramebufferCreateInfo offscreenFramebufferFullSizeCreateInfo;
		offscreenFramebufferFullSizeCreateInfo.setAttachment(0, _deviceResources->offscreenColorAttachmentImageViewFullSize[i]);
		if (!_isImageOutputMode)
		{
			offscreenFramebufferFullSizeCreateInfo.setAttachment(1, _deviceResources->offscreenDepthAttachmentImageViewFullSize[i]);
		}
		offscreenFramebufferFullSizeCreateInfo.setDimensions(getWidth(), getHeight());
		offscreenFramebufferFullSizeCreateInfo.setRenderPass(_deviceResources->offScreenGeometryRenderPass);
		_deviceResources->offscreenFramebufferFullSize.push_back(_deviceResources->device->createFramebuffer(offscreenFramebufferFullSizeCreateInfo));

		// Build framebuffer for the half size offscreen scene pass
		pvrvk::FramebufferCreateInfo offscreenFramebufferHalfSizeCreateInfo;
		offscreenFramebufferHalfSizeCreateInfo.setAttachment(0, _deviceResources->offscreenColorAttachmentImageViewHalfSize[i]);
		if (!_isImageOutputMode)
		{
			offscreenFramebufferHalfSizeCreateInfo.setAttachment(1, _deviceResources->offscreenDepthAttachmentImageViewHalfSize[i]);
		}
		offscreenFramebufferHalfSizeCreateInfo.setDimensions(getWidth() / 2, getHeight() / 2);
		offscreenFramebufferHalfSizeCreateInfo.setRenderPass(_deviceResources->offScreenGeometryRenderPass);
		_deviceResources->offscreenFramebufferHalfSize.push_back(_deviceResources->device->createFramebuffer(offscreenFramebufferHalfSizeCreateInfo));
	}
}

void VulkanSupernova::createGraphicsPipelines()
{
	if (_isImageOutputMode)
	{
		_deviceResources->offscreenPipelineFullSize = createScenePipeline(_deviceResources->offScreenGeometryRenderPass, _deviceResources->scenePipelineLayout,
			ShaderFiles::AttributelessVertexShaderFile, ShaderFiles::ImageOutputFragShader, true, 1);

		_deviceResources->offscreenPipelineHalfSize = createScenePipeline(_deviceResources->offScreenGeometryRenderPass, _deviceResources->scenePipelineLayout,
			ShaderFiles::AttributelessVertexShaderFile, ShaderFiles::ImageOutputFragShader, true, 2);
	}
	else
	{
		_deviceResources->offscreenPipelineFullSize = createScenePipeline(_deviceResources->offScreenGeometryRenderPass, _deviceResources->scenePipelineLayout,
			ShaderFiles::VertexShaderFile, ShaderFiles::FragmentShaderFile, false, 1);

		_deviceResources->offscreenPipelineHalfSize = createScenePipeline(_deviceResources->offScreenGeometryRenderPass, _deviceResources->scenePipelineLayout,
			ShaderFiles::VertexShaderFile, ShaderFiles::FragmentShaderFile, false, 2);
	}

	_deviceResources->offscreenPipelineFullSize->setObjectName(_isImageOutputMode ? "imageModePipelineFullSize" : "offscreenPipelineFullSize");
	_deviceResources->offscreenPipelineHalfSize->setObjectName(_isImageOutputMode ? "imageModePipelineHalfSize" : "offscreenPipelineHalfSize");
}

void VulkanSupernova::transitionSwapchainImageLayoutsToPresent(pvrvk::CommandBuffer commandBuffer)
{
	pvrvk::MemoryBarrierSet barrierSet;
	std::vector<pvrvk::ImageMemoryBarrier> vectorRenderImageBarrier(_swapchainLength);
	std::vector<pvrvk::ImageMemoryBarrier> vectorOffscreenColorAttachmentFullSizeImageBarrier(_swapchainLength);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		vectorRenderImageBarrier[i].setSrcAccessMask(pvrvk::AccessFlags::e_NONE);
		vectorRenderImageBarrier[i].setDstAccessMask(pvrvk::AccessFlags::e_NONE);
		vectorRenderImageBarrier[i].setOldLayout(pvrvk::ImageLayout::e_UNDEFINED);
		vectorRenderImageBarrier[i].setNewLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
		vectorRenderImageBarrier[i].setImage(_deviceResources->swapchain->getImage(i));
		vectorRenderImageBarrier[i].setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));

		vectorOffscreenColorAttachmentFullSizeImageBarrier[i].setSrcAccessMask(pvrvk::AccessFlags::e_NONE);
		vectorOffscreenColorAttachmentFullSizeImageBarrier[i].setDstAccessMask(pvrvk::AccessFlags::e_NONE);
		vectorOffscreenColorAttachmentFullSizeImageBarrier[i].setOldLayout(pvrvk::ImageLayout::e_UNDEFINED);
		vectorOffscreenColorAttachmentFullSizeImageBarrier[i].setNewLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
		vectorOffscreenColorAttachmentFullSizeImageBarrier[i].setImage(_deviceResources->offscreenColorAttachmentImageFullSize[i]);
		vectorOffscreenColorAttachmentFullSizeImageBarrier[i].setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));

		barrierSet.addBarrier(vectorRenderImageBarrier[i]);
		barrierSet.addBarrier(vectorOffscreenColorAttachmentFullSizeImageBarrier[i]);
	}

	// Submit barriers to transition layout
	commandBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TOP_OF_PIPE_BIT, pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, barrierSet);
}

void VulkanSupernova::fillAttachmentDescription(
	int numColorAttachments, bool addDepthAttachment, pvrvk::SampleCountFlags numSamplesPerPixel, std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription)
{
	vectorAttachmentDescription.clear();

	for (int i = 0; i < numColorAttachments; ++i)
	{
		vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createColorDescription(pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, numSamplesPerPixel));
	}

	if (addDepthAttachment)
	{
		vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createDepthStencilDescription(_deviceResources->depthImages[0]->getFormat(),
			pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, numSamplesPerPixel));
	}
}

pvrvk::RenderPass VulkanSupernova::createRenderPass(const std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription)
{
	pvrvk::RenderPassCreateInfo renderPassInfo;
	pvrvk::SubpassDescription subpass;

	bool depthAttachmentPresent = false;

	for (uint32_t i = 0; i < vectorAttachmentDescription.size(); ++i)
	{
		renderPassInfo.setAttachmentDescription(i, vectorAttachmentDescription[i]);

		pvrvk::ImageLayout finalLayout = vectorAttachmentDescription[i].getFinalLayout();

		if (finalLayout == pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			subpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(i, finalLayout));
			depthAttachmentPresent = true;
		}
		else
		{
			// Assuming the only other value used in this sample, e_COLOR_ATTACHMENT_OPTIMAL
			subpass.setColorAttachmentReference(i, pvrvk::AttachmentReference(i, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
		}
	}

	if (depthAttachmentPresent && vectorAttachmentDescription.back().getFinalLayout() != pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		assertion(false, "Render pass depth attachment not present at last index of vectorAttachmentDescription");
	}

	renderPassInfo.setSubpass(0, subpass);

	// Add external subpass dependencies to avoid the implicit subpass dependencies and to provide more optimal pipeline stage task synchronisation
	pvrvk::SubpassDependency dependencies[2];

	dependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
	dependencies[0].setDstSubpass(0);
	dependencies[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependencies[0].setDstStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependencies[0].setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependencies[0].setDstAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependencies[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	dependencies[1].setSrcSubpass(0);
	dependencies[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
	dependencies[1].setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependencies[1].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependencies[1].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependencies[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependencies[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	renderPassInfo.addSubpassDependency(dependencies[0]);
	renderPassInfo.addSubpassDependency(dependencies[1]);

	return _deviceResources->device->createRenderPass(renderPassInfo);
}

void VulkanSupernova::createOffScreenGeometryRenderPass()
{
	std::vector<pvrvk::AttachmentDescription> vectorAttachmentDescription;
	fillAttachmentDescription(1, !_isImageOutputMode, pvrvk::SampleCountFlags::e_1_BIT, vectorAttachmentDescription);
	_deviceResources->offScreenGeometryRenderPass = createRenderPass(vectorAttachmentDescription);
	_deviceResources->offScreenGeometryRenderPass->setObjectName("OnScreenGeometryRenderPass");
}

void VulkanSupernova::initializeComandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->supernovaV1Mode1XCommandBuffer.push_back(_deviceResources->commandPool->allocateCommandBuffer());
		_deviceResources->supernovaV1Mode2XCommandBuffer.push_back(_deviceResources->commandPool->allocateCommandBuffer());
	}
}

bool VulkanSupernova::isImageOutputMode()
{
	if (_cmdLine.hasOption("-imageOutput"))
	{
		bool stringOptionResult = _cmdLine.getStringOption("-imageOutput", _imageOutputPath);

		if (stringOptionResult)
		{
			// The image has to be in .pvr format
			std::string extension = _imageOutputPath.substr(_imageOutputPath.size() - 4, 4);
			if (strcmp(extension.c_str(), ".pvr") == 0)
			{
				return true;
			}
		}
	}

	return false;
}


void VulkanSupernova::loadImageOutputMode(pvrvk::Device device, pvrvk::CommandBuffer utilityCommandBuffer, pvr::utils::vma::Allocator vmaAllocator)
{
	_deviceResources->_textureModeImageView = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, _imageOutputPath.c_str(), true, utilityCommandBuffer, *this,
		pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
}

void VulkanSupernova::loadTextures(pvrvk::Device device, pvrvk::CommandBuffer utilityCommandBuffer, pvr::utils::vma::Allocator vmaAllocator)
{
	bool astcSupported = pvr::utils::isSupportedFormat(_deviceResources->device->getPhysicalDevice(), pvrvk::Format::e_ASTC_4x4_UNORM_BLOCK);

	_deviceResources->albedoImageView =
		pvr::utils::loadAndUploadImageAndView(_deviceResources->device, (SceneElements::StatueTexFile + (astcSupported ? "_astc.pvr" : ".pvr")).c_str(), true, utilityCommandBuffer,
			*this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	_deviceResources->normalMapImageView = pvr::utils::loadAndUploadImageAndView(_deviceResources->device,
		(SceneElements::StatueNormalMapTexFile + (astcSupported ? "_astc.pvr" : ".pvr")).c_str(), true, utilityCommandBuffer, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
}

void VulkanSupernova::createSamplers()
{
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;

	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	_deviceResources->samplerNearest = _deviceResources->device->createSampler(samplerInfo);
}

/// <summary>Draws an assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <param name="cmdBuffer">The secondary command buffer to record rendering commands to.</param>
/// <param name="nodeIndex">Node index of the mesh to draw</param>
void VulkanSupernova::drawMesh(pvrvk::CommandBuffer cmdBuffer, int nodeIndex)
{
	const uint32_t meshId = _scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _scene->getMesh(meshId);

	// bind the VBO for the mesh
	cmdBuffer->bindVertexBuffer(_deviceResources->sceneVertexBuffer[meshId], 0, 0);

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		// Indexed Triangle list
		if (_deviceResources->sceneIndexBuffer[meshId])
		{
			cmdBuffer->bindIndexBuffer(_deviceResources->sceneIndexBuffer[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
			cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			cmdBuffer->draw(0, mesh.getNumFaces() * 3, 0, 1);
		}
	}
	else
	{
		uint32_t offset = 0;
		for (uint32_t i = 0; i < mesh.getNumStrips(); ++i)
		{
			if (_deviceResources->sceneIndexBuffer[meshId])
			{
				// Indexed Triangle strips
				cmdBuffer->bindIndexBuffer(_deviceResources->sceneIndexBuffer[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
				cmdBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				cmdBuffer->draw(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

pvrvk::GraphicsPipeline VulkanSupernova::createScenePipeline(const pvrvk::RenderPass renderpass, pvrvk::PipelineLayout pipelineLayout, const char* vertexShader,
	const char* fragmentShader, bool isPostProcessing, uint32_t viewportDimensionDivisor)
{
	pvrvk::GraphicsPipelineCreateInfo pipelineInfo;

	pvrvk::Extent2D viewportDimensions = _deviceResources->swapchain->getDimension();

	viewportDimensions.setWidth(viewportDimensions.getWidth() / viewportDimensionDivisor);
	viewportDimensions.setHeight(viewportDimensions.getHeight() / viewportDimensionDivisor);

	pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDimensions.getWidth()), static_cast<float>(viewportDimensions.getHeight())),
		pvrvk::Rect2D(0, 0, viewportDimensions.getWidth(), viewportDimensions.getHeight()));

	pipelineInfo.rasterizer.setCullMode(isPostProcessing ? pvrvk::CullModeFlags::e_FRONT_BIT : pvrvk::CullModeFlags::e_BACK_BIT);

	if (!isPostProcessing)
	{
		// Add depth testing
		pipelineInfo.depthStencil.enableDepthWrite(true);
		pipelineInfo.depthStencil.enableDepthTest(true);
		pipelineInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
		pipelineInfo.depthStencil.enableStencilTest(false);
	}

	// blend state
	pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	pipelineInfo.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(vertexShader)->readToEnd<uint32_t>())));
	pipelineInfo.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(fragmentShader)->readToEnd<uint32_t>())));
	
	if (isPostProcessing)
	{
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);
	}
	else
	{
		const pvr::assets::Mesh& mesh = _scene->getMesh(0);
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToPVRVk(mesh.getPrimitiveType()));
		pvr::utils::populateInputAssemblyFromMesh(
			mesh, VertexAttribBindings, sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipelineInfo.vertexInput, pipelineInfo.inputAssembler);	
	}

	pipelineInfo.renderPass = renderpass;
	pipelineInfo.subpass = 0;
	pipelineInfo.pipelineLayout = pipelineLayout;

	return _deviceResources->device->createGraphicsPipeline(pipelineInfo, _deviceResources->pipelineCache);
}

void VulkanSupernova::createSceneDataUniformBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::MVPMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::LightDirModel, pvr::GpuDatatypes::vec3);

	_deviceResources->sceneStructuredBufferView.initDynamic(desc, _scene->getNumMeshNodes() * _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->sceneUniformBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->sceneStructuredBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->sceneUniformBuffer->setObjectName("SceneUniformBufferUBO");
	_deviceResources->sceneStructuredBufferView.pointToMappedMemory(_deviceResources->sceneUniformBuffer->getDeviceMemory()->getMappedData());
}

void VulkanSupernova::createSceneDescriptorSets()
{
	pvrvk::DescriptorSetLayoutCreateInfo vertDescSetLayout;
	vertDescSetLayout.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
	_deviceResources->sceneVertexDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(vertDescSetLayout);

	pvrvk::DescriptorSetLayoutCreateInfo texDescSetLayout;
	texDescSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	texDescSetLayout.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->sceneFragmentDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(texDescSetLayout);

	pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.addDescSetLayout(_deviceResources->sceneFragmentDescriptorSetLayout).addDescSetLayout(_deviceResources->sceneVertexDescriptorSetLayout);

	_deviceResources->scenePipelineLayout = _deviceResources->device->createPipelineLayout(pipelineLayoutInfo);

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->sceneVertexDescriptorSets.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->sceneVertexDescriptorSetLayout));
		_deviceResources->sceneFragmentDescriptorSets.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->sceneFragmentDescriptorSetLayout));

		_deviceResources->sceneVertexDescriptorSets.back()->setObjectName("SceneVertexSwapchain" + std::to_string(i) + "DescriptorSet");
		_deviceResources->sceneFragmentDescriptorSets.back()->setObjectName("SceneFragmentSwapchain" + std::to_string(i) + "DescriptorSet");

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->sceneFragmentDescriptorSets[i], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->albedoImageView, _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->sceneFragmentDescriptorSets[i], 1)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->normalMapImageView, _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->sceneVertexDescriptorSets[i], 0)
									.setBufferInfo(0,
										pvrvk::DescriptorBufferInfo(_deviceResources->sceneUniformBuffer, _deviceResources->sceneStructuredBufferView.getDynamicSliceOffset(i),
											_deviceResources->sceneStructuredBufferView.getDynamicSliceSize())));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanSupernova::createImageOutputDescriptorSets()
{
	pvrvk::DescriptorSetLayoutCreateInfo texDescSetLayout;
	texDescSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->sceneFragmentDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(texDescSetLayout);

	pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.addDescSetLayout(_deviceResources->sceneFragmentDescriptorSetLayout);

	_deviceResources->scenePipelineLayout = _deviceResources->device->createPipelineLayout(pipelineLayoutInfo);

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->sceneFragmentDescriptorSets.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->sceneFragmentDescriptorSetLayout));

		_deviceResources->sceneFragmentDescriptorSets.back()->setObjectName("SceneFragmentSwapchain" + std::to_string(i) + "DescriptorSet");

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->sceneFragmentDescriptorSets[i], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->_textureModeImageView, _deviceResources->samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanSupernova::updateSceneUniformBuffer(int swapchainIndex)
{
	_sceneInformationBuffer.lightDirModel = glm::vec3(SceneElements::LightDir * _modelMatrix);
	_sceneInformationBuffer.modelViewProjectionMatrix = _viewProjMatrix * _modelMatrix * _worldMatrix;

	_deviceResources->sceneStructuredBufferView.getElementByName(BufferEntryNames::MVPMatrix, 0, swapchainIndex).setValue(_sceneInformationBuffer.modelViewProjectionMatrix);
	_deviceResources->sceneStructuredBufferView.getElementByName(BufferEntryNames::LightDirModel, 0, swapchainIndex).setValue(_sceneInformationBuffer.lightDirModel);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->sceneUniformBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->sceneUniformBuffer->getDeviceMemory()->flushRange(
			_deviceResources->sceneStructuredBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->sceneStructuredBufferView.getDynamicSliceSize());
	}
}

void VulkanSupernova::recordUIRendererCommands(pvrvk::CommandBuffer cmdBuffer, int uiRendererIndex)
{
	_deviceResources->arrayUIRenderer[uiRendererIndex].beginRendering(cmdBuffer);
	_deviceResources->arrayUIRenderer[uiRendererIndex].getDefaultTitle()->render();
	_deviceResources->arrayUIRenderer[uiRendererIndex].getSdkLogo()->render();
	_deviceResources->arrayUIRenderer[uiRendererIndex].endRendering();
}

void VulkanSupernova::recordSupernovaV1Mode1X()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->supernovaV1Mode1XCommandBuffer[i]->setObjectName("SupernovaV1Mode1XCommandBufferSwapchain[" + std::to_string(i) + "]");

		_deviceResources->supernovaV1Mode1XCommandBuffer[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->supernovaV1Mode1XCommandBuffer[i], pvrvk::DebugUtilsLabel("Supernova v1 mode 1X offscreen pass"));

		// Do an initial offscreen pass writing the scene to a color attachment
		_deviceResources->supernovaV1Mode1XCommandBuffer[i]->beginRenderPass(
			_deviceResources->offscreenFramebufferFullSize[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, _clearValues, _isImageOutputMode ? 1 : 2);
		_deviceResources->supernovaV1Mode1XCommandBuffer[i]->bindPipeline(_deviceResources->offscreenPipelineFullSize);
		_deviceResources->supernovaV1Mode1XCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneFragmentDescriptorSets[i]);
		if (_isImageOutputMode)
		{
			_deviceResources->supernovaV1Mode1XCommandBuffer[i]->draw(0, 3);
		}
		else
		{
			_deviceResources->supernovaV1Mode1XCommandBuffer[i]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 1u, _deviceResources->sceneVertexDescriptorSets[i]);

			drawMesh(_deviceResources->supernovaV1Mode1XCommandBuffer[i], 0);
		}

		if (_showUIRendererText)
		{
			recordUIRendererCommands(_deviceResources->supernovaV1Mode1XCommandBuffer[i], static_cast<int>(SupernovaTechnique::SUPERNOVA_V1_MODE_1X));
		}
		
		_deviceResources->supernovaV1Mode1XCommandBuffer[i]->endRenderPass();
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->supernovaV1Mode1XCommandBuffer[i]);

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->supernovaV1Mode1XCommandBuffer[i], pvrvk::DebugUtilsLabel("Supernova v1 mode 1X library call pass"));

		// Call the Supernova library for the Supernova V1 Mode 1X postprocessing pass
		_deviceResources->supernovaV1Mode1X->recordCommands(const_cast<VkCommandBuffer>(_deviceResources->supernovaV1Mode1XCommandBuffer[i]->getVkHandle()), i);

		// After the call, the swapchain image has the sharpened result and is ready for present
		
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->supernovaV1Mode1XCommandBuffer[i]);

		_deviceResources->supernovaV1Mode1XCommandBuffer[i]->end();
	}
}

void VulkanSupernova::recordSupernovaV1Mode2X()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->supernovaV1Mode2XCommandBuffer[i]->setObjectName("SupernovaV1Mode1XCommandBufferSwapchain[" + std::to_string(i) + "]");

		_deviceResources->supernovaV1Mode2XCommandBuffer[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->supernovaV1Mode2XCommandBuffer[i], pvrvk::DebugUtilsLabel("Supernova v1 mode 2X offscreen pass"));

		// Do an initial offscreen pass writing the scene to a color attachment
		_deviceResources->supernovaV1Mode2XCommandBuffer[i]->beginRenderPass(
			_deviceResources->offscreenFramebufferHalfSize[i], pvrvk::Rect2D(0, 0, getWidth() / 2, getHeight() / 2), true, _clearValues, _isImageOutputMode ? 1 : 2);
		_deviceResources->supernovaV1Mode2XCommandBuffer[i]->bindPipeline(_deviceResources->offscreenPipelineHalfSize);
		_deviceResources->supernovaV1Mode2XCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneFragmentDescriptorSets[i]);
		if (_isImageOutputMode)
		{
			_deviceResources->supernovaV1Mode2XCommandBuffer[i]->draw(0, 3);
		}
		else
		{
			_deviceResources->supernovaV1Mode2XCommandBuffer[i]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 1u, _deviceResources->sceneVertexDescriptorSets[i]);
			drawMesh(_deviceResources->supernovaV1Mode2XCommandBuffer[i], 0);
		}

		if (_showUIRendererText)
		{
			recordUIRendererCommands(_deviceResources->supernovaV1Mode2XCommandBuffer[i], static_cast<int>(SupernovaTechnique::SUPERNOVA_V1_MODE_2X));
		}
		
		_deviceResources->supernovaV1Mode2XCommandBuffer[i]->endRenderPass();
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->supernovaV1Mode2XCommandBuffer[i]);

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->supernovaV1Mode2XCommandBuffer[i], pvrvk::DebugUtilsLabel("Supernova v1 mode 2X library call pass"));

		// Call the Supernova library for the Supernova V1 Mode 2X postprocessing pass
		_deviceResources->supernovaV1Mode2X->recordCommands(const_cast<VkCommandBuffer>(_deviceResources->supernovaV1Mode2XCommandBuffer[i]->getVkHandle()), i);

		// After the call, the swapchain image has the sharpened result and is ready for present

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->supernovaV1Mode2XCommandBuffer[i]);

		_deviceResources->supernovaV1Mode2XCommandBuffer[i]->end();
	}
}

void VulkanSupernova::initializeSuperResolution()
{
	_deviceResources->supernovaV1Mode1X = new pvr::SuperResolution();

	pvr::VulkanInitializationData postProcessingInitializationData = {};
	postProcessingInitializationData.device = _deviceResources->device->getVkHandle();
	postProcessingInitializationData.physicalDevice = _deviceResources->device->getPhysicalDevice()->getVkHandle();
	std::vector<VkImageView> vectorOutputImageView;
	for (size_t i = 0; i < _deviceResources->offscreenColorAttachmentImageViewFullSize.size(); ++i)
	{
		vectorOutputImageView.push_back(_deviceResources->swapchain->getImageView(static_cast<uint32_t>(i))->getVkHandle());

		std::vector<VkImageView> vectorInputImageView;
		vectorInputImageView.push_back(_deviceResources->offscreenColorAttachmentImageViewFullSize[i]->getVkHandle());

		postProcessingInitializationData.vectorInputImageView.push_back(vectorInputImageView);
		postProcessingInitializationData.inputImageFormat.push_back(static_cast<VkFormat>(_deviceResources->offscreenColorAttachmentImageFullSize[0]->getFormat()));
		postProcessingInitializationData.inputImageLayout.push_back(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	postProcessingInitializationData.inputImageExtent = { getWidth(), getHeight() };
	postProcessingInitializationData.vectorOutputImageView = vectorOutputImageView;
	postProcessingInitializationData.outputImageFormat = static_cast<VkFormat>(_deviceResources->swapchain->getImageFormat());
	postProcessingInitializationData.outputImageInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	postProcessingInitializationData.outputImageFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	postProcessingInitializationData.outputImageExtent = { getWidth(), getHeight() };
	postProcessingInitializationData.queueFlagBits = VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT;
	postProcessingInitializationData.queue = _deviceResources->graphicsQueue->getVkHandle();
	postProcessingInitializationData.queueFamilyIndex = _deviceResources->graphicsQueue->getFamilyIndex();
	postProcessingInitializationData.numberCommandBuffer = _deviceResources->swapchain->getSwapchainLength();
	postProcessingInitializationData.vk = &_deviceResources->device->getVkBindings();
	postProcessingInitializationData.vkInstance = &_deviceResources->instance->getVkBindings();
	postProcessingInitializationData.application = static_cast<void*>(getOSApplication());
	postProcessingInitializationData.postProcessingMethod = pvr::PostProcessingMethod::SupernovaV1Mode1X;
	_deviceResources->supernovaV1Mode1X->init(postProcessingInitializationData);

	_deviceResources->supernovaV1Mode2X = new pvr::SuperResolution();
	postProcessingInitializationData.vectorInputImageView.clear();
	for (size_t i = 0; i < _deviceResources->offscreenColorAttachmentImageViewHalfSize.size(); ++i)
	{
		std::vector<VkImageView> vectorInputImageView;
		vectorInputImageView.push_back(_deviceResources->offscreenColorAttachmentImageViewHalfSize[i]->getVkHandle());

		postProcessingInitializationData.vectorInputImageView.push_back(vectorInputImageView);
	}
	postProcessingInitializationData.postProcessingMethod = pvr::PostProcessingMethod::SupernovaV1Mode2X;
	postProcessingInitializationData.inputImageExtent = { getWidth() / 2, getHeight() / 2 };
	_deviceResources->supernovaV1Mode2X->init(postProcessingInitializationData);
}

void VulkanSupernova::submitCommandBuffer(const std::vector<pvrvk::PipelineStageFlags>& vectorSubmitWaitFlags, const std::vector<pvrvk::Semaphore>& vectorCommandBufferSemaphoresToWaitFor,
	const std::vector<pvrvk::Semaphore>& vectorCommandBufferSemaphoresToSignal, const pvrvk::Fence fence,
	const pvrvk::CommandBuffer commandBuffer)
{
	pvrvk::SubmitInfo submitInfo;
	submitInfo.waitDstStageMask = vectorSubmitWaitFlags.data();
	submitInfo.numWaitSemaphores = static_cast<uint32_t>(vectorCommandBufferSemaphoresToWaitFor.size());
	submitInfo.waitSemaphores = vectorCommandBufferSemaphoresToWaitFor.data();
	submitInfo.numSignalSemaphores = static_cast<uint32_t>(vectorCommandBufferSemaphoresToSignal.size());
	submitInfo.signalSemaphores = vectorCommandBufferSemaphoresToSignal.data();
	submitInfo.numCommandBuffers = 1;
	submitInfo.commandBuffers = &commandBuffer;
	_deviceResources->graphicsQueue->submit(&submitInfo, 1, fence);
}

void VulkanSupernova::submitSupernovaV1Mode1XCommands()
{
	std::vector<pvrvk::PipelineStageFlags> vectorSubmitWaitFlags = { pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT };
	std::vector<pvrvk::Semaphore> vectorSemaphoresToWaitFor = { _deviceResources->imageAcquiredSemaphores[_frameId] };
	std::vector<pvrvk::Semaphore> vectorSemaphoresToSignal = { _deviceResources->graphicsSemaphores[_swapchainIndex] };
	submitCommandBuffer(vectorSubmitWaitFlags, vectorSemaphoresToWaitFor, vectorSemaphoresToSignal,
		_deviceResources->perFrameResourcesFences[_swapchainIndex], _deviceResources->supernovaV1Mode1XCommandBuffer[_swapchainIndex]);
}

void VulkanSupernova::submitSupernovaV1Mode2XCommands()
{
	std::vector<pvrvk::PipelineStageFlags> vectorSubmitWaitFlags = { pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT };
	std::vector<pvrvk::Semaphore> vectorSemaphoresToWaitFor = { _deviceResources->imageAcquiredSemaphores[_frameId] };
	std::vector<pvrvk::Semaphore> vectorSemaphoresToSignal = { _deviceResources->graphicsSemaphores[_swapchainIndex] };
	submitCommandBuffer(vectorSubmitWaitFlags, vectorSemaphoresToWaitFor, vectorSemaphoresToSignal,
		_deviceResources->perFrameResourcesFences[_swapchainIndex], _deviceResources->supernovaV1Mode2XCommandBuffer[_swapchainIndex]);
}

pvr::Result VulkanSupernova::initApplication()
{
	_cmdLine = this->getCommandLine();
	_isImageOutputMode = isImageOutputMode();

	if (!_isImageOutputMode)
	{
		_scene = pvr::assets::loadModel(*this, SceneElements::SceneFile);
	}
	else
	{
		pvr::TextureHeader textureHeader;
		pvr::utils::readImageTextureHeader(_imageOutputPath.c_str(), *this, textureHeader);
		_imageOutPutDimensions = glm::ivec2(textureHeader.width, textureHeader.height);
	}

	if (_cmdLine.hasOption("-SupernovaV1Mode1X"))
	{
		_inputIndex = 0;
		changeCurrentTechnique();
	}

	if (_cmdLine.hasOption("-SupernovaV1Mode2X"))
	{
		_inputIndex = 1;
		changeCurrentTechnique();

		if (_isImageOutputMode)
		{
			_imageOutPutDimensions *= 2;
		}
	}

	if (_isImageOutputMode)
	{
		setDimensions(_imageOutPutDimensions.x, _imageOutPutDimensions.y);
	}
	
	return pvr::Result::Success;
}

pvr::Result VulkanSupernova::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

pvr::Result VulkanSupernova::quitApplication()
{
	_scene.reset();
	_deviceResources.reset();
	return pvr::Result::Success;
}

pvr::Result VulkanSupernova::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// Update uniforms
	if (!_isImageOutputMode)
	{
		float viewportHeight = 0.0f;
		float viewportWidth = 0.0f;

		switch (_currentTechnique)
		{
		case SupernovaTechnique::SUPERNOVA_V1_MODE_1X: {
			viewportHeight = static_cast<float>(this->getHeight());
			viewportWidth = static_cast<float>(this->getWidth());
			break;
		}
		case SupernovaTechnique::SUPERNOVA_V1_MODE_2X: {
			viewportHeight = static_cast<float>(this->getHeight() / 2);
			viewportWidth = static_cast<float>(this->getWidth() / 2);
			break;
		}
		default: {
			Log(LogLevel::Error, "Wrong technique");
			break;
		}
		}

		// Is the screen rotated
		const bool bRotate = this->isScreenRotated();

		//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
		_viewProjMatrix = (bRotate
				? pvr::math::perspectiveFov(pvr::Api::Vulkan, SceneElements::_cameraFov, viewportHeight, viewportWidth, _scene->getCamera(0).getNear(),
					  _scene->getCamera(0).getFar(), glm::pi<float>() * .5f)
				: pvr::math::perspectiveFov(pvr::Api::Vulkan, SceneElements::_cameraFov, viewportWidth, viewportHeight, _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar()));

		_viewProjMatrix = _viewProjMatrix * SceneElements::_cameraLookAt;

		// Calculate the model matrix
		_modelMatrix = glm::rotate(SceneElements::angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
		SceneElements::angleY -= SceneElements::RotateY * 0.05f * getFrameTime();

		updateSceneUniformBuffer(_swapchainIndex);
	}

	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	// Send commands depending on the technique selected
	switch (_currentTechnique)
	{
	case SupernovaTechnique::SUPERNOVA_V1_MODE_1X: {
		submitSupernovaV1Mode1XCommands();
		break;
	}
	case SupernovaTechnique::SUPERNOVA_V1_MODE_2X: {
		submitSupernovaV1Mode2XCommands();
		break;
	}
	default: {
		Log(LogLevel::Error, "Wrong technique");
		break;
	}
	}

	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->graphicsSemaphores[_swapchainIndex];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;

	// As above we must present using the same VkQueue as submitted to previously
	_deviceResources->graphicsQueue->present(presentInfo);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->graphicsQueue, _deviceResources->commandPool, _deviceResources->swapchain, _swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	_frameId = (_frameId + 1) % _swapchainLength;

	return pvr::Result::Success;
}

void VulkanSupernova::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::ActionClose: {
		exitShell();
		break;
	}
	case pvr::SimplifiedInput::Action1: {
		_inputIndex++;
		changeCurrentTechnique();
		break;
	}
	default: {
		_currentTechnique = SupernovaTechnique::SUPERNOVA_V1_MODE_1X;
		break;
	}
	}
}

void VulkanSupernova::changeCurrentTechnique()
{
	_inputIndex = _inputIndex % static_cast<uint32_t>(SupernovaTechnique::SUPERNOVA_SIZE);
	_currentTechnique = static_cast<SupernovaTechnique>(_inputIndex);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its Shell object defining the behaviour of the application.</summary>
/// <returns>Return an unique_ptr to a new Demo class,supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanSupernova>(); }
