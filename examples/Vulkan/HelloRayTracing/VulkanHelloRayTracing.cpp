/*!
\brief [todo] Shows how to setup vulkan for ray tracing a textured triangle, with a simplified approach of what is usually done for larger and more complex scenes
\file VulkanHelloRayTracing.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include <vector>
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "vulkan/vulkan_beta.h"
#include "glm/glm.hpp"
#include "PVRUtils/Vulkan/AccelerationStructure.h"

/// <summary>Struct used to store all the material information that will be needed in the material buffer _deviceResources::materialBuffer
/// used in the ray hit shader(raytrace.rchit), built in buildMaterialBuffer(). In this sample there's only one field in the struct, which is
/// for the texture index the material has, which is the texture sampled for the triangle geometry being ray traced.</summary>
struct Material
{
	/// <summary>Texture index.</summary>
	int textureID = -1;
};

/// <summary>Struct used to store all the camera information that will be needed in the camera buffer _deviceResources::cameraBuffer used in the
/// ray generation shader (raytrace.rgen), built in buildCameraBuffer(). Only the inverse of the view and the projection matrices are needed, to generate
/// the origin and the directions of the rays.</summary>
struct CameraData
{
	/// <summary>Default constructor.</summary>
	CameraData() : viewMatrixInverse(glm::mat4(1.0f)), projectionMatrixInverse(glm::mat4(1.0f)) {}

	/// <summary>Inverse of the view matrix.</summary>
	glm::mat4 viewMatrixInverse;

	/// <summary>Inverse of the projection matrix.</summary>
	glm::mat4 projectionMatrixInverse;
};

/// <summary>Struct used to encapsulate all the resources and information for each texture to be ray traced.</summary>
struct TextureAS
{
	/// <summary>Texture image.</summary>
	pvrvk::Image image;

	/// <summary>Texture image view.</summary>
	pvrvk::ImageView imageView;

	/// <summary>Texture sampler.</summary>
	pvrvk::Sampler sampler;

	/// <summary>Texture information (tiling, format, etc).</summary>
	pvrvk::DescriptorImageInfo imageInfo;
};

struct DeviceResources
{
	/// <summary>Encapsulation of a Vulkan instance.</summary>
	pvrvk::Instance instance;

	/// <summary>Encapsulation of a Vulkan logical device.</summary>
	pvrvk::Device device;

	/// <summary>Callbacks and messengers for debug messages.</summary>
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;

	/// <summary>vma allocator, only used to build the swapchain.</summary>
	pvr::utils::vma::Allocator vmaAllocator;

	/// <summary>Encapsulation of a Vulkan swapchain.</summary>
	pvrvk::Swapchain swapchain;

	/// <summary>One framebuffer per swapchain.</summary>
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

	/// <summary>Queue where to submit commands.</summary>
	pvrvk::Queue queue;

	/// <summary>Command ppol to allocate command buffers.</summary>
	pvrvk::CommandPool commandPool;

	/// <summary>Array of command buffers, one per swapchain.</summary>
	std::vector<pvrvk::CommandBuffer> cmdBuffers;

	/// <summary>Semaphores for when acquiring the next image from the swap chain, one per swapchain image.</summary>
	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	/// <summary>Semaphores for when submitting the command buffer for the current swapchain image.</summary>
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	/// <summary>Fences for each of the per-frame command buffers, one per swapchain image.</summary>
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	/// <summary>Offscreen ray tracing render target, image resouce.</summary>
	pvrvk::Image renderImages[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	/// <summary>Offscreen ray tracing render target, image view resouce.</summary>
	pvrvk::ImageView renderImageViews[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	/// <summary>This buffer will contain the vertex data for the geometry to be ray traced, in this case, just the three vertices of the triangle.</summary>
	pvrvk::Buffer vertexBuffer;

	/// <summary>This buffer will contain the indices of the geometry to be ray traced, in this case just the three indices of thr triangle.</summary>
	pvrvk::Buffer indexBuffer;

	/// <summary>This buffer will contain all the materials information. In this sample, just one material that has only one field (struct Material).</summary>
	pvrvk::Buffer materialBuffer;

	/// <summary>This buffer will contain the indices to map each hit triangle's material index to the actual material information.</summary>
	pvrvk::Buffer materialIndexBuffer;

	pvrvk::Buffer bottomLevelBuffer;

	/// <summary>Texture used for the geometry to be ray traced.</summary>
	TextureAS materialTexture;

	/// <summary>Device-Host of the _camera matrices.</summary>
	pvrvk::Buffer cameraBuffer;

	/// <summary>Device buffer of the scene element instances.</summary>
	pvrvk::Buffer sceneDescription;

	/// <summary>Descriptor pool to allocate the descriptor sets.</summary>
	pvrvk::DescriptorPool descriptorPool;

	/// <summary>One of the two descriptor set layouts for the ray tracing pass.</summary>
	pvrvk::DescriptorSetLayout descSetLayout;

	/// <summary>One of the two descriptor set layouts for the ray tracing pass.</summary>
	pvrvk::DescriptorSetLayout descSetLayoutRT;

	/// <summary>One of the two descriptor sets used in the ray tracing pass.</summary>
	pvrvk::DescriptorSet descriptorSet;

	/// <summary>One of the two descriptor sets used in the ray tracing pass.</summary>
	pvrvk::DescriptorSet descriptorSetRTs[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	/// <summary>UIRenderer used to display text.</summary>
	pvr::ui::UIRenderer uiRenderer;

	/// <summary>The pipeline layout for the ray tracing pipeline.</summary>
	pvrvk::PipelineLayout pipelineLayoutRT;

	/// <summary>Pipeline used in the offscreen ray tracing pass.</summary>
	pvrvk::RaytracingPipeline pipelineRT;

	/// <summary>GPU buffer where to store the shader binding table.</summary>
	pvrvk::Buffer shaderBindingTable;

	/// <summary>Top level acceleration structure.</summary>
	pvrvk::AccelerationStructure _tlas;

	/// <summary>Vector with all the bottom level acceleration structures used to generate the whole bottom acceleration structure.</summary>
	pvrvk::AccelerationStructure _blas;

	/// <summary>Bottom level information about the geometries in the acceleration structure representing scene elements, in this case, just one element.</summary>
	pvr::utils::RTModelInfo _rtModelInfo;

	/// <summary>Top level information about the single instance in the scene.</summary>
	pvr::utils::RTInstance _instance;

	/// <summary>Top level information about the single instance in the scene for the scene descriptor buffer used.</summary>
	pvr::utils::SceneDescription _sceneDescription;

	/// <summary>Default destructor.</summary>
	~DeviceResources()
	{
		if (device) { device->waitIdle(); }

		if (swapchain)
		{
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanHelloRayTracing : public pvr::Shell
{
	/// <summary>Pointer to struct encapsulating all the resources made with the current logical device.</summary>
	std::unique_ptr<DeviceResources> _deviceResources;

	/// <summary>Swapchain image index, in interval [0, numSwapChainImages - 1].</summary>
	uint32_t _frameId;

	/// <summary>Ray Tracing properties struct holding important information like the size of a sahder group for the Shader Binding Table.</summary>
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR _rtProperties;

	/// <summary>Number of shader groups, three in this sample: Ray generation, ray miss and ray hit.</summary>
	uint32_t _shaderGroupCount;

	/// <summary>Struct holding the inverse of the camera view and projection matrices, needed in a device buffer for the ray generation shader.</summary>
	CameraData _camera;

	/// <summary>Platform agnostic command line argument parser.</summary>
	pvr::CommandLine _cmdLine{};

	/// <summary>format of the texture used to render the offscreen ray tracing pass results to.</summary>
	pvrvk::Format _renderImageFormat;

public:
	/// <summary>Default constructor.</summary>
	VulkanHelloRayTracing() : _frameId(0), _rtProperties({}), _shaderGroupCount(0) {}

	/// <summary>This event represents application start.When implementing, return a suitable error code to signify failure. If pvr::Result::Success
	/// is not returned, the Shell will detect that, clean up, and exit. It will be fired once, on start, before any other callback and before
	/// Graphics Context aquisition.It is suitable to do per - run initialisation, load assets files and similar tasks. A context does not exist yet,
	/// hence if the user tries to create API objects, they will fail and the behaviour is undefined.</summary>
	/// <returns>When implementing, return a suitable error code to signify failure. If pvr::Result::Success is not
	/// returned , the Shell will detect that, clean up, and exit.</returns>
	virtual pvr::Result initApplication();

	/// <summary>Allow the user to set through command line the format of the offscreen texture where the results of the ray tracing
	/// pass are stored. Command line format options are R8G8B8A8_SRGB, B8G8R8A8_UNORM and B8G8R8A8_SRGB.
	/// Use as command line options one of the following to specify the format (if the format does not support optimal tiling and
	/// image store operations, it will be discarded and the final format picked will default to R8G8B8A8_UNORM):
	/// -offscreenTextureFormat=R8G8B8A8_SRGB
	/// -offscreenTextureFormat=B8G8R8A8_UNORM
	/// -offscreenTextureFormat=B8G8R8A8_SRGB</summary>
	/// <param name="physicalDevice">Physical device used for the test.</param>
	void setOffscreenRTTextureFormat(pvrvk::PhysicalDevice physicalDevice);

	/// <summary> If pvr::Result::Success is not returned, the Shell will detect that, clean up, and exit. This function will be fired once after
	/// every time the main Graphics Context (the one the Application Window is using) is initialized. This is usually once per application run,
	/// but in some cases (context lost) it may be called more than once. If the context is lost, the releaseView() callback will be fired,
	/// and if it is reaquired this function will be called again. This callback is suitable to do all do-once tasks that require a graphics context,
	/// such as creating an On-Screen Framebuffer, and for simple applications creating the graphics objects.</summary>
	/// <returns>Return a suitable error code to signify failure. If pvr::Result::Success is not
	/// returned , the Shell will detect that, clean up, and exit.</returns>
	virtual pvr::Result initView();

	/// <summary>Builds the device and gets access to the queues specified in queuePopulateInfo, adding here all the extensions
	/// needed in the sample including those neccessary for Ray Tracing.</summary>
	/// <param name="physicalDevice">Physical device.</param>
	/// <param name="queuePopulateInfo">Information for the type of GPU queues to gather information about for later submission of commands.</param>
	/// <param name="queueAccessInfo">Helper struct with information for GPU queue family and queue ID access.</param>
	/// <param name="vectorExtensionNames">Vector with the names of the extensions needed.</param>
	/// <returns>pvr::Result Success if initialisation succeeded, pvr::Result UnsupportedRequest if the extensions are not
	/// supported by the physical device.</returns>
	pvr::Result buildDeviceAndQueues(pvrvk::PhysicalDevice physicalDevice, pvr::utils::QueuePopulateInfo* queuePopulateInfo, pvr::utils::QueueAccessInfo& queueAccessInfo,
		std::vector<std::string> vectorExtensionNames);

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

	/// <summary>Builds the image _deviceResources::renderImages where to store the Ray Tracing offscreen pass and its corresponding
	/// image view _deviceResources::renderImageViewfor.</summary>
	void buildOffscreenRenderImage();

	/// <summary>Builds a vertex buffer with the geometry to be ray traced, in this case three vertices for the triangle.</summary>
	void buildVertexBuffer();

	/// <summary>Builds an index buffer with the geometry to be ray traced, in this case three indices for the triangle.</summary>
	void buildIndexBuffer();

	/// <summary>Builds a material buffer to know the associated texture to sample for each ray tracing hit when ray tracing the triangle geometry.</summary>
	void buildMaterialBuffer();

	/// <summary>Builds a buffer to know, for each triangle, where is the material associated for that triangle.</summary>
	void buildMaterialIndexBuffer();

	/// <summary>Builds the imageview and sampler for the texture to be sampled in the triangle geometry to be ray traced.</summary>
	void buildMaterialTexture();

	/// <summary>Fills the member variables _rtModelInfo, _instance and _sceneDescription used for the top level and bottom
	/// level acceleration structures needed. Information about the single geometry mesh and scene element to be ray traced is filled here.</summary>
	/// <param name="vertexBuffer">Buffer with the triangle vertex information.</param>
	/// <param name="indexBuffer">Buffer with the indices of the triangle geometry.</param>
	/// <param name="verticesSize">Array with the amount of vertices of the geometry for this scene model.</param>
	/// <param name="indicesSize">Array with the amount of indices of the geometry for this scene model.</param>
	void buildASModelDescription(pvrvk::Buffer vertexBuffer, pvrvk::Buffer indexBuffer, int verticesSize, int indicesSize);

	/// <summary>Builds the acceleration structures _tlas and _blas (both the top and the bottom level ones).</summary>
	/// <param name="buildASFlags">Build options for the acceleration structure. NOTE: Some flags are not implemented yet
	/// like e_ALLOW_COMPACTION_BIT_KHR, currently intended use is e_PREFER_FAST_TRACE_BIT_KHR.</param>
	void buildAS(pvrvk::BuildAccelerationStructureFlagsKHR buildASFlags = pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR);

	/// <summary>Builds all the information needed in _deviceResources::_blas to generate the geometry that will be ray traced, which is
	/// the so called bottom level acceleration structure, starting from the index and vertex buffers generated in buildVertexBuffer and buildIndexBuffer.</summary>
	void buildBottomLevelASModel();

	/// <summary>Builds the top level acceleration structure, consisting in the TLAS Vulkan handle, and the transforms and some flags per scene element,
	/// called "instance", to be ray traced.</summary>
	/// <param name="flags">Flags for the acceleration struct to be built.</param>
	void buildTopLevelASAndInstances(pvrvk::BuildAccelerationStructureFlagsKHR flags);

	/// <summary>Helper function to convert information from elements in _deviceResources::_instance to VkAccelerationStructureInstanceKHR
	/// equivalents, which comprise the instances buffer to be used for the top level acceleration structure, in this case just a single element
	/// is added since in this example only a single scene element (a triangle) is considered.</summary>
	/// <param name="geometryInstances">Vector where to store the VkAccelerationStructureInstanceKHR elements generated.</param>
	void setupGeometryInstances(std::vector<VkAccelerationStructureInstanceKHR>& geometryInstances);

	/// <summary>Sets the camera matrices needed for the ray generation. Inverse view and projection matrices are needed
	/// for the ray generation. The results are stored in the _camera member variable, which is used later to generate
	/// a GPU buffer to make this data available in the raytrace.rtgen shader.</summary>
	void fillCameraData();

	/// <summary>Builds a GPU buffer with room for a single CameraData element, holding the data set in fillCameraData()
	/// in the _camera member variable.</summary>
	void buildCameraBuffer();

	/// <summary>Builds a buffer (_deviceResources::sceneDescription) with the information present in _deviceResources::_sceneDescription
	/// regarding all the instances in the scene. In this example, only once instance of the bottom level acceleration
	/// structure mesh is present (a triangle geometry). This buffer is used in the raytrace.rchit to recover the
	/// object id of the triangle hit by the ray.</summary>
	void buildSceneDescriptionBuffer();

	/// <summary>Builds the descriptor pool to generate the two descriptor sets used in this sample.</summary>
	void buildDescriptorPool();

	/// <summary>Builds the descriptor set layout for the _deviceResources::descriptorSet descriptor set, which comprises all
	/// the resources are needed in the raytrace.rgen and raytrace.rchit shaders to generate the rays to be traced and act
	/// accordingly when the scene triangle is hit.</summary>
	void buildDescriptorSetLayout();

	/// <summary>Allocate the descriptor set _deviceResources::descriptorSet which comprises the resources needed for the ray generation and ray
	/// hit shaders.</summary>
	void buildDescriptorSet();

	/// <summary>Build the descriptor set layout _deviceResources::descSetLayoutRT used to trace rays and store the final color in the offscreen image for each
	/// texel when the offscreen ray trace pass is performed.</summary>
	void buildRayTracingDescriptorSetLayout();

	/// <summary>Allocate the descriptor set _deviceResources::descriptorSetRT which comprises the acceleration structure needed in the raygen.rgen shader and
	/// the image where to store the results of the ray tracing offscreen pass.</summary>
	void buildRayTracingDescriptorSets();

	/// <summary>Build the pipeline used for the offscreen ray tracing pass, using the ray tracing shaders for ray generation,
	/// ray hit and ray miss, and the descriptor sets _deviceResources::descSetLayoutRT and _deviceResources::descSetLayout.</summary>
	void buildRayTracingPipeline();

	/// <summary>Builds the shader binding table, used to know what shaders to call depending on what happens with the rays to be traced in the ray tracing pass.
	/// This way, the shader to be called for ray generation can be specified (the .rgen shader), and the shader to be called if a ray hits
	/// geometry (the .rchit shader), and as well the shader to be called if a ray hits no geometry (the .rmiss shader). In more general cases where more
	/// scene elements and hit shaders are present, this shader binding table helps point to the correct hit shader to be called depending on the
	/// instance whose geometry is hit by a ray.</summary>
	void buildShaderBindingTable();

	/// <summary>Record for each swapchain image the ray tracing commands, the copy from the texture where the results of the
	/// Ray Tracing pass are stored (during an offscreen pass) to the corresponding swapchain image for display, and the UI.</summary>
	void recordCommandBuffer();

	/// <summary>Trace rays, here the ray tracing pipeline _deviceResources::pipelineRT and the two descriptor sets _deviceResources::descriptorSetRTs
	/// and _deviceResources::descriptorSet are used, together with a set of four structs reproducing the information in the Shader Binding Table
	/// (the shader groups and its sizes).</summary>
	/// <param name="cmdBuf">Command buffer to record to the Ray Tracing commands.</param>
	/// <param name="imageIndex">Swap chain image index to index into _deviceResources::descriptorSetRTs.</param>
	void raytrace(const pvrvk::CommandBuffer& cmdBuf, uint32_t imageIndex);

	/// <summary>Copy the results of the Ray Tracing offscreen pass stored in _deviceResources::renderImages to the swapchain image with
	/// index given by the imageIndex parameter.</summary>
	/// <param name="cmdBuf">Command buffer to record all the required commands.</param>
	/// <param name="imageIndex">Swap chain image index to copy to the contents of _deviceResources::renderImage.</param>
	void recordRenderImageCopy(pvrvk::CommandBuffer& cmdBuf, uint32_t imageIndex);

	/// <summary>Utility function to allocate a new command buffer and start recording, returning it.</summary>
	/// <returns>Return the allocated command buffer.</returns>
	pvrvk::CommandBuffer beginCommandBuffer();

	/// <summary>Utility function to finish recording and submit a command buffer.</summary>
	/// <param name="commandBuffer">Command buffer to submit.</param>
	void endAndSubmitCommandBuffer(pvrvk::CommandBuffer commandBuffer);

	/// <summary>Record UIRenderer commands.</summary>
	/// <param name="commandBuff">Commandbuffer to record.</param>
	void recordCommandUIRenderer(pvrvk::CommandBuffer& cmdBuffers);
};

pvr::Result VulkanHelloRayTracing::initApplication()
{
	_cmdLine = this->getCommandLine();
	return pvr::Result::Success;
}

void VulkanHelloRayTracing::setOffscreenRTTextureFormat(pvrvk::PhysicalDevice physicalDevice)
{
	pvrvk::Format tempFormat = pvrvk::Format::e_MAX_ENUM;
	std::string textureFormat;
	
	if (_cmdLine.hasOption("-offscreenTextureFormat"))
	{
		bool stringOptionResult = _cmdLine.getStringOption("-offscreenTextureFormat", textureFormat);

		if (stringOptionResult)
		{
			if (textureFormat == "R8G8B8A8_SRGB")
			{
				tempFormat  = pvrvk::Format::e_R8G8B8A8_SRGB;
			}
			else if (textureFormat == "B8G8R8A8_UNORM")
			{
				tempFormat = pvrvk::Format::e_B8G8R8A8_UNORM;
			}
			else if (textureFormat == "B8G8R8A8_SRGB")
			{
				tempFormat = pvrvk::Format::e_B8G8R8A8_SRGB;
			}
			else
			{
				Log(LogLevel::Warning, "Format chosen for the offscreen render target %s not recognized, options are R8G8B8A8_SRGB, B8G8R8A8_UNORM and B8G8R8A8_SRGB. Fallback format R8G8B8A8_UNORM will be used.", textureFormat.c_str());
			}
		}
	}
	else
	{
		Log(LogLevel::Information, "No offscreen render target format specified, using default R8G8B8A8_UNORM. Use -offscreenTextureFormat=Format with Format either R8G8B8A8_SRGB, B8G8R8A8_UNORM or B8G8R8A8_SRGB to specify it.");
	}

	if (tempFormat != pvrvk::Format::e_MAX_ENUM)
	{
		if (pvr::utils::formatWithTilingSupportsFeatureFlags(
			tempFormat, pvrvk::ImageTiling::e_OPTIMAL, pvrvk::FormatFeatureFlags::e_STORAGE_IMAGE_BIT, _deviceResources->instance, physicalDevice))
		{
			_renderImageFormat = tempFormat;
		}
		else
		{
			Log(LogLevel::Warning, "Format chosen for the offscreen render target %s does not support image store feature (VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) for image optimal tiling, fallback format R8G8B8A8_UNORM will be used.", textureFormat.c_str());
		}
	}
}

pvr::Result VulkanHelloRayTracing::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create instance and retrieve compatible physical devices
	_deviceResources->instance =
		pvr::utils::createInstance(this->getApplicationName(), pvr::utils::VulkanVersion(1, 1), pvr::utils::InstanceExtensions(), pvr::utils::InstanceLayers(true));

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// One important step is the device extensions that are required. Some of them follow the latest Vulkan approach, building a linked list of
	// structs through the pNext field in all of them. This linked list is built in this SDK through the addExtensionFeature() method and used in the
	// Device constructor (Device_::Device_), being assigned to the pNext VkDeviceCreateInfo struct used to create the device in the call to vkCreateDevice
	// Note that the extensions needed for this Ray Tracing sample are:
	//
	// VK_KHR_RAY_TRACING_EXTENSION_NAME:              Allows the use of all the Vulkan API calls from the Ray Tracing extension.
	//
	// VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME:    Allows to obtain the address of a GPU buffer (device) through the call to vkGetBufferDeviceAddress,
	//                                                 needed for many of the operations to setup bottom and top level acceleration structures.
	//
	// VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME:      Modifies the alignment rules for uniform buffers, storage buffers and push constants, allowing non-scalar
	//                                                 types to be aligned solely based on the size of their components, without additional requirements.
	//
	// VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME:      Allows to bind all textures at once as an unsized array, and later in the shader to index into any
	//                                                 of those textures. This is due to the fact that, when a ray hits a triangle, we don't knw oin advance what
	//                                                 textures will be assigned to the material assigned to tha triangle, meaning any ray could access any
	//                                                 texture in a single ray trace pass.
	//
	// VK_KHR_MAINTENANCE3_EXTENSION_NAME:             Adds detail to the limits of some functionalities, like the maximum number of descriptors supported in a single
	//                                                 descritpr set layout (some implementations only have a limit for the total size of descriptors). Also adds a
	//                                                 limit to the maximum size of a memory allocation, being this sometimes limited by the kernel in some platforms.
	//
	// VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME:         Allows a special pipeline that defines shaders / shader groups that can be linked into other pipelines
	//                                                 (a "pipeline library" is a special pipeline that cannot be bound, instead it defines a set of shaders and
	//                                                 shader groups which can be linked into other pipelines.)
	//
	// VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME: Defines the infrastructure and usage patterns for deferrable commands, but does not specify
	//                                                 any commands as deferrable. This is left to additional dependant extensions (more information in
	//                                                 https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#deferred-host-operations-requesting)

	std::vector<std::string> vectorExtensionNames{ VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, VK_KHR_SPIRV_1_4_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME };

	std::vector<int> vectorPhysicalDevicesIndex = pvr::utils::validatePhysicalDeviceExtensions(_deviceResources->instance, vectorExtensionNames);

	if (vectorPhysicalDevicesIndex.size() == 0)
	{
		throw pvrvk::ErrorInitializationFailed("Could not find all the required Vulkan extensions.");
		return pvr::Result::UnsupportedRequest;
	}

	// Cache the selected physical device
	pvrvk::PhysicalDevice physicalDevice = _deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0]);

	// Create the surface
	pvrvk::Surface surface = pvr::utils::createSurface(_deviceResources->instance, physicalDevice, this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	// create device and queues
	pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;
	pvr::Result resultDeviceAndQueues = buildDeviceAndQueues(physicalDevice, &queuePopulateInfo, queueAccessInfo, vectorExtensionNames);

	if (resultDeviceAndQueues != pvr::Result::Success) { return resultDeviceAndQueues; }

	// get queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	// create vulkan memory allocatortea
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, swapchainImageUsage | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		// Add screenshot support if is supported.
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// The swapchain image will be blitted to, so there are extra image flags that need to be supported.
	pvr::utils::CreateSwapchainParameters swapchainCreationPreferences = pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator);
	swapchainCreationPreferences.setColorImageUsageFlags(swapchainImageUsage);
	swapchainCreationPreferences.colorLoadOp = pvrvk::AttachmentLoadOp::e_DONT_CARE;

	// Create the swapchain, on screen framebuffers
	auto swapchainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(), swapchainCreationPreferences);

	_deviceResources->swapchain = swapchainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapchainCreateOutput.framebuffer;

	// Create command pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	_deviceResources->commandPool->setObjectName("Main Command Pool");

	// create the per swapchain command buffers and syncronization objects
	_deviceResources->cmdBuffers.resize(_deviceResources->swapchain->getSwapchainLength());
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->cmdBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->cmdBuffers[i]->setObjectName(std::string("Main CommandBuffer [") + std::to_string(i) + "]");

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i]->setObjectName(std::string("Presentation Semaphore [") + std::to_string(i) + "]");
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName(std::string("Image Acquisition Semaphore [") + std::to_string(i) + "]");
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameResourcesFences[i]->setObjectName(std::string("Per Frame Command Buffer Fence [") + std::to_string(i) + "]");
	}

	// Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("Hello Ray Tracing");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	// Get ray tracing properties
	_rtProperties.sType = static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR);
	_rtProperties.pNext = nullptr;
	VkPhysicalDeviceProperties2 properties{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_PROPERTIES_2) };
	properties.pNext = &_rtProperties;
	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceProperties2(_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0])->getVkHandle(), &properties);

	_renderImageFormat = pvrvk::Format::e_R8G8B8A8_UNORM;
	setOffscreenRTTextureFormat(physicalDevice);

	// Setup ray tracing resources
	buildOffscreenRenderImage();
	buildVertexBuffer();
	buildIndexBuffer();
	buildMaterialBuffer();
	buildMaterialIndexBuffer();
	buildMaterialTexture();
	buildASModelDescription(_deviceResources->vertexBuffer, _deviceResources->indexBuffer, 3, 3);
	buildAS();
	fillCameraData();
	buildCameraBuffer();
	buildSceneDescriptionBuffer();
	buildDescriptorPool();
	buildDescriptorSetLayout();
	buildDescriptorSet();
	buildRayTracingDescriptorSetLayout();
	buildRayTracingDescriptorSets();
	buildRayTracingPipeline();
	buildShaderBindingTable();

	// Record the command buffer for Ray Tracing
	recordCommandBuffer();

	return pvr::Result::Success;
}

pvr::Result VulkanHelloRayTracing::buildDeviceAndQueues(pvrvk::PhysicalDevice physicalDevice, pvr::utils::QueuePopulateInfo* queuePopulateInfo,
	pvr::utils::QueueAccessInfo& queueAccessInfo, std::vector<std::string> vectorExtensionNames)
{
	pvr::utils::DeviceExtensions deviceExtensions = pvr::utils::DeviceExtensions();

	for (const std::string& extensionName : vectorExtensionNames) { deviceExtensions.addExtension(extensionName); }

	// Get the physical device features for all of the raytracing extensions through a continual pNext chain
	VkPhysicalDeviceFeatures2 deviceFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_FEATURES_2) };

	// Raytracing Pipeline Features
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingPipelineFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR) };
	deviceFeatures.pNext = &raytracingPipelineFeatures;

	// Acceleration Structure Features
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{ static_cast<VkStructureType>(
		pvrvk::StructureType::e_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR) };
	raytracingPipelineFeatures.pNext = &accelerationStructureFeatures;

	// Device Address Features
	VkPhysicalDeviceBufferDeviceAddressFeatures deviceBufferAddressFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES) };
	accelerationStructureFeatures.pNext = &deviceBufferAddressFeatures;

	// Scalar Block Layout Features
	VkPhysicalDeviceScalarBlockLayoutFeaturesEXT scalarFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES) };
	deviceBufferAddressFeatures.pNext = &scalarFeatures;

	// Descriptor Indexing Features
	VkPhysicalDeviceDescriptorIndexingFeatures indexFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES) };
	scalarFeatures.pNext = &indexFeatures;

	// Fill in all of these device features with one call
	physicalDevice->getInstance()->getVkBindings().vkGetPhysicalDeviceFeatures2(physicalDevice->getVkHandle(), &deviceFeatures);

	// Add these device features to the physical device, since they're all connected by a pNext chain, we only need to explicitly attach the top feature
	deviceExtensions.addExtensionFeatureVk<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&raytracingPipelineFeatures);

	// create the device
	_deviceResources->device = pvr::utils::createDeviceAndQueues(physicalDevice, queuePopulateInfo, 1, &queueAccessInfo, deviceExtensions);
	return pvr::Result::Success;
}

pvr::Result VulkanHelloRayTracing::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

pvr::Result VulkanHelloRayTracing::quitApplication() { return pvr::Result::Success; }

pvr::Result VulkanHelloRayTracing::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	// submit
	pvr::utils::beginQueueDebugLabel(_deviceResources->queue, pvrvk::DebugUtilsLabel("Submitting per frame command buffers"));
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// present
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();
	return pvr::Result::Success;
}

std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanHelloRayTracing>(); }

void VulkanHelloRayTracing::buildOffscreenRenderImage()
{
	// Build a set of new images _deviceResources::renderImages where to store the Ray Tracing offscreen pass

	pvrvk::ImageCreateInfo imageInfo;
	imageInfo.setImageType(pvrvk::ImageType::e_2D);
	imageInfo.setFormat(_renderImageFormat);
	imageInfo.setExtent(pvrvk::Extent3D(getWidth(), getHeight(), 1));
	imageInfo.setTiling(pvrvk::ImageTiling::e_OPTIMAL);
	imageInfo.setUsageFlags(
		pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT);
	imageInfo.setSharingMode(pvrvk::SharingMode::e_EXCLUSIVE);

	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->renderImages[i] = pvr::utils::createImage(_deviceResources->device, imageInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

		pvr::utils::setImageLayout(_deviceResources->renderImages[i], pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_GENERAL, uploadCmd);

		// Build an image view _deviceResources::renderImageViewfor the newly built image
		pvrvk::ImageViewCreateInfo imageViewInfo;
		imageViewInfo.setFormat(_renderImageFormat);
		imageViewInfo.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		imageViewInfo.setImage(_deviceResources->renderImages[i]);
		_deviceResources->renderImageViews[i] = _deviceResources->device->createImageView(imageViewInfo);
	}
	
	endAndSubmitCommandBuffer(uploadCmd);
}

void VulkanHelloRayTracing::buildVertexBuffer()
{
	// Build a vertex buffer consisting on a simple triangle, so only three vertices are needed. The information is set filling the
	// fields in a ASVertexFormat array. The only important data for this demo is stored in the position and texture coordinate fields.
	std::vector<pvr::utils::ASVertexFormat> vertices(3);

	vertices[0].pos = glm::vec3(-0.3f, -0.4f, -0.64f);
	vertices[0].nrm = glm::vec3(0.0f, 0.0f, 1.0f);
	vertices[0].texCoord = glm::vec2(0.0f, 0.0f);
	vertices[0].tangent = glm::vec3(0.0f, 1.0f, 0.0f);

	vertices[1].pos = glm::vec3(0.3f, -0.4f, -0.64f);
	vertices[1].nrm = glm::vec3(0.0f, 0.0f, 1.0f);
	vertices[1].texCoord = glm::vec2(0.0f, 1.0f);
	vertices[1].tangent = glm::vec3(0.0f, 1.0f, 0.0f);

	vertices[2].pos = glm::vec3(0.0f, 0.4f, -0.64f);
	vertices[2].nrm = glm::vec3(0.0f, 0.0f, 1.0f);
	vertices[2].texCoord = glm::vec2(1.0f, 1.0f);
	vertices[2].tangent = glm::vec3(0.0f, 1.0f, 0.0f);

	// The vertex buffer memory needs to have one particular flag, e_SHADER_DEVICE_ADDRESS_BIT, needed to retrieve the buffer address
	// through vkGetBufferDeviceAddress API call, to be able to access this buffer's information from a shader.
	pvrvk::BufferCreateInfo vertexBufferInfo;
	pvrvk::DeviceSize vertexBufferSize =
		sizeof(pvr::utils::ASVertexFormat) * vertices.size(); // The buffer size is the size of the ASVertexFormat struct multiplied by the amount of vertices, three in this case
	vertexBufferInfo.setSize(vertexBufferSize);
	pvrvk::BufferUsageFlags vertexBufferUsageFlags = pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT |
		pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT | pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT |
		pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	vertexBufferInfo.setUsageFlags(vertexBufferUsageFlags);

	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();

	// The generated pvrvk::Buffer buffer is assigned to DeviceResources::vertexBuffer since it'll be needed later to build the
	// bottom level acceleration structure.
	_deviceResources->vertexBuffer = pvr::utils::createBuffer(_deviceResources->device, vertexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
		pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->vertexBuffer, uploadCmd, vertices.data(), 0, vertexBufferSize);

	endAndSubmitCommandBuffer(uploadCmd);
}

void VulkanHelloRayTracing::buildIndexBuffer()
{
	// Build an index buffer consisting on a simple triangle, so only three indices are needed.
	std::vector<uint32_t> indices = { 0, 1, 2 };

	// The vertex buffer memory needs to have one particular flag, e_SHADER_DEVICE_ADDRESS_BIT, needed to retrieve the buffer address
	// through vkGetBufferDeviceAddress API call, to be able to access this buffer's information from a shader.
	pvrvk::BufferCreateInfo indexBufferInfo;
	pvrvk::DeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();
	indexBufferInfo.setSize(indexBufferSize);
	pvrvk::BufferUsageFlags indexBufferUsageFlags = pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT |
		pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT | pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT |
		pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	indexBufferInfo.setUsageFlags(indexBufferUsageFlags);

	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();

	// The generated pvrvk::Buffer buffer is assigned to DeviceResources::indexBuffer since it'll be needed later to build the
	// bottom level acceleration structure.
	_deviceResources->indexBuffer = pvr::utils::createBuffer(_deviceResources->device, indexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
		pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->indexBuffer, uploadCmd, indices.data(), 0, indexBufferSize);

	endAndSubmitCommandBuffer(uploadCmd);
}

void VulkanHelloRayTracing::buildMaterialBuffer()
{
	// Build a buffer containing the material information. For this example only one material struct is added, and the struct
	// only contains a texture id used by this material. The strcut can be extended to store more information.
	Material material = { 0 };

	// Unlike the vertex and index buffers built at buildVertexBuffer and buildIndexBuffer respectively, no buffer address is needed
	// for this storage buffer so the flag e_SHADER_DEVICE_ADDRESS_BIT is not needed.
	pvrvk::BufferCreateInfo materialColorBufferInfo;
	pvrvk::DeviceSize materialBufferSize = sizeof(Material);
	materialColorBufferInfo.setSize(sizeof(Material));
	materialColorBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);

	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();

	// The generated pvrvk::Buffer buffer is assigned to DeviceResources::materialBuffer since it'll be needed later to be added to a descriptor set
	_deviceResources->materialBuffer = pvr::utils::createBuffer(_deviceResources->device, materialColorBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT);
	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->materialBuffer, uploadCmd, &material, 0, materialBufferSize);

	endAndSubmitCommandBuffer(uploadCmd);
}

void VulkanHelloRayTracing::buildMaterialIndexBuffer()
{
	// Build a buffer containing indices to know what is the material associated with a triangle when there's a ray hitting that triangle.
	uint32_t materialIndex = 0;

	// Unlike the vertex and index buffers built at buildVertexBuffer and buildIndexBuffer respectively, no buffer address is needed
	// for this storage buffer so the flag e_SHADER_DEVICE_ADDRESS_BIT is not needed.
	pvrvk::BufferCreateInfo materialIndexBufferInfo;
	pvrvk::DeviceSize materialIndexSize = sizeof(materialIndex);
	materialIndexBufferInfo.setSize(materialIndexSize);
	materialIndexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);

	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();

	// The generated pvrvk::Buffer buffer is assigned to DeviceResources::materialIndexBuffer since it'll be needed later to be added to a descriptor set
	_deviceResources->materialIndexBuffer = pvr::utils::createBuffer(_deviceResources->device, materialIndexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT);
	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->materialIndexBuffer, uploadCmd, &materialIndex, 0, materialIndexSize);

	endAndSubmitCommandBuffer(uploadCmd);
}

void VulkanHelloRayTracing::buildMaterialTexture()
{
	// Build the imageview and sampler for the texture to be sampled in the triangle geometry to be ray traced
	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();

	// Build image view
	_deviceResources->materialTexture.imageView = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, "albedo.pvr", true, uploadCmd, *this,
		pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, nullptr, nullptr);
	_deviceResources->materialTexture.image = _deviceResources->materialTexture.imageView->getImage();

	// Build sampler
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	_deviceResources->materialTexture.sampler = _deviceResources->device->createSampler(samplerInfo);

	// Store the image view and sampler in _deviceResources::materialTexture for later, needed in the _deviceResources->descriptorSet descriptor set
	_deviceResources->materialTexture.imageInfo.sampler = _deviceResources->materialTexture.sampler;
	_deviceResources->materialTexture.imageInfo.imageView = _deviceResources->materialTexture.imageView;
	_deviceResources->materialTexture.imageInfo.imageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

	endAndSubmitCommandBuffer(uploadCmd);
}

void VulkanHelloRayTracing::buildASModelDescription(pvrvk::Buffer vertexBuffer, pvrvk::Buffer indexBuffer, int verticesSize, int indicesSize)
{
	// Fill information from each bottom level acceleration structure (containing the goemetry to be ray traced of a particular scene element). In this case,
	// only a triangle is considered so all the information needed in _deviceResources::_rtModelInfo, _deviceResources::_instance and
	// _deviceResources::_sceneDescription is referred to this single triangle scene element, both for bottom level (its index and vertex buffer,
	// and the related information like the amount of geometry on those buffers), and for top level (the instance representing a scene element of that bottom level
	// acceleration structure).
	glm::mat4 instanceTransform = glm::mat4(1.0);
	glm::mat4 instanceTransformInverse = glm::transpose(glm::inverse(instanceTransform));

	// This helper variable is used to store all the information for the bottom level acceleration structures. Since only one element
	// is considered in this sample (a simple triangle), only information for that element is stored. The amount of primitives is the amount of indices divided
	// by three since the primitive used are triangles.
	_deviceResources->_rtModelInfo = pvr::utils::RTModelInfo{ vertexBuffer, indexBuffer, static_cast<uint32_t>(indicesSize / 3 + (indicesSize % 3 == 0 ? 0 : 1)),
		static_cast<uint32_t>(verticesSize), sizeof(pvr::utils::ASVertexFormat) };

	// Instance information representing a scene element of the bottom level acceleration struture (i.e., the triangle mesh).
	_deviceResources->_instance = pvr::utils::RTInstance{ 0, 0, 0, 0xFF, pvrvk::GeometryInstanceFlagsKHR::e_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR, instanceTransform };

	// Scene description representing a scene element of the bottom level acceleration struture (i.e., the triangle mesh).
	_deviceResources->_sceneDescription = pvr::utils::SceneDescription{ 0, instanceTransform, instanceTransformInverse };
}

void VulkanHelloRayTracing::buildAS(pvrvk::BuildAccelerationStructureFlagsKHR buildASFlags)
{
	// Build a bottom level acceleration structure for each scene element, in this case just a triangle, store results in _blas
	buildBottomLevelASModel();
	buildTopLevelASAndInstances(buildASFlags);
}

void VulkanHelloRayTracing::buildBottomLevelASModel()
{
	// The addresses of the index and vertex buffers generated in buildVertexBuffer and buildIndexBuffer are required here
	// as part of the information to be provided for the bottom level acceleration structure.
	VkDeviceAddress vertexBufferAddress = _deviceResources->_rtModelInfo.vertexBuffer->getDeviceAddress(_deviceResources->device);
	VkDeviceAddress indexBufferAddress = _deviceResources->_rtModelInfo.indexBuffer->getDeviceAddress(_deviceResources->device);

	VkAccelerationStructureGeometryTrianglesDataKHR accelerationStructureGeometryTrianglesData = { static_cast<VkStructureType>(
		pvrvk::StructureType::e_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR) };
	accelerationStructureGeometryTrianglesData.vertexFormat = static_cast<VkFormat>(pvrvk::Format::e_R32G32B32_SFLOAT);
	accelerationStructureGeometryTrianglesData.vertexData = VkDeviceOrHostAddressConstKHR{ vertexBufferAddress };
	accelerationStructureGeometryTrianglesData.vertexStride = static_cast<VkDeviceSize>(_deviceResources->_rtModelInfo.vertexStride);
	accelerationStructureGeometryTrianglesData.indexType = static_cast<VkIndexType>(pvrvk::IndexType::e_UINT32);
	accelerationStructureGeometryTrianglesData.indexData = VkDeviceOrHostAddressConstKHR{ indexBufferAddress };
	accelerationStructureGeometryTrianglesData.maxVertex = _deviceResources->_rtModelInfo.vertexCount;

	// Fill an acceleration structure geometry info struct with the index and vertex buffer and add it to _deviceResources->_blas
	// together with a description of the geometry format expected for this mesh
	VkAccelerationStructureGeometryKHR accelerationStructureGeometry = { static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_GEOMETRY_KHR) };
	accelerationStructureGeometry.geometryType = static_cast<VkGeometryTypeKHR>(pvrvk::GeometryTypeKHR::e_TRIANGLES_KHR);
	accelerationStructureGeometry.flags = static_cast<VkGeometryFlagBitsKHR>(pvrvk::GeometryFlagsKHR::e_OPAQUE_BIT_KHR);
	accelerationStructureGeometry.geometry.triangles = accelerationStructureGeometryTrianglesData;

	// Get the size requirements for buffers involved in the acceleration structure build process
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.flags = static_cast<VkBuildAccelerationStructureFlagBitsKHR>(pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR);
	accelerationStructureBuildGeometryInfo.sType = static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR);
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationStructureBuildGeometryInfo.mode = static_cast<VkBuildAccelerationStructureModeKHR>(pvrvk::BuildAccelerationStructureModeKHR::e_BUILD_KHR);
	accelerationStructureBuildGeometryInfo.type = static_cast<VkAccelerationStructureTypeKHR>(pvrvk::AccelerationStructureTypeKHR::e_BOTTOM_LEVEL_KHR);
	accelerationStructureBuildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;

	std::vector<uint32_t> maxPrimitiveCount = { _deviceResources->_rtModelInfo.primitiveCount };
	VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfo{ static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR) };
	_deviceResources->device->getVkBindings().vkGetAccelerationStructureBuildSizesKHR(_deviceResources->device->getVkHandle(),
		static_cast<VkAccelerationStructureBuildTypeKHR>(pvrvk::AccelerationStructureBuildTypeKHR::e_DEVICE_KHR), &accelerationStructureBuildGeometryInfo, maxPrimitiveCount.data(),
		&asBuildSizesInfo);

	pvrvk::Buffer blasBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(
			asBuildSizesInfo.accelerationStructureSize, pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE,
		pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	pvrvk::AccelerationStructureCreateInfo accelerationStructureCreateInfo;
	accelerationStructureCreateInfo.setType(pvrvk::AccelerationStructureTypeKHR::e_BOTTOM_LEVEL_KHR);
	accelerationStructureCreateInfo.setSize(asBuildSizesInfo.accelerationStructureSize); // Will be used to allocate memory.
	accelerationStructureCreateInfo.setBuffer(blasBuffer->getVkHandle());

	_deviceResources->_blas = _deviceResources->device->createAccelerationStructure(accelerationStructureCreateInfo, blasBuffer);
	_deviceResources->_blas->setFlags(pvrvk::BuildAccelerationStructureFlagsKHR::e_NONE);

	pvrvk::DeviceSize scratchSize = asBuildSizesInfo.buildScratchSize;

	// A scratch buffer with the size of the biggest bottom level acceleration structure geometry element needs to be built and provided
	// to build the bottom level acceleration structure, in this case, there's only a single element to be ray traced.
	pvrvk::Buffer scratchBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(scratchSize, pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE,
		pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	// Get the address of the scratch buffer.
	VkDeviceAddress scratchAddress = scratchBuffer->getDeviceAddress(_deviceResources->device);

	// Setup two fields in the struct for the acceleration structure building step
	accelerationStructureBuildGeometryInfo.dstAccelerationStructure = _deviceResources->_blas->getVkHandle();
	accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchAddress;

	pvrvk::CommandBuffer commandBuffer = beginCommandBuffer();

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo = { _deviceResources->_rtModelInfo.primitiveCount, 0, 0, 0 };
	std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> vectorAccelerationStructureBuildRangeInfo(1);
	vectorAccelerationStructureBuildRangeInfo[0] = &accelerationStructureBuildRangeInfo;

	_deviceResources->device->getVkBindings().vkCmdBuildAccelerationStructuresKHR(
		commandBuffer->getVkHandle(), 1, &accelerationStructureBuildGeometryInfo, vectorAccelerationStructureBuildRangeInfo.data());

	pvrvk::MemoryBarrierSet barriers;
	barriers.addBarrier(pvrvk::MemoryBarrier(pvrvk::AccessFlags::e_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, pvrvk::AccessFlags::e_ACCELERATION_STRUCTURE_READ_BIT_NV));
	commandBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, pvrvk::PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, barriers);

	endAndSubmitCommandBuffer(commandBuffer);
}

void VulkanHelloRayTracing::buildTopLevelASAndInstances(pvrvk::BuildAccelerationStructureFlagsKHR flags)
{
	// Now, build the information needed by the top level acceleration structure, which is, for each scene element, it's transform and some flags
	std::vector<VkAccelerationStructureInstanceKHR> vectorAccelerationStructureInstances;
	vectorAccelerationStructureInstances.reserve(1); // Just a single instance is considered for the single acceleration structure geometry which is the triangle to be ray traced
	setupGeometryInstances(vectorAccelerationStructureInstances);

	pvrvk::CommandBuffer commandBuffer = beginCommandBuffer();

	// The instance information is put in a buffer
	pvrvk::Buffer instancesBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(sizeof(VkAccelerationStructureInstanceKHR) * vectorAccelerationStructureInstances.size(),
			pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
				pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	pvr::utils::updateHostVisibleBuffer(
		instancesBuffer, vectorAccelerationStructureInstances.data(), 0, sizeof(VkAccelerationStructureInstanceKHR) * vectorAccelerationStructureInstances.size(), true);

	// As with the scratch buffer, the address of the instance buffer is retrieved and will be used to build the top level acceleration structure.
	VkDeviceAddress instanceBufferAddress = instancesBuffer->getDeviceAddress(_deviceResources->device);

	pvrvk::MemoryBarrierSet barriers;
	barriers.addBarrier(pvrvk::MemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_ACCELERATION_STRUCTURE_WRITE_BIT_KHR));
	commandBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, barriers);

	VkAccelerationStructureGeometryInstancesDataKHR accelerationStructureGeometryInstancesData{ static_cast<VkStructureType>(
		pvrvk::StructureType::e_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR) };
	accelerationStructureGeometryInstancesData.arrayOfPointers = VK_FALSE;
	accelerationStructureGeometryInstancesData.data.deviceAddress = instanceBufferAddress;

	VkAccelerationStructureGeometryKHR accelerationStructureGeometryTopLevel{ static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_GEOMETRY_KHR) };
	accelerationStructureGeometryTopLevel.geometryType = static_cast<VkGeometryTypeKHR>(pvrvk::GeometryTypeKHR::e_INSTANCES_KHR);
	accelerationStructureGeometryTopLevel.geometry.instances = accelerationStructureGeometryInstancesData;

	// The top level acceleration structure has the handle to the TLAS, the address to the instances buffer, and the address to the scratch buffer.
	// In this case, only one instance will be ray traced, which is a triangle.
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryTopLevel{ static_cast<VkStructureType>(
		pvrvk::StructureType::e_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR) };
	accelerationStructureBuildGeometryTopLevel.flags = static_cast<VkBuildAccelerationStructureFlagsKHR>(flags);
	accelerationStructureBuildGeometryTopLevel.geometryCount = 1;
	accelerationStructureBuildGeometryTopLevel.pGeometries = &accelerationStructureGeometryTopLevel;
	accelerationStructureBuildGeometryTopLevel.mode = static_cast<VkBuildAccelerationStructureModeKHR>(pvrvk::BuildAccelerationStructureModeKHR::e_BUILD_KHR);
	accelerationStructureBuildGeometryTopLevel.type = static_cast<VkAccelerationStructureTypeKHR>(pvrvk::AccelerationStructureTypeKHR::e_TOP_LEVEL_KHR);
	accelerationStructureBuildGeometryTopLevel.srcAccelerationStructure = VK_NULL_HANDLE;

	uint32_t count = 1;
	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{ static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR) };
	_deviceResources->device->getVkBindings().vkGetAccelerationStructureBuildSizesKHR(_deviceResources->device->getVkHandle(),
		static_cast<VkAccelerationStructureBuildTypeKHR>(pvrvk::AccelerationStructureBuildTypeKHR::e_DEVICE_KHR), &accelerationStructureBuildGeometryTopLevel, &count,
		&accelerationStructureBuildSizesInfo);

	pvrvk::AccelerationStructureCreateInfo accelerationStructureCreateInfo;
	accelerationStructureCreateInfo.setType(pvrvk::AccelerationStructureTypeKHR::e_TOP_LEVEL_KHR);
	accelerationStructureCreateInfo.setSize(accelerationStructureBuildSizesInfo.accelerationStructureSize);

	pvrvk::Buffer asBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(accelerationStructureBuildSizesInfo.accelerationStructureSize,
			pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	accelerationStructureCreateInfo.setBuffer(asBuffer->getVkHandle());

	_deviceResources->_tlas = _deviceResources->device->createAccelerationStructure(accelerationStructureCreateInfo, asBuffer);
	_deviceResources->_tlas->setAccelerationStructureBuffer(asBuffer);

	pvrvk::Buffer scratchBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(accelerationStructureBuildSizesInfo.buildScratchSize,
			pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT |
				pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	// Get scratch buffer device address, reuse instancesDeviceAddressInfo struct
	VkDeviceAddress scratchAddress = scratchBuffer->getDeviceAddress(_deviceResources->device);

	// Update build information
	accelerationStructureBuildGeometryTopLevel.srcAccelerationStructure = VK_NULL_HANDLE;
	accelerationStructureBuildGeometryTopLevel.dstAccelerationStructure = _deviceResources->_tlas->getVkHandle();
	accelerationStructureBuildGeometryTopLevel.scratchData.deviceAddress = scratchAddress;

	// Build Offsets info: n instances
	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{ static_cast<uint32_t>(/*instances.size()*/ 1), 0, 0, 0 };
	const VkAccelerationStructureBuildRangeInfoKHR* pAccelerationStructureBuildRangeInfo = &accelerationStructureBuildRangeInfo;

	_deviceResources->device->getVkBindings().vkCmdBuildAccelerationStructuresKHR(
		commandBuffer->getVkHandle(), 1, &accelerationStructureBuildGeometryTopLevel, &pAccelerationStructureBuildRangeInfo);
	endAndSubmitCommandBuffer(commandBuffer);
}

