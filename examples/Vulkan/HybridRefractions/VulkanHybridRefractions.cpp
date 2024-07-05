/*!
\brief Implements a hybrid rendering technique with ray traced refractions and Phong diffuse rasterized scene elements, with a directional light
\file VulkanHybridRefractions.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRUtils/Vulkan/AccelerationStructure.h"

/// <summary>Maximum number of swap images supported.</summary>
constexpr uint32_t MAX_NUMBER_OF_SWAP_IMAGES = 4;

/// <summary>Enum to map the three colur attachments used in the deferred shading pass to build the G-Buffer.</summary>
enum GBuffer
{
	Reflectance = 0,
	NormalMaterialID,
	WorldPositionIOR,
	Size
};

/// <summary>LightData struct for the scene, replicated in the shaders.</summary>
struct LightData
{
	/// <summary>Light color.</summary>
	glm::vec4 lightColor;

	/// <summary>Light position and max ray recursion.</summary>
	glm::vec4 lightPositionMaxRayRecursion;

	/// <summary>Ambient color and light intensity.</summary>
	glm::vec4 ambientColorIntensity;
};

/// <summary>Struct used to encapsulate all the resources and information for each texture to be ray traced.</summary>
struct TextureAS
{
	/// <summary>Texture name.</summary>
	std::string name;

	/// <summary>Texture format.</summary>
	pvrvk::Format format = pvrvk::Format::e_R8G8B8A8_SRGB;

	/// <summary>Texture image.</summary>
	pvrvk::Image image;

	/// <summary>Texture image view.</summary>
	pvrvk::ImageView imageView;
};

/// <summary>Struct where to store information about the scene elements for the deferred shading pass.</summary>
struct MeshAS
{
	/// <summary>Parameter constructor.</summary>
	MeshAS(int materialIdxParam, int indexOffsetParam, int numIndicesParam, glm::mat4 worldMatrixParam, pvrvk::IndexType indexTypeParam)
		: materialIdx(materialIdxParam), indexOffset(indexOffsetParam), numIndices(numIndicesParam), worldMatrix(worldMatrixParam), indexType(indexTypeParam)
	{}

	/// <summary>Material index used by this scene element.</summary>
	int materialIdx;

	/// <summary>Offset inside the index buffer for rasterizing this scene element.</summary>
	int indexOffset;

	/// <summary>Num indices of this scene element, used when rasterizing.</summary>
	int numIndices;

	/// <summary>Scene element transform.</summary>
	glm::mat4 worldMatrix;

	/// <summary>Enum to specify whether the indices of the index buffer are 16-bit or 32-bit unsigned int values.</summary>
	pvrvk::IndexType indexType;
};

/// <summary>Struct encapsulating a scene element that might be composed of multiple meshes.</summary>
struct ModelAS
{
	/// <summary>Vector with each one of the meshes that compose a scene element.</summary>
	std::vector<MeshAS> meshes;
};

/// <summary>Material struct for each scene mesh, replicated in the shaders.</summary>
struct Material
{
	/// <summary>Base color in case no texture is available to sample.</summary>
	glm::vec4 baseColor = glm::vec4(1.0f);

	/// <summary>Reflectance texture index.</summary>
	int reflectanceTextureIndex = -1;

	/// <summary>Index of refraction.</summary>
	float indexOfRefraction = 1.0f;

	/// <summary>Attenuation coefficient.</summary>
	float attenuationCoefficient = 1.0f;
};

/// <summary>Namespace with the names of the fields updated in teh shaders through the structured buffer views
/// globalBufferView and lightDataBufferView, note that tjose names have to match the variable name used in the demo shaders.</summary>
namespace ShaderStructFieldName {
/// <summary>LightData struct field name for the view matrix.</summary>
const char* const viewMatrix = "viewMatrix";

/// <summary>LightData struct field name for the projection matrix.</summary>
const char* const projectionMatrix = "projectionMatrix";

/// <summary>LightData struct field name for the inverse projection matrix.</summary>
const char* const inverseViewProjectionMatrix = "inverseViewProjectionMatrix";

/// <summary>LightData struct field name for the camera position.</summary>
const char* const cameraPosition = "cameraPosition";

/// <summary>Material struct field name for the light color.</summary>
const char* const lightColor = "lightColor";

/// <summary>Material struct field name for the light position and max ray recursion.</summary>
const char* const lightPositionMaxRayRecursion = "lightPositionMaxRayRecursion";

/// <summary>Material struct field name for the light ambient color and intensity.</summary>
const char* const ambientColorIntensity = "ambientColorIntensity";
}; // namespace ShaderStructFieldName

struct DeviceResources
{
	/// <summary>Encapsulation of a Vulkan instance.</summary>
	pvrvk::Instance instance;

	/// <summary>Callbacks and messengers for debug messages.</summary>
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;

	/// <summary>Encapsulation of the Vulkan surface, a rendereable part of the screen.</summary>
	pvrvk::Surface surface;

	/// <summary>Encapsulation of a Vulkan logical device.</summary>
	pvrvk::Device device;

	/// <summary>Queue where to submit commands.</summary>
	pvrvk::Queue queue;

	/// <summary>Struct with the familty and id of a particular queue.</summary>
	pvr::utils::QueueAccessInfo queueAccessInfo;

	/// <summary>Encapsulation of a Vulkan swapchain.</summary>
	pvrvk::Swapchain swapchain;

	/// <summary>vma allocator, only used to build the swapchain.</summary>
	pvr::utils::vma::Allocator vmaAllocator;

	/// <summary>Command ppol to allocate command buffers.</summary>
	pvrvk::CommandPool commandPool;

	/// <summary>Descriptor pool to allocate the descriptor sets.</summary>
	pvrvk::DescriptorPool descriptorPool;

	/// <summary>Array with the G-Buffer color attachment image views for the deferred rendering pass.</summary>
	pvrvk::ImageView gbufferImages[GBuffer::Size];

	/// <summary>Depth attachment image view for the deferred rendering pass.</summary>
	pvrvk::ImageView gbufferDepthStencilImage;

	/// <summary>Ray traced refractions image view for the ray tracing pass for dielectic materials, also
	/// the result of a shadow ray towards the scene light result is stored here.</summary>
	pvrvk::ImageView raytraceRefractionsImage;

	/// <summary>Temp image used for sotring gaussian blur pass for the ray traced refractions image.</summary>
	pvrvk::ImageView raytraceRefractionsGaussianBlurImage;

	/// <summary>Framebuffer for the deferred rendering pass.</summary>
	pvrvk::Framebuffer gbufferFramebuffer;

	/// <summary>Framebuffer for the Gaussian blur horizontal pass.</summary>
	pvrvk::Framebuffer gaussianBlurHorizontalPassFramebuffer;

	/// <summary>Framebuffer for the Gaussian blur vertical pass.</summary>
	pvrvk::Framebuffer gaussianBlurVerticalPassFramebuffer;

	/// <summary>Array with the on screen framebuffers (as many as the swap chain number of images).</summary>
	std::vector<pvrvk::Framebuffer> onScreenFramebuffer;

	/// <summary>render pass used for the GBuffer pass.</summary>
	pvrvk::RenderPass gbufferRenderPass;

	/// <summary>Render pass used for the Gaussian Blur.</summary>
	pvrvk::RenderPass gaussianBlurRenderPass;

	/// <summary>Primary command buffer where all the specific render pass secondary command buffers are recorded to.</summary>
	pvrvk::CommandBuffer cmdBufferMainDeferred[MAX_NUMBER_OF_SWAP_IMAGES];

	/// <summary>Secondary command buffer used for the deferred shading pass.</summary>
	pvrvk::SecondaryCommandBuffer cmdBufferGBuffer[MAX_NUMBER_OF_SWAP_IMAGES];

	/// <summary>Secondary command buffer used for the post processing part that does the final composition pass.</summary>
	pvrvk::SecondaryCommandBuffer cmdBufferDeferredShading[MAX_NUMBER_OF_SWAP_IMAGES];

	/// <summary>Secondary command buffer used for the Gaussian Blur horizontal pass.</summary>
	pvrvk::SecondaryCommandBuffer cmdBufferGaussianBlurHorizontal[MAX_NUMBER_OF_SWAP_IMAGES];

	/// <summary>Secondary command buffer used for the Gaussian Blur vertical pass.</summary>
	pvrvk::SecondaryCommandBuffer cmdBufferGaussianBlurVertical[MAX_NUMBER_OF_SWAP_IMAGES];

	/// <summary>Secondary command buffer used for the ray traced refractions pass.</summary>
	pvrvk::SecondaryCommandBuffer cmdBufferRayTracedRefractions[MAX_NUMBER_OF_SWAP_IMAGES];

	/// <summary>Descriptor set layout with most of the resources used in the ray tracing pass some in the GBuffer pass:
	/// camera data, light data, material buffer, material indices,	textures to sample array,
	/// acceleration structure to ray trace, vertex buffers array, index buffers array and scene description array.</summary>
	pvrvk::DescriptorSetLayout commonDescriptorSetLayout;

	/// <summary>Descriptor set layout to use with the three G-Buffer render targets and the sky box map in the ray tracing pass pass.</summary>
	pvrvk::DescriptorSetLayout gbufferSkyBoxDescriptorSetLayout;

	/// <summary>Descriptor set layout used in the ray traced refractions pass to store results.</summary>
	pvrvk::DescriptorSetLayout rtImageStoreDescriptorSetLayout;

	/// <summary>Descriptor set layout used in the deferred shading pass to sample results.</summary>
	pvrvk::DescriptorSetLayout rtImageSampleDescriptorSetLayout;

	/// <summary>Descriptor set layout used in the Gaussian blur horizontal pass.</summary>
	pvrvk::DescriptorSetLayout gaussianBlurDescriptorSetLayout;

	/// <summary>Descriptor set with most of the resources used in the ray tracing pass:
	/// camera data, light data, material buffer, material indices,	textures to sample array,
	/// acceleration structure to ray trace, vertex buffers array, index buffers array and scene description array.</summary>
	pvrvk::DescriptorSet commonDescriptorSet;

	/// <summary>Descriptor set to use with the three G-Buffer render targets and the sky box in the ray tracing pass pass.</summary>
	pvrvk::DescriptorSet gbufferSkyBoxDescriptorSet;

	/// <summary>Descriptor set to use with the horizontal Gaussian Blur pass.</summary>
	pvrvk::DescriptorSet gaussianBlurHorizontalDescriptorSet;

	/// <summary>Descriptor set to use with the vertical Gaussian Blur pass.</summary>
	pvrvk::DescriptorSet gaussianBlurVerticalDescriptorSet;

	/// <summary>Descriptor set used in the ray traced refractions pass to store results.</summary>
	pvrvk::DescriptorSet rtImageStoreDescriptorSet;

	/// <summary>Descriptor set used in the deferred shading pass to store results.</summary>
	pvrvk::DescriptorSet rtImageSampleDescriptorSet;

	/// <summary>Pipeline layout used in the deferred shading G-Buffer pass.</summary>
	pvrvk::PipelineLayout gbufferPipelineLayout;

	/// <summary>Pipeline layout used in the final final post processing composition pass.</summary>
	pvrvk::PipelineLayout deferredShadingPipelineLayout;

	/// <summary>Pipeline layout used in the ray traced refractions pass.</summary>
	pvrvk::PipelineLayout raytraceRefractionsPipelineLayout;

	/// <summary>Pipeline layout used in the Gaussian Blur horizontal and vertical passes.</summary>
	pvrvk::PipelineLayout gaussianBlurPipelineLayout;

	/// <summary>Vector with the vertex buffer data of each scene element to be ray traced.</summary>
	std::vector<pvrvk::Buffer> vertexBuffers;

	/// <summary>Vector with the index buffer data of each scene element to be ray traced.</summary>
	std::vector<pvrvk::Buffer> indexBuffers;

	/// <summary>Vector with general information of each scene element, used to raster scene elements in the GBuffer pass.</summary>
	std::vector<ModelAS> models;

	/// <summary>Vector with the amount of vertices each scene element to be ray traced has, used to build the Bottom Level Acceleration Structure.</summary>
	std::vector<int> verticesSize;

	/// <summary>Vector with the amount of indices each scene element to be ray traced has, used to build the Bottom Level Acceleration Structure.</summary>
	std::vector<int> indicesSize;

	/// <summary>Vector with the material index each triangle has, used for ray tracing.</summary>
	std::vector<pvrvk::Buffer> materialIndexBuffers;

	/// <summary>Vector with all the textures used by all the scene elements, sampled in the ray tracing pass.</summary>
	std::vector<TextureAS> textures;

	/// <summary>Acceleration structure wrapper encapsulating a TLAS and its corresponding BLAS.</summary>
	pvr::utils::AccelerationStructureWrapper accelerationStructure;

	/// <summary>Buffer with all the material information for all scene elements, each element of this buffer is a struct of type "Material" befined at the beginning of this file.</summary>
	pvrvk::Buffer materialBuffer;

	/// <summary>Buffer with the shader binding table information used in ray tracing.</summary>
	pvrvk::Buffer raytraceRefractionShaderBindingTable;

	/// <summary>Structured buffer view with camera information, where a set of fields in the shader are defined and updated from host side.</summary>
	pvr::utils::StructuredBufferView globalBufferView;

	/// <summary>Buffer holding the information managed in the structured buffer view globalBufferView.</summary>
	pvrvk::Buffer globalBuffer;

	/// <summary>Structured buffer view with the scene light information.</summary>
	pvr::utils::StructuredBufferView lightDataBufferView;

	/// <summary>Buffer holding the information managed in the structured buffer view lightDataBufferView.</summary>
	pvrvk::Buffer lightDataBuffer;

	/// <summary>Structured buffer view with transform information per scene element.</summary>
	pvr::utils::StructuredBufferView perMeshTransformBufferView;

	/// <summary>Buffer holding the information managed in the structured buffer view perMeshTransformBufferView.</summary>
	pvrvk::Buffer perMeshTransformBuffer;

	/// <summary>Buffer with the information in _sceneDescription which is used for ray tracing (to transform each ray intersection into world space).</summary>
	pvrvk::Buffer sceneDescription;

	/// <summary>Top level acceleration structure information about the single instance in the scene for the scene descriptor buffer used.</summary>
	std::vector<pvr::utils::SceneDescription> _sceneDescription;

	/// <summary>Semaphores for when acquiring the next image from the swap chain, one per swapchain image.</summary>
	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;

	/// <summary>Semaphores for when submitting the command buffer for the current swapchain image.</summary>
	std::vector < pvrvk::Semaphore> presentationSemaphores;

	/// <summary>Fences for each of the per-frame command buffers, one per swapchain image.</summary>
	std::vector<pvrvk::Fence> perFrameResourcesFences;

	/// <summary>Graphics pipeline used in the Deferred shading pass to fill the G-Buffer.</summary>
	pvrvk::GraphicsPipeline gbufferPipeline;

	/// <summary>Graphics pipeline used in the final composition pass to merge the ray traced refractions with rasterized elements.</summary>
	pvrvk::GraphicsPipeline deferredShadingPipeline;

	/// <summary>Graphics pipeline used in the Gaussian Blur horizontal pass.</summary>
	pvrvk::GraphicsPipeline gaussianBlurHorizontalPassPipeline;

	/// <summary>Graphics pipeline used in the Gaussian Blur vertical pass.</summary>
	pvrvk::GraphicsPipeline gaussianBlurVerticalPassPipeline;

	/// <summary>Ray tracing pipeline used to ray trace the refractive scene elements.</summary>
	pvrvk::RaytracingPipeline raytraceRefractionPipeline;

	/// <summary>Image view used to sample the skybox used in the demo.</summary>
	pvrvk::ImageView skyBoxMap;

	/// <summary>Pipeline cache used to build the pipelines.</summary>
	pvrvk::PipelineCache pipelineCache;

	/// <summary>UIRenderer used to display text.</summary>
	pvr::ui::UIRenderer uiRenderer;

	/// <summary>Create info struct used for several postprocessing steps (Gaussian blur horizontal and vetical
	/// passes, final deferred shading pass).</summary>
	pvrvk::GraphicsPipelineCreateInfo _postProcessingPipelineCreateInfo;

	/// <summary>Linear sample to be used for several descriptor sets.</summary>
	pvrvk::Sampler samplerLinear;

	/// <summary>Default destructor.</summary>
	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanHybridRefractions : public pvr::Shell
{
public:
	/// <summary>Number of swap chain images used.</summary>
	uint32_t _numSwapImages;

	/// <summary>Current swap chain image index.</summary>
	uint32_t _swapchainIndex;

	/// <summary>Pointer to struct encapsulating all the resources made with the current logical device.</summary>
	std::unique_ptr<DeviceResources> _deviceResources;

	/// <summary>Ray Tracing properties struct holding important information like the size of a shader group for the Shader Binding Table.</summary>
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR _rtProperties;

	/// <summary>Number of shader groups, three in this sample: Ray generation, ray miss and ray hit.</summary>
	uint32_t _shaderGroupCount = 0;

	/// <summary>Swapchain image index, in interval [0, numSwapChainImages - 1].</summary>
	uint32_t _frameId;

	/// <summary>Scene camera position.</summary>
	glm::vec3 _cameraPosition;

	/// <summary>Scene camera look at position.</summary>
	glm::vec3 _cameraLookAt;

	/// <summary>Scene camera up direction.</summary>
	glm::vec3 _cameraUpDirection;

	/// <summary>Scene camera field of view.</summary>
	float _cameraFieldOfView;

	/// <summary>Struct holding the scene light data, which passed to the buffer lightDataBuffer through the structured buffer view lightDataBufferView.</summary>
	LightData _lightData;

	/// <summary>Camera view matrix, used in the globalBuffer buffer and updated through the globalBufferView structured buffer view.</summary>
	glm::mat4 _viewMatrix;

	/// <summary>Camera projection matrix, used in the globalBuffer buffer and updated through the globalBufferView structured buffer view.</summary>
	glm::mat4 _projectionMatrix;

	/// <summary>Camera view projection matrix, used in the globalBuffer buffer and updated through the globalBufferView structured buffer view.</summary>
	glm::mat4 _viewProjectionMatrix;

	/// <summary>Camera inverse view matrix, used in the globalBuffer buffer and updated through the globalBufferView structured buffer view.</summary>
	glm::mat4 _inverseViewMatrix;

	/// <summary>Window width.</summary>
	uint32_t _windowWidth;

	/// <summary>Window height.</summary>
	uint32_t _windowHeight;

	/// <summary>Framebuffer width.</summary>
	uint32_t _framebufferWidth;

	/// <summary>Framebuffer height.</summary>
	uint32_t _framebufferHeight;

	/// <summary>Viewport offsets to define rendering areas.</summary>
	int32_t _viewportOffsets[2];

	/// <summary>Vector with the scene elements.</summary>
	std::vector<pvr::assets::ModelHandle> _models;

	/// <summary>Vector with the transform of each scene element.</summary>
	std::vector<glm::mat4> _vectorModelTransform;

	/// <summary>Initial transform of the torus mesh, used to animate the two torus meshes in the scene
	glm::mat4 _initialTorusTransform;

	/// <summary>Initial transform of the baloon mesh, used to animate the three baloon meshes in the scene
	glm::mat4 _initialBaloonTransform;

	/// <summary>Vector with the angle to apply to each scene element as part of animating the scene.</summary>
	std::vector<float> _vectorModelAngleRotation;

	/// <summary>Helper variable to rotate the scene elements (baloons) to animate the scene.</summary>
	glm::vec3 _positiveYAxis;

	/// <summary>Array with the formats of the render targets used in the G-Buffer deferred shading pass.</summary>
	pvrvk::Format _renderpassStorageFormats[GBuffer::Size];

	/// <summary>Flag to toggle the scene elements update.</summary>
	bool _updateScene;

	/// <summary>Offsets for the sampling texture coordinates for the Gaussian blur passes using linear sampling.</summary>
	std::vector<double> _gaussianOffsets;

	/// <summary>Weights for the sampling texture coordinates for the Gaussian blur passes using linear sampling.</summary>
	std::vector<double> _gaussianWeights;

	/// <summary>Flag to know whether astc iss upported by the physical device.</summary>
	bool _astcSupported;

	/// <summary>Filter performance warning UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation Best Practices which
	/// has ID -602362517 for TLAS buffer build and update. This warning recommends buffer allocations to be of size at least
	/// 256KB which collides with each BLAS node built for each scene element and the size of the TLAS buffer, details of the warning:
	/// https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/layers/best_practices_validation.h</summary>
	std::vector<int> _vectorValidationIDFilter;

	/// <summary>Depth stencil format to use.</summary>
	pvrvk::Format _depthStencilFormat;

	/// <summary>Value of VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayRecursionDepth to know the maximum recursion depth when using ray tracing.</summary>
	uint32_t _maxRayRecursionDepth;

	/// <summary>Default constructor.</summary>
	VulkanHybridRefractions()
		: _numSwapImages(0), _swapchainIndex(0), _rtProperties({}), _shaderGroupCount(0), _frameId(0), _cameraPosition(glm::vec3(0.0f)), _cameraLookAt(glm::vec3(0.0f)),
		  _cameraUpDirection(glm::vec3(0.0f)), _cameraFieldOfView(0.0f), _lightData({}), _viewMatrix(glm::mat4(1.0f)), _projectionMatrix(glm::mat4(1.0f)),
		  _viewProjectionMatrix(glm::mat4(1.0f)), _windowWidth(0), _windowHeight(0), _framebufferWidth(0), _framebufferHeight(0), _viewportOffsets{ 0 },
		  _positiveYAxis(glm::vec3(0.0f, 1.0f, 0.0f)), _renderpassStorageFormats{ pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Format::e_R16G16B16A16_SFLOAT,
			  pvrvk::Format::e_R16G16B16A16_SFLOAT },
		  _updateScene(true), _depthStencilFormat(pvrvk::Format::e_UNDEFINED)
	{}

	/// <summary>This event represents application start. When implementing, return a suitable error code to signify failure. If pvr::Result::Success
	/// is not returned, the Shell will detect that, clean up, and exit. It will be fired once, on start, before any other callback and before
	/// Graphics Context aquisition.It is suitable to do per - run initialisation, load assets files and similar tasks. A context does not exist yet,
	/// hence if the user tries to create API objects, they will fail and the behaviour is undefined.</summary>
	/// <returns>When implementing, return a suitable error code to signify failure. If pvr::Result::Success is not
	/// returned, the Shell will detect that, clean up, and exit.</returns>
	virtual pvr::Result initApplication();

	/// <summary>Build the device and queues, adding the required extensions for the demo, including the Vulkan ray tracing ones.</summary>
	/// <returns>pvr::Result Success if initialisation succeeded, pvr::Result UnsupportedRequest if the extensions are not
	/// supported by the physical device.</returns>
	pvr::Result buildDeviceAndQueues();

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

	/// <summary>Build the descriptor set layout with resources mainly used in the ray tracing pass.</summary>
	void buildCommonDescriptorSetLayout();

	/// <summary>Build the descriptor set layout with the three textures used in the GBuffer and the skybox texture.</summary>
	void buildGBufferSkyboxDescriptorSetLayout();

	/// <summary>Build the descriptor set layout with the image where the results of the ray traced refractions pass are stored.</summary>
	void buildWriteRefractionsImageDescriptorSetLayout();

	/// <summary>Build the descriptor set layout with the texture used for the Gaussian Blur pass.</summary>
	void buildGaussianBlurDescriptorSetLayout();

	/// <summary>Build all the descriptor sets used in the sample(common descriptor set with all the resources from ray tracing, deferred shading pass,
	///	deferred shading per scene element dynamic uniform buffer, ray traced image and deferred shading final composition pass.</summary>
	void buildDescriptorSetLayouts();

	/// <summary>Build the descriptor set with resources mainly used in the ray tracing pass.</summary>
	void buildCommonDescriptorSet();

	/// <summary>Build the descriptor set with the three textures used in the GBuffer and the skybox texture.</summary>
	void buildGBufferSkyboxDescriptorSet();

	/// <summary>Build the descriptor set with the image where the results of the ray traced refractions pass are stored.</summary>
	void buildWriteRefractionsImageDescriptorSet();

	/// <summary>Build the descriptor set with the texture used for the horizontal Gaussian Blur pass.</summary>
	void buildGaussianBlurHorizontalDescriptorSet();

	/// <summary>Build the descriptor set with the texture used for the vertical Gaussian Blur pass.</summary>
	void buildGaussianBlurVerticalDescriptorSet();

	/// <summary>Build the descriptor sets to be used in all the shaders in the sample.</summary>
	void buildDescriptorSets();

	/// <summary>Helper function to define the vertex format and input attributes.</summary>
	void definePipelineVertexInputState(pvrvk::PipelineVertexInputStateCreateInfo& pipelineVertexInputStateCreateInfo);

	/// <summary>Build the pipeline used in the deferred shading pass pass.</summary>
	void buildGBufferPipeline();

	/// <summary>Build the pipeline used in the ray traced refractions pass.</summary>
	void buildRayTracingPipeline();

	/// <summary>Build the pipeline for the final composition pass where the ray traced refractions are added to the rasterization of the not ray traced scene elements.</summary>
	void buildDeferredShadingPipeline();

	/// <summary>Build the pipeline for the Gaussian Blur horizontal pass.</summary>
	void buildGaussianBlurHorizontalPipeline();

	/// <summary>Build the pipeline for the Gaussian Blur vertical pass.</summary>
	void buildGaussianBlurVerticalPipeline();

	/// <summary>Build the shader binding table buffer, gather the shader group handles from the pipeline and prepare the shader binding table content in the buffer.</summary>
	void buildShaderBindingTable();

	/// <summary>Build each one of the pipelines for this example (G-Buffer, ray tracing, deferred shading and sky box).</summary>
	void buildPipelines();

	/// <summary>Build the textures used in the G-Buffer the imaghe where to store ray tracing results.</summary>
	void buildFramebufferAndRayTracingStoreImage();

	/// <summary>Build the renderpass used for the GBuffer pass.</summary>
	void buildRenderPass();

	/// <summary>Build the renderpass used for the Gaussian Blur.</summary>
	void buildGaussianBlurRenderPass();

	/// <summary>Takes the list of populated textures used in the scene and loads them into memory, uploads them into a Vulkan image and creates image views.</summary>
	/// <param name="uploadCmd">Command Buffer used to record the image upload commands.</param>
	void createTextures(pvrvk::CommandBuffer& uploadCmd);

	/// <summary>Builds the scene description buffer, sceneDescription, with the information present at
	/// DeviceResources::_sceneDescription to compute in the ray hit shader the world position of the ray hit coordinates.</ summary>
	void buildSceneDescriptionBuffer();

	/// <summary>Override of Shell::eventMappedInput. This event abstracts, maps and unifies several input devices.</summary>
	/// <param name="key">The Simplified Unified Event</param>
	virtual void eventMappedInput(pvr::SimplifiedInput key);

	/// <summary>Builds the vertex, index and material index buffers of each scene element to be used for ray tracing.</summary>
	/// <param name="uploadCmd">Command Buffer used to record the buffer upload commands.</param>
	void buildModelBuffers(pvrvk::CommandBuffer& uploadCmd);

	/// <summary>Build the globalBuffer and perMeshTransformBuffer buffers with camera information and transform per-scene
	/// element respectively. Those buffers are updated through the globalBufferView and perMeshTransformBufferView structured
	/// buffer view.</summary>
	void buildCameraBuffer();

	/// <summary>Build the buffer structured view DeviceResources::perMeshTransformBufferView and the buffer that will ohld the
	/// information, DeviceResources::perMeshTransformBuffer.</summary>
	void buildSceneElementTransformBuffer();

	/// <summary>Define the fields of the lightDataBufferView and build the lightDataBuffer buffer which holds the actual values.</summary>
	void buildLightDataBuffer();

	/// <summary>Build the material buffer and the texture information to be sampled for the different scene elements in the ray tracing pass.</summary>
	void buildMaterialBuffer(pvrvk::CommandBuffer& uploadCmd);

	/// <summary>Update globalBuffer and lightDataBuffer information through the globalBufferView and lightDataBufferView structured buffer views.</summary>
	void updateCameraLightData();

	/// <summary>Updates scene elements transforms for both raster and ray tracing.</summary>
	void updateScene();

	/// <summary>Records in the main command buffer cmdBufferMainDeferred all the secondary command buffers for the whole set of passes in the demo.</summary>
	void recordMainCommandBuffer();

	/// <summary>record all the secondary command buffers used in the whole sample (recordCommandBufferRenderGBuffer, recordCommandBufferDeferredShading, recordCommandUIRenderer, recordCommandBufferRayTraceRefractions).</summary>
	void recordSecondaryCommandBuffers();

	/// <summary>Record to secondary command buffer cmdBufferGBuffer the defered shading pass commands.</summary>
	/// <param name="cmdBuffer">SecondaryCommandbuffer to record.</param>
	/// <param name="swapchainIndex">Index of the current swapchain image.</param>
	void recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex);

	/// <summary>Record to secondary command buffer cmdBufferRayTracedRefractions the defered shading pass commands.</summary>
	/// <param name="cmdBuffer">SecondaryCommandbuffer to record.</param>
	/// <param name="swapchainIndex">Index of the current swapchain image.</param>
	void recordCommandBufferRayTraceRefractions(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex);

	/// <summary>Record to secondary command buffer cmdBufferDeferredShading the defered shading pass commands.</summary>
	/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
	/// <param name="swapchainIndex">Index of the current swapchain image.</param>
	void recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex);

	/// <summary>Record to secondary command buffer cmdBufferGaussianBlur the Gaussian Blur horizontal shading pass commands.</summary>
	/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
	/// <param name="swapchainIndex">Index of the current swapchain image.</param>
	void recordCommandBufferHorizontalGaussianBlur(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex);

	/// <summary>Record to secondary command buffer cmdBufferGaussianBlur the Gaussian Blur vertical shading pass commands.</summary>
	/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
	/// <param name="swapchainIndex">Index of the current swapchain image.</param>
	void recordCommandBufferVerticalGaussianBlur(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex);

	/// <summary>Record to secondary command buffer cmdBufferDeferredShading the user interface rendering commands.</summary>
	/// <param name="commandBuff">Commandbuffer to record.</param>
	void recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& cmdBuffer);
};

pvr::Result VulkanHybridRefractions::initApplication()
{
	const char* const torusMeshFile = "Refractions.pod"; // Name of the POD scene file with the torus mesh
	const char* const baloonMeshFile = "Balloon.pod"; // Name of the POD scene file with the balloon mesh

	//  Load the scene, two torus mesh and three baloon mesh
	_models.resize(5);
	_models[0] = pvr::assets::loadModel(*this, torusMeshFile);
	_models[1] = _models[0];
	_models[2] = pvr::assets::loadModel(*this, baloonMeshFile);
	_models[3] = _models[2];
	_models[4] = _models[3];

	// Store the initial scene transforms of the torus and baloon meshes
	_initialTorusTransform = _models[0]->getWorldMatrix(_models[0]->getNode(0).getObjectId());
	_initialBaloonTransform = _models[2]->getWorldMatrix(_models[2]->getNode(0).getObjectId());

	_vectorModelAngleRotation = { 0.6370f, 0.3141f, 0.0f, glm::pi<float>() / 5.0f, glm::pi<float>() * 2.0f / 5.0f };

	// Rotate the torus and baloon meshes to animate them
	_vectorModelTransform.resize(5);
	_vectorModelTransform[0] = _initialTorusTransform * glm::rotate(0.6370f, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.57f));
	_vectorModelTransform[1] = _initialTorusTransform * glm::rotate(0.3141f, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::vec3(0.295f));
	_vectorModelTransform[2] = _initialBaloonTransform * glm::rotate(_vectorModelAngleRotation[2], _positiveYAxis) *
		glm::translate(glm::vec3(80.f + 0 * 40.f, sin(_vectorModelAngleRotation[2] * 3.0f) * 20.0f, 0.0f)) * glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
	_vectorModelTransform[3] = _initialBaloonTransform * glm::rotate(_vectorModelAngleRotation[3], _positiveYAxis) *
		glm::translate(glm::vec3(80.f + 1 * 40.f, sin(_vectorModelAngleRotation[3] * 3.0f) * 20.0f, 0.0f)) * glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
	_vectorModelTransform[4] = _initialBaloonTransform * glm::rotate(_vectorModelAngleRotation[4], _positiveYAxis) *
		glm::translate(glm::vec3(80.f + 2 * 40.f, sin(_vectorModelAngleRotation[4] * 3.0f) * 20.0f, 0.0f)) * glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));

	// Setup scene camera information
	uint32_t cameraIndex = 0;
	_models[0]->getCameraProperties(cameraIndex, _cameraFieldOfView, _cameraPosition, _cameraLookAt, _cameraUpDirection);
	_cameraLookAt = glm::normalize(_cameraLookAt - _cameraPosition);
	_cameraPosition = _cameraPosition + (_cameraLookAt * 50.0f);
	_cameraLookAt += glm::vec3(-9.75f, -4.5f, 0.0f);
	_viewMatrix = glm::lookAt(_cameraPosition, _cameraLookAt, _cameraUpDirection);
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	_inverseViewMatrix = glm::inverse(_viewMatrix);

	pvr::math::generateGaussianKernelWeightsAndOffsets(3, false, true, _gaussianWeights, _gaussianOffsets);

	return pvr::Result::Success;
}

pvr::Result VulkanHybridRefractions::buildDeviceAndQueues()
{
	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, _deviceResources->surface };

	// device extensions
	std::vector<std::string> vectorExtensionNames{ VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, VK_KHR_SPIRV_1_4_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME };

	std::vector<int> vectorPhysicalDevicesIndex = pvr::utils::validatePhysicalDeviceExtensions(_deviceResources->instance, vectorExtensionNames);

	if (vectorPhysicalDevicesIndex.size() == 0)
	{
		throw pvrvk::ErrorInitializationFailed("Could not find all the required Vulkan extensions.");
		return pvr::Result::UnsupportedRequest;
	}

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
	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceFeatures2(_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0])->getVkHandle(), &deviceFeatures);

	// Add these device features to the physical device, since they're all connected by a pNext chain, we only need to explicitly attach the top feature
	deviceExtensions.addExtensionFeatureVk<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&raytracingPipelineFeatures);

	// create device and queues
	_deviceResources->device = pvr::utils::createDeviceAndQueues(
		_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0]), &queuePopulateInfo, 1, &_deviceResources->queueAccessInfo, deviceExtensions);

	return pvr::Result::Success;
}

pvr::Result VulkanHybridRefractions::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create instance and targetting Vulkan version 1.1 and retrieve compatible physical devices
	pvr::utils::VulkanVersion vulkanVersion(1, 1, 0);
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), vulkanVersion, pvr::utils::InstanceExtensions(vulkanVersion));

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	_deviceResources->surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Filter UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation Best Practices performance warning which has ID -602362517 for TLAS buffer build and
	// update (VkBufferDeviceAddressInfo requires VkBuffer handle so in general it's not possible to make a single buffer to put all information
	// and use offsets inside it.
	// Filter UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation with ID -1277938581 related with allocation sizes.
	_vectorValidationIDFilter.push_back(-602362517);
	_vectorValidationIDFilter.push_back(-1277938581);

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance, (void*)&_vectorValidationIDFilter);

	// Create device and queues
	pvr::Result resultDeviceAndQueues = buildDeviceAndQueues();

	if (resultDeviceAndQueues != pvr::Result::Success) { return resultDeviceAndQueues; }

	// get queue
	_deviceResources->queue = _deviceResources->device->getQueue(_deviceResources->queueAccessInfo.familyId, _deviceResources->queueAccessInfo.queueId);
	_deviceResources->queue->setObjectName("GraphicsQueue");

	// create the command pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	// create vulkan memory allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->device->getPhysicalDevice()->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	} // create the swapchain

	// We do not support automatic MSAA for this demo.
	if (getDisplayAttributes().aaSamples > 1)
	{
		Log(LogLevel::Warning, "Full Screen Multisample Antialiasing requested, but not supported for this demo's configuration.");
		getDisplayAttributes().aaSamples = 1;
	}

	// Create the Swapchain
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, _deviceResources->surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters(false).setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// Get the number of swap images
	_numSwapImages = _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->imageAcquiredSemaphores.resize(_numSwapImages);
	_deviceResources->presentationSemaphores.resize(_numSwapImages);
	_deviceResources->perFrameResourcesFences.resize(_numSwapImages);

	// Get current swap index
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, static_cast<uint16_t>(16 * _numSwapImages))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, static_cast<uint16_t>(16 * _numSwapImages))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(16 * _numSwapImages))
																						  .setMaxDescriptorSets(static_cast<uint16_t>(16 * _numSwapImages)));

	_deviceResources->descriptorPool->setObjectName("DescriptorPool");

	// calculate the frame buffer width and heights
	_framebufferWidth = this->getWidth();
	_windowWidth = this->getWidth();
	_framebufferHeight = this->getHeight();
	_windowHeight = this->getHeight();

	const pvr::CommandLine& commandOptions = getCommandLine();
	int32_t intFramebufferWidth = -1;
	int32_t intFramebufferHeight = -1;
	commandOptions.getIntOption("-fbowidth", intFramebufferWidth);
	commandOptions.getIntOption("-fboheight", intFramebufferHeight);
	_framebufferWidth = intFramebufferWidth == -1 ? _windowWidth : intFramebufferWidth;
	_framebufferHeight = intFramebufferHeight == -1 ? _windowHeight : intFramebufferHeight;

	_viewportOffsets[0] = (_windowWidth - _framebufferWidth) / 2;
	_viewportOffsets[1] = (_windowHeight - _framebufferHeight) / 2;

	Log(LogLevel::Information, "Framebuffer dimensions: %d x %d\n", _framebufferWidth, _framebufferHeight);
	Log(LogLevel::Information, "On-screen Framebuffer dimensions: %d x %d\n", _windowWidth, _windowHeight);

	// setup command buffers
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		// main command buffer
		_deviceResources->cmdBufferMainDeferred[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->cmdBufferGBuffer[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferDeferredShading[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferGaussianBlurHorizontal[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferGaussianBlurVertical[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferRayTracedRefractions[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->cmdBufferMainDeferred[i]->setObjectName("DeferredCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferGBuffer[i]->setObjectName("GBufferSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferDeferredShading[i]->setObjectName("DeferredShadingSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferGaussianBlurHorizontal[i]->setObjectName("GaussianBlurHorizontalSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferGaussianBlurVertical[i]->setObjectName("GaussianBlurVerticalSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferRayTracedRefractions[i]->setObjectName("RayTracedRefractionsSecondaryCommandBufferSwapchain" + std::to_string(i));

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));
	}

	// Handle device rotation
	bool isRotated = this->isScreenRotated();
	if (isRotated)
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _models[0]->getCamera(0).getFOV(), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()),
			_models[0]->getCamera(0).getNear(), _models[0]->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _models[0]->getCamera(0).getFOV(),
			static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), _models[0]->getCamera(0).getNear(), _models[0]->getCamera(0).getFar());
	}

	// Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("Hybrid Refractions");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action 1: Toggle Animate");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// get ray tracing properties
	_rtProperties.sType = static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR);
	_rtProperties.pNext = nullptr;
	VkPhysicalDeviceProperties2 properties{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_PROPERTIES_2) };
	properties.pNext = &_rtProperties;
	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceProperties2(_deviceResources->device->getPhysicalDevice()->getVkHandle(), &properties);

	_astcSupported = pvr::utils::isSupportedFormat(_deviceResources->device->getPhysicalDevice(), pvrvk::Format::e_ASTC_4x4_UNORM_BLOCK);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	_deviceResources->cmdBufferMainDeferred[0]->begin();

	buildModelBuffers(_deviceResources->cmdBufferMainDeferred[0]);
	buildMaterialBuffer(_deviceResources->cmdBufferMainDeferred[0]);
	createTextures(_deviceResources->cmdBufferMainDeferred[0]);
	_deviceResources->cmdBufferMainDeferred[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBufferMainDeferred[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle(); // wait

	_maxRayRecursionDepth = _rtProperties.maxRayRecursionDepth;

	buildSceneDescriptionBuffer();
	buildFramebufferAndRayTracingStoreImage();
	buildRenderPass();
	buildGaussianBlurRenderPass();
	buildCameraBuffer();
	buildSceneElementTransformBuffer();
	buildLightDataBuffer();
	buildDescriptorSetLayouts();
	buildPipelines();
	buildShaderBindingTable();

	_deviceResources->accelerationStructure.buildASModelDescription(
		_deviceResources->vertexBuffers, _deviceResources->indexBuffers, _deviceResources->verticesSize, _deviceResources->indicesSize, _vectorModelTransform);
	_deviceResources->accelerationStructure.buildAS(_deviceResources->device, _deviceResources->queue, _deviceResources->cmdBufferMainDeferred[0],
		pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR | pvrvk::BuildAccelerationStructureFlagsKHR::e_ALLOW_UPDATE_BIT_KHR);

	buildDescriptorSets();
	recordSecondaryCommandBuffers();
	recordMainCommandBuffer();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanHybridRefractions::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

pvr::Result VulkanHybridRefractions::quitApplication() { return pvr::Result::Success; }

pvr::Result VulkanHybridRefractions::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	//  Handle user input and update object animations
	updateScene();

	// Upload dynamic data
	updateCameraLightData();

	// submit the main command buffer
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;

	submitInfo.commandBuffers = &_deviceResources->cmdBufferMainDeferred[_swapchainIndex];

	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[_swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, _swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _numSwapImages;

	return pvr::Result::Success;
}

void VulkanHybridRefractions::buildCommonDescriptorSetLayout()
{
	// Binding 0: Camera data
	// Binding 1: Light data
	// Binding 2: Material buffer
	// Binding 3: Material indices
	// Binding 4: Textures to sample array
	// Binding 5: Acceleration structure to ray trace
	// Binding 6: Vertex buffers array
	// Binding 7: Index buffers array
	// Binding 8: Scene description array
	// Binding 9: Per-mesh transform buffer

	// Dynamic per scene buffer
	pvrvk::DescriptorSetLayoutCreateInfo commonDescSetInfo;
	commonDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);

	// Dynamic per light buffer
	commonDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);

	// Static material data buffer
	commonDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u,
		pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_ANY_HIT_BIT_KHR);

	// Static material indices buffer
	commonDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(_deviceResources->models.size()),
		pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_ANY_HIT_BIT_KHR);

	// Static material image array
	commonDescSetInfo.setBinding(4, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(_deviceResources->textures.size()),
		pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_ANY_HIT_BIT_KHR);

	// TLAS
	commonDescSetInfo.setBinding(5, pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// Vertex buffers
	commonDescSetInfo.setBinding(6, pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(_deviceResources->vertexBuffers.size()),
		pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// Index buffers
	commonDescSetInfo.setBinding(7, pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(_deviceResources->indexBuffers.size()),
		pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// Scene descriptor set
	commonDescSetInfo.setBinding(8, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u, pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);

	// Per mesh transform buffer
	commonDescSetInfo.setBinding(9, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

	_deviceResources->commonDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(commonDescSetInfo);
}

void VulkanHybridRefractions::buildGBufferSkyboxDescriptorSetLayout()
{
	// Binding 0: GBuffer reflectance image
	// Binding 1: GBuffer normalMaterialID image
	// Binding 2: GBuffer worldPositionIOR image
	// Binding 3: Sky box

	pvrvk::DescriptorSetLayoutCreateInfo gbufferDescSetInfo;

	// GBuffer reflectance image
	gbufferDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_MISS_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// GBuffer normalMaterialID image
	gbufferDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_MISS_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// GBuffer worldPositionIOR image
	gbufferDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_MISS_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// Sky box
	gbufferDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_MISS_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->gbufferSkyBoxDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(gbufferDescSetInfo);
}

void VulkanHybridRefractions::buildWriteRefractionsImageDescriptorSetLayout()
{
	// Binding 0: ray traced refractions store image

	pvrvk::DescriptorSetLayoutCreateInfo imageDescSetInfo;
	imageDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->rtImageStoreDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(imageDescSetInfo);
}

void VulkanHybridRefractions::buildGaussianBlurDescriptorSetLayout()
{
	// Binding 0: Texture to sample from

	pvrvk::DescriptorSetLayoutCreateInfo imageDescSetInfo;
	imageDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->gaussianBlurDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(imageDescSetInfo);
}

void VulkanHybridRefractions::buildDescriptorSetLayouts()
{
	buildCommonDescriptorSetLayout();
	buildGBufferSkyboxDescriptorSetLayout();
	buildWriteRefractionsImageDescriptorSetLayout();
	buildGaussianBlurDescriptorSetLayout();
}

void VulkanHybridRefractions::buildCommonDescriptorSet()
{
	// Binding 0: Camera data
	// Binding 1: Light data
	// Binding 2: Material buffer
	// Binding 3: Material indices
	// Binding 4: Textures to sample array
	// Binding 5: Acceleration structure to ray trace
	// Binding 6: Vertex buffers array
	// Binding 7: Index buffers array
	// Binding 8: Scene description array
	// Binding 9: Per-mesh transform buffer

	// Scene Sampler
	pvrvk::SamplerCreateInfo samplerDesc;
	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = pvrvk::SamplerAddressMode::e_REPEAT;
	samplerDesc.minFilter = pvrvk::Filter::e_LINEAR;
	_deviceResources->samplerLinear = _deviceResources->device->createSampler(samplerDesc);

	// Allocate Descriptor Set
	_deviceResources->commonDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->commonDescriptorSetLayout);
	_deviceResources->commonDescriptorSet->setObjectName("CommonDescriptorSet");

	pvrvk::WriteDescriptorSet globalBufferWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 0);
	pvrvk::WriteDescriptorSet lightDataWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 1);
	pvrvk::WriteDescriptorSet materialBufferWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 2);
	pvrvk::WriteDescriptorSet materialIndicesWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 3);
	pvrvk::WriteDescriptorSet sampledTexturesWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->commonDescriptorSet, 4);
	pvrvk::WriteDescriptorSet accelerationStructureWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, _deviceResources->commonDescriptorSet, 5);
	pvrvk::WriteDescriptorSet vertexBufferArrayWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 6);
	pvrvk::WriteDescriptorSet indexBufferArrayWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 7);
	pvrvk::WriteDescriptorSet sceneDescriptionWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 8);
	pvrvk::WriteDescriptorSet sceneTransformWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 9);

	globalBufferWDS.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->globalBuffer, 0, _deviceResources->globalBufferView.getDynamicSliceSize()));
	lightDataWDS.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->lightDataBuffer, 0, _deviceResources->lightDataBufferView.getDynamicSliceSize()));

	materialBufferWDS.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->materialBuffer, 0, VK_WHOLE_SIZE));
	for (size_t i = 0; i < _deviceResources->materialIndexBuffers.size(); i++)
	{
		materialIndicesWDS.setBufferInfo(i, pvrvk::DescriptorBufferInfo(_deviceResources->materialIndexBuffers[i], 0, _deviceResources->materialIndexBuffers[i]->getSize()));
	}

	for (size_t i = 0; i < _deviceResources->textures.size(); i++)
	{
		sampledTexturesWDS.setImageInfo(
			i, pvrvk::DescriptorImageInfo(_deviceResources->textures[i].imageView, _deviceResources->samplerLinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	}

	accelerationStructureWDS.setAccelerationStructureInfo(0, _deviceResources->accelerationStructure.getTopLevelAccelerationStructure());

	for (size_t i = 0; i < _deviceResources->vertexBuffers.size(); i++)
	{
		vertexBufferArrayWDS.setBufferInfo(i, pvrvk::DescriptorBufferInfo(_deviceResources->vertexBuffers[i], 0, _deviceResources->vertexBuffers[i]->getSize()));
	}

	for (size_t i = 0; i < _deviceResources->indexBuffers.size(); i++)
	{
		indexBufferArrayWDS.setBufferInfo(i, pvrvk::DescriptorBufferInfo(_deviceResources->indexBuffers[i], 0, _deviceResources->indexBuffers[i]->getSize()));
	}

	sceneDescriptionWDS.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->sceneDescription, 0, VK_WHOLE_SIZE));

	sceneTransformWDS.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->perMeshTransformBuffer, 0, _deviceResources->perMeshTransformBufferView.getDynamicSliceSize()));

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets = { globalBufferWDS, lightDataWDS, materialBufferWDS, materialIndicesWDS, sampledTexturesWDS, accelerationStructureWDS,
		vertexBufferArrayWDS, indexBufferArrayWDS, sceneDescriptionWDS, sceneTransformWDS };

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanHybridRefractions::buildGBufferSkyboxDescriptorSet()
{
	// Binding 0: GBuffer reflectance image
	// Binding 1: GBuffer normalMaterialID image
	// Binding 2: GBuffer worldPositionIOR image
	// Binding 3: Sky box image

	// Allocate Descriptor Set
	_deviceResources->gbufferSkyBoxDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->gbufferSkyBoxDescriptorSetLayout);
	_deviceResources->gbufferSkyBoxDescriptorSet->setObjectName("GBufferSkyBoxDescriptorSet");

	pvrvk::WriteDescriptorSet reflectanceImageWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferSkyBoxDescriptorSet, 0);
	pvrvk::WriteDescriptorSet normalMaterialIDImageWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferSkyBoxDescriptorSet, 1);
	pvrvk::WriteDescriptorSet worldPositionIORImageWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferSkyBoxDescriptorSet, 2);
	pvrvk::WriteDescriptorSet skyBoxWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferSkyBoxDescriptorSet, 3);

	reflectanceImageWDS.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferImages[0], _deviceResources->samplerLinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	normalMaterialIDImageWDS.setImageInfo(
		0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferImages[1], _deviceResources->samplerLinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	worldPositionIORImageWDS.setImageInfo(
		0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferImages[2], _deviceResources->samplerLinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	skyBoxWDS.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->skyBoxMap, _deviceResources->samplerLinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets = { reflectanceImageWDS, normalMaterialIDImageWDS, worldPositionIORImageWDS, skyBoxWDS };
	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanHybridRefractions::buildWriteRefractionsImageDescriptorSet()
{
	// Binding 0: ray traced refractions store image

	// Allocate Descriptor Set
	_deviceResources->rtImageStoreDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->rtImageStoreDescriptorSetLayout);
	_deviceResources->rtImageStoreDescriptorSet->setObjectName("RTImageStoreDescriptorSet");

	pvrvk::WriteDescriptorSet writeImageWDS = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->rtImageStoreDescriptorSet, 0);
	writeImageWDS.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->raytraceRefractionsImage, pvrvk::ImageLayout::e_GENERAL));

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets = { writeImageWDS };

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanHybridRefractions::buildGaussianBlurHorizontalDescriptorSet()
{
	// Binding 0: ray traced refractions image

	// Allocate Descriptor Set
	_deviceResources->gaussianBlurHorizontalDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->gaussianBlurDescriptorSetLayout);
	_deviceResources->gaussianBlurHorizontalDescriptorSet->setObjectName("GaussianBlurHorizontalDescriptorSet");

	pvrvk::WriteDescriptorSet gaussianBlurHorizontalPassWDS =
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gaussianBlurHorizontalDescriptorSet, 0);

	gaussianBlurHorizontalPassWDS.setImageInfo(
		0, pvrvk::DescriptorImageInfo(_deviceResources->raytraceRefractionsImage, _deviceResources->samplerLinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets = { gaussianBlurHorizontalPassWDS };
	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanHybridRefractions::buildGaussianBlurVerticalDescriptorSet()
{
	// Binding 0: result of the horizontal blur pass

	// Allocate Descriptor Set
	_deviceResources->gaussianBlurVerticalDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->gaussianBlurDescriptorSetLayout);
	_deviceResources->gaussianBlurVerticalDescriptorSet->setObjectName("GaussianBlurVerticalDescriptorSet");

	pvrvk::WriteDescriptorSet gaussianBlurVerticalPassWDS =
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gaussianBlurVerticalDescriptorSet, 0);

	gaussianBlurVerticalPassWDS.setImageInfo(
		0, pvrvk::DescriptorImageInfo(_deviceResources->raytraceRefractionsGaussianBlurImage, _deviceResources->samplerLinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets = { gaussianBlurVerticalPassWDS };
	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanHybridRefractions::buildDescriptorSets()
{
	buildCommonDescriptorSet();
	buildGBufferSkyboxDescriptorSet();
	buildWriteRefractionsImageDescriptorSet();
	buildGaussianBlurHorizontalDescriptorSet();
	buildGaussianBlurVerticalDescriptorSet();
}

void VulkanHybridRefractions::definePipelineVertexInputState(pvrvk::PipelineVertexInputStateCreateInfo& pipelineVertexInputStateCreateInfo)
{
	pvrvk::VertexInputAttributeDescription posAttrib;
	posAttrib.setBinding(0);
	posAttrib.setFormat(pvrvk::Format::e_R32G32B32_SFLOAT);
	posAttrib.setLocation(0);
	posAttrib.setOffset(0);

	pvrvk::VertexInputAttributeDescription normalAttrib;
	normalAttrib.setBinding(0);
	normalAttrib.setFormat(pvrvk::Format::e_R32G32B32_SFLOAT);
	normalAttrib.setLocation(1);
	normalAttrib.setOffset(offsetof(pvr::utils::ASVertexFormat, nrm));

	pvrvk::VertexInputAttributeDescription texCoordAttrib;
	texCoordAttrib.setBinding(0);
	texCoordAttrib.setFormat(pvrvk::Format::e_R32G32_SFLOAT);
	texCoordAttrib.setLocation(2);
	texCoordAttrib.setOffset(offsetof(pvr::utils::ASVertexFormat, texCoord));

	pvrvk::VertexInputAttributeDescription tangentAttrib;
	tangentAttrib.setBinding(0);
	tangentAttrib.setFormat(pvrvk::Format::e_R32G32B32_SFLOAT);
	tangentAttrib.setLocation(3);
	tangentAttrib.setOffset(offsetof(pvr::utils::ASVertexFormat, tangent));

	pvrvk::VertexInputBindingDescription binding;
	binding.setBinding(0);
	binding.setInputRate(pvrvk::VertexInputRate::e_VERTEX);
	binding.setStride(sizeof(pvr::utils::ASVertexFormat));

	pipelineVertexInputStateCreateInfo.addInputAttribute(posAttrib);
	pipelineVertexInputStateCreateInfo.addInputAttribute(normalAttrib);
	pipelineVertexInputStateCreateInfo.addInputAttribute(texCoordAttrib);
	pipelineVertexInputStateCreateInfo.addInputAttribute(tangentAttrib);
	pipelineVertexInputStateCreateInfo.addInputBinding(binding);
}

void VulkanHybridRefractions::buildGBufferPipeline()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.clear();

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->commonDescriptorSetLayout);
	pipeLayoutInfo.addPushConstantRange(pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, sizeof(uint32_t)));

	_deviceResources->gbufferPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pvrvk::GraphicsPipelineCreateInfo renderGBufferPipelineCreateInfo;
	renderGBufferPipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));
	// enable back face culling
	renderGBufferPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

	// set counter clockwise winding order for front faces
	renderGBufferPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// enable depth testing
	renderGBufferPipelineCreateInfo.depthStencil.enableDepthTest(true);
	renderGBufferPipelineCreateInfo.depthStencil.enableDepthWrite(true);

	// set the blend state for the colour attachments
	pvrvk::PipelineColorBlendAttachmentState renderGBufferColorAttachment;
	// number of colour blend states must equal number of colour attachments for the subpass
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(0, renderGBufferColorAttachment);
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(1, renderGBufferColorAttachment);
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(2, renderGBufferColorAttachment);

	// load and create appropriate shaders
	renderGBufferPipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("GBufferVertexShader.vsh.spv")->readToEnd<uint32_t>())));

	renderGBufferPipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("GBufferFragmentShader.fsh.spv")->readToEnd<uint32_t>())));

	// setup vertex inputs
	renderGBufferPipelineCreateInfo.vertexInput.clear();

	// create vertex input attrib desc
	definePipelineVertexInputState(renderGBufferPipelineCreateInfo.vertexInput);

	pvrvk::PipelineInputAssemblerStateCreateInfo inputAssembler;
	inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
	renderGBufferPipelineCreateInfo.inputAssembler = inputAssembler;

	// renderpass/subpass
	renderGBufferPipelineCreateInfo.renderPass = _deviceResources->gbufferRenderPass;

	// enable stencil testing
	pvrvk::StencilOpState stencilState;

	// only replace stencil buffer when the depth test passes
	stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
	stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

	// set stencil reference to 1
	stencilState.setReference(1);

	// enable stencil writing
	stencilState.setWriteMask(0xFF);

	// enable the stencil tests
	renderGBufferPipelineCreateInfo.depthStencil.enableStencilTest(true);
	// set stencil states
	renderGBufferPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	renderGBufferPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	renderGBufferPipelineCreateInfo.pipelineLayout = _deviceResources->gbufferPipelineLayout;
	_deviceResources->gbufferPipeline = _deviceResources->device->createGraphicsPipeline(renderGBufferPipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->gbufferPipeline->setObjectName("GBufferGraphicsPipeline");
}

void VulkanHybridRefractions::buildRayTracingPipeline()
{
	// pipeline layout
	pvrvk::PipelineLayoutCreateInfo pipeLayout;
	pipeLayout.addDescSetLayout(_deviceResources->gbufferSkyBoxDescriptorSetLayout);
	pipeLayout.addDescSetLayout(_deviceResources->rtImageStoreDescriptorSetLayout);
	pipeLayout.addDescSetLayout(_deviceResources->commonDescriptorSetLayout);

	_deviceResources->raytraceRefractionsPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayout);

	pvrvk::ShaderModule raygenSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("RayTraceRefractions.rgen.spv")->readToEnd<uint32_t>()));
	pvrvk::ShaderModule missSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("RayTraceRefractions.rmiss.spv")->readToEnd<uint32_t>()));
	pvrvk::ShaderModule missShadowSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("RayTraceShadows.rmiss.spv")->readToEnd<uint32_t>()));
	pvrvk::ShaderModule chitSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("RayTraceRefractions.rchit.spv")->readToEnd<uint32_t>()));
	pvrvk::ShaderModule chitShadowSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("RayTraceShadows.rchit.spv")->readToEnd<uint32_t>()));

	pvrvk::RaytracingPipelineCreateInfo raytracingPipeline;

	pvrvk::PipelineShaderStageCreateInfo generateCreateInfo;
	generateCreateInfo.setShader(raygenSM);
	generateCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	raytracingPipeline.stages.push_back(generateCreateInfo);

	pvrvk::PipelineShaderStageCreateInfo missCreateInfo;
	missCreateInfo.setShader(missSM);
	missCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_MISS_BIT_KHR);
	raytracingPipeline.stages.push_back(missCreateInfo);

	pvrvk::PipelineShaderStageCreateInfo missShadowCreateInfo;
	missShadowCreateInfo.setShader(missShadowSM);
	missShadowCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_MISS_BIT_KHR);
	raytracingPipeline.stages.push_back(missShadowCreateInfo);

	pvrvk::PipelineShaderStageCreateInfo hitCreateInfo;
	hitCreateInfo.setShader(chitSM);
	hitCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	raytracingPipeline.stages.push_back(hitCreateInfo);

	pvrvk::PipelineShaderStageCreateInfo hitShadowCreateInfo;
	hitShadowCreateInfo.setShader(chitShadowSM);
	hitShadowCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	raytracingPipeline.stages.push_back(hitShadowCreateInfo);

	pvrvk::RayTracingShaderGroupCreateInfo rayGenCI = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo missCI = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo missShadowCI = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo hitCI = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_TRIANGLES_HIT_GROUP_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo hitShadowCI = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_TRIANGLES_HIT_GROUP_KHR);

	rayGenCI.setGeneralShader(static_cast<uint32_t>(0));
	missCI.setGeneralShader(static_cast<uint32_t>(1));
	missShadowCI.setGeneralShader(static_cast<uint32_t>(2));
	hitCI.setGeneralShader(static_cast<uint32_t>(3));
	hitShadowCI.setGeneralShader(static_cast<uint32_t>(4));

	raytracingPipeline.shaderGroups = { rayGenCI, missCI, missShadowCI, hitCI, hitShadowCI };
	_shaderGroupCount = static_cast<uint32_t>(raytracingPipeline.shaderGroups.size());

	raytracingPipeline.maxRecursionDepth = _maxRayRecursionDepth; // Ray depth
	raytracingPipeline.pipelineLayout = _deviceResources->raytraceRefractionsPipelineLayout;

	_deviceResources->raytraceRefractionPipeline = _deviceResources->device->createRaytracingPipeline(raytracingPipeline, nullptr);
	_deviceResources->raytraceRefractionPipeline->setObjectName("RefractionRaytracingPipeline");
}

void VulkanHybridRefractions::buildDeferredShadingPipeline()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->commonDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->gbufferSkyBoxDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(2, _deviceResources->gaussianBlurDescriptorSetLayout);

	_deviceResources->deferredShadingPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	_deviceResources->_postProcessingPipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));

	// enable front face culling
	_deviceResources->_postProcessingPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);

	// set counter clockwise winding order for front faces
	_deviceResources->_postProcessingPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// enable stencil testing
	pvrvk::StencilOpState stencilState;

	// only replace stencil buffer when the depth test passes
	stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
	stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

	// set stencil reference to 1
	stencilState.setReference(1);

	// disable stencil writing
	stencilState.setWriteMask(0);

	// blend state
	pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;

	colorAttachmentState.setBlendEnable(false);
	_deviceResources->_postProcessingPipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

	// enable the stencil tests
	_deviceResources->_postProcessingPipelineCreateInfo.depthStencil.enableStencilTest(false);
	// set stencil states
	_deviceResources->_postProcessingPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	_deviceResources->_postProcessingPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	// enable depth testing
	_deviceResources->_postProcessingPipelineCreateInfo.pipelineLayout = _deviceResources->deferredShadingPipelineLayout;
	_deviceResources->_postProcessingPipelineCreateInfo.depthStencil.enableDepthTest(false);
	_deviceResources->_postProcessingPipelineCreateInfo.depthStencil.enableDepthWrite(false);

	// setup vertex inputs
	_deviceResources->_postProcessingPipelineCreateInfo.vertexInput.clear();
	_deviceResources->_postProcessingPipelineCreateInfo.inputAssembler = pvrvk::PipelineInputAssemblerStateCreateInfo();

	// renderpass/subpass
	_deviceResources->_postProcessingPipelineCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();

	// load and create appropriate shaders
	_deviceResources->_postProcessingPipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("FullscreenQuadVertexShader.vsh.spv")->readToEnd<uint32_t>())));
	_deviceResources->_postProcessingPipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("DeferredShadingFragmentShader.fsh.spv")->readToEnd<uint32_t>())));

	_deviceResources->deferredShadingPipeline = _deviceResources->device->createGraphicsPipeline(_deviceResources->_postProcessingPipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->deferredShadingPipeline->setObjectName("DeferredShadingGraphicsPipeline");
}

void VulkanHybridRefractions::buildGaussianBlurHorizontalPipeline()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.clear();
	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->gaussianBlurDescriptorSetLayout);
	pipeLayoutInfo.addPushConstantRange(pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, 6 * sizeof(float)));

	_deviceResources->gaussianBlurPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	_deviceResources->_postProcessingPipelineCreateInfo.pipelineLayout = _deviceResources->gaussianBlurPipelineLayout;

	// renderpass
	_deviceResources->_postProcessingPipelineCreateInfo.renderPass = _deviceResources->gaussianBlurRenderPass;

	// load and create appropriate shaders
	_deviceResources->_postProcessingPipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("SeparableGaussianBlurPass.vsh.spv")->readToEnd<uint32_t>())));
	_deviceResources->_postProcessingPipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("SeparableGaussianBlurPass.fsh.spv")->readToEnd<uint32_t>())));

	_deviceResources->gaussianBlurHorizontalPassPipeline =
		_deviceResources->device->createGraphicsPipeline(_deviceResources->_postProcessingPipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->gaussianBlurHorizontalPassPipeline->setObjectName("GaussianBlurHorizontalPassGraphicsPipeline");
}

void VulkanHybridRefractions::buildGaussianBlurVerticalPipeline()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.clear();
	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->gaussianBlurDescriptorSetLayout);
	pipeLayoutInfo.addPushConstantRange(pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, 6 * sizeof(float)));

	_deviceResources->gaussianBlurPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	_deviceResources->_postProcessingPipelineCreateInfo.pipelineLayout = _deviceResources->gaussianBlurPipelineLayout;

	// renderpass
	_deviceResources->_postProcessingPipelineCreateInfo.renderPass = _deviceResources->gaussianBlurRenderPass;

	// load and create appropriate shaders
	_deviceResources->_postProcessingPipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("SeparableGaussianBlurPass.vsh.spv")->readToEnd<uint32_t>())));
	_deviceResources->_postProcessingPipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("SeparableGaussianBlurPass.fsh.spv")->readToEnd<uint32_t>())));

	_deviceResources->gaussianBlurVerticalPassPipeline =
		_deviceResources->device->createGraphicsPipeline(_deviceResources->_postProcessingPipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->gaussianBlurVerticalPassPipeline->setObjectName("GaussianBlurVerticalPassGraphicsPipeline");
}

void VulkanHybridRefractions::buildShaderBindingTable()
{
	uint32_t groupHandleSize = _rtProperties.shaderGroupHandleSize; // Size of a program identifier
	uint32_t baseAlignment = _rtProperties.shaderGroupBaseAlignment; // Size of shader alignment

	// Fetch all the shader handles used in the pipeline, so that they can be written in the SBT
	uint32_t sbtSize = _shaderGroupCount * baseAlignment;

	std::vector<uint8_t> shaderHandleStorage(sbtSize);
	_deviceResources->device->getVkBindings().vkGetRayTracingShaderGroupHandlesKHR(
		_deviceResources->device->getVkHandle(), _deviceResources->raytraceRefractionPipeline->getVkHandle(), 0, _shaderGroupCount, sbtSize, shaderHandleStorage.data());

	// Create a buffer to store Shader Binding Table in
	_deviceResources->raytraceRefractionShaderBindingTable = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(
			sbtSize, pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::BufferUsageFlags::e_SHADER_BINDING_TABLE_BIT_KHR | pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);
	_deviceResources->raytraceRefractionShaderBindingTable->setObjectName("RaytraceRefractionShaderBindingTable");

	// Write the handles in the SBT
	void* mapped = _deviceResources->raytraceRefractionShaderBindingTable->getDeviceMemory()->map(0, VK_WHOLE_SIZE);

	auto* pData = reinterpret_cast<uint8_t*>(mapped);
	for (uint32_t g = 0; g < _shaderGroupCount; g++)
	{
		memcpy(pData, shaderHandleStorage.data() + static_cast<uint64_t>(g) * static_cast<uint64_t>(groupHandleSize), groupHandleSize); // raygen
		pData += baseAlignment;
	}

	_deviceResources->raytraceRefractionShaderBindingTable->getDeviceMemory()->unmap();
}

void VulkanHybridRefractions::buildPipelines()
{
	buildGBufferPipeline();
	buildRayTracingPipeline();
	buildDeferredShadingPipeline();
	buildGaussianBlurHorizontalPipeline();
	buildGaussianBlurVerticalPipeline();
}

void VulkanHybridRefractions::buildFramebufferAndRayTracingStoreImage()
{
	const pvrvk::Extent3D dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);

	for (int i = 0; i < GBuffer::Size; i++)
	{
		pvrvk::Image image = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _renderpassStorageFormats[i], dimension, pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

		_deviceResources->gbufferImages[i] = _deviceResources->device->createImageView(
			pvrvk::ImageViewCreateInfo(image, pvrvk::ImageViewType::e_2D, image->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));
	}

	const std::vector<pvrvk::Format> preferredDepthFormats = { pvrvk::Format::e_D24_UNORM_S8_UINT, pvrvk::Format::e_D32_SFLOAT_S8_UINT, pvrvk::Format::e_D16_UNORM_S8_UINT };
	_depthStencilFormat = pvr::utils::getSupportedDepthStencilFormat(_deviceResources->device, preferredDepthFormats);

	pvrvk::Image image = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _depthStencilFormat, dimension, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

	_deviceResources->gbufferDepthStencilImage = _deviceResources->device->createImageView(
		pvrvk::ImageViewCreateInfo(image, pvrvk::ImageViewType::e_2D, image->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_DEPTH_BIT)));

	pvrvk::Image raytraceRefractionsImage = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_UNORM, dimension,
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

	_deviceResources->raytraceRefractionsImage = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(
		raytraceRefractionsImage, pvrvk::ImageViewType::e_2D, raytraceRefractionsImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));

	pvrvk::Image raytraceRefractionsGaussianBlurImage = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_UNORM, dimension,
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

	_deviceResources->raytraceRefractionsGaussianBlurImage = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(raytraceRefractionsGaussianBlurImage,
		pvrvk::ImageViewType::e_2D, raytraceRefractionsGaussianBlurImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));
}

void VulkanHybridRefractions::buildRenderPass()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	pvrvk::AttachmentDescription gbufferAttachment0 =
		pvrvk::AttachmentDescription::createColorDescription(_renderpassStorageFormats[GBuffer::Reflectance], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachment1 =
		pvrvk::AttachmentDescription::createColorDescription(_renderpassStorageFormats[GBuffer::NormalMaterialID], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachment2 =
		pvrvk::AttachmentDescription::createColorDescription(_renderpassStorageFormats[GBuffer::WorldPositionIOR], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachmentDepth = pvrvk::AttachmentDescription::createDepthStencilDescription(_depthStencilFormat, pvrvk::ImageLayout::e_UNDEFINED,
		pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE);

	pvrvk::AttachmentReference gbufferAttachmentRef0 = pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef1 = pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef2 = pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRefDepth = pvrvk::AttachmentReference(3, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	pvrvk::SubpassDescription subpassDesc = pvrvk::SubpassDescription()
												.setColorAttachmentReference(0, gbufferAttachmentRef0)
												.setColorAttachmentReference(1, gbufferAttachmentRef1)
												.setColorAttachmentReference(2, gbufferAttachmentRef2)
												.setDepthStencilAttachmentReference(gbufferAttachmentRefDepth);

	pvrvk::SubpassDependency dependency[2];

	dependency[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
	dependency[0].setDstSubpass(0);
	dependency[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependency[0].setDstStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependency[0].setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependency[0].setDstAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependency[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	dependency[1].setSrcSubpass(0);
	dependency[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
	dependency[1].setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependency[1].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependency[1].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependency[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependency[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	pvrvk::RenderPassCreateInfo renderPassCreateInfo = pvrvk::RenderPassCreateInfo()
														   .setAttachmentDescription(0, gbufferAttachment0)
														   .setAttachmentDescription(1, gbufferAttachment1)
														   .setAttachmentDescription(2, gbufferAttachment2)
														   .setAttachmentDescription(3, gbufferAttachmentDepth)
														   .setSubpass(0, subpassDesc)
														   .addSubpassDependencies(dependency, 2);

	_deviceResources->gbufferRenderPass = _deviceResources->device->createRenderPass(renderPassCreateInfo);
	_deviceResources->gbufferRenderPass->setObjectName("GBufferRenderPass");

	const pvrvk::Extent3D dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);

	pvrvk::ImageView imageViews[] = { _deviceResources->gbufferImages[0], _deviceResources->gbufferImages[1], _deviceResources->gbufferImages[2],
		_deviceResources->gbufferDepthStencilImage };

	_deviceResources->gbufferFramebuffer = _deviceResources->device->createFramebuffer(
		pvrvk::FramebufferCreateInfo(dimension.getWidth(), dimension.getHeight(), 1, _deviceResources->gbufferRenderPass, 4, &imageViews[0]));
}

void VulkanHybridRefractions::buildGaussianBlurRenderPass()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	pvrvk::AttachmentDescription attachmentDescription = pvrvk::AttachmentDescription::createColorDescription(pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::ImageLayout::e_UNDEFINED,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);

	pvrvk::AttachmentReference attachment = pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);

	pvrvk::SubpassDescription subpassDesc = pvrvk::SubpassDescription().setColorAttachmentReference(0, attachment);

	pvrvk::SubpassDependency dependency[2];

	dependency[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
	dependency[0].setDstSubpass(0);
	dependency[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependency[0].setDstStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependency[0].setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependency[0].setDstAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependency[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	dependency[1].setSrcSubpass(0);
	dependency[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
	dependency[1].setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependency[1].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependency[1].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependency[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependency[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	pvrvk::RenderPassCreateInfo renderPassCreateInfo =
		pvrvk::RenderPassCreateInfo().setAttachmentDescription(0, attachmentDescription).setSubpass(0, subpassDesc).addSubpassDependencies(dependency, 2);

	_deviceResources->gaussianBlurRenderPass = _deviceResources->device->createRenderPass(renderPassCreateInfo);
	_deviceResources->gaussianBlurRenderPass->setObjectName("GaussianBlurRenderPass");

	const pvrvk::Extent3D dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);

	pvrvk::ImageView imageViews[] = { _deviceResources->raytraceRefractionsGaussianBlurImage };

	_deviceResources->gaussianBlurHorizontalPassFramebuffer = _deviceResources->device->createFramebuffer(
		pvrvk::FramebufferCreateInfo(dimension.getWidth(), dimension.getHeight(), 1, _deviceResources->gaussianBlurRenderPass, 1, &imageViews[0]));

	imageViews[0] = _deviceResources->raytraceRefractionsImage;

	_deviceResources->gaussianBlurVerticalPassFramebuffer = _deviceResources->device->createFramebuffer(
		pvrvk::FramebufferCreateInfo(dimension.getWidth(), dimension.getHeight(), 1, _deviceResources->gaussianBlurRenderPass, 1, &imageViews[0]));
}

void VulkanHybridRefractions::createTextures(pvrvk::CommandBuffer& uploadCmd)
{
	// load textures
	for (TextureAS& tex : _deviceResources->textures)
	{
		pvr::Texture textureObject = pvr::textureLoad(*getAssetStream(tex.name.c_str()), pvr::TextureFileFormat::PVR);

		tex.imageView = pvr::utils::uploadImageAndView(_deviceResources->device, textureObject, true, uploadCmd, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

		tex.image = tex.imageView->getImage();
	}

	_deviceResources->skyBoxMap = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(
		pvr::utils::loadAndUploadImage(_deviceResources->device, (std::string("HeroesSquare") + (_astcSupported ? "_astc.pvr" : ".pvr")).c_str(), true, uploadCmd, *this,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator)));
}

void VulkanHybridRefractions::buildSceneDescriptionBuffer()
{
	_deviceResources->_sceneDescription.resize(_vectorModelTransform.size());

	for (size_t i = 0; i < _vectorModelTransform.size(); ++i)
	{
		_deviceResources->_sceneDescription[i].modelIndex = 0;
		_deviceResources->_sceneDescription[i].transform = _vectorModelTransform[i];
		_deviceResources->_sceneDescription[i].transformIT = glm::transpose(glm::inverse(_vectorModelTransform[i]));
	}

	pvrvk::BufferCreateInfo bufferCreateInfo = pvrvk::BufferCreateInfo(sizeof(pvr::utils::SceneDescription) * _deviceResources->_sceneDescription.size(),
		pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);

	pvrvk::MemoryPropertyFlags memoryPropertyFlags =
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT;

	_deviceResources->sceneDescription = pvr::utils::createBuffer(_deviceResources->device, bufferCreateInfo, memoryPropertyFlags);
	pvrvk::DeviceSize dataSize = sizeof(pvr::utils::SceneDescription) * _deviceResources->_sceneDescription.size();
	_deviceResources->sceneDescription->setObjectName("sceneDescriptionSBO");
	pvr::utils::updateHostVisibleBuffer(_deviceResources->sceneDescription, &_deviceResources->_sceneDescription, 0, dataSize, true);
}

void VulkanHybridRefractions::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	// Handle input
	case pvr::SimplifiedInput::ActionClose: {
		exitShell();
		break;
	}
	case pvr::SimplifiedInput::Action1: {
		_updateScene = !_updateScene;
		break;
	}
	default: {
		break;
	}
	}
}

void VulkanHybridRefractions::buildModelBuffers(pvrvk::CommandBuffer& uploadCmd)
{
	int numModels = static_cast<int>(_models.size());

	_deviceResources->models.reserve(numModels);
	_deviceResources->vertexBuffers.reserve(numModels);
	_deviceResources->indexBuffers.reserve(numModels);
	_deviceResources->materialIndexBuffers.reserve(numModels);
	_deviceResources->verticesSize.reserve(numModels);
	_deviceResources->indicesSize.reserve(numModels);

	for (int32_t j = 0; j < numModels; ++j)
	{
		ModelAS modelAS;
		std::vector<uint32_t> indices;
		std::vector<pvr::utils::ASVertexFormat> vertices;
		std::vector<uint32_t> materialIndices;

		// populate vertices, indices and material indices
		uint32_t numMeshes = _models[j]->getNumMeshes();
		uint32_t totalIndices = 0;

		for (uint32_t meshIdx = 0; meshIdx < numMeshes; meshIdx++)
		{
			pvr::assets::Mesh mesh = _models[j]->getMesh(meshIdx);

			// indices
			uint32_t numIndices = mesh.getNumIndices();
			auto indicesWrapper = mesh.getFaces();

			if (indicesWrapper.getDataType() == pvr::IndexType::IndexType16Bit)
			{
				uint16_t* indicesPointer = (uint16_t*)indicesWrapper.getData();
				indices.insert(indices.begin(), indicesPointer, indicesPointer + numIndices);
			}
			else
			{
				uint32_t* indicesPointer = (uint32_t*)indicesWrapper.getData();
				indices.insert(indices.begin(), indicesPointer, indicesPointer + numIndices);
			}

			// vertices
			pvr::StridedBuffer verticesWrapper = mesh.getVertexData(0);
			uint32_t vertexStrideBytes = static_cast<uint32_t>(verticesWrapper.stride);
			uint32_t vertexStrideFloats = vertexStrideBytes / sizeof(float);
			uint32_t numVertices = static_cast<uint32_t>(verticesWrapper.size()) / vertexStrideBytes;

			auto verticesStart = reinterpret_cast<float*>(verticesWrapper.data());
			auto verticesEnd = verticesStart + static_cast<size_t>(numVertices) * vertexStrideFloats;
			uint32_t vertexIndex = 0;
			for (auto v = verticesStart; v < verticesEnd; v += vertexStrideFloats)
			{
				vertices.insert(vertices.begin() + vertexIndex,
					{
						glm::vec3(v[0], v[1], v[2]), // position
						glm::vec3(v[3], v[4], v[5]), // normals
						glm::vec2(v[6], v[7]), // texture coordinates
						glm::vec3(1.0) // tangent
					});
				vertexIndex++;
			}

			modelAS.meshes.push_back(MeshAS(j, totalIndices, numIndices, _vectorModelTransform[j], pvrvk::IndexType::e_UINT32));
			totalIndices += numIndices;

			// material indices
			std::vector<uint32_t> materialIndicesTemp(static_cast<uint64_t>(numIndices) / 3u + (static_cast<uint64_t>(numIndices) % 3u == 0u ? 0u : 1u), j);
			materialIndices.insert(materialIndices.end(), materialIndicesTemp.begin(), materialIndicesTemp.end());
		}

		_deviceResources->models.push_back(modelAS);

		// create vertex buffer
		pvrvk::BufferCreateInfo vertexBufferInfo;
		vertexBufferInfo.setSize(sizeof(pvr::utils::ASVertexFormat) * vertices.size());
		vertexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
			pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
		_deviceResources->vertexBuffers.push_back(pvr::utils::createBuffer(_deviceResources->device, vertexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT));
		_deviceResources->vertexBuffers.back()->setObjectName("VBO");
		pvr::utils::updateBufferUsingStagingBuffer(
			_deviceResources->device, _deviceResources->vertexBuffers[j], uploadCmd, vertices.data(), 0, sizeof(pvr::utils::ASVertexFormat) * vertices.size());

		// create index buffer
		pvrvk::BufferCreateInfo indexBufferInfo;
		indexBufferInfo.setSize(sizeof(uint32_t) * indices.size());
		indexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
			pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
		_deviceResources->indexBuffers.push_back(pvr::utils::createBuffer(_deviceResources->device, indexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT));
		_deviceResources->indexBuffers.back()->setObjectName("IBO");
		pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->indexBuffers[j], uploadCmd, indices.data(), 0, sizeof(uint32_t) * indices.size());

		// create material index buffer
		pvrvk::BufferCreateInfo materialIndexBufferInfo;
		materialIndexBufferInfo.setSize(sizeof(uint32_t) * materialIndices.size());
		materialIndexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);
		_deviceResources->materialIndexBuffers.push_back(pvr::utils::createBuffer(_deviceResources->device, materialIndexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT));
		_deviceResources->materialIndexBuffers.back()->setObjectName("MaterialSBO");
		pvr::utils::updateBufferUsingStagingBuffer(
			_deviceResources->device, _deviceResources->materialIndexBuffers[j], uploadCmd, materialIndices.data(), 0, sizeof(uint32_t) * materialIndices.size());

		_deviceResources->verticesSize.push_back(static_cast<int32_t>(vertices.size()));
		_deviceResources->indicesSize.push_back(static_cast<int32_t>(indices.size()));
	}
}

void VulkanHybridRefractions::buildCameraBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(ShaderStructFieldName::viewMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(ShaderStructFieldName::projectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(ShaderStructFieldName::inverseViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(ShaderStructFieldName::cameraPosition, pvr::GpuDatatypes::vec4);

	_deviceResources->globalBufferView.initDynamic(desc, _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
	_deviceResources->globalBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->globalBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->globalBuffer->setObjectName("GlobalUBO");

	_deviceResources->globalBufferView.pointToMappedMemory(_deviceResources->globalBuffer->getDeviceMemory()->getMappedData());
}

void VulkanHybridRefractions::buildSceneElementTransformBuffer()
{
	std::vector<pvrvk::WriteDescriptorSet> descUpdate{ _numSwapImages };
	pvr::utils::StructuredMemoryDescription description;
	description.addElement("ModelMatrix", pvr::GpuDatatypes::mat4x4);

	_deviceResources->perMeshTransformBufferView.initDynamic(description, static_cast<uint32_t>(_vectorModelTransform.size()), pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
	_deviceResources->perMeshTransformBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->perMeshTransformBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->perMeshTransformBuffer->setObjectName("PerMeshTransformUBO");
	_deviceResources->perMeshTransformBufferView.pointToMappedMemory(_deviceResources->perMeshTransformBuffer->getDeviceMemory()->getMappedData());
	_deviceResources->perMeshTransformBuffer->setObjectName("PerMeshTransformBuffer");

	for (size_t i = 0; i < _vectorModelTransform.size(); ++i)
	{
		_deviceResources->perMeshTransformBufferView.getElementByName("ModelMatrix", 0, i).setValue(_vectorModelTransform[i]);
	}

	if (static_cast<uint32_t>(_deviceResources->perMeshTransformBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->perMeshTransformBuffer->getDeviceMemory()->flushRange(
			_deviceResources->perMeshTransformBufferView.getDynamicSliceOffset(5), _deviceResources->perMeshTransformBufferView.getDynamicSliceSize());
	}
}

void VulkanHybridRefractions::buildLightDataBuffer()
{
	_models[0]->getLightPosition(0, _lightData.lightPositionMaxRayRecursion);
	_lightData.lightPositionMaxRayRecursion.w = float(_maxRayRecursionDepth);
	_lightData.lightColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0);
	_lightData.ambientColorIntensity = glm::vec4(0.1f, 0.1f, 0.1f, 80000.0);

	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(ShaderStructFieldName::lightColor, pvr::GpuDatatypes::vec4);
	desc.addElement(ShaderStructFieldName::lightPositionMaxRayRecursion, pvr::GpuDatatypes::vec4);
	desc.addElement(ShaderStructFieldName::ambientColorIntensity, pvr::GpuDatatypes::vec4);

	_deviceResources->lightDataBufferView.initDynamic(desc, _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->lightDataBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->lightDataBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->lightDataBuffer->setObjectName("lightDataUBO");

	_deviceResources->lightDataBufferView.pointToMappedMemory(_deviceResources->lightDataBuffer->getDeviceMemory()->getMappedData());
}

void VulkanHybridRefractions::buildMaterialBuffer(pvrvk::CommandBuffer& uploadCmd)
{
	std::vector<Material> vectorMaterial(5);

	Material mat = Material();

	// Outer torus material values
	mat.indexOfRefraction = 1.4f;
	mat.baseColor = glm::vec4(1.0f, 0.8f, 0.4f, 1.0f);
	mat.attenuationCoefficient = 1.5f;
	vectorMaterial[0] = mat;

	// Inner torus material values
	mat.indexOfRefraction = 1.2f;
	mat.baseColor = glm::vec4(0.43f, 0.94f, 0.2f, 1.0f);
	vectorMaterial[1] = mat;

	// Baloons material values
	mat = Material();
	vectorMaterial[2] = mat;
	vectorMaterial[3] = mat;
	vectorMaterial[4] = mat;

	// Add also the textures for the balloon meshes, which are not in the exported .pod model
	_deviceResources->textures.push_back(TextureAS{ std::string("BalloonTex") + (_astcSupported ? "_astc.pvr" : ".pvr"), pvrvk::Format::e_R8G8B8A8_SRGB, VK_NULL_HANDLE, VK_NULL_HANDLE });
	_deviceResources->textures.push_back(TextureAS{ std::string("BalloonTex2") + (_astcSupported ? "_astc.pvr" : ".pvr"), pvrvk::Format::e_R8G8B8A8_SRGB, VK_NULL_HANDLE, VK_NULL_HANDLE });
	_deviceResources->textures.push_back(TextureAS{ std::string("BalloonTex3") + (_astcSupported ? "_astc.pvr" : ".pvr"), pvrvk::Format::e_R8G8B8A8_SRGB, VK_NULL_HANDLE, VK_NULL_HANDLE });
	vectorMaterial[2].reflectanceTextureIndex = 0;
	vectorMaterial[3].reflectanceTextureIndex = 1;
	vectorMaterial[4].reflectanceTextureIndex = 2;

	// create material data buffer
	pvrvk::BufferCreateInfo materialColorBufferInfo;
	materialColorBufferInfo.setSize(sizeof(Material) * vectorMaterial.size());
	materialColorBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);
	_deviceResources->materialBuffer = pvr::utils::createBuffer(_deviceResources->device, materialColorBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT);
	_deviceResources->materialBuffer->setObjectName("materialSBO");
	pvr::utils::updateBufferUsingStagingBuffer(
		_deviceResources->device, _deviceResources->materialBuffer, uploadCmd, vectorMaterial.data(), 0, sizeof(Material) * vectorMaterial.size());
}

void VulkanHybridRefractions::updateCameraLightData()
{
	uint32_t dynamicSliceIdx = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->globalBufferView.getElementByName(ShaderStructFieldName::viewMatrix, 0, dynamicSliceIdx).setValue(_viewMatrix);
	_deviceResources->globalBufferView.getElementByName(ShaderStructFieldName::projectionMatrix, 0, dynamicSliceIdx).setValue(_projectionMatrix);
	_deviceResources->globalBufferView.getElementByName(ShaderStructFieldName::inverseViewProjectionMatrix, 0, dynamicSliceIdx).setValue(glm::inverse(_projectionMatrix * _viewMatrix));
	_deviceResources->globalBufferView.getElementByName(ShaderStructFieldName::cameraPosition, 0, dynamicSliceIdx).setValue(glm::vec4(_cameraPosition, 0.0f));

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->globalBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->globalBuffer->getDeviceMemory()->flushRange(
			_deviceResources->globalBufferView.getDynamicSliceOffset(dynamicSliceIdx), _deviceResources->globalBufferView.getDynamicSliceSize());
	}

	_deviceResources->lightDataBufferView.getElementByName(ShaderStructFieldName::lightColor, 0, dynamicSliceIdx).setValue(_lightData.lightColor);
	_deviceResources->lightDataBufferView.getElementByName(ShaderStructFieldName::lightPositionMaxRayRecursion, 0, dynamicSliceIdx).setValue(_lightData.lightPositionMaxRayRecursion);
	_deviceResources->lightDataBufferView.getElementByName(ShaderStructFieldName::ambientColorIntensity, 0, dynamicSliceIdx).setValue(_lightData.ambientColorIntensity);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->lightDataBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->lightDataBuffer->getDeviceMemory()->flushRange(
			_deviceResources->lightDataBufferView.getDynamicSliceOffset(dynamicSliceIdx), _deviceResources->lightDataBufferView.getDynamicSliceSize());
	}
}

void VulkanHybridRefractions::updateScene()
{
	if (!_updateScene) { return; }

	float angleAdd = float(getFrameTime()) * 0.0002f;

	_vectorModelAngleRotation[0] += angleAdd;
	_vectorModelAngleRotation[1] += angleAdd;
	_vectorModelAngleRotation[2] += angleAdd;
	_vectorModelAngleRotation[3] += angleAdd * 1.3f;
	_vectorModelAngleRotation[4] += angleAdd * 1.6f;

	_vectorModelTransform[0] = _initialTorusTransform * glm::rotate(_vectorModelAngleRotation[0], glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.57f));
	_vectorModelTransform[1] = _initialTorusTransform * glm::rotate(_vectorModelAngleRotation[1], glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::vec3(0.295f));
	_vectorModelTransform[2] = _initialBaloonTransform * glm::rotate(_vectorModelAngleRotation[2], _positiveYAxis) *
		glm::translate(glm::vec3(80.f + 0 * 40.f, sin(_vectorModelAngleRotation[2] * 3.0f) * 20.0f, 0.0f)) * glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
	_vectorModelTransform[3] = _initialBaloonTransform * glm::rotate(_vectorModelAngleRotation[3], _positiveYAxis) *
		glm::translate(glm::vec3(80.f + 1 * 40.f, sin(_vectorModelAngleRotation[3] * 3.0f) * 20.0f, 0.0f)) * glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
	_vectorModelTransform[4] = _initialBaloonTransform * glm::rotate(_vectorModelAngleRotation[4], _positiveYAxis) *
		glm::translate(glm::vec3(80.f + 2 * 40.f, sin(_vectorModelAngleRotation[4] * 3.0f) * 20.0f, 0.0f)) * glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));

	_deviceResources->accelerationStructure.updateInstanceTransformData(_vectorModelTransform);

	pvrvk::CommandBuffer commandBuffer = _deviceResources->commandPool->allocateCommandBuffer();

	_deviceResources->accelerationStructure.buildTopLevelASAndInstances(_deviceResources->device, commandBuffer, _deviceResources->queue,
		pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR | pvrvk::BuildAccelerationStructureFlagsKHR::e_ALLOW_UPDATE_BIT_KHR, true);

	for (size_t i = 0; i < _vectorModelTransform.size(); ++i)
	{
		// Update scene element transforms through the structured buffer view
		_deviceResources->perMeshTransformBufferView.getElementByName("ModelMatrix", 0, i).setValue(_vectorModelTransform[i]);

		// Update TLAS scene transform data
		_deviceResources->_sceneDescription[i].modelIndex = 0;
		_deviceResources->_sceneDescription[i].transform = _vectorModelTransform[i];
		_deviceResources->_sceneDescription[i].transformIT = glm::transpose(glm::inverse(_vectorModelTransform[i]));
	}

	if (static_cast<uint32_t>(_deviceResources->perMeshTransformBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->perMeshTransformBuffer->getDeviceMemory()->flushRange(
			_deviceResources->perMeshTransformBufferView.getDynamicSliceOffset(5), _deviceResources->perMeshTransformBufferView.getDynamicSliceSize());
	}

	pvrvk::DeviceSize dataSize = sizeof(pvr::utils::SceneDescription) * _vectorModelTransform.size();
	pvr::utils::updateHostVisibleBuffer(_deviceResources->sceneDescription, _deviceResources->_sceneDescription.data(), 0, dataSize, true);
}

void VulkanHybridRefractions::recordMainCommandBuffer()
{
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->cmdBufferMainDeferred[i]->begin();

		pvrvk::Rect2D renderArea(0, 0, _windowWidth, _windowHeight);

		// specify a clear colour per attachment
		const uint32_t numClearValues = GBuffer::Size + 1u;

		pvrvk::ClearValue gbufferClearValues[] = { pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f), pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f), pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f),
			pvrvk::ClearValue(1.f, 0u) };

		// Begin the gbuffer renderpass
		_deviceResources->cmdBufferMainDeferred[i]->beginRenderPass(_deviceResources->gbufferFramebuffer, renderArea, false, gbufferClearValues, numClearValues);

		// Render the models
		_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferGBuffer[i]);
		_deviceResources->cmdBufferMainDeferred[i]->endRenderPass();

		// Render raytraced refractions
		_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferRayTracedRefractions[i]);

		// Gaussian Blur pass on thre raytraced refractions render target
		pvrvk::ClearValue gaussianBlurClearValues[] = { pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f) };
		_deviceResources->cmdBufferMainDeferred[i]->beginRenderPass(_deviceResources->gaussianBlurHorizontalPassFramebuffer, renderArea, false, gaussianBlurClearValues, 1);
		_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferGaussianBlurHorizontal[i]);
		_deviceResources->cmdBufferMainDeferred[i]->endRenderPass();

		_deviceResources->cmdBufferMainDeferred[i]->beginRenderPass(_deviceResources->gaussianBlurVerticalPassFramebuffer, renderArea, false, gaussianBlurClearValues, 1);
		_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferGaussianBlurVertical[i]);
		_deviceResources->cmdBufferMainDeferred[i]->endRenderPass();

		pvrvk::ClearValue onscreenClearValues[] = { pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f), pvrvk::ClearValue(1.f, 0u) };

		// Render ui render text
		_deviceResources->cmdBufferMainDeferred[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i], renderArea, false, onscreenClearValues, 2);
		_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferDeferredShading[i]);
		_deviceResources->cmdBufferMainDeferred[i]->endRenderPass();
		_deviceResources->cmdBufferMainDeferred[i]->end();
	}
}

void VulkanHybridRefractions::recordSecondaryCommandBuffers()
{
	pvrvk::ClearValue clearStenciLValue(pvrvk::ClearValue::createStencilClearValue(0));

	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->cmdBufferGBuffer[i]->begin(_deviceResources->gbufferFramebuffer);
		recordCommandBufferRenderGBuffer(_deviceResources->cmdBufferGBuffer[i], i);
		_deviceResources->cmdBufferGBuffer[i]->end();

		_deviceResources->cmdBufferRayTracedRefractions[i]->begin();
		recordCommandBufferRayTraceRefractions(_deviceResources->cmdBufferRayTracedRefractions[i], i);
		_deviceResources->cmdBufferRayTracedRefractions[i]->end();

		_deviceResources->cmdBufferGaussianBlurHorizontal[i]->begin(_deviceResources->gaussianBlurHorizontalPassFramebuffer);
		recordCommandBufferHorizontalGaussianBlur(_deviceResources->cmdBufferGaussianBlurHorizontal[i], i);
		_deviceResources->cmdBufferGaussianBlurHorizontal[i]->end();

		_deviceResources->cmdBufferGaussianBlurVertical[i]->begin(_deviceResources->gaussianBlurVerticalPassFramebuffer);
		recordCommandBufferVerticalGaussianBlur(_deviceResources->cmdBufferGaussianBlurVertical[i], i);
		_deviceResources->cmdBufferGaussianBlurVertical[i]->end();

		_deviceResources->cmdBufferDeferredShading[i]->begin(_deviceResources->onScreenFramebuffer[i]);
		recordCommandBufferDeferredShading(_deviceResources->cmdBufferDeferredShading[i], i);
		recordCommandUIRenderer(_deviceResources->cmdBufferDeferredShading[i]);
		_deviceResources->cmdBufferDeferredShading[i]->end();
	}
}

void VulkanHybridRefractions::recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("G-Buffer - Swapchain (%i)", swapchainIndex)));

	uint32_t offsets[3] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex);

	for (uint32_t modelIdx = 0; modelIdx < _deviceResources->models.size(); modelIdx++)
	{
		auto& model = _deviceResources->models[modelIdx];

		for (uint32_t meshIdx = 0; meshIdx < model.meshes.size(); ++meshIdx)
		{
			auto& mesh = model.meshes[meshIdx];

			cmdBuffer->bindPipeline(_deviceResources->gbufferPipeline);

			int32_t matID = mesh.materialIdx;
			cmdBuffer->pushConstants(_deviceResources->gbufferPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, sizeof(int32_t), &matID);

			offsets[2] = _deviceResources->perMeshTransformBufferView.getDynamicSliceOffset(modelIdx);
			cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->gbufferPipelineLayout, 0u, _deviceResources->commonDescriptorSet, offsets, 3);

			cmdBuffer->bindVertexBuffer(_deviceResources->vertexBuffers[modelIdx], 0, 0);
			cmdBuffer->bindIndexBuffer(_deviceResources->indexBuffers[modelIdx], 0, mesh.indexType);
			cmdBuffer->drawIndexed(mesh.indexOffset, mesh.numIndices, 0, 0, 1);
		}
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
}

void VulkanHybridRefractions::recordCommandBufferRayTraceRefractions(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Ray Trace Refractions - Swapchain (%i)", swapchainIndex)));

	{
		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_UNDEFINED;
		pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_GENERAL;

		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
			_deviceResources->raytraceRefractionsImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
			_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_RAY_TRACING_SHADER_BIT_KHR, layoutTransitions);
	}

	cmdBuffer->bindPipeline(_deviceResources->raytraceRefractionPipeline);

	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->gbufferSkyBoxDescriptorSet, _deviceResources->rtImageStoreDescriptorSet, _deviceResources->commonDescriptorSet };

	uint32_t offsets[3] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[2] = 0; // This value can be 0, it is the per mesh transform data, which is not used here

	cmdBuffer->bindDescriptorSets(pvrvk::PipelineBindPoint::e_RAY_TRACING_KHR, _deviceResources->raytraceRefractionsPipelineLayout, 0, arrayDS, 3, offsets, 3);

	VkDeviceAddress sbtAddress = _deviceResources->raytraceRefractionShaderBindingTable->getDeviceAddress(_deviceResources->device);

	uint64_t shaderGroupSize =
		static_cast<uint64_t>((_rtProperties.shaderGroupHandleSize + (uint32_t(_rtProperties.shaderGroupBaseAlignment) - 1)) & ~uint32_t(_rtProperties.shaderGroupBaseAlignment - 1));
	uint64_t shaderGroupStride = shaderGroupSize;

	VkDeviceSize rayGenOffset = 0u * shaderGroupSize; // Start at the beginning of m_sbtBuffer
	VkDeviceSize missOffset = 1u * shaderGroupSize; // Jump over raygen
	VkDeviceSize hitGroupOffset = 3u * shaderGroupSize; // Jump over the previous shaders

	pvrvk::StridedDeviceAddressRegionKHR raygenShaderBindingTable = { sbtAddress + rayGenOffset, shaderGroupStride, shaderGroupSize };
	pvrvk::StridedDeviceAddressRegionKHR missShaderBindingTable = { sbtAddress + missOffset, shaderGroupStride, shaderGroupSize * 2 };
	pvrvk::StridedDeviceAddressRegionKHR hitShaderBindingTable = { sbtAddress + hitGroupOffset, shaderGroupStride, shaderGroupSize * 2 };
	pvrvk::StridedDeviceAddressRegionKHR callableShaderBindingTable = {};

	cmdBuffer->traceRays(raygenShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable, getWidth(), getHeight(), 1);

	{
		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_GENERAL;
		pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
			_deviceResources->raytraceRefractionsImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
			_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_RAY_TRACING_SHADER_BIT_KHR, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransitions);
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
}

void VulkanHybridRefractions::recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Deferred Shading - Swapchain (%i)", swapchainIndex)));

	cmdBuffer->bindPipeline(_deviceResources->deferredShadingPipeline);

	pvrvk::DescriptorSet dsArray[] = { _deviceResources->commonDescriptorSet, _deviceResources->gbufferSkyBoxDescriptorSet, _deviceResources->gaussianBlurHorizontalDescriptorSet };

	uint32_t offsets[3] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[2] = 0; // This value can be 0, it is the per mesh transform data, which is not used here

	cmdBuffer->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->deferredShadingPipelineLayout, 0u, dsArray, 3, offsets, 3);

	cmdBuffer->draw(0, 6);

	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
}

void VulkanHybridRefractions::recordCommandBufferHorizontalGaussianBlur(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Gaussian Blur Horizontal Pass - Swapchain (%i)", swapchainIndex)));

	cmdBuffer->bindPipeline(_deviceResources->gaussianBlurHorizontalPassPipeline);

	// Supply through push constant the exact offset needed to sample for the horizontal pass of the Gaussian blur.
	float width = float(_deviceResources->swapchain->getDimension().getWidth());
	float arrayOffsetWeight[6];
	arrayOffsetWeight[0] = (1.0f / width) * float(_gaussianOffsets[0]); // First offset x component
	arrayOffsetWeight[1] = 0.0f; // First offset y component
	arrayOffsetWeight[2] = (1.0f / width) * float(_gaussianOffsets[1]); // Second offset x component
	arrayOffsetWeight[3] = 0.0f; // Second offset y component
	arrayOffsetWeight[4] = float(_gaussianWeights[0]); // First sample weight
	arrayOffsetWeight[5] = float(_gaussianWeights[1]); // Second sample weight

	cmdBuffer->pushConstants(_deviceResources->gaussianBlurPipelineLayout, pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, 6 * sizeof(float), &arrayOffsetWeight[0]);
	cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->gaussianBlurPipelineLayout, 0u, _deviceResources->gaussianBlurHorizontalDescriptorSet, 0, 0);

	cmdBuffer->draw(0, 6);

	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
}

void VulkanHybridRefractions::recordCommandBufferVerticalGaussianBlur(pvrvk::SecondaryCommandBuffer& cmdBuffer, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Gaussian Blur Vertical Pass - Swapchain (%i)", swapchainIndex)));

	cmdBuffer->bindPipeline(_deviceResources->gaussianBlurVerticalPassPipeline);

	// Supply through push constant the exact offset needed to sample for the vertical pass of the Gaussian blur.
	float height = float(_deviceResources->swapchain->getDimension().getHeight());
	float arrayOffsetWeight[6];
	arrayOffsetWeight[0] = 0.0f; // First offset y component
	arrayOffsetWeight[1] = (1.0f / height) * float(_gaussianOffsets[0]); // First offset x component
	arrayOffsetWeight[2] = 0.0f; // Second offset y component
	arrayOffsetWeight[3] = (1.0f / height) * float(_gaussianOffsets[1]); // Second offset x component
	arrayOffsetWeight[4] = float(_gaussianWeights[0]); // First sample weight
	arrayOffsetWeight[5] = float(_gaussianWeights[1]); // Second sample weight

	cmdBuffer->pushConstants(_deviceResources->gaussianBlurPipelineLayout, pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, 6 * sizeof(float), &arrayOffsetWeight[0]);
	cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->gaussianBlurPipelineLayout, 0u, _deviceResources->gaussianBlurVerticalDescriptorSet, 0, 0);

	cmdBuffer->draw(0, 6);

	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
}

void VulkanHybridRefractions::recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& cmdBuffer)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("UI"));

	_deviceResources->uiRenderer.beginRendering(cmdBuffer);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();

	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its Shell object defining the
/// behaviour of the application.</summary>
/// <returns>Return an unique_ptr to a new Demo class,supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanHybridRefractions>(); }
