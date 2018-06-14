/*!*********************************************************************************************************************
\File         VulkanHelloAPI.h
\Title        VulkanHello API Header
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Header file for VulkanHelloAPI class. It Also contains helper functions and structs.
***********************************************************************************************************************/
#pragma once
#include "vk_getProcAddrs.h"
#include <string>

#include <sstream>
#include <iostream>

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#include "../../../external/glm/glm.hpp"
#include "../../../external/glm/gtc/matrix_transform.hpp"
#include "../../../external/glm/gtx/transform.hpp"
#include "../../../external/glm/gtc/type_ptr.hpp"

// The Surface Data structure is different based on the platform we're using
// here we define the structure and its members inside Vulkan provided preprocessors

#ifndef NDEBUG
#define DEBUG
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct SurfaceData
{
	float width, height;

	HINSTANCE connection;
	HWND window;

	SurfaceData() = default;
};
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
struct SurfaceData
{
	float width, height;

	Display* display;
	Window window;

	SurfaceData() {}
};
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct SurfaceData
{
	float width, height;

	ANativeWindow* window;

	SurfaceData()
	{
		width = height = 0;
	}
};

#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
struct SurfaceData
{
	float width, height;

	wl_display* display;
	wl_surface* surface;
	wl_registry* wlRegistry;
	wl_compositor* wlCompositor;
	wl_shell* wlShell;
	wl_seat* wlSeat;
	wl_pointer* wlPointer;
	wl_shell_surface* wlShellSurface;

	SurfaceData()
	{
		width = height = 0;
		display = NULL;
		surface = NULL;
		wlRegistry = NULL;
		wlCompositor = NULL;
		wlShell = NULL;
		wlSeat = NULL;
		wlPointer = NULL;
		wlShellSurface = NULL;
	}
};
#endif

#ifdef USE_PLATFORM_NULLWS
struct SurfaceData
{
	float width, height;

	VkDisplayKHR nativeDisplay;
	VkSurfaceKHR surface;

	SurfaceData()
	{
		nativeDisplay = VK_NULL_HANDLE;
		surface = VK_NULL_HANDLE;
		width = height = 0;
	}
};
#endif

#ifdef DEBUG
#define PVR_DEBUG 1
#define LOGERRORSONLY 0
#endif

// Returns a human readable string from VKResults.
static std::string debugGetVKResultString(const VkResult inRes)
{
	switch (inRes)
	{
	case 0:
		return "VK_SUCCESS";
	case 1:
		return "VK_NOT_READY";
	case 2:
		return "VK_TIMEOUT";
	case 3:
		return "VK_EVENT_SET";
	case 4:
		return "VK_EVENT_RESET";
	case 5:
		return "VK_INCOMPLETE";

	case -1:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case -2:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case -3:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case -4:
		return "VK_ERROR_DEVICE_LOST";
	case -5:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case -6:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case -7:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case -8:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case -9:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case -10:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case -11:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case -12:
		return "VK_ERROR_FRAGMENTED_POOL";

	default:
		return "Unknown VkResult Value";
	}
}

// Logs returns from Vulkan function calls.
inline void debugAssertFunctionResult(const VkResult inRes, const std::string inOperation)
{
#ifdef DEBUG
	if (LOGERRORSONLY)
	{
		Log(true, (inOperation + " -- " + debugGetVKResultString(inRes)).c_str());
		assert(inRes == VK_SUCCESS);
	}
#endif
}

// Constants used throughout the example.
#define FENCE_TIMEOUT std::numeric_limits<uint64_t>::max()
#define NUM_DESCRIPTOR_SETS 2

const float PI = 3.14159265359f;
const float TORAD = PI / 180.0f;

class VulkanHelloAPI
{
private:
	// Custom structs that encapsulates related data to help us keep track of
	// the multiple aspects of different Vulkan objects.
	struct SwapchainImage
	{
		VkImage image;
		VkImageView view;
	};

	struct BufferData
	{
		VkMemoryPropertyFlags memPropFlags;
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo bufferInfo;
		size_t size;
		void* mappedData;