void VulkanHelloRayTracing::setupGeometryInstances(std::vector<VkAccelerationStructureInstanceKHR>& geometryInstances)
{
	// Retrieve the address of this bottom level acceleration structure, needed as one of the fields in the struct to be added to geometryInstances
	VkDeviceAddress bottomLevelASAddress = _deviceResources->_blas->getAccelerationStructureDeviceAddress(_deviceResources->device);

	// The information for each scene element, expressed through an instance, is added here to geometryInstances. Since only one triangle is
	// present in the scene, the information from _deviceResources::_instance is added to geometryInstances.
	VkAccelerationStructureInstanceKHR accelerationStructureInstance{};
	glm::mat4 transp = glm::transpose(_deviceResources->_instance.transform);
	memcpy(&accelerationStructureInstance.transform, &transp, sizeof(accelerationStructureInstance.transform));

	accelerationStructureInstance.instanceCustomIndex = _deviceResources->_instance.instanceId;
	accelerationStructureInstance.mask = _deviceResources->_instance.mask;
	accelerationStructureInstance.instanceShaderBindingTableRecordOffset = _deviceResources->_instance.hitGroupId;
	accelerationStructureInstance.flags = static_cast<VkGeometryInstanceFlagsKHR>(_deviceResources->_instance.flags);
	accelerationStructureInstance.accelerationStructureReference = bottomLevelASAddress;
	geometryInstances.push_back(accelerationStructureInstance);
}

