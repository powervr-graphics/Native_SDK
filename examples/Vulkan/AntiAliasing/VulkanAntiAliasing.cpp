/*!
\brief Shows how to implement shader based anti aliasing in vulkan.
\file VulkanAntiAliasing.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRCore/cameras/TPSCamera.h"
#include "PVRShell/PVRShell.h"
#include "PVRAssets/fileio/GltfReader.h"
#include "PVRCore/cameras/TPSCamera.h"
#include <thread>
#include <mutex>

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
const char MsAntialiasing[] = "Multi Sampled Anti Aliasing";
const char FxAntiAliasing[] = "Fast Approximate Anti Aliasing";
const char TxAntiAliasing[] = "Temporal Approximate Anti Aliasing";
} // namespace UIText

/// <summary>Shader Source Files.</summary>
namespace ShaderFiles {
const char AttributelessVertexShaderFile[] = "AttributelessVertexShader.vsh.spv";
const char NOAAVertexShaderFile[] = "NOAA_VertShader.vsh.spv";
const char NOAAFragmentShaderFile[] = "NOAA_FragShader.fsh.spv";
const char MSAAFragmentShaderFile[] = "MSAA_FragShader.fsh.spv";
const char VelocityTXAAVertexShaderFile[] = "VelocityTXAA_VertShader.vsh.spv";
const char VelocityTXAAFragmentShaderFile[] = "VelocityTXAA_FragShader.fsh.spv";
const char ResolveTXAAFragmentShaderFile[] = "ResolveTXAA_FragShader.fsh.spv";
const char FXAAFragmentShaderFile[] = "FXAA_FragShader.fsh.spv";
} // namespace ShaderFiles

/// <summary>Shader Source Files.</summary>
namespace BufferEntryNames {
namespace NOAA {
const char* const MVPMatrix = "MVPMatrix";
const char* const LightDirModel = "LightDirModel";
} // namespace NOAA

namespace MSAA {
const char* const MVPMatrix = "mvpMatrix";
const char* const WorldMatrix = "worldMatrix";
} // namespace MSAA

namespace FXAA {
const char* const MVPMatrix = "mvpMatrix";
const char* const WorldMatrix = "worldMatrix";
} // namespace FXAA

namespace TXAA {
const char* const prevModelMatrix = "prevModelMatrix";
const char* const prevViewMatrix = "prevViewMatrix";
const char* const prevProjectionMatrix = "prevProjMatrix";
const char* const prevProjViewMatrix = "prevProjViewMatrix";
const char* const prevWorldMatrix = "prevWorld";

const char* const currModelMatrix = "currModelMatrix";
const char* const currViewMatrix = "currViewMatrix";
const char* const currProjectionMatrix = "currProjMatrix";
const char* const currProjViewMatrix = "currProjViewMatrix";
const char* const currWorldMatrix = "currWorldMatrix";

const char* const jitter = "uJitter";
const char* const currLightDir = "currLightDir";

const int numberFramesForJitter = 16;
} // namespace TXAA
} // namespace BufferEntryNames

/// <summary>Vertex Attribute Bindings.</summary>
const pvr::utils::VertexBindings VertexAttribBindings[] = {
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
	{ "TANGENT", 3 },
};

/// <summary>Selected Anti Aliasing techniques.</summary>
enum class AntiAliasingTechnique
{
	NOAA = 0,
	MSAA = 1,
	FXAA = 2,
	TXAA = 3
};

/// <summary>Values are used for calculation in vertex shaders of no antialiasing, FXAA and MSAA.</summary>
struct SceneInformationBuffer
{
	/// <summary>Model view projection matrix.</summary>
	glm::mat4 modelViewProjectionMatrix;

	/// <summary>Light direction.</summary>
	glm::vec3 lightDirModel;
};

/// <summary>Values are used for calculation in vertex shaders of TAA pipeline including velocity and resolve stage.</summary>
struct SceneInformationBufferTAA
{
	SceneInformationBufferTAA() {}

	/// <summary>Previous frame model matrix.</summary>
	glm::mat4 preModel;

	/// <summary>Previous frame view projection matrix.</summary>
	glm::mat4 preProjView;

	/// <summary>Previous frame world matrix.</summary>
	glm::mat4 preWorld;

	/// <summary>Current frame model view projection.</summary>
	glm::mat4 currMVPMatrix;

	/// <summary>Light direction.</summary>
	glm::vec3 currLightDir;

	/// <summary>Current frame model matrix.</summary>
	glm::mat4 currModel;

	/// <summary>Current frame view projection matrix.</summary>
	glm::mat4 currProjView;

	/// <summary>Current frame view matrix.</summary>
	glm::mat4 currWorld;

	/// <summary>Jitter value applied.</summary>
	glm::vec2 jitter;
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
	pvrvk::Queue queue;

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

	/// <summary>Nearest sampler used in TAA.</summary>
	pvrvk::Sampler samplerNearest;

	/// <summary>Bilinear sampler used in most of the techniques.</summary>
	pvrvk::Sampler samplerBilinear;

	/// <summary>Trilinear sampler used in most of the techniques.</summary>
	pvrvk::Sampler samplerTrilinear;

	/// <summary>Helper command buffer used for initial resource loading.</summary>
	pvrvk::CommandBuffer utilityCommandBuffer;

	/// <summary>UIRenderer used to display text.</summary>
	pvr::ui::UIRenderer uiRenderer;

	/// <summary>UIRenderer used to display text in the MSAA technique.</summary>
	pvr::ui::UIRenderer msaaUIRenderer;

	/// <summary>Albedo image view for the scene model.</summary>
	pvrvk::ImageView albedoImageView;

	/// <summary>Normal image view for the scene model.</summary>
	pvrvk::ImageView normalMapImageView;

	/// <summary>Vector with the semaphores used at the beginning of each frame to acquire the next swap chain image index (so there is one element in this vector per swap chain image).</summary>
	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;

	/// <summary>Vector with the semaphores used to signal the command buffer with all the commands for the current frame (one element in this vector per swap chain image).</summary>
	std::vector<pvrvk::Semaphore> presentationSemaphores;

	/// <summary>Vector with the fences used in the command buffer submit done each frame which contains all the commands for the current frame. It is waited for at the beginning of each frame, after after the next swap chain image is acquired with _imageAcquiredSemaphores (one element in this vector per swap chain image).</summary>
	std::vector<pvrvk::Fence> perFrameResourcesFences;

	/// <summary>Vertex buffer object with the scene element geometry.</summary>
	std::vector<pvrvk::Buffer> sceneVertexBuffer;

	/// <summary>Index buffer object for the scene element geometry.</summary>
	std::vector<pvrvk::Buffer> sceneIndexBuffer;

	/// <summary>Descriptor set layout for the vertex shader (uniform buffer object information) in render passes where the scene is rendered.</summary>
	pvrvk::DescriptorSetLayout sceneVertexDescriptorSetLayout;

	/// <summary>Descriptor set layout for the fragment shader (textures to sample from) in render passes where the scene is rendered.</summary>
	pvrvk::DescriptorSetLayout sceneFragmentDescriptorSetLayout;

	/// <summary>Descriptor set layout used by those postprocessing passes reading from a single texture.</summary>
	pvrvk::DescriptorSetLayout postProcessDescriptorSetLayout;

	/// <summary>Descriptor set layout for the TAA resolve pass.</summary>
	pvrvk::DescriptorSetLayout taaResolveDescriptorSetLayout;

	/// <summary>Descriptor sets for render passes where the scene is rendered.</summary>
	std::vector<pvrvk::DescriptorSet> sceneFragmentDescriptorSets;

	/// <summary>Descriptor sets for render passes where the scene is rendered.</summary>
	std::vector<pvrvk::DescriptorSet> sceneVertexDescriptorSets;

	/// <summary>Descriptor set used in the FXAA reoslve pass (when sampling from the offscreen rendered scene and putting the results in the swapchain).</summary>
	std::vector<pvrvk::DescriptorSet> fxaaResolvePassDescriptorSet;

	/// <summary>Descriptor set used in the MSAA resolve pass (when sampling from the offscreen rendered scene in a 4 samples per pixel color attachment and putting the results in the swapchain).</summary>
	std::vector<pvrvk::DescriptorSet> msaaResolvePassDescriptorSets;

	/// <summary>Descriptor set used in the TAA technique when doing the offscreen scene geometry pass.</summary>
	std::vector<pvrvk::DescriptorSet> taaFragmentDescriptorSets;

	/// <summary>Descriptor set used in the TAA technique when doing the offscreen scene geometry pass.</summary>
	std::vector<pvrvk::DescriptorSet> taaVertexDescriptorSets;

	/// <summary>Descriptor set used for the TAA resolve pass, reading from the offscreen texture with the scene, history and velocity, and putting the results in the swapchain for presentation.</summary>
	std::vector<pvrvk::DescriptorSet> taaResolveDescriptorSet;

	/// <summary>Pipeline layout for the TAA Resolve pass, reading from the offscreen texture with the scene, history and velocity, and putting the results in the swapchain for presentation.</summary>
	pvrvk::PipelineLayout taaResolvePipelineLayout;

	/// <summary>Pipeline layout used in the render passes where the scene is rendered.</summary>
	pvrvk::PipelineLayout scenePipelineLayout;

	/// <summary>Pipeline layout used by those postprocessing passes reading from a single texture.</summary>
	pvrvk::PipelineLayout postProcessPipelineLayout;

	/// <summary>Buffer used by the structured buffer view.</summary>
	pvrvk::Buffer sceneUniformBuffer;

	/// <summary>Structured buffer view for the uniform buffer values used in the render passes where the scene is rendered.</summary>
	pvr::utils::StructuredBufferView sceneStructuredBufferView;

	/// <summary>Buffer used by the structured buffer view for the offscreen scene geometry pass in TAA.</summary>
	pvrvk::Buffer taaUniformBuffer;

	/// <summary>Structured buffer view to help update the information in the buffer used for the offscreen scene geometry pass in TAA.</summary>
	pvr::utils::StructuredBufferView taaStructuredBufferView;

	/// <summary>Vector of depth images from the swapchain.</summary>
	std::vector<pvrvk::ImageView> depthImages;

	/// <summary>Vector with the color images used as color attachment in the offscreenFramebuffer1SPP framebuffer, used in the FXAA and TAA techniques.</summary>
	std::vector<pvrvk::Image> offscreenColorAttachmentImage1SPP;

	/// <summary>Vector with the color image views used as color attachment in the offscreenFramebuffer1SPP framebuffer, used in the FXAA and TAA techniques.</summary>
	std::vector<pvrvk::ImageView> offscreenColorAttachmentImageView1SPP;

	/// <summary>Vector with the depth images used as color attachment in the offscreenFramebuffer1SPP framebuffer, used in the FXAA and TAA techniques.</summary>
	std::vector<pvrvk::Image> offscreenDepthAttachmentImage1SPP;

	/// <summary>Vector with the depth image views used as color attachment in the offscreenFramebuffer1SPP framebuffer, used in the FXAA and TAA techniques.</summary>
	std::vector<pvrvk::ImageView> offscreenDepthAttachmentImageView1SPP;

	/// <summary>Vector with the color images used as color attachment in the offscreenFramebuffer4SPP framebuffer, used in the MSAA technique.</summary>
	std::vector<pvrvk::Image> offscreenColorAttachmentImage4SPP;

	/// <summary>Vector with the color image views used as color attachment in the offscreenFramebuffer4SPP framebuffer, used in the MSAA technique.</summary>
	std::vector<pvrvk::ImageView> offscreenColorAttachmentImageView4SPP;

	/// <summary>Vector with the depth images used as color attachment in the offscreenFramebuffer4SPP framebuffer, used in the MSAA technique.</summary>
	std::vector<pvrvk::Image> offscreenDepthAttachmentImage4SPP;

	/// <summary>Vector with the depth image views used as color attachment in the offscreenFramebuffer4SPP framebuffer, used in the MSAA technique.</summary>
	std::vector<pvrvk::ImageView> offscreenDepthAttachmentImageView4SPP;

	/// <summary>Vector with the images used as color attachment to store velocity information in the TAA technique.</summary>
	std::vector<pvrvk::Image> offscreenVelocityAttachmentImage;

	/// <summary>Vector with the image views used as color attachment to store velocity information in the TAA technique.</summary>
	std::vector<pvrvk::ImageView> offscreenVelocityAttachmentImageView;

	/// <summary>Vector with the images used as color attachment to store the previous frame to use as history information in the TAA technique.</summary>
	std::vector<pvrvk::Image> taaHistoryImage;

	/// <summary>Vector with the image views used as color attachment to store the previous frame to use as history information in the TAA technique.</summary>
	std::vector<pvrvk::ImageView> taaHistoryImageView;

	/// <summary>Vector with the framebuffers with the swapchain images for presenting on screen.</summary>
	std::vector<pvrvk::Framebuffer> onScreenFramebuffers;

	/// <summary>Framebuffer vector for the offscreen pass in FXAA and TAA.</summary>
	std::vector<pvrvk::Framebuffer> offscreenFramebuffer1SPP;

	/// <summary>Framebuffer for the MSAA offscreen pass.</summary>
	std::vector<pvrvk::Framebuffer> offscreenFramebuffer4SPP;

	/// <summary>Framebuffer for the TAA offscreen pass (has two color attachments as output).</summary>
	std::vector<pvrvk::Framebuffer> taaOffscreenFramebuffer;

	/// <summary>Render pass used for the on screen pass, either rendering the scene directly to the swapchain image or doing a postprocess pass applying some anti aliasing technique and storing the results in the swapchain image.</summary>
	pvrvk::RenderPass onScreenRenderPass;

	/// <summary>Render pass used in postprocessing steps of the different techniques implemented, sampling from a single texture (MSAA and FXAA).</summary>
	pvrvk::RenderPass postprocessRenderPass;

	/// <summary>Render pass used when rendering the scene geometry to the swapchain (in the first case, where no anti aliasing technique is being applied).</summary>
	pvrvk::RenderPass onScreenGeometryRenderPass;

	/// <summary>Render pass used when rendering the UI and IMG logo in the MSAA technique.</summary>
	pvrvk::RenderPass onScreenGeometryRenderPassNoClear;

	/// <summary>Render pass used when rendering the scene geometry to a 4 samples per pixel offscreen color attachment in the MSAA technique.</summary>
	pvrvk::RenderPass msaaOffscreenGeometryRenderPass;

	/// <summary>Render pass used when rendering the scene geometry offscreen to a color attachment in the TAA technique.</summary>
	pvrvk::RenderPass taaOffscreenGeometryRenderPass;

	/// <summary>Graphics pipeline used when rendering the scene geometry on screen (in the first case, where no anti aliasing technique is being applied).</summary>
	pvrvk::GraphicsPipeline onScreenGeometryPipeline;

	/// <summary>Graphics pipeline used when rendering the scene geometry to an offscreen color attachment with one sample per pixel (FXAA).</summary>
	pvrvk::GraphicsPipeline offscreenPipeline1SPP;

	/// <summary>Graphics pipeline to apply FXAA to an offscreen image with the scene and put results in the swapchain for presentation.</summary>
	pvrvk::GraphicsPipeline fxaaResolvePassPipeline;

	/// <summary>Graphics pipeline to apply MSAA to an offscreen image with the scene and put results in the swapchain for presentation.</summary>
	pvrvk::GraphicsPipeline msaaResolvePassPipeline;

	/// <summary>Graphics pipeline used when rendering the scene offscreen to a 4 samples per pixel offscreen color attachment in the MSAA technique.</summary>
	pvrvk::GraphicsPipeline msaaOffscreenGeometryPipeline;

	/// <summary>Graphics pipeline used when rendering the scene offscreen in the TAA technique.</summary>
	pvrvk::GraphicsPipeline taaOffscreenPipeline;

	/// <summary>Graphics pipeline used when resolving the TAA technique, reading from the offscreen texture with the scene, history and velocity, and putting the results in the swapchain for presentation.</summary>
	pvrvk::GraphicsPipeline taaResolvePassipeline;

	/// <summary>Command buffer used for the no anti aliasing case where the scene is rendered directly to the swap chain image for presenting.</summary>
	std::vector<pvrvk::CommandBuffer> noAntiAliasingCommandBuffer;

	/// <summary>Command buffer used for the MSAA technqiue (first the scene is rendered offline to a color attachment with four samples per pixel, then resolved onto the swapchain in a postprocessing pass).</summary>
	std::vector<pvrvk::CommandBuffer> msaaCommandBuffer;

	/// <summary>Command buffer used for the FXAA technique (first the scene is rendered offline to a color attachment, then resolved onto the swapchain in a postprocessing pass).</summary>
	std::vector<pvrvk::CommandBuffer> fxaaCommandBuffer;

	/// <summary>Command buffer used for the TAA technique (first the scene is rendered offline to a color attachment outputting also the velocity of each fragment, then resolved onto the swapchain in a postprocessing pass, also a copy of the current frmae is done for the next framw as history).</summary>
	std::vector<pvrvk::CommandBuffer> taaCommandBuffer;

	~DeviceResources()
	{
		if (device) { device->waitIdle(); }
		uint32_t l = swapchain->getSwapchainLength();
		for (uint32_t i = 0; i < l; ++i)
		{
			if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
		}
	}
};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanAntiAliasing : public pvr::Shell
{
public:
	/// <summary>Default constructor.</summary>
	VulkanAntiAliasing() {}

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

	/// <summary>Change the image layout of the TAA history images to SHADER_READ_ONLY_OPTIMAL, this images are updated through a image copy command and never rendered to.</summary>
	void changeTAAHistoryImageLayout(pvrvk::CommandBuffer utilityCommandBuffer);

	/// <summary>Helper method used to fill the color and depth attachment description for each of the render passes used by each anti aliasing technique,
	/// given all color attachments have the same format and all depth attachments have the same format.</summary>
	/// <param name="numColorAttachments">Number of color attachments to add to the attachment description vector.</param>
	/// <param name="addDepthAttachment">Whether to add or not a depth attachment to the attachment description vector.</param>
	/// <param name="numSamplesPerPixel">Number of samples per pixel for the color and depth attachments added to the attachment description vector.</param>
	/// <param name="vectorAttachmentDescription">Vector with the attachment description to fill.</param>
	void fillAttachmentDescription(
		int numColorAttachments, bool addDepthAttachment, pvrvk::SampleCountFlags numSamplesPerPixel, std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription);

	/// <summary>Creates the render pass used in the initial case, where no anti aliasing technique is being applied.</summary>
	void createOnScreenGeometryRenderPass();

	/// <summary>Creates the render pass used in the Multi Sampled Anti Aliasing technique. Sampled color and depth images have four samples per pixel.
	/// Output of this pass will be used later stages for resolving MSAA and draw on the swapchain image for presentation. </summary>
	void createMSAAGeometryRenderPass();

	/// <summary>Creates a render pass used in postprocessing passes, used in several of techniques implemented (MSAA, FXAA).</summary>
	void createPostProcessRenderPass();

	/// <summary>Creates the render pass used in the Temporal Anti Aliasing technique. Color, velocity and depth images are outputted. Output of this pass will be used later stages.</summary>
	void createTAAGeometryRenderPass();

	/// <summary>Helper method to build the render passes for each one of the techniques. Uses a vector to provide the attachment description.
	/// Adds a subpass self-dependency to change the access layout of all color attachments from e_COLOR_ATTACHMENT_WRITE_BIT to e_SHADER_READ_BIT.
	/// <param name="vectorAttachmentDescription">Vector with the attachment description. Depth attachment must be provided as the last element, as
	/// the attachment binding index for color attachments in VkRenderPassCreateInfo::pSubpasses::pColorAttachments is taken from the array index,
	/// interleaving a depth attachment would provide wrong information.
	/// <returns>Return the built render pass.</returns>
	pvrvk::RenderPass createTechniqueRenderPass(const std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription);

	/// <summary>Generate quasi-random values using the Halton sequence to initialize the jitter values stored in TAAGeometryPass::jitter.
	/// For a two dimensional set of samples for the jitter values, two coprime values should be picked, in this case 2 and 3.</summary>
	/// <param name="index">Index into the Halton sequence.</param>
	/// <param name="base">Base prime number to use in the sequence.</param>
	float createHaltonSequence(unsigned int index, int base);

	/// <summary>Compute jitter parameters for the TXAA technique based on the Halton sequence of quasi-random numbers, taking into account
	/// the width and height of the screen where TXAA will be applied.
	/// <param name="screenWidth">Screen width.</param>
	/// <param name="screenHeight">Screen height.</param>
	void calculateJitterParameter(int screenWidth, int screenHeight);

private:
	/// <summary>Initialize primary and secondary command buffers used for each of the techniques.</summary>
	void initializeComandBuffers();

	/// <summary>Creates the textures used for rendering the statue.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param> <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	void loadTextures(pvrvk::Device device, pvrvk::CommandBuffer utilityCommandBuffer, pvr::utils::vma::Allocator vmaAllocator);

	/// <summary>Creates the various samplers used throughout the demo.</summary>
	void createSamplers();

	/// <summary> Transitions an input image from shader readable layout to color attachment layout.
	/// Puts memoory barrier to pipeline.</summary>
	void transitionFromShaderReadToColorAttachment(pvrvk::CommandBuffer cmdBuffer, pvrvk::ImageView inputImage);

	/// <summary> Transitions an input image from color attachment layout to shader readable layout.
	/// Puts memoory barrier to pipeline.</summary>
	void transitionFromColorAttachmentToShaderRead(pvrvk::CommandBuffer cmdBuffer, pvrvk::ImageView& inputImage);

	/// <summary>Draws an assets::Mesh after the model view matrix has been set and the material prepared.</summary>
	/// <param name="cmdBuffer">The command buffer to record rendering commands to.</param>
	/// <param name="nodeIndex">Node index of the mesh to draw</param>
	void drawMesh(pvrvk::CommandBuffer cmdBuffer, int nodeIndex);

	/// <summary>Build a graphics pipeline used for postprocessing.</summary>
	/// <param name="renderpass">Render pass to use in the graphics pipeline.</param>
	/// <param name="subpassIndex">Subpass indes for this graphics pipeline.</param>
	/// <param name="pipelineLayout">Pipeline layout to use in the graphics pipeline.</param>
	/// <param name="vertexShader">Vertex shader to use in the pipeline.</param>
	/// <param name="fragmentShader">Fragment shader to use in the pipeline.</param>
	pvrvk::GraphicsPipeline createPostProcessingPipeline(
		const pvrvk::RenderPass renderpass, int subpassIndex, pvrvk::PipelineLayout pipelineLayout, const char* vertexShader, const char* fragmentShader);

	/// <summary>Build a graphics pipeline used for drawing the scene geometry.</summary>
	/// <param name="renderpass">Render pass to use in the graphics pipeline.</param>
	/// <param name="pipelineLayout">Pipeline layout to use in the graphics pipeline.</param>
	/// <param name="vertexShader">Vertex shader to use in the pipeline.</param>
	/// <param name="fragmentShader">Fragment shader to use in the pipeline.</param>
	/// <param name="addStencilTest">Whether to add stencil testing in the stencil state part of the pipeline information.</param>
	/// <param name="addMultiSampling">Whether to add multi sampling.</param>
	/// <param name="addExtraColorAttachment">Whether to add an extra color attachment for the pipeline.</param>
	pvrvk::GraphicsPipeline createScenePipeline(const pvrvk::RenderPass renderpass, pvrvk::PipelineLayout pipelineLayout, const char* vertexShader, const char* fragmentShader,
		bool addStencilTest, bool addMultiSampling, bool addExtraColorAttachment);

	/// <summary>Creates a structured buffer view with the scene information for rendering the scene geometry.</summary>
	void createSceneDataUniformBuffer();

	/// <summary>Creates a structured buffer view with information used in the TAA technique.</summary>
	void createTAAUniformBuffer();

	/// <summary>Creates and updates the descriptor sets used for rendering the scene geometry.</summary>
	void createSceneDescriptorSets();

	/// <summary>Creates the descriptor set layouts used for the resolve pass in the TAA technique.</summary>
	void createTAAResolveDescriptorSetsLayout();

	/// <summary>Creates and updates the descriptor sets for the resolve pass in the TAA technique.</summary>
	void createTAAResolveDescriptorSet();

	/// <summary>Creates and updated the descriptor sets used in TAA.</summary>
	void createTAADescriptorSets();

	/// <summary>Creates and updated the descriptor set layout and pipeline layout used for postprocessing passes in several techniques.</summary>
	void createPostprocessPassDescriptorSetsLayouts();

	/// <summary>Creates and updated the descriptor sets used for postprocessing passes in several techniques.</summary>
	void createPostprocessPassDescriptorSets();

	/// <summary>Update the structured buffer view DeviceResources::sceneStructuredBufferView with the latest values of the variables used in it.</summary>
	/// <param name="swapchainIndex">Swapchain index to udpate the structued buffer view.</param>
	void updateSceneUniformBuffer(int swapchainIndex);

	/// <summary>Update the structured buffer view DeviceResources::taaStructuredBufferView with the latest values of the variables used in it.</summary>
	/// <param name="swapchainIndex">Swapchain index to udpate the structued buffer view.</param>
	void updateSceneUniformBufferTAA(int swapchainIndex);

	/// <summary>Record the UI rendering commands (text with current technique being applied and the IMG logo).</summary>
	/// <param name="cmdBuffer">Command buffer to record to.</param>
	void recordUIRendererCommands(pvrvk::CommandBuffer cmdBuffer);

	/// <summary>Record the command buffers for the initial case where no anti aliasing technique is being applied.</summary>
	void recordNoAntialiasingComandBuffers();

	/// <summary>Record the command buffers for the MSAA anti aliasing technique.</summary>
	void recordMSAAComandBuffers();

	/// <summary>Record the command buffers for the FXAA anti aliasing technique.</summary>
	void recordFXAAComandBuffers();

	/// <summary>Record the command buffers for the TAA anti aliasing technique.</summary>
	void recordTAACommandBuffers();

	/// <summary>Handle to the scene loaded.</summary>
	pvr::assets::ModelHandle _scene;

	/// <summary>Model matrix for the scene element.</summary>
	glm::mat4 _modelMatrix = glm::mat4(1.0f);

	/// <summary>World matrix for the scene element.</summary>
	glm::mat4 _worldMatrix = glm::mat4(1.0f);

	/// <summary>View matrix for the scene element.</summary>
	glm::mat4 _viewProjMatrix = glm::mat4(1.0f);

	/// <summary>Model matrix for the previous frame for TXAA.</summary>
	glm::mat4 _preModelMatrix = glm::mat4(1.0);

	/// <summary>View projection matrix for the previous frame for TXAA.</summary>
	glm::mat4 _preProjectionViewMatrix = glm::mat4(1.0);

	/// <summary>World matrix for the previous frame for TXAA.</summary>
	glm::mat4 _preWorldMatrix = glm::mat4(1.0);

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
	AntiAliasingTechnique _currentTechniques = AntiAliasingTechnique::NOAA;

	/// <summary>Variable to map the jitter values read from jitter on each frame, ranging from 0 to frameCount.</summary>
	int _frameOffset = 0;

	/// <summary>Number of frames to apply jitter.</summary>
	int _frameCount = BufferEntryNames::TXAA::numberFramesForJitter;

	/// <summary>Jitter values to apply.</summary>
	float _jitter2DArray[BufferEntryNames::TXAA::numberFramesForJitter][2];

	/// <summary>Clear values for the color attachment for the offscreen passes of the no anti aliasing case and FXAA technique.</summary>
	pvrvk::ClearValue _clearValues[2] = { pvrvk::ClearValue(0.7f, 0.8f, 0.9f, 1.0f), pvrvk::ClearValue::createDefaultDepthStencilClearValue() };

	/// <summary>Clear values for the color attachment for the multi sampling anti aliasing technique.</summary>
	pvrvk::ClearValue _msaaClearValues[3] = { pvrvk::ClearValue(0.7f, 0.8f, 0.9f, 1.0f), pvrvk::ClearValue(0.7f, 0.8f, 0.9f, 1.0f),
		pvrvk::ClearValue::createDefaultDepthStencilClearValue() };

	/// <summary>Clear values for the color attachment for the offscreen passes of the TAA technique.</summary>
	pvrvk::ClearValue _taaClearValues[3] = { pvrvk::ClearValue(0.7f, 0.8f, 0.9f, 1.0f), pvrvk::ClearValue(0.7f, 0.8f, 0.9f, 1.0f),
		pvrvk::ClearValue::createDefaultDepthStencilClearValue() };

	/// <summary>Variable holding all the information needed to draw the scene element for the no antialiasing, MSAA and FXAA techniques.</summary>
	SceneInformationBuffer _sceneInformationBuffer;

	/// <summary>Variable holding all the information needed to draw the scene element for the TAA technique.</summary>
	SceneInformationBufferTAA _sceneInformationBufferTAA;
};

pvr::Result VulkanAntiAliasing::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create a Vulkan 1.0 instance and retrieve compatible physical devices
	pvr::utils::VulkanVersion VulkanVersion(1, 0, 0);

	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), VulkanVersion);

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

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(physicalDevice), &queueCreateInfo, 1, &queueAccessInfo);

	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);
	_deviceResources->queue->setObjectName("GraphicsQueue");

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->device->getPhysicalDevice()->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}
	else
	{
		Log(LogLevel::Information, "Error: swapchain images do not support VK_IMAGE_USAGE_TRANSFER_SRC_BIT, needed for TAA.");
		return pvr::Result::InitializationError;
	}

	// Create memory allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Create the swapchain, framebuffers and main rendering images
	// Note the use of the colour attachment load operation (pvrvk::AttachmentLoadOp::e_DONT_CARE). The final composition pass will be a full screen render
	// so we don't need to clear the attachment prior to rendering to the image as each pixel will get a new value either way
	// The final render is a full screen pass, so no depth is required (see below)...
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenRenderPass = swapChainCreateOutput.renderPass;
	_deviceResources->onScreenRenderPass->setObjectName("OnScreenRenderPass");
	_deviceResources->onScreenFramebuffers = swapChainCreateOutput.framebuffer;

	// Get current swap index
	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->depthImages.resize(_swapchainLength);

	_deviceResources->imageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->presentationSemaphores.resize(_swapchainLength);
	_deviceResources->perFrameResourcesFences.resize(_swapchainLength);

	// create the command pool and the descriptor pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	pvrvk::Extent2D swapchainSize = _deviceResources->swapchain->getDimension();
	calculateJitterParameter(swapchainSize.getWidth(), swapchainSize.getHeight());

	// This demo application makes use of quite a large number of Images and Buffers and therefore we're making possible for the descriptor pool to allocate descriptors with various limits.maxDescriptorSet*
	_deviceResources->descriptorPool =
		_deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
														   .setMaxDescriptorSets(static_cast<uint16_t>(80 * _swapchainLength))
														   .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(80 * _swapchainLength))
														   .addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, static_cast<uint16_t>(80 * _swapchainLength))
														   .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, static_cast<uint16_t>(80 * _swapchainLength)));

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

	bool requiresCommandBufferSubmission = false;
	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_scene, _deviceResources->sceneVertexBuffer, _deviceResources->sceneIndexBuffer,
		_deviceResources->utilityCommandBuffer, requiresCommandBufferSubmission, _deviceResources->vmaAllocator);

	loadTextures(_deviceResources->device, _deviceResources->utilityCommandBuffer, _deviceResources->vmaAllocator);
	createSamplers();
	createSceneDataUniformBuffer();
	createTAAUniformBuffer();
	createSceneDescriptorSets();
	createTAADescriptorSets();
	createPostprocessPassDescriptorSetsLayouts();
	createTAAResolveDescriptorSetsLayout();
	createOnScreenGeometryRenderPass();
	createMSAAGeometryRenderPass();
	createPostProcessRenderPass();
	createTAAGeometryRenderPass();
	createImagesAndFramebuffers();
	createGraphicsPipelines();
	createTAAResolveDescriptorSet();
	createPostprocessPassDescriptorSets();
	changeTAAHistoryImageLayout(_deviceResources->utilityCommandBuffer);

	//  Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), swapChainCreateOutput.renderPass, 0, getBackBufferColorspace() == pvr::ColorSpace::sRGB,
		_deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("Anti Aliasing");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	_deviceResources->msaaUIRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->msaaOffscreenGeometryRenderPass, 1,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->msaaUIRenderer.getDefaultTitle()->setText("Anti Aliasing");
	_deviceResources->msaaUIRenderer.getDefaultTitle()->commitUpdates();

	_deviceResources->utilityCommandBuffer->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->utilityCommandBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle(); // wait

	initializeComandBuffers();
	recordNoAntialiasingComandBuffers();
	recordMSAAComandBuffers();
	recordFXAAComandBuffers();
	recordTAACommandBuffers();

	// create the synchronisation primitives
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->presentationSemaphores[i] = (_deviceResources->device->createSemaphore());
		_deviceResources->imageAcquiredSemaphores[i] = (_deviceResources->device->createSemaphore());

		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = (_deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT));
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));
	}

	_scene->getCameraProperties(0, SceneElements::_cameraFov, SceneElements::_cameraFrom, SceneElements::_cameraTo, SceneElements::_cameraUp);
	_worldMatrix = _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
	SceneElements::_cameraLookAt = glm::lookAt(SceneElements::_cameraFrom, SceneElements::_cameraTo, SceneElements::_cameraUp);

	return pvr::Result::Success;
}

void VulkanAntiAliasing::createImagesAndFramebuffers()
{
	pvrvk::ImageCreateInfo colorImageInfo1SPP = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _deviceResources->swapchain->getImageFormat(),
		pvrvk::Extent3D(getWidth(), getHeight(), 1u), pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT);

	pvrvk::ImageCreateInfo depthImageInfo1SPP = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _deviceResources->depthImages[0]->getFormat(),
		pvrvk::Extent3D(getWidth(), getHeight(), 1u), pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT);

	pvrvk::ImageCreateInfo colorImageInfo4SPP =
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _deviceResources->swapchain->getImageFormat(), pvrvk::Extent3D(getWidth(), getHeight(), 1u),
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_INPUT_ATTACHMENT_BIT);
	colorImageInfo4SPP.setNumSamples(pvrvk::SampleCountFlags::e_4_BIT);

	pvrvk::ImageCreateInfo depthImageInfo4SPP =
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _deviceResources->depthImages[0]->getFormat(), pvrvk::Extent3D(getWidth(), getHeight(), 1u),
			pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_INPUT_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_INPUT_ATTACHMENT_BIT);
	depthImageInfo4SPP.setNumSamples(pvrvk::SampleCountFlags::e_4_BIT);

	pvrvk::ImageCreateInfo taaHistoryColorImageInfo = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _deviceResources->swapchain->getImageFormat(),
		pvrvk::Extent3D(getWidth(), getHeight(), 1u), pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// Build color and depth attachment image and image views for the offscreen pass of FXAA (1 sample per pixel)
		pvrvk::Image colorImage1SPP = pvr::utils::createImage(_deviceResources->device, colorImageInfo1SPP, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		pvrvk::ImageView colorImageView1SPP = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(colorImage1SPP));
		_deviceResources->offscreenColorAttachmentImage1SPP.push_back(colorImage1SPP);
		_deviceResources->offscreenColorAttachmentImageView1SPP.push_back(colorImageView1SPP);

		pvrvk::Image depthImage1SPP = pvr::utils::createImage(
			_deviceResources->device, depthImageInfo1SPP, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
		pvrvk::ImageView depthImageView1SPP = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(depthImage1SPP));
		_deviceResources->offscreenDepthAttachmentImage1SPP.push_back(depthImage1SPP);
		_deviceResources->offscreenDepthAttachmentImageView1SPP.push_back(depthImageView1SPP);

		// Build color and depth attachment image and image views for the offscreen pass of MSAA (4 samples per pixel)
		pvrvk::Image colorImage4SPP = pvr::utils::createImage(_deviceResources->device, colorImageInfo4SPP, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		pvrvk::ImageView colorImageView4SPP = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(colorImage4SPP));
		_deviceResources->offscreenColorAttachmentImage4SPP.push_back(colorImage4SPP);
		_deviceResources->offscreenColorAttachmentImageView4SPP.push_back(colorImageView4SPP);

		pvrvk::Image depthImage4SPP = pvr::utils::createImage(
			_deviceResources->device, depthImageInfo4SPP, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
		pvrvk::ImageView depthImageView4SPP = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(depthImage4SPP));
		_deviceResources->offscreenDepthAttachmentImage4SPP.push_back(depthImage4SPP);
		_deviceResources->offscreenDepthAttachmentImageView4SPP.push_back(depthImageView4SPP);

		// Build history and velocity images and image views for TAA
		pvrvk::Image taaHistoryImage = pvr::utils::createImage(_deviceResources->device, taaHistoryColorImageInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		pvrvk::ImageView taaHistoryImageView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(taaHistoryImage));
		_deviceResources->taaHistoryImage.push_back(taaHistoryImage);
		_deviceResources->taaHistoryImageView.push_back(taaHistoryImageView);

		pvrvk::Image velocityImage = pvr::utils::createImage(_deviceResources->device, colorImageInfo1SPP, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		pvrvk::ImageView velocityImageView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(velocityImage));
		_deviceResources->offscreenVelocityAttachmentImage.push_back(velocityImage);
		_deviceResources->offscreenVelocityAttachmentImageView.push_back(velocityImageView);

		// Build framebuffer for the offscreen scene pass for FXAA with one sample per pixel
		pvrvk::FramebufferCreateInfo offscreenFramebuffer1SPPCreateInfo;
		offscreenFramebuffer1SPPCreateInfo.setAttachment(0, _deviceResources->offscreenColorAttachmentImageView1SPP[i]);
		offscreenFramebuffer1SPPCreateInfo.setAttachment(1, _deviceResources->offscreenDepthAttachmentImageView1SPP[i]);
		offscreenFramebuffer1SPPCreateInfo.setDimensions(getWidth(), getHeight());
		offscreenFramebuffer1SPPCreateInfo.setRenderPass(_deviceResources->onScreenGeometryRenderPass);
		_deviceResources->offscreenFramebuffer1SPP.push_back(_deviceResources->device->createFramebuffer(offscreenFramebuffer1SPPCreateInfo));

		// Build framebuffer for the offscreen scene pass for MSAA with four samples per pixel
		pvrvk::FramebufferCreateInfo offscreenFramebuffer4SPPCreateInfo;
		offscreenFramebuffer4SPPCreateInfo.setAttachment(0, _deviceResources->swapchain->getImageView(i));
		offscreenFramebuffer4SPPCreateInfo.setAttachment(1, _deviceResources->offscreenColorAttachmentImageView4SPP[i]);
		offscreenFramebuffer4SPPCreateInfo.setAttachment(2, _deviceResources->offscreenDepthAttachmentImageView4SPP[i]);
		offscreenFramebuffer4SPPCreateInfo.setDimensions(getWidth(), getHeight());
		offscreenFramebuffer4SPPCreateInfo.setRenderPass(_deviceResources->msaaOffscreenGeometryRenderPass);
		_deviceResources->offscreenFramebuffer4SPP.push_back(_deviceResources->device->createFramebuffer(offscreenFramebuffer4SPPCreateInfo));

		// Build framebuffer for the offscreen scene pass for TAA with four samples per pixel
		pvrvk::FramebufferCreateInfo offscreenFramebufferTAACreateInfo;
		offscreenFramebufferTAACreateInfo.setAttachment(0, _deviceResources->offscreenColorAttachmentImageView1SPP[i]);
		offscreenFramebufferTAACreateInfo.setAttachment(1, _deviceResources->offscreenVelocityAttachmentImageView[i]);
		offscreenFramebufferTAACreateInfo.setAttachment(2, _deviceResources->offscreenDepthAttachmentImageView1SPP[i]);
		offscreenFramebufferTAACreateInfo.setDimensions(getWidth(), getHeight());
		offscreenFramebufferTAACreateInfo.setRenderPass(_deviceResources->taaOffscreenGeometryRenderPass);
		_deviceResources->taaOffscreenFramebuffer.push_back(_deviceResources->device->createFramebuffer(offscreenFramebufferTAACreateInfo));
	}
}

void VulkanAntiAliasing::createGraphicsPipelines()
{
	_deviceResources->onScreenGeometryPipeline = createScenePipeline(_deviceResources->onScreenGeometryRenderPass, _deviceResources->scenePipelineLayout,
		ShaderFiles::NOAAVertexShaderFile, ShaderFiles::NOAAFragmentShaderFile, true, false, false);
	_deviceResources->onScreenGeometryPipeline->setObjectName("OnScreenGeometryGraphicsPipeline");

	_deviceResources->msaaOffscreenGeometryPipeline = createScenePipeline(_deviceResources->msaaOffscreenGeometryRenderPass, _deviceResources->scenePipelineLayout,
		ShaderFiles::NOAAVertexShaderFile, ShaderFiles::NOAAFragmentShaderFile, true, true, false);
	_deviceResources->msaaOffscreenGeometryPipeline->setObjectName("MSAAOffScreenGeometryGraphicsPipeline");

	_deviceResources->msaaResolvePassPipeline = createPostProcessingPipeline(_deviceResources->msaaOffscreenGeometryRenderPass, 1, _deviceResources->postProcessPipelineLayout,
		ShaderFiles::AttributelessVertexShaderFile, ShaderFiles::MSAAFragmentShaderFile);
	_deviceResources->msaaResolvePassPipeline->setObjectName("MSAAResolvePassGraphicsPipeline");

	_deviceResources->fxaaResolvePassPipeline = createPostProcessingPipeline(
		_deviceResources->onScreenRenderPass, 0, _deviceResources->postProcessPipelineLayout, ShaderFiles::AttributelessVertexShaderFile, ShaderFiles::FXAAFragmentShaderFile);
	_deviceResources->fxaaResolvePassPipeline->setObjectName("FXAAResolvePassGraphicsPipeline");

	_deviceResources->offscreenPipeline1SPP = createScenePipeline(_deviceResources->onScreenGeometryRenderPass, _deviceResources->scenePipelineLayout,
		ShaderFiles::NOAAVertexShaderFile, ShaderFiles::NOAAFragmentShaderFile, false, false, false);
	_deviceResources->offscreenPipeline1SPP->setObjectName("OffScreenGraphicsPipeline");

	_deviceResources->taaOffscreenPipeline = createScenePipeline(_deviceResources->taaOffscreenGeometryRenderPass, _deviceResources->scenePipelineLayout,
		ShaderFiles::VelocityTXAAVertexShaderFile, ShaderFiles::VelocityTXAAFragmentShaderFile, false, false, true);
	_deviceResources->taaOffscreenPipeline->setObjectName("TAAOffScreenGraphicsPipeline");

	_deviceResources->taaResolvePassipeline = createPostProcessingPipeline(_deviceResources->onScreenRenderPass, 0, _deviceResources->taaResolvePipelineLayout,
		ShaderFiles::AttributelessVertexShaderFile, ShaderFiles::ResolveTXAAFragmentShaderFile);
	_deviceResources->taaResolvePassipeline->setObjectName("TAAResolvePassGraphicsPipeline");
}

void VulkanAntiAliasing::changeTAAHistoryImageLayout(pvrvk::CommandBuffer utilityCommandBuffer)
{
	pvrvk::MemoryBarrierSet barrier;
	pvrvk::ImageMemoryBarrier imageBarrier;
	imageBarrier.setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	imageBarrier.setOldLayout(pvrvk::ImageLayout::e_UNDEFINED);
	imageBarrier.setNewLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
	imageBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));

	// TAA update history render pass
	for (uint32_t i = 0; i < _swapchainLength; i++)
	{
		imageBarrier.setImage(_deviceResources->taaHistoryImage[i]);
		barrier.addBarrier(imageBarrier);
	}

	utilityCommandBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, barrier);
}

void VulkanAntiAliasing::fillAttachmentDescription(
	int numColorAttachments, bool addDepthAttachment, pvrvk::SampleCountFlags numSamplesPerPixel, std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription)
{
	vectorAttachmentDescription.clear();

	for (int i = 0; i < numColorAttachments; ++i)
	{
		vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createColorDescription(_deviceResources->swapchain->getImageFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, numSamplesPerPixel));
	}

	if (addDepthAttachment)
	{
		vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createDepthStencilDescription(_deviceResources->depthImages[0]->getFormat(),
			pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, numSamplesPerPixel));
	}
}

void VulkanAntiAliasing::createOnScreenGeometryRenderPass()
{
	std::vector<pvrvk::AttachmentDescription> vectorAttachmentDescription;
	fillAttachmentDescription(1, true, pvrvk::SampleCountFlags::e_1_BIT, vectorAttachmentDescription);
	_deviceResources->onScreenGeometryRenderPass = createTechniqueRenderPass(vectorAttachmentDescription);
	_deviceResources->onScreenGeometryRenderPass->setObjectName("OnScreenGeometryRenderPass");
}

void VulkanAntiAliasing::createMSAAGeometryRenderPass()
{
	std::vector<pvrvk::AttachmentDescription> vectorAttachmentDescription;

	vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createColorDescription(_deviceResources->swapchain->getImageFormat(), pvrvk::ImageLayout::e_UNDEFINED,
		pvrvk::ImageLayout::e_PRESENT_SRC_KHR, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createColorDescription(_deviceResources->swapchain->getImageFormat(), pvrvk::ImageLayout::e_UNDEFINED,
		pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_4_BIT));

	vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createDepthStencilDescription(_deviceResources->depthImages[0]->getFormat(),
		pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE,
		pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_4_BIT));

	pvrvk::RenderPassCreateInfo renderPassInfo;

	renderPassInfo.setAttachmentDescription(0, vectorAttachmentDescription[0]);
	renderPassInfo.setAttachmentDescription(1, vectorAttachmentDescription[1]);
	renderPassInfo.setAttachmentDescription(2, vectorAttachmentDescription[2]);

	std::vector<pvrvk::SubpassDescription> subpassDescription(2);
	// First subpass: Render the scene to an offscreen framebuffer with color and depth attachments having each 4 samples per pixel
	subpassDescription[0].setColorAttachmentReference(0, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	subpassDescription[0].setDepthStencilAttachmentReference(pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	subpassDescription[0].setPreserveAttachmentReference(0, 0);

	// Second subpass: Render a postprocessing pass to put the results from the offscreen MSAA color attachment onto the swapchain
	subpassDescription[1].setInputAttachmentReference(0, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	subpassDescription[1].setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

	renderPassInfo.setSubpass(0, subpassDescription[0]);
	renderPassInfo.setSubpass(1, subpassDescription[1]);

	// Add external subpass dependencies to avoid the implicit subpass dependencies and to provide more optimal pipeline stage task synchronisation
	pvrvk::SubpassDependency dependencies[1];

	dependencies[0].setSrcSubpass(0);
	dependencies[0].setDstSubpass(1);
	dependencies[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT | pvrvk::PipelineStageFlags::e_LATE_FRAGMENT_TESTS_BIT);
	dependencies[0].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT | pvrvk::PipelineStageFlags::e_EARLY_FRAGMENT_TESTS_BIT);
	dependencies[0].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependencies[0].setDstAccessMask(pvrvk::AccessFlags::e_INPUT_ATTACHMENT_READ_BIT);
	dependencies[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	renderPassInfo.addSubpassDependency(dependencies[0]);

	pvrvk::SubpassDependency externalDependencies[2];
	externalDependencies[0] =
		pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
			pvrvk::AccessFlags::e_NONE, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::DependencyFlags::e_BY_REGION_BIT);
	externalDependencies[1] =
		pvrvk::SubpassDependency(1, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
			pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_NONE, pvrvk::DependencyFlags::e_BY_REGION_BIT);

	renderPassInfo.addSubpassDependency(externalDependencies[0]);
	renderPassInfo.addSubpassDependency(externalDependencies[1]);

	_deviceResources->msaaOffscreenGeometryRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);
	_deviceResources->msaaOffscreenGeometryRenderPass->setObjectName("MSAAOffscreenGeometryRenderPass");
}

void VulkanAntiAliasing::createPostProcessRenderPass()
{
	std::vector<pvrvk::AttachmentDescription> vectorAttachmentDescription;
	fillAttachmentDescription(1, false, pvrvk::SampleCountFlags::e_1_BIT, vectorAttachmentDescription);
	_deviceResources->postprocessRenderPass = createTechniqueRenderPass(vectorAttachmentDescription);
	_deviceResources->postprocessRenderPass->setObjectName("PostProcessRenderPass");
}

void VulkanAntiAliasing::createTAAGeometryRenderPass()
{
	std::vector<pvrvk::AttachmentDescription> vectorAttachmentDescription;
	fillAttachmentDescription(2, true, pvrvk::SampleCountFlags::e_1_BIT, vectorAttachmentDescription);
	_deviceResources->taaOffscreenGeometryRenderPass = createTechniqueRenderPass(vectorAttachmentDescription);
	_deviceResources->taaOffscreenGeometryRenderPass->setObjectName("TAAOffscreenGeometryRenderPass");
}

void VulkanAntiAliasing::initializeComandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->noAntiAliasingCommandBuffer.push_back(_deviceResources->commandPool->allocateCommandBuffer());
		_deviceResources->msaaCommandBuffer.push_back(_deviceResources->commandPool->allocateCommandBuffer());
		_deviceResources->fxaaCommandBuffer.push_back(_deviceResources->commandPool->allocateCommandBuffer());
		_deviceResources->taaCommandBuffer.push_back(_deviceResources->commandPool->allocateCommandBuffer());
	}
}

void VulkanAntiAliasing::loadTextures(pvrvk::Device device, pvrvk::CommandBuffer utilityCommandBuffer, pvr::utils::vma::Allocator vmaAllocator)
{
	bool astcSupported = pvr::utils::isSupportedFormat(_deviceResources->device->getPhysicalDevice(), pvrvk::Format::e_ASTC_4x4_UNORM_BLOCK);

	_deviceResources->albedoImageView =
		pvr::utils::loadAndUploadImageAndView(_deviceResources->device, (SceneElements::StatueTexFile + (astcSupported ? "_astc.pvr" : ".pvr")).c_str(), true, utilityCommandBuffer,
			*this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	_deviceResources->normalMapImageView = pvr::utils::loadAndUploadImageAndView(_deviceResources->device,
		(SceneElements::StatueNormalMapTexFile + (astcSupported ? "_astc.pvr" : ".pvr")).c_str(), true, utilityCommandBuffer, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
}

void VulkanAntiAliasing::createSamplers()
{
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;

	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	_deviceResources->samplerBilinear = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	_deviceResources->samplerNearest = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);
}

pvrvk::RenderPass VulkanAntiAliasing::createTechniqueRenderPass(const std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription)
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
			subpass.setColorAttachmentReference(i, pvrvk::AttachmentReference(i, finalLayout));
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

void VulkanAntiAliasing::transitionFromColorAttachmentToShaderRead(pvrvk::CommandBuffer cmdBuffer, pvrvk::ImageView& inputImage)
{
	pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL;
	pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

	pvrvk::MemoryBarrierSet fromColorAttachmentToShaderReadTransition;
	fromColorAttachmentToShaderReadTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
		inputImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout, 0, 0));

	cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, fromColorAttachmentToShaderReadTransition);
}

/// <summary>Draws an assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <param name="cmdBuffer">The secondary command buffer to record rendering commands to.</param>
/// <param name="nodeIndex">Node index of the mesh to draw</param>
void VulkanAntiAliasing::drawMesh(pvrvk::CommandBuffer cmdBuffer, int nodeIndex)
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

pvrvk::GraphicsPipeline VulkanAntiAliasing::createPostProcessingPipeline(
	const pvrvk::RenderPass renderpass, int subpassIndex, pvrvk::PipelineLayout pipelineLayout, const char* vertexShader, const char* fragmentShader)
{
	pvrvk::GraphicsPipelineCreateInfo pipelineInfo;

	pvrvk::Extent2D viewportDimensions = _deviceResources->swapchain->getDimension();

	pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDimensions.getWidth()), static_cast<float>(viewportDimensions.getHeight())),
		pvrvk::Rect2D(0, 0, viewportDimensions.getWidth(), viewportDimensions.getHeight()));

	pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
	pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// depth stencil state
	pipelineInfo.depthStencil.enableDepthWrite(false);
	pipelineInfo.depthStencil.enableDepthTest(false);

	// blend state
	pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

	pipelineInfo.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(vertexShader)->readToEnd<uint32_t>())));
	pipelineInfo.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(fragmentShader)->readToEnd<uint32_t>())));

	pipelineInfo.vertexInput.clear();
	pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);
	pipelineInfo.renderPass = renderpass;
	pipelineInfo.pipelineLayout = pipelineLayout;

	pipelineInfo.subpass = subpassIndex;

	return _deviceResources->device->createGraphicsPipeline(pipelineInfo, _deviceResources->pipelineCache);
}

pvrvk::GraphicsPipeline VulkanAntiAliasing::createScenePipeline(const pvrvk::RenderPass renderpass, pvrvk::PipelineLayout pipelineLayout, const char* vertexShader,
	const char* fragmentShader, bool addStencilTest, bool addMultiSampling, bool addExtraColorAttachment)
{
	pvrvk::GraphicsPipelineCreateInfo pipelineInfo;

	pvrvk::Extent2D viewportDimensions = _deviceResources->swapchain->getDimension();

	pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDimensions.getWidth()), static_cast<float>(viewportDimensions.getHeight())),
		pvrvk::Rect2D(0, 0, viewportDimensions.getWidth(), viewportDimensions.getHeight()));

	pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

	if (addStencilTest)
	{
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

		pipelineInfo.depthStencil.setStencilFront(stencilState);
		pipelineInfo.depthStencil.setStencilBack(stencilState);
		pipelineInfo.depthStencil.enableAllStates(true);
	}

	// depth stencil state
	pipelineInfo.depthStencil.enableDepthWrite(true);
	pipelineInfo.depthStencil.enableDepthTest(true);
	pipelineInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
	pipelineInfo.depthStencil.enableStencilTest(false);

	// blend state
	pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

	if (addExtraColorAttachment) { pipelineInfo.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState()); }

	pipelineInfo.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(vertexShader)->readToEnd<uint32_t>())));
	pipelineInfo.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(fragmentShader)->readToEnd<uint32_t>())));

	const pvr::assets::Mesh& mesh = _scene->getMesh(0);
	pipelineInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToPVRVk(mesh.getPrimitiveType()));
	pvr::utils::populateInputAssemblyFromMesh(
		mesh, VertexAttribBindings, sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipelineInfo.vertexInput, pipelineInfo.inputAssembler);

	pipelineInfo.renderPass = renderpass;
	pipelineInfo.pipelineLayout = pipelineLayout;

	if (addMultiSampling)
	{
		// Sample count selected as 4 by default
		pipelineInfo.multiSample.setNumRasterizationSamples(pvrvk::SampleCountFlags::e_4_BIT);
		pipelineInfo.multiSample.setSampleShading(true);
		pipelineInfo.multiSample.setMinSampleShading(0.2f);
	}

	return _deviceResources->device->createGraphicsPipeline(pipelineInfo, _deviceResources->pipelineCache);
}

void VulkanAntiAliasing::createSceneDataUniformBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::NOAA::MVPMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::NOAA::LightDirModel, pvr::GpuDatatypes::vec3);

	_deviceResources->sceneStructuredBufferView.initDynamic(desc, _scene->getNumMeshNodes() * _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->sceneUniformBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->sceneStructuredBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->sceneUniformBuffer->setObjectName("SceneUniformBufferUBO");
	_deviceResources->sceneStructuredBufferView.pointToMappedMemory(_deviceResources->sceneUniformBuffer->getDeviceMemory()->getMappedData());
}

void VulkanAntiAliasing::createTAAUniformBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::TXAA::prevModelMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::TXAA::prevProjViewMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::TXAA::prevWorldMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::TXAA::currModelMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::TXAA::currProjViewMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::TXAA::currWorldMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::TXAA::currLightDir, pvr::GpuDatatypes::vec3);
	desc.addElement(BufferEntryNames::TXAA::jitter, pvr::GpuDatatypes::vec2);

	_deviceResources->taaStructuredBufferView.initDynamic(desc, _scene->getNumMeshNodes() * _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->taaUniformBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->taaStructuredBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->taaUniformBuffer->setObjectName("TAAUniformBufferUBO");
	_deviceResources->taaStructuredBufferView.pointToMappedMemory(_deviceResources->taaUniformBuffer->getDeviceMemory()->getMappedData());
}

void VulkanAntiAliasing::createSceneDescriptorSets()
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

void VulkanAntiAliasing::createTAAResolveDescriptorSetsLayout()
{
	pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

	descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	descSetLayout.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	descSetLayout.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->taaResolveDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayout);

	pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.addDescSetLayout(_deviceResources->taaResolveDescriptorSetLayout);
	_deviceResources->taaResolvePipelineLayout = _deviceResources->device->createPipelineLayout(pipelineLayoutInfo);
}

void VulkanAntiAliasing::createTAAResolveDescriptorSet()
{
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->taaResolveDescriptorSet.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->taaResolveDescriptorSetLayout));
		_deviceResources->taaResolveDescriptorSet.back()->setObjectName("TAASwapchain" + std::to_string(i) + "DescriptorSet");

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->taaResolveDescriptorSet[i], 0)
									.setImageInfo(0,
										pvrvk::DescriptorImageInfo(_deviceResources->offscreenColorAttachmentImageView1SPP[i], _deviceResources->samplerBilinear,
											pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->taaResolveDescriptorSet[i], 1)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->taaHistoryImageView[i], _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->taaResolveDescriptorSet[i], 2)
									.setImageInfo(0,
										pvrvk::DescriptorImageInfo(_deviceResources->offscreenVelocityAttachmentImageView[i], _deviceResources->samplerNearest,
											pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanAntiAliasing::createTAADescriptorSets()
{
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->taaVertexDescriptorSets.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->sceneVertexDescriptorSetLayout));
		_deviceResources->taaFragmentDescriptorSets.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->sceneFragmentDescriptorSetLayout));

		_deviceResources->taaVertexDescriptorSets.back()->setObjectName("TAAVertexSwapchain" + std::to_string(i) + "DescriptorSet");
		_deviceResources->taaFragmentDescriptorSets.back()->setObjectName("TAAFragmentSwapchain" + std::to_string(i) + "DescriptorSet");

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->taaFragmentDescriptorSets[i], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->albedoImageView, _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->taaFragmentDescriptorSets[i], 1)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->normalMapImageView, _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->taaVertexDescriptorSets[i], 0)
									.setBufferInfo(0,
										pvrvk::DescriptorBufferInfo(_deviceResources->taaUniformBuffer, _deviceResources->taaStructuredBufferView.getDynamicSliceOffset(i),
											_deviceResources->taaStructuredBufferView.getDynamicSliceSize())));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanAntiAliasing::createPostprocessPassDescriptorSetsLayouts()
{
	pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;
	descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->postProcessDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayout);

	pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.addDescSetLayout(_deviceResources->postProcessDescriptorSetLayout);
	_deviceResources->postProcessPipelineLayout = _deviceResources->device->createPipelineLayout(pipelineLayoutInfo);
}

void VulkanAntiAliasing::updateSceneUniformBuffer(int swapchainIndex)
{
	_sceneInformationBuffer.lightDirModel = glm::vec3(SceneElements::LightDir * _modelMatrix);
	_sceneInformationBuffer.modelViewProjectionMatrix = _viewProjMatrix * _modelMatrix * _worldMatrix;

	_deviceResources->sceneStructuredBufferView.getElementByName(BufferEntryNames::NOAA::MVPMatrix, 0, swapchainIndex).setValue(_sceneInformationBuffer.modelViewProjectionMatrix);
	_deviceResources->sceneStructuredBufferView.getElementByName(BufferEntryNames::NOAA::LightDirModel, 0, swapchainIndex).setValue(_sceneInformationBuffer.lightDirModel);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->sceneUniformBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->sceneUniformBuffer->getDeviceMemory()->flushRange(
			_deviceResources->sceneStructuredBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->sceneStructuredBufferView.getDynamicSliceSize());
	}
}

void VulkanAntiAliasing::updateSceneUniformBufferTAA(int swapchainIndex)
{
	_frameOffset++;
	_frameOffset = _frameOffset % _frameCount;

	_sceneInformationBufferTAA.currLightDir = glm::vec3(SceneElements::LightDir * _modelMatrix);
	_sceneInformationBufferTAA.currModel = _modelMatrix;
	_sceneInformationBufferTAA.currProjView = _viewProjMatrix;
	_sceneInformationBufferTAA.currWorld = _worldMatrix;
	_sceneInformationBufferTAA.preModel = _preModelMatrix;
	_sceneInformationBufferTAA.preProjView = _preProjectionViewMatrix;
	_sceneInformationBufferTAA.preWorld = _preWorldMatrix;
	_sceneInformationBufferTAA.jitter = glm::vec2(_jitter2DArray[_frameOffset][0] * (1.0 / getWidth()), _jitter2DArray[_frameOffset][1] * (1.0 / getHeight()));

	_deviceResources->taaStructuredBufferView.getElementByName(BufferEntryNames::TXAA::prevModelMatrix, 0, swapchainIndex).setValue(_sceneInformationBufferTAA.preModel);
	_deviceResources->taaStructuredBufferView.getElementByName(BufferEntryNames::TXAA::prevProjViewMatrix, 0, swapchainIndex).setValue(_sceneInformationBufferTAA.preProjView);
	_deviceResources->taaStructuredBufferView.getElementByName(BufferEntryNames::TXAA::prevWorldMatrix, 0, swapchainIndex).setValue(_sceneInformationBufferTAA.preWorld);
	_deviceResources->taaStructuredBufferView.getElementByName(BufferEntryNames::TXAA::currModelMatrix, 0, swapchainIndex).setValue(_sceneInformationBufferTAA.currModel);
	_deviceResources->taaStructuredBufferView.getElementByName(BufferEntryNames::TXAA::currProjViewMatrix, 0, swapchainIndex).setValue(_sceneInformationBufferTAA.currProjView);
	_deviceResources->taaStructuredBufferView.getElementByName(BufferEntryNames::TXAA::currWorldMatrix, 0, swapchainIndex).setValue(_sceneInformationBufferTAA.currWorld);
	_deviceResources->taaStructuredBufferView.getElementByName(BufferEntryNames::TXAA::currLightDir, 0, swapchainIndex).setValue(_sceneInformationBufferTAA.currLightDir);
	_deviceResources->taaStructuredBufferView.getElementByName(BufferEntryNames::TXAA::jitter, 0, swapchainIndex).setValue(_sceneInformationBufferTAA.jitter);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->taaUniformBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->taaUniformBuffer->getDeviceMemory()->flushRange(
			_deviceResources->taaStructuredBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->taaStructuredBufferView.getDynamicSliceSize());
	}
}

void VulkanAntiAliasing::recordUIRendererCommands(pvrvk::CommandBuffer cmdBuffer)
{
	_deviceResources->uiRenderer.beginRendering(cmdBuffer);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
}

void VulkanAntiAliasing::recordNoAntialiasingComandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->noAntiAliasingCommandBuffer[i]->setObjectName("NoAACommandBufferSwapchain" + std::to_string(i));

		_deviceResources->noAntiAliasingCommandBuffer[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->noAntiAliasingCommandBuffer[i], pvrvk::DebugUtilsLabel("No antialiasing"));

		// Draw the scene directly to the swapchain image to be presented
		_deviceResources->noAntiAliasingCommandBuffer[i]->beginRenderPass(_deviceResources->onScreenFramebuffers[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, _clearValues, 2);
		_deviceResources->noAntiAliasingCommandBuffer[i]->bindPipeline(_deviceResources->onScreenGeometryPipeline);
		_deviceResources->noAntiAliasingCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneFragmentDescriptorSets[i]);
		_deviceResources->noAntiAliasingCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 1u, _deviceResources->sceneVertexDescriptorSets[i]);
		drawMesh(_deviceResources->noAntiAliasingCommandBuffer[i], 0);

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->noAntiAliasingCommandBuffer[i]);

		recordUIRendererCommands(_deviceResources->noAntiAliasingCommandBuffer[i]);

		_deviceResources->noAntiAliasingCommandBuffer[i]->endRenderPass();
		_deviceResources->noAntiAliasingCommandBuffer[i]->end();
	}
}

void VulkanAntiAliasing::recordMSAAComandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->msaaCommandBuffer[i]->setObjectName("MSAACommandBufferSwapchain" + std::to_string(i));

		_deviceResources->msaaCommandBuffer[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->msaaCommandBuffer[i], pvrvk::DebugUtilsLabel("MSAA offscreen pass"));

		// Do an initial offscreen pass writing the scene to a color attachment with 4 samples MSAA
		_deviceResources->msaaCommandBuffer[i]->beginRenderPass(_deviceResources->offscreenFramebuffer4SPP[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, _msaaClearValues, 3);
		_deviceResources->msaaCommandBuffer[i]->bindPipeline(_deviceResources->msaaOffscreenGeometryPipeline);
		_deviceResources->msaaCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneFragmentDescriptorSets[i]);
		_deviceResources->msaaCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 1u, _deviceResources->sceneVertexDescriptorSets[i]);
		drawMesh(_deviceResources->msaaCommandBuffer[i], 0);
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->msaaCommandBuffer[i]);
		_deviceResources->msaaCommandBuffer[i]->nextSubpass(pvrvk::SubpassContents::e_INLINE);

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->msaaCommandBuffer[i], pvrvk::DebugUtilsLabel("MSAA resolve pass"));

		// Do a postprocessing pass to copy the results from each sample and put the results in the swap chain image to be presented
		//_deviceResources->msaaCommandBuffer[i]->beginRenderPass(_deviceResources->onScreenFramebuffers[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, _clearValues, 2);
		_deviceResources->msaaCommandBuffer[i]->bindPipeline(_deviceResources->msaaResolvePassPipeline);
		_deviceResources->msaaCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->postProcessPipelineLayout, 0u, _deviceResources->msaaResolvePassDescriptorSets[i]);
		_deviceResources->msaaCommandBuffer[i]->draw(0, 3);

		_deviceResources->msaaUIRenderer.beginRendering(_deviceResources->msaaCommandBuffer[i]);
		_deviceResources->msaaUIRenderer.getDefaultTitle()->render();
		_deviceResources->msaaUIRenderer.getSdkLogo()->render();
		_deviceResources->msaaUIRenderer.endRendering();

		_deviceResources->msaaCommandBuffer[i]->endRenderPass();
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->msaaCommandBuffer[i]);

		_deviceResources->msaaCommandBuffer[i]->end();
	}
}

void VulkanAntiAliasing::recordFXAAComandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->fxaaCommandBuffer[i]->setObjectName("FXAACommandBufferSwapchain" + std::to_string(i));

		_deviceResources->fxaaCommandBuffer[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->fxaaCommandBuffer[i], pvrvk::DebugUtilsLabel("FXAA offscreen pass"));

		// Do an initial offscreen pass writing the scene to a color attachment
		_deviceResources->fxaaCommandBuffer[i]->beginRenderPass(_deviceResources->offscreenFramebuffer1SPP[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, _clearValues, 2);
		_deviceResources->fxaaCommandBuffer[i]->bindPipeline(_deviceResources->offscreenPipeline1SPP);
		_deviceResources->fxaaCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneFragmentDescriptorSets[i]);
		_deviceResources->fxaaCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 1u, _deviceResources->sceneVertexDescriptorSets[i]);
		drawMesh(_deviceResources->fxaaCommandBuffer[i], 0);
		_deviceResources->fxaaCommandBuffer[i]->endRenderPass();
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->fxaaCommandBuffer[i]);

		// Do a postprocessing pass to apply FXAA to the offscreen texture, putting the results in the swap chain image to be presented
		transitionFromColorAttachmentToShaderRead(_deviceResources->fxaaCommandBuffer[i], _deviceResources->offscreenColorAttachmentImageView1SPP[i]);
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->fxaaCommandBuffer[i], pvrvk::DebugUtilsLabel("FXAA resolve pass"));

		_deviceResources->fxaaCommandBuffer[i]->beginRenderPass(_deviceResources->onScreenFramebuffers[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, _clearValues, 2);
		_deviceResources->fxaaCommandBuffer[i]->bindPipeline(_deviceResources->fxaaResolvePassPipeline);
		_deviceResources->fxaaCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->postProcessPipelineLayout, 0u, _deviceResources->fxaaResolvePassDescriptorSet[i]);
		_deviceResources->fxaaCommandBuffer[i]->draw(0, 3);
		recordUIRendererCommands(_deviceResources->fxaaCommandBuffer[i]);
		_deviceResources->fxaaCommandBuffer[i]->endRenderPass();

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->fxaaCommandBuffer[i]);

		transitionFromShaderReadToColorAttachment(_deviceResources->fxaaCommandBuffer[i], _deviceResources->offscreenColorAttachmentImageView1SPP[i]);
		_deviceResources->fxaaCommandBuffer[i]->end();
	}
}

void VulkanAntiAliasing::recordTAACommandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->taaCommandBuffer[i]->setObjectName("TAACommandBufferSwapchain" + std::to_string(i));

		_deviceResources->taaCommandBuffer[i]->begin();

		// Do an initial offscreen pass writing to two color attachments (scene and velocity)
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->taaCommandBuffer[i], pvrvk::DebugUtilsLabel("TAA offscreen pass"));

		_deviceResources->taaCommandBuffer[i]->beginRenderPass(_deviceResources->taaOffscreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, _taaClearValues, 3);
		_deviceResources->taaCommandBuffer[i]->bindPipeline(_deviceResources->taaOffscreenPipeline);
		_deviceResources->taaCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->taaFragmentDescriptorSets[i]);
		_deviceResources->taaCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 1u, _deviceResources->taaVertexDescriptorSets[i]);
		drawMesh(_deviceResources->taaCommandBuffer[i], 0);
		_deviceResources->taaCommandBuffer[i]->endRenderPass();
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->taaCommandBuffer[i]);

		transitionFromColorAttachmentToShaderRead(_deviceResources->taaCommandBuffer[i], _deviceResources->offscreenColorAttachmentImageView1SPP[i]);
		transitionFromColorAttachmentToShaderRead(_deviceResources->taaCommandBuffer[i], _deviceResources->offscreenVelocityAttachmentImageView[i]);
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->taaCommandBuffer[i], pvrvk::DebugUtilsLabel("TAA resolve pass"));

		// Do a postprocessing pass to apply TAA using the velocity and scene from the previous pass and the history texture, which has the content of the previous frame
		_deviceResources->taaCommandBuffer[i]->beginRenderPass(_deviceResources->onScreenFramebuffers[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, _clearValues, 2);
		_deviceResources->taaCommandBuffer[i]->bindPipeline(_deviceResources->taaResolvePassipeline);
		_deviceResources->taaCommandBuffer[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->taaResolvePipelineLayout, 0u, _deviceResources->taaResolveDescriptorSet[i]);
		_deviceResources->taaCommandBuffer[i]->draw(0, 3);
		recordUIRendererCommands(_deviceResources->taaCommandBuffer[i]);
		_deviceResources->taaCommandBuffer[i]->endRenderPass();

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->taaCommandBuffer[i]);

		transitionFromShaderReadToColorAttachment(_deviceResources->taaCommandBuffer[i], _deviceResources->offscreenVelocityAttachmentImageView[i]);
		transitionFromShaderReadToColorAttachment(_deviceResources->taaCommandBuffer[i], _deviceResources->offscreenColorAttachmentImageView1SPP[i]);

		// Copy the result from the swapchain to the history texture for the next frame
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->taaCommandBuffer[i], pvrvk::DebugUtilsLabel("TAA history image copy"));
		pvrvk::ImageMemoryBarrier swapchainBarrier;
		swapchainBarrier.setDstAccessMask(pvrvk::AccessFlags::e_TRANSFER_READ_BIT);
		swapchainBarrier.setOldLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
		swapchainBarrier.setNewLayout(pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL);
		swapchainBarrier.setImage(_deviceResources->swapchain->getImage(i));
		swapchainBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));

		pvrvk::ImageMemoryBarrier taaHistoryBarrier;
		taaHistoryBarrier.setDstAccessMask(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT);
		taaHistoryBarrier.setOldLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
		taaHistoryBarrier.setNewLayout(pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);
		taaHistoryBarrier.setImage(_deviceResources->taaHistoryImage[i]);
		taaHistoryBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));

		// Submit barriers to transition layout
		pvrvk::MemoryBarrierSet barrierSet;
		barrierSet.addBarrier(swapchainBarrier);
		barrierSet.addBarrier(taaHistoryBarrier);
		_deviceResources->taaCommandBuffer[i]->pipelineBarrier(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_TRANSFER_BIT, barrierSet);

		pvrvk::ImageSubresourceLayers imageSubresourceLayers = pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 0, 1);
		pvrvk::Offset3D offset = pvrvk::Offset3D(0, 0, 0);
		pvrvk::Extent3D extent = pvrvk::Extent3D(getWidth(), getHeight(), 1);
		pvrvk::ImageCopy imageCopyInformation = pvrvk::ImageCopy(imageSubresourceLayers, offset, imageSubresourceLayers, offset, extent);
		_deviceResources->taaCommandBuffer[i]->copyImage(_deviceResources->swapchain->getImage(i), _deviceResources->taaHistoryImage[i], pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
			pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, 1, &imageCopyInformation);

		barrierSet.clearAllBarriers();

		// Transition back _deviceResources::renderImages[imageIndex] from e_TRANSFER_SRC_OPTIMAL to e_GENERAL
		taaHistoryBarrier.setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
		taaHistoryBarrier.setOldLayout(pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);
		taaHistoryBarrier.setNewLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);

		// Transition back the swapchain image from e_TRANSFER_DST_OPTIMAL to e_PRESENT_SRC_KHR
		swapchainBarrier.setDstAccessMask(pvrvk::AccessFlags::e_NONE);
		swapchainBarrier.setOldLayout(pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL);
		swapchainBarrier.setNewLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);

		barrierSet.addBarrier(taaHistoryBarrier);
		barrierSet.addBarrier(swapchainBarrier);

		// Submit barriers to transition layout
		_deviceResources->taaCommandBuffer[i]->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, barrierSet);
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->taaCommandBuffer[i]);
		_deviceResources->taaCommandBuffer[i]->end();
	}
}

void VulkanAntiAliasing::createPostprocessPassDescriptorSets()
{
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// Descriptor sets for MSAAResolvePass
		_deviceResources->msaaResolvePassDescriptorSets.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->postProcessDescriptorSetLayout));
		_deviceResources->msaaResolvePassDescriptorSets.back()->setObjectName("MSAAResolvePassSwapchain" + std::to_string(i) + "DescriptorSet");

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->msaaResolvePassDescriptorSets[i], 0)
									.setImageInfo(0,
										pvrvk::DescriptorImageInfo(_deviceResources->offscreenColorAttachmentImageView4SPP[i], _deviceResources->samplerBilinear,
											pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Descriptor sets for FXAAResolvePass
		_deviceResources->fxaaResolvePassDescriptorSet.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->postProcessDescriptorSetLayout));
		_deviceResources->fxaaResolvePassDescriptorSet.back()->setObjectName("FXAAResolvePassSwapchain" + std::to_string(i) + "DescriptorSet");

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->fxaaResolvePassDescriptorSet[i], 0)
									.setImageInfo(0,
										pvrvk::DescriptorImageInfo(_deviceResources->offscreenColorAttachmentImageView1SPP[i], _deviceResources->samplerBilinear,
											pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanAntiAliasing::transitionFromShaderReadToColorAttachment(pvrvk::CommandBuffer cmdBuffer, pvrvk::ImageView inputImage)
{
	pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;
	pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL;

	pvrvk::MemoryBarrierSet fromShaderReadToColorAttachmentTransition;
	fromShaderReadToColorAttachmentTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
		inputImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout, 0, 0));

	cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, fromShaderReadToColorAttachmentTransition);
}

pvr::Result VulkanAntiAliasing::initApplication()
{
	_scene = pvr::assets::loadModel(*this, SceneElements::SceneFile);
	return pvr::Result::Success;
}

pvr::Result VulkanAntiAliasing::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

pvr::Result VulkanAntiAliasing::quitApplication()
{
	_scene.reset();
	_deviceResources.reset();
	return pvr::Result::Success;
}

pvr::Result VulkanAntiAliasing::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	// Is the screen rotated
	const bool bRotate = this->isScreenRotated();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	_viewProjMatrix = (bRotate ? pvr::math::perspectiveFov(pvr::Api::Vulkan, SceneElements::_cameraFov, static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()),
									 _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f)
							   : pvr::math::perspectiveFov(pvr::Api::Vulkan, SceneElements::_cameraFov, static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()),
									 _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar()));

	_viewProjMatrix = _viewProjMatrix * SceneElements::_cameraLookAt;

	// Calculate the model matrix
	_modelMatrix = glm::rotate(SceneElements::angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	SceneElements::angleY -= SceneElements::RotateY * 0.05f * getFrameTime();

	const pvrvk::CommandBuffer* techniqueCommandBuffer = nullptr;

	switch (_currentTechniques)
	{
	case AntiAliasingTechnique::NOAA: {
		updateSceneUniformBuffer(_swapchainIndex);
		_deviceResources->uiRenderer.getDefaultTitle()->setText(UIText::NoAntialiasing);
		_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
		techniqueCommandBuffer = &_deviceResources->noAntiAliasingCommandBuffer[_swapchainIndex];
		break;
	}

	case AntiAliasingTechnique::MSAA: {
		updateSceneUniformBuffer(_swapchainIndex);
		_deviceResources->msaaUIRenderer.getDefaultTitle()->setText(UIText::MsAntialiasing);
		_deviceResources->msaaUIRenderer.getDefaultTitle()->commitUpdates();
		techniqueCommandBuffer = &_deviceResources->msaaCommandBuffer[_swapchainIndex];
		break;
	}

	case AntiAliasingTechnique::FXAA: {
		updateSceneUniformBuffer(_swapchainIndex);
		_deviceResources->uiRenderer.getDefaultTitle()->setText(UIText::FxAntiAliasing);
		_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
		techniqueCommandBuffer = &_deviceResources->fxaaCommandBuffer[_swapchainIndex];
		break;
	}

	case AntiAliasingTechnique::TXAA: {
		updateSceneUniformBufferTAA(_swapchainIndex);
		_deviceResources->uiRenderer.getDefaultTitle()->setText(UIText::TxAntiAliasing);
		_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
		techniqueCommandBuffer = &_deviceResources->taaCommandBuffer[_swapchainIndex];
		break;
	}

	default: {
		Log(LogLevel::Error, "Wrong anti aliasing technique");
		break;
	}
	}

	_preModelMatrix = _modelMatrix;
	_preWorldMatrix = _worldMatrix;
	_preProjectionViewMatrix = _viewProjMatrix;

	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags submitWaitFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitDstStageMask = &submitWaitFlags;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.numCommandBuffers = 1;
	submitInfo.commandBuffers = techniqueCommandBuffer;

	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[_swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, _swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_swapchainIndex];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;

	// As above we must present using the same VkQueue as submitted to previously
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _swapchainLength;

	return pvr::Result::Success;
}

void VulkanAntiAliasing::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::ActionClose: {
		exitShell();
		break;
	}
	case pvr::SimplifiedInput::Action1: {
		changeCurrentTechnique();
		break;
	}
	default: {
		_currentTechniques = AntiAliasingTechnique::NOAA;
		break;
	}
	}
}

void VulkanAntiAliasing::changeCurrentTechnique()
{
	_inputIndex++;
	_inputIndex = _inputIndex % 4;
	_currentTechniques = static_cast<AntiAliasingTechnique>(_inputIndex);
}

float VulkanAntiAliasing::createHaltonSequence(unsigned int index, int base)
{
	float f = 1;
	float r = 0;

	int current = index;
	do {
		f = f / base;
		r = r + f * (current % base);
		current = static_cast<int>(glm::floor(static_cast<float>(current) / static_cast<float>(base)));
	} while (current > 0);

	return r;
}

void VulkanAntiAliasing::calculateJitterParameter(int screenWidth, int screenHeight)
{
	for (int i = 0; i < 16; i++)
	{
		float x = createHaltonSequence(i + 1, 2);
		float y = createHaltonSequence(i + 1, 3);

		_jitter2DArray[i][0] = x;
		_jitter2DArray[i][1] = y;

		_jitter2DArray[i][0] = ((x - 0.5f) / (float)screenWidth) * 2;
		_jitter2DArray[i][1] = ((y - 0.5f) / (float)screenHeight) * 2;
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its Shell object defining the behaviour of the application.</summary>
/// <returns>Return an unique_ptr to a new Demo class,supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanAntiAliasing>(); }