		BufferData() : buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE), size(0), memPropFlags(0), mappedData(nullptr) {}
	};

	struct TextureData
	{
		std::vector<uint8_t> data;
		VkExtent2D textureDimensions;
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkSampler sampler;
	};

	struct AppManager
	{
		std::vector<const char*> instanceLayerNames;
		std::vector<const char*> instanceExtensionNames;
		std::vector<const char*> deviceExtensionNames;

		std::vector<VkPhysicalDevice> gpus;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		std::vector<SwapchainImage> swapChainImages;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkFramebuffer> frameBuffers;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

		std::vector<VkSemaphore> acquireSemaphore;
		std::vector<VkSemaphore> presentSemaphores;
		std::vector<VkFence> frameFences;

		VkInstance instance;
		VkPhysicalDevice physicalDevice;

		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		VkPhysicalDeviceProperties deviceProperties;
		uint32_t graphicsQueueFamilyIndex;
		uint32_t presentQueueFamilyIndex;
		VkDevice device;
		VkQueue graphicQueue;
		VkQueue presentQueue;
		VkSurfaceKHR surface;
		VkSurfaceFormatKHR surfaceFormat;
		VkSwapchainKHR swapchain;
		VkPresentModeKHR presentMode;
		VkExtent2D swapchainExtent;
		VkPipelineShaderStageCreateInfo shaderStages[2];
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
		VkCommandPool commandPool;
		VkViewport viewport;
		VkRect2D scissor;
		VkDescriptorPool descriptorPool;
		VkDescriptorSet dynamicDescSet;
		VkDescriptorSet staticDescSet;
		VkDescriptorSetLayout staticDescriptorSetLayout;
		VkDescriptorSetLayout dynamicDescriptorSetLayout;

		BufferData vertexBuffer;
		BufferData dynamicUniformBufferData;
		TextureData texture;

		float angle;
		uint32_t offset;
	};

	struct Vertex
	{
		float x, y, z, w; // coordinates
		float u, v; // texture UVs
	};
	glm::mat4 viewProj;

	// Check type of memory using the device memory properties.
	bool getMemoryTypeFromProperties(const VkPhysicalDeviceMemoryProperties& memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex)
	{
		// Search memory types to find first index with those properties
		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				// Type is available, does it match user properties?
				if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
				{
					*typeIndex = i;
					return true;
				}
			}
			typeBits >>= 1;
		}
		// No memory types matched, return failure
		return false;
	}
	// this method checks for physical device compatibility.
	VkPhysicalDevice getCompatibleDevice();
	// Get a compatible Queue Family with the properties and it's a graphical one.
	void getCompatibleQueueFamilies(uint32_t& graphicsfamilyindex, uint32_t& presentfamilyindex);
	// checks if the mode we want is compatible with the surface, if not we default to the standard (FIFO)
	VkPresentModeKHR getCompatiblePresentMode(const VkPresentModeKHR& inReqMode, const std::vector<VkPresentModeKHR>& inModes);
	// We make sure the Extent is correct and if it's not we set the same sizes as the window.
	VkExtent2D getCorrectExtent(const VkSurfaceCapabilitiesKHR& inSurfCap);

	void initUniformBuffers();

public:
	// initialize the Validation layers.
	std::vector<std::string> initLayers();
	// initialize the needed extensions.
	std::vector<std::string> initInstanceExtensions();
	std::vector<std::string> initDeviceExtensions();
	// initialize the Application & Instance.
	void initApplicationAndInstance(std::vector<std::string>& extensionNames, std::vector<std::string>& layerNames);
	// fetch the physical devices and get a compatible one.
	void initPhysicalDevice();
	// Find queues families and individual queues from device.
	void initQueuesFamilies();
	// initialize the logical device.
	void initLogicalDevice(std::vector<std::string>& deviceExtensions);
	// fetch queues from device.
	void initQueues();
	// Create the surface we'll render on (Platform Dependent).
	void initSurface();
	// Create The SwapChain
	void initSwapChain();
	// create the Images and Image Views to be used with the SwapChain.
	void initImagesAndViews();
	// create a vertex buffers to draw our primitive;
	void initVertexBuffers();
	// We create a texture to apply to our primitive.
	void initTexture();
	// Create Rotation Descriptors and Matrix.
	void initDynamicUniformBuffers();
	// create a descriptor pool and allocate descriptor sets for our buffers.
	void initDescriptorPoolAndSet();
	// compile and convert the shaders that we'll be using.
	void initShaders();
	// create the pipeline we'll be using for the rendering
	void initPipeline();
	// create the render pass we'll use to render the triangle.
	void initRenderPass();
	// create the Frame buffers for rendering
	void initFrameBuffers();
	// create the command buffer to be sent to the GPU from our command pool.
	void initCommandPoolAndBuffer();
	// initialize the view port and scissor for the rendering
	void initViewportAndScissor();
	// Create the semaphore to deal with our command queue.
	void initSemaphoreAndFence();

	// Generic method to initialize buffers (both Vertex and Uniform use this);
	void createBuffer(BufferData& inBuffer, const uint8_t* inData, const VkBufferUsageFlags& inUsage);
	// Generic Method for creating a Dynamic Uniform buffer.
	void createDynamicUniformBuffer(BufferData& inBuffer);
	// Generic method for creating a shader module.
	void createShader(const uint32_t* spvShader, size_t spvShaderSize, int indx, VkShaderStageFlagBits shaderStage);
	// Generates a texture without having to load an image file.
	void generateTexture();
	// Changes the rotation of the per frame Uniform buffer
	void applyRotation(int idx = 0);

	// Initializes all the needed Vulkan objects but calling all the Init__ methods.
	void initialize();
	// Cleans up every thing when we're done with our application.
	void deinitialize();

	// Record the command buffer for rendering our example.
	void recordCommandBuffer();
	// Execute the command buffer and present the result to the surface.
	void drawFrame();

	// Holds all the Vulkan Handles that we need access to "globally"
	AppManager appManager;
	// used for debugging mostly to show the VKResult return from the Vulkan function calls.
	VkResult lastRes;
	// Keeps track of the frame for synchronization purposes.
	int frameId;
	// Surface data needed to distinguish between the different platforms.
	SurfaceData surfaceData;
};