void VulkanHelloRayTracing::fillCameraData()
{
	// Build an orthogonal projection matrix, this sample has a fixed camera
	float width = static_cast<float>(getWidth());
	float height = static_cast<float>(getHeight());
	float aspect;
	if (width < height) { aspect = height / width; }
	else
	{
		aspect = width / height;
	}

	glm::mat4 proj = glm::mat4(1.0);
	float left = aspect;
	float right = -aspect;
	float bottom = 1.0;
	float top = -1.0f;

	proj[0][0] = 2.0f / (right - left);
	proj[1][1] = 2.0f / (top - bottom);
	proj[2][2] = -1.0f;
	proj[3][0] = -(right + left) / (right - left);
	proj[3][1] = -(top + bottom) / (top - bottom);
	proj[3][3] = 1.0f;

	// Inverse view and projection matrices are needed for tracing the rays in the raytrace.rgen shader.
	// The view matrix is just an identity matrix, position the camera at origin looking towards the
	// negative z axis, it's inverse is also an identity matrix.
	_camera.viewMatrixInverse = glm::mat4(1.0f);
	_camera.projectionMatrixInverse = glm::inverse(proj);
}

void VulkanHelloRayTracing::buildCameraBuffer()
{
	// Build a buffer where to store the _camera information, the inverse view and projection matrices in this buffer
	// are used in the raytrace.rgen shader to generate the ray directions for the rays to be traced
	_deviceResources->cameraBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(sizeof(CameraData), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT);

	pvr::utils::updateHostVisibleBuffer(_deviceResources->cameraBuffer, &_camera, 0, sizeof(CameraData), true);
}

void VulkanHelloRayTracing::buildSceneDescriptionBuffer()
{
	// Build a buffer to stre the information for each one of the scene elements to be ray traced, which are represented by the
	// instance data in _deviceResources::_sceneDescription. This buffer is accessed in the raytrace.rchit to recover the
	// object id of the triangle hit by the ray
	pvrvk::BufferCreateInfo bufferCreateInfo = pvrvk::BufferCreateInfo(
		sizeof(pvr::utils::SceneDescription) * 1 /*Just one scene element*/, pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);

	pvrvk::MemoryPropertyFlags memoryPropertyFlags =
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT;

	_deviceResources->sceneDescription = pvr::utils::createBuffer(_deviceResources->device, bufferCreateInfo, memoryPropertyFlags);
	pvrvk::DeviceSize dataSize = sizeof(pvr::utils::SceneDescription) * 1; // Just one scene element
	pvr::utils::updateHostVisibleBuffer(_deviceResources->sceneDescription, &_deviceResources->_sceneDescription, 0, dataSize, true);
}

void VulkanHelloRayTracing::buildDescriptorPool()
{
	// Build the descriptor pool to generate the descriptor sets used in this sample

	uint32_t numTextures = 1;

	pvrvk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = pvrvk::DescriptorPoolCreateInfo();
	descriptorPoolCreateInfo.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1);
	descriptorPoolCreateInfo.addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER, 5);
	descriptorPoolCreateInfo.addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->swapchain->getSwapchainLength());
	descriptorPoolCreateInfo.addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(numTextures));
	descriptorPoolCreateInfo.addDescriptorInfo(pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, _deviceResources->swapchain->getSwapchainLength());
	descriptorPoolCreateInfo.setMaxDescriptorSets(1 + _deviceResources->swapchain->getSwapchainLength());

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(descriptorPoolCreateInfo);
}

void VulkanHelloRayTracing::buildDescriptorSetLayout()
{
	// The descriptor set layout for the _deviceResources::descriptorSet descriptor set is explained below and comprises all
	// the resources are needed in the raytrace.rgen and raytrace.rchit shaders to generate the rays to be traced and act accordingly
	// when the scene triangle is hit. The shader stage flags help distinguish where each descriptor is used
	// (e_RAYGEN_BIT_KHR for raytrace.rgen and e_CLOSEST_HIT_BIT_KHR for raytrace.rchit):

	// Camera matrices (binding = 0)
	// Materials (binding = 1)
	// Scene description (binding = 2)
	// Textures (binding = 3)
	// Material indices (binding = 4)
	// Geometry vertices (binding = 5)
	// Geometry indices (binding = 6)

	pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
	pvrvk::ShaderStageFlags shaderStageFlags = pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR;
	descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	descSetInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, shaderStageFlags);
	descSetInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, shaderStageFlags);
	descSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	descSetInfo.setBinding(4, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	descSetInfo.setBinding(5, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	descSetInfo.setBinding(6, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);

	_deviceResources->descSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
}

void VulkanHelloRayTracing::buildDescriptorSet()
{
	// Allocate the descriptor set _deviceResources::descriptorSet using the descriptor set layout built in
	// buildDescriptorSetLayout() and the descriptor pool built in buildDescriptorPool()
	_deviceResources->descriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayout);

	// The descriptor set is described as below:
	// Camera matrices (binding = 0)
	// Materials (binding = 1)
	// Scene description (binding = 2)
	// Textures (binding = 3)
	// Material indices (binding = 4)
	// Geometry vertices (binding = 5)
	// Geometry indices (binding = 6)

	// To update the newly built descriptor set _deviceResources::descriptorSet, WriteDescriptorSet structs need to be filled mapping the corresponding fields
	pvrvk::WriteDescriptorSet camera = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->descriptorSet, 0);
	pvrvk::WriteDescriptorSet material = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->descriptorSet, 1);
	pvrvk::WriteDescriptorSet scene = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->descriptorSet, 2);
	pvrvk::WriteDescriptorSet imageSampler = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descriptorSet, 3);
	pvrvk::WriteDescriptorSet materialIndex = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->descriptorSet, 4);
	pvrvk::WriteDescriptorSet vertices = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->descriptorSet, 5);
	pvrvk::WriteDescriptorSet indices = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->descriptorSet, 6);

	// For the image sampler, specify the image view and a sampler
	pvrvk::DescriptorImageInfo descriptorImageInfo =
		pvrvk::DescriptorImageInfo(_deviceResources->materialTexture.imageView, _deviceResources->materialTexture.sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);

	// Overwrite the whole descriptor's buffer using VK_WHOLE_SIZE
	camera.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->cameraBuffer, 0, VK_WHOLE_SIZE));
	scene.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->sceneDescription, 0, VK_WHOLE_SIZE));
	material.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->materialBuffer, 0, VK_WHOLE_SIZE));
	materialIndex.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->materialIndexBuffer, 0, VK_WHOLE_SIZE));
	vertices.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->vertexBuffer, 0, VK_WHOLE_SIZE));
	indices.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->indexBuffer, 0, VK_WHOLE_SIZE));
	imageSampler.setImageInfo(0, descriptorImageInfo);

	// Build the array of write descriptor sets to update _deviceResources::descriptorSet
	std::vector<pvrvk::WriteDescriptorSet> writes = { camera, scene, material, materialIndex, vertices, indices, imageSampler };

	// Write the information
	_deviceResources->device->updateDescriptorSets(writes.data(), static_cast<uint32_t>(writes.size()), nullptr, 0);
}

void VulkanHelloRayTracing::buildRayTracingDescriptorSetLayout()
{
	// Build the descriptor set layout _deviceResources::descSetLayoutRT used to trace rays and store the final color in the offscreen image for each
	// texel when the offscreen ray trace pass is performed (see bindings DescriptorType enum)

	pvrvk::DescriptorSetLayoutCreateInfo descriptorSetLayout;
	descriptorSetLayout.setBinding(0, pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, 1u, pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	descriptorSetLayout.setBinding(1, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);

	_deviceResources->descSetLayoutRT = _deviceResources->device->createDescriptorSetLayout(descriptorSetLayout);
}

void VulkanHelloRayTracing::buildRayTracingDescriptorSets()
{
	std::vector<pvrvk::WriteDescriptorSet> writes;
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// Allocate the descriptor set _deviceResources::descriptorSetRT which comprises the acceleration structure needed in the raygen.rgen shader and
		// the image where to store the results of the ray tracing offscreen pass
		_deviceResources->descriptorSetRTs[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayoutRT);

		// The descriptors used in this descriptor set are described below:
		// Aceleration structure (binding = 0)
		// Image to store offscreen Ray Tracing pass (binding = 1)
		writes.emplace_back(pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, _deviceResources->descriptorSetRTs[i], 0);
		writes.back().setAccelerationStructureInfo(0, _deviceResources->_tlas);

		writes.emplace_back(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->descriptorSetRTs[i], 1);
		writes.back().setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->renderImageViews[i], pvrvk::ImageLayout::e_GENERAL));
	}
	
	// Write the information
	_deviceResources->device->updateDescriptorSets(writes.data(), static_cast<uint32_t>(writes.size()), nullptr, 0);
}

void VulkanHelloRayTracing::buildRayTracingPipeline()
{
	// Here all the ray tracing shaders and descriptor set layouts are specified to build a ray tracing pipeline

	// First, the ray tracing shaders are loaded and shader module is generated
	pvrvk::ShaderModuleCreateInfo generateSMCI = pvrvk::ShaderModuleCreateInfo(getAssetStream("raytrace.rgen.spv")->readToEnd<uint32_t>());
	pvrvk::ShaderModuleCreateInfo missSMCI = pvrvk::ShaderModuleCreateInfo(getAssetStream("raytrace.rmiss.spv")->readToEnd<uint32_t>());
	pvrvk::ShaderModuleCreateInfo hitSMCI = pvrvk::ShaderModuleCreateInfo(getAssetStream("raytrace.rchit.spv")->readToEnd<uint32_t>());

	// Shader modules are needed to specify the different ray tracing pipeline stages, which can vary from one application to another. In this
	// sample, only ray generation, ray hit and ray miss shaders are considered.
	pvrvk::ShaderModule generateSM = _deviceResources->device->createShaderModule(generateSMCI);
	pvrvk::ShaderModule missSM = _deviceResources->device->createShaderModule(missSMCI);
	pvrvk::ShaderModule hitSM = _deviceResources->device->createShaderModule(hitSMCI);

	pvrvk::RaytracingPipelineCreateInfo raytracingPipeline;

	pvrvk::PipelineShaderStageCreateInfo generateCreateInfo;
	generateCreateInfo.setShader(generateSM);
	generateCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	raytracingPipeline.stages.push_back(generateCreateInfo);

	pvrvk::PipelineShaderStageCreateInfo missCreateInfo;
	missCreateInfo.setShader(missSM);
	missCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_MISS_BIT_KHR);
	raytracingPipeline.stages.push_back(missCreateInfo);

	pvrvk::PipelineShaderStageCreateInfo hitCreateInfo;
	hitCreateInfo.setShader(hitSM);
	hitCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	raytracingPipeline.stages.push_back(hitCreateInfo);

	// Ray tracing shader group create info structs are built mapping the ones in the ray tracing pipeline variable raytracingPipeline
	pvrvk::RayTracingShaderGroupCreateInfo generateCI = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo missCI = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo hitCI = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_TRIANGLES_HIT_GROUP_KHR);

	generateCI.setGeneralShader(static_cast<uint32_t>(0));
	missCI.setGeneralShader(static_cast<uint32_t>(1));
	hitCI.setClosestHitShader(static_cast<uint32_t>(2));

	raytracingPipeline.shaderGroups = { generateCI, missCI, hitCI };

	_shaderGroupCount = static_cast<uint32_t>(raytracingPipeline.shaderGroups.size());

	// The pipeline layout for the ray tracing pipeline has two descriptor sets: _deviceResources::descSetLayoutRT and
	// _deviceResources::descSetLayout explained in buildRayTracingDescriptorSetLayout and buildDescriptorSetLayout
	pvrvk::PipelineLayoutCreateInfo pipeLayout;
	pipeLayout.addDescSetLayout(_deviceResources->descSetLayoutRT);
	pipeLayout.addDescSetLayout(_deviceResources->descSetLayout);
	_deviceResources->pipelineLayoutRT = _deviceResources->device->createPipelineLayout(pipeLayout);

	raytracingPipeline.pipelineLayout = _deviceResources->pipelineLayoutRT;

	// Rays are traced from the camera, the rays hitting the scene mesh (in this case, just a simple triangle)
	// do not test emitter visibility from the hitting point through a shadow ray, nor continue iterating in case the triangle had a specular
	// material, so only a single recursion level is needed
	raytracingPipeline.maxRecursionDepth = 1;

	_deviceResources->pipelineRT = _deviceResources->device->createRaytracingPipeline(raytracingPipeline, nullptr);
}

void VulkanHelloRayTracing::buildShaderBindingTable()
{
	// The shader binding table is used to know what shaders to call depending on what happens with the rays to be traced in the ray tracing pass.
	// This way, the shader to be called for ray generation can be specified (the .rgen shader), and the shader to be called if a ray hits
	// geometry (the .rchit shader), and as well the shader to be called if a ray hits no geometry (the .rmiss shader). In more general cases where more
	// scene elements and hit shaders are present, this shader binding table helps point to the correct hit shader to be called depending on the
	// instance whose geometry is hit by a ray.

	// The hitGroupId field in the _deviceResources::_instance variable is used to specify the index in the shader binding table
	// for the hit group for each particular instance. In this sample there is only a single scene element (a triangle mesh) to be ray traced.

	// The way to compute the proper shader group aligned size is alignedSize = (size + (alignment - 1)) & (~alignment - 1), rounding up to guarantee
	// the alignment divides the shader group size.
	uint32_t shaderGroupSize = (_rtProperties.shaderGroupHandleSize + (uint32_t(_rtProperties.shaderGroupBaseAlignment) - 1)) & ~uint32_t(_rtProperties.shaderGroupBaseAlignment - 1);

	// Compute the total size of the shader handlers used in the pipeline, to be written in the shader binding table
	uint32_t shaderBindingTableSize = _shaderGroupCount * shaderGroupSize;

	// Retrieve the handles through the vkGetRayTracingShaderGroupHandlesKHR API call
	std::vector<uint8_t> shaderHandleStorage(shaderBindingTableSize);
	_deviceResources->device->getVkBindings().vkGetRayTracingShaderGroupHandlesKHR(
		_deviceResources->device->getVkHandle(), _deviceResources->pipelineRT->getVkHandle(), 0, _shaderGroupCount, shaderBindingTableSize, shaderHandleStorage.data());

	// The shader binding table is a buffer, make a buffer of size shaderBindingTableSize where to write the details of the shader binding table
	_deviceResources->shaderBindingTable = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(shaderBindingTableSize,
			pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::BufferUsageFlags::e_SHADER_BINDING_TABLE_BIT_KHR | pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, nullptr,
		pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	// Map memory to the buffer
	void* mappedData = nullptr;
	_deviceResources->device->getVkBindings().vkMapMemory(
		_deviceResources->device->getVkHandle(), _deviceResources->shaderBindingTable->getDeviceMemory()->getVkHandle(), 0, VK_WHOLE_SIZE, 0, &mappedData);

	uint8_t* castedMappedData = nullptr;
	castedMappedData = reinterpret_cast<uint8_t*>(mappedData);

	// Write in the mapped buffer the information corresponding to each whole shader group from shaderHandleStorage.
	// Blocks of shaderGroupSize bytes are considered when writing this shader groups, following a stride.
	// _rtProperties.shaderGroupHandleSize is the size of a shader program identifier
	// shaderGroupSize is the size of alligned shader group
	// Following the pipeline where the ray generation, ray miss and ray hit where considered, we map that order
	// (which can be changed as long as is respected) to the shader binding table.
	for (uint32_t i = 0; i < _shaderGroupCount; i++)
	{
		memcpy(castedMappedData, shaderHandleStorage.data() + i * _rtProperties.shaderGroupHandleSize, _rtProperties.shaderGroupHandleSize);
		castedMappedData += shaderGroupSize;
	}

	_deviceResources->device->getVkBindings().vkUnmapMemory(_deviceResources->device->getVkHandle(), _deviceResources->shaderBindingTable->getDeviceMemory()->getVkHandle());
}

void VulkanHelloRayTracing::recordCommandBuffer()
{
	// This method prepares all the Ray Tracing Vulkan command recording and copy from offscreen texture to swap chain texture for each
	// one of the command buffers. There are a number of command buffers equivalent to the number of swapchains,
	// and the same commands are recorded to each of them.

	const uint32_t numSwapchains = _deviceResources->swapchain->getSwapchainLength();
	pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(0.0f, 0.45f, 0.41f, 1.f), pvrvk::ClearValue(1.f, 0u) };
	for (uint32_t i = 0; i < numSwapchains; ++i)
	{
		_deviceResources->cmdBuffers[i]->begin();

		// This is the actual method that performs the ray tracing
		raytrace(_deviceResources->cmdBuffers[i], i);

		// Ray Tracing results are stored in an offscreen texture, copy the results to the corresponding swapchain image
		recordRenderImageCopy(_deviceResources->cmdBuffers[i], i);

		_deviceResources->cmdBuffers[i]->beginRenderPass(
			_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		recordCommandUIRenderer(_deviceResources->cmdBuffers[i]);

		_deviceResources->cmdBuffers[i]->endRenderPass();
		_deviceResources->cmdBuffers[i]->end();
	}
}

void VulkanHelloRayTracing::raytrace(const pvrvk::CommandBuffer& cmdBuf, uint32_t imageIndex)
{
	// Last step of the Ray Tracing setup, here the ray tracing pipeline is bound together with the two descriptor sets
	// _deviceResources::descriptorSetRT and _deviceResources::descriptorSet.
	cmdBuf->bindPipeline(_deviceResources->pipelineRT);

	pvrvk::DescriptorSet descSets[2] = { _deviceResources->descriptorSetRTs[imageIndex], _deviceResources->descriptorSet };
	cmdBuf->bindDescriptorSets(pvrvk::PipelineBindPoint::e_RAY_TRACING_KHR, _deviceResources->pipelineLayoutRT, 0, descSets, ARRAY_SIZE(descSets), 0, 0);

	// The way to compute the proper shader group aligned size is alignedSize = (size + (alignment - 1)) & (~alignment - 1), rounding up to guarantee
	// the alignment divides the shader group size.
	uint32_t shaderGroupSize = (_rtProperties.shaderGroupHandleSize + (uint32_t(_rtProperties.shaderGroupBaseAlignment) - 1)) & ~uint32_t(_rtProperties.shaderGroupBaseAlignment - 1);
	uint32_t shaderGroupStride = shaderGroupSize;

	// Four variables are required for the ray trace Vulkan API call in cmdBuf->traceRays. Due to the fact that the Shader Binding Table is defined
	// by the developer, those four variables are "Strided Buffer Region" structs specifying the Shader Binding Table data for
	// the ray generation, ray miss, ray hit and callable shaders. This is basically giving data about the Shader Binding Table to the Ray Tracing Pipeline.
	// This uses again the concept of shader group size and shader group stride exposed in buildShaderBindingTable.
	// Note that since there's only one shader in each shader group, the offsets in rayGenOffset, missOffset and hitGroupOffset are proportional.
	// In this case, the stride is given by shaderGroupStride which is the same as the size of a shader group, but if for example more shaders are
	// used in the miss shader group, the shader group stride for the miss shader group would need to reflect the amount of shaders in that group.

	VkDeviceSize rayGenOffset = 0u * shaderGroupSize; // Start at the beginning of m_sbtBuffer
	VkDeviceSize missOffset = 1u * shaderGroupSize; // Ray gen size, after it then the ray miss data starts.
	VkDeviceSize hitGroupOffset = 2u * shaderGroupSize; // Ray gen and ray miss shaders, after those the hit shader data starts

	VkDeviceAddress sbtAddress = _deviceResources->shaderBindingTable->getDeviceAddress(_deviceResources->device);

	pvrvk::StridedDeviceAddressRegionKHR raygenShaderBindingTable = { sbtAddress + rayGenOffset, shaderGroupStride, shaderGroupSize };
	pvrvk::StridedDeviceAddressRegionKHR missShaderBindingTable = { sbtAddress + missOffset, shaderGroupStride, shaderGroupSize };
	pvrvk::StridedDeviceAddressRegionKHR hitShaderBindingTable = { sbtAddress + hitGroupOffset, shaderGroupStride, shaderGroupSize };
	pvrvk::StridedDeviceAddressRegionKHR callableShaderBindingTable = {};

	cmdBuf->traceRays(raygenShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable, getWidth(), getHeight(), 1);
}

void VulkanHelloRayTracing::recordRenderImageCopy(pvrvk::CommandBuffer& cmdBuf, uint32_t imageIndex)
{
	// Transition offscreen texture used for storing the results of the ray tracing pass (_deviceResources::renderImage)
	// from e_GENERAL to e_TRANSFER_SRC_OPTIMAL to be able to copy it to the swapchain image.
	pvrvk::ImageMemoryBarrier renderImageBarrier;
	renderImageBarrier.setDstAccessMask(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT);
	renderImageBarrier.setNewLayout(pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL);
	renderImageBarrier.setImage(_deviceResources->renderImages[imageIndex]);
	renderImageBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));

	// Transition swapchain image from e_PRESENT_SRC_KHR to e_TRANSFER_DST_OPTIMAL to receive the offscreen ray
	// tracing results stored in _deviceResources::renderImage
	pvrvk::ImageMemoryBarrier swapchainBarrier;
	swapchainBarrier.setDstAccessMask(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT);
	swapchainBarrier.setNewLayout(pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);
	swapchainBarrier.setImage(_deviceResources->swapchain->getImage(imageIndex));
	swapchainBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));

	// Submit barriers to transition layout
	pvrvk::MemoryBarrierSet barrierSet;
	barrierSet.addBarrier(renderImageBarrier);
	barrierSet.addBarrier(swapchainBarrier);
	cmdBuf->pipelineBarrier(pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, barrierSet);

	// Copy from _deviceResources::renderImages[imageIndex] to the corresponding swapchain image
	pvrvk::ImageSubresourceLayers subresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 0, 1);
	pvrvk::Offset3D offsets[2] = { pvrvk::Offset3D(0, 0, 0), pvrvk::Offset3D(getWidth(), getHeight(), 1) };
	pvrvk::ImageBlit imageRegion(subresourceLayers, offsets, subresourceLayers, offsets);
	cmdBuf->blitImage(_deviceResources->renderImages[imageIndex], _deviceResources->swapchain->getImage(imageIndex), &imageRegion, 1, pvrvk::Filter::e_LINEAR,
		pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);

	barrierSet.clearAllBarriers();

	// Transition back _deviceResources::renderImages[imageIndex] from e_TRANSFER_SRC_OPTIMAL to e_GENERAL
	renderImageBarrier.setDstAccessMask(pvrvk::AccessFlags::e_SHADER_WRITE_BIT);
	renderImageBarrier.setNewLayout(pvrvk::ImageLayout::e_GENERAL);

	// Transition back the swapchain image from e_TRANSFER_DST_OPTIMAL to e_PRESENT_SRC_KHR
	swapchainBarrier.setDstAccessMask(pvrvk::AccessFlags::e_NONE);
	swapchainBarrier.setNewLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);

	barrierSet.addBarrier(renderImageBarrier);
	barrierSet.addBarrier(swapchainBarrier);

	// Submit barriers to transition layout
	cmdBuf->pipelineBarrier(pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, barrierSet);
}

pvrvk::CommandBuffer VulkanHelloRayTracing::beginCommandBuffer()
{
	pvrvk::CommandBuffer uploadCmd = _deviceResources->commandPool->allocateCommandBuffer();
	uploadCmd->begin();
	return uploadCmd;
}

void VulkanHelloRayTracing::endAndSubmitCommandBuffer(pvrvk::CommandBuffer commandBuffer)
{
	commandBuffer->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &commandBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();
}

void VulkanHelloRayTracing::recordCommandUIRenderer(pvrvk::CommandBuffer& commandBuff)
{
	_deviceResources->uiRenderer.beginRendering(commandBuff);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
}
