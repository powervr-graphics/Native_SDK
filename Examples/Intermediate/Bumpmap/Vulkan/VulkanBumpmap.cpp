/*!*********************************************************************************************************************
\File         VulkanBumpMap.cpp
\Title        Bump mapping
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to perform tangent space bump mapping
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"

const pvr::float32 RotateY = glm::pi<pvr::float32>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

/*!*********************************************************************************************************************
 shader attributes
 ***********************************************************************************************************************/
// vertex attributes
namespace VertexAttrib {
enum Enum
{
	VertexArray, NormalArray, TexCoordArray, TangentArray, numAttribs
};
}

const pvr::utils::VertexBindings VertexAttribBindings[] =
{
	{ "POSITION", 0 },
	{ "NORMAL",   1 },
	{ "UV0",    2 },
	{ "TANGENT",  3 },
};

// shader uniforms
namespace Uniform {
enum Enum { MVPMatrix, LightDir, NumUniforms };
}

/*!*********************************************************************************************************************
 Content file names
 ***********************************************************************************************************************/

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";

// PVR texture files
const char StatueTexFile[] = "Marble.pvr";
const char StatueNormalMapFile[] = "MarbleNormalMap.pvr";

const char ShadowTexFile[] = "Shadow.pvr";
const char ShadowNormalMapFile[] = "ShadowNormalMap.pvr";

// POD _scene files
const char SceneFile[] = "scene.pod";

/*!*********************************************************************************************************************
 Class implementing the Shell functions.
 ***********************************************************************************************************************/
class VulkanBumpMap : public pvr::Shell
{
	struct UboPerMeshData
	{
		glm::mat4 mvpMtx;
		glm::vec3 lightDirModel;
	};

	// Print3D class used to display text
	pvr::ui::UIRenderer _uiRenderer;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and view matrix
	glm::mat4 _viewProj;

	struct DeviceResources
	{
		std::vector<pvr::api::Buffer> vbo;
		std::vector<pvr::api::Buffer> ibo;
		pvr::api::DescriptorSetLayout texLayout;
		pvr::api::DescriptorSetLayout uboLayoutDynamic;
		pvr::api::PipelineLayout pipelayout;
		pvr::api::DescriptorSet texDescSet;

		pvr::api::GraphicsPipeline pipe;
		pvr::Multi<pvr::api::CommandBuffer> commandBuffer;// per swapchain
		pvr::api::FboSet fboOnScreen;// per swapchain
		pvr::utils::StructuredMemoryView ubo;//per swapchain
		pvr::Multi<pvr::api::DescriptorSet> uboDescSet;
	};

	pvr::GraphicsContext _context;
	pvr::utils::AssetStore _assetManager;
	// The translation and Rotate parameter of Model
	pvr::float32 _angleY;
	std::auto_ptr<DeviceResources> _deviceResource;

public:
	VulkanBumpMap() {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool createImageSamplerDescriptor();
	bool createUbo();
	bool loadPipeline();
	void drawMesh(pvr::api::CommandBuffer& cmdBuffer, int i32NodeIndex);
	void recordCommandBuffer();
};

/*!*********************************************************************************************************************
\return return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************************************/
bool VulkanBumpMap::createImageSamplerDescriptor()
{
	pvr::api::TextureView texBase;
	pvr::api::TextureView texNormalMap;

	// create the bilinear sampler
	pvr::assets::SamplerCreateParam samplerInfo;
	samplerInfo.magnificationFilter = pvr::types::SamplerFilter::Linear;
	samplerInfo.minificationFilter = pvr::types::SamplerFilter::Linear;
	samplerInfo.mipMappingFilter = pvr::types::SamplerFilter::Nearest;
	pvr::api::Sampler samplerMipBilinear = _context->createSampler(samplerInfo);

	samplerInfo.mipMappingFilter = pvr::types::SamplerFilter::Linear;
	pvr::api::Sampler samplerTrilinear = _context->createSampler(samplerInfo);

	if (!_assetManager.getTextureWithCaching(getGraphicsContext(), StatueTexFile,  &texBase, NULL) ||
	    !_assetManager.getTextureWithCaching(getGraphicsContext(), StatueNormalMapFile, &texNormalMap, NULL))
	{
		setExitMessage("ERROR: Failed to load texture.");
		return false;
	}
	// create the descriptor set
	pvr::api::DescriptorSetUpdate descSetCreateInfo;
	descSetCreateInfo.setCombinedImageSampler(0, texBase, samplerMipBilinear);
	descSetCreateInfo.setCombinedImageSampler(1, texNormalMap, samplerTrilinear);

	// create the descriptor set
	_deviceResource->texDescSet = _context->createDescriptorSetOnDefaultPool(_deviceResource->texLayout);

	if (!_deviceResource->texDescSet.isValid())
	{
		setExitMessage("ERROR: Failed to create Combined Image Sampler Descriptor set.");
		return false;
	}

	_deviceResource->texDescSet->update(descSetCreateInfo);

	return true;
}

bool VulkanBumpMap::createUbo()
{
	pvr::api::DescriptorSetUpdate descUpdate;
	_deviceResource->ubo.addEntryPacked("MVPMatrix", pvr::types::GpuDatatypes::mat4x4);
	_deviceResource->ubo.addEntryPacked("LightDirModel", pvr::types::GpuDatatypes::vec3);
	_deviceResource->ubo.finalize(_context, 1, pvr::types::BufferBindingUse::UniformBuffer, true, false);
	_deviceResource->ubo.createConnectedBuffers(getPlatformContext().getSwapChainLength(), _context);

	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		_deviceResource->uboDescSet.add(_context->createDescriptorSetOnDefaultPool(_deviceResource->uboLayoutDynamic));
		descUpdate.setDynamicUbo(0, _deviceResource->ubo.getConnectedBuffer(i));
		_deviceResource->uboDescSet[i]->update(descUpdate);
	}

	return true;
}

/*!*********************************************************************************************************************
\return  Return true if no error occurred
\brief  Loads and compiles the shaders and create a pipeline
***********************************************************************************************************************/
bool VulkanBumpMap::loadPipeline()
{
	pvr::types::BlendingConfig colorAttachemtState;
	pvr::api::GraphicsPipelineCreateParam pipeInfo;
	colorAttachemtState.blendEnable = false;

	//--- create the texture-sampler descriptor set layout
	{
		pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);/*binding 0*/
		descSetLayoutInfo.setBinding(1, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);/*binding 1*/
		_deviceResource->texLayout = _context->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the ubo descriptorset layout
	{
		pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1, pvr::types::ShaderStageFlags::Vertex); /*binding 0*/
		_deviceResource->uboLayoutDynamic = _context->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the pipeline layout
	{
		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo
		.addDescSetLayout(_deviceResource->texLayout)/*set 0*/
		.addDescSetLayout(_deviceResource->uboLayoutDynamic);/*set 1*/
		_deviceResource->pipelayout = _context->createPipelineLayout(pipeLayoutInfo);
	}

	pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);

	pipeInfo.colorBlend.setAttachmentState(0, colorAttachemtState);

	pvr::assets::ShaderFile fileVersioner;
	fileVersioner.populateValidVersions(VertShaderSrcFile, *this);
	pipeInfo.vertexShader = _context->createShader(*fileVersioner.getBestStreamForContext(_context), pvr::types::ShaderType::VertexShader);

	fileVersioner.populateValidVersions(FragShaderSrcFile, *this);
	pipeInfo.fragmentShader = _context->createShader(*fileVersioner.getBestStreamForContext(_context), pvr::types::ShaderType::FragmentShader);

	const pvr::assets::Mesh& mesh = _scene->getMesh(0);
	pipeInfo.inputAssembler.setPrimitiveTopology(mesh.getPrimitiveType());
	pipeInfo.pipelineLayout = _deviceResource->pipelayout;
	pipeInfo.renderPass = _deviceResource->fboOnScreen[0]->getRenderPass();
	pipeInfo.subPass = 0;
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.setDepthTestEnable(true);
	pipeInfo.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::Less);
	pipeInfo.depthStencil.setDepthWrite(true);
	pvr::utils::createInputAssemblyFromMesh(mesh, VertexAttribBindings, sizeof(VertexAttribBindings) /
	                                        sizeof(VertexAttribBindings[0]), pipeInfo);
	_deviceResource->pipe = _context->createGraphicsPipeline(pipeInfo);
	return (_deviceResource->pipe.isValid());
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering _context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
    If the rendering _context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanBumpMap::initApplication()
{
	// Load the _scene
	_assetManager.init(*this);
	if (!_assetManager.loadModel(SceneFile, _scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}
	_angleY = 0.0f;


	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
    If the rendering _context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result VulkanBumpMap::quitApplication() { return pvr::Result::Success;}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering _context.
    Used to initialize variables that are dependent on the rendering _context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanBumpMap::initView()
{
	_context = getGraphicsContext();
	_deviceResource.reset(new DeviceResources());
	// load the vbo and ibo data
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *_scene, _deviceResource->vbo, _deviceResource->ibo);

	// create an onscreen fbo per swap chain
	_deviceResource->fboOnScreen = _context->createOnScreenFboSet();

	//  Initialize UIRenderer
	if (_uiRenderer.init(_deviceResource->fboOnScreen[0]->getRenderPass(), 0) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::UnknownError;
	}
	_uiRenderer.getDefaultTitle()->setText("BumpMap");
	_uiRenderer.getDefaultTitle()->commitUpdates();

	// load the pipeline
	if (!loadPipeline()) {  return pvr::Result::UnknownError;  }

	// create the image samplers
	if (!createImageSamplerDescriptor()) { return pvr::Result::UnknownError; }

	// create the uniform buffers
	if (!createUbo()) { return pvr::Result::UnknownError; }

	glm::vec3 from, to, up;
	pvr::float32 fov;
	_scene->getCameraProperties(0, fov, from, to, up);

	// Is the screen rotated
	bool bRotate = this->isScreenRotated() && this->isFullScreen();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	_viewProj = (bRotate ?
	             pvr::math::perspectiveFov(getApiType(), fov, (float)this->getHeight(), (float)this->getWidth(),
	                                       _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f) :
	             pvr::math::perspectiveFov(getApiType(), fov, (float)this->getWidth(), (float)this->getHeight(),
	                                       _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar()));

	_viewProj = _viewProj * glm::lookAt(from, to, up);

	// record the command buffers
	recordCommandBuffer();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Code in releaseView() will be called by PVRShell when theapplication quits or before a change in the rendering _context.
\return Return Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result VulkanBumpMap::releaseView()
{
	_deviceResource.reset();
	_uiRenderer.release();
	_scene.reset();
	_assetManager.releaseAll();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanBumpMap::renderFrame()
{
	// Calculate the model matrix
	glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	_angleY += -RotateY * 0.05f  * getFrameTime();

	// Set light Direction in model space
	//  The inverse of a rotation matrix is the transposed matrix
	//  Because of v * M = transpose(M) * v, this means:
	//  v * R == inverse(R) * v
	//  So we don't have to actually invert or transpose the matrix
	//  to transform back from world space to model space

	// update the ubo
	{
		UboPerMeshData srcWrite;
		srcWrite.lightDirModel = glm::vec3(LightDir * mModel);
		srcWrite.mvpMtx = _viewProj * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
		_deviceResource->ubo.map(getSwapChainIndex());
		_deviceResource->ubo.setValue("MVPMatrix", srcWrite.mvpMtx);
		_deviceResource->ubo.setValue("LightDirModel", srcWrite.lightDirModel);
		_deviceResource->ubo.unmap(getSwapChainIndex());
	}
	_deviceResource->commandBuffer[getPlatformContext().getSwapChainIndex()]->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Draws a assets::Mesh after the model view matrix has been set and the material prepared.
\param  nodeIndex Node index of the mesh to draw
***********************************************************************************************************************/
void VulkanBumpMap::drawMesh(pvr::api::CommandBuffer& cmdBuffer, int nodeIndex)
{
	pvr::uint32 meshId = _scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _scene->getMesh(meshId);

	// bind the VBO for the mesh
	cmdBuffer->bindVertexBuffer(_deviceResource->vbo[meshId], 0, 0);

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		// Indexed Triangle list
		if (_deviceResource->ibo[meshId].isValid())
		{
			cmdBuffer->bindIndexBuffer(_deviceResource->ibo[meshId], 0, mesh.getFaces().getDataType());
			cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			cmdBuffer->drawArrays(0, mesh.getNumFaces() * 3, 0, 1);
		}
	}
	else
	{
		for (int i = 0; i < (int)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (_deviceResource->ibo[meshId].isValid())
			{
				// Indexed Triangle strips
				cmdBuffer->bindIndexBuffer(_deviceResource->ibo[meshId], 0,
				                           mesh.getFaces().getDataType());
				cmdBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				cmdBuffer->drawArrays(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

/*!*********************************************************************************************************************
\brief  Pre record the commands
***********************************************************************************************************************/
void VulkanBumpMap::recordCommandBuffer()
{
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		// create the swap chain command buffer
		_deviceResource->commandBuffer.add(_context->createCommandBufferOnDefaultPool());

		// begin recording commands for the current swap chain command buffer
		_deviceResource->commandBuffer[i]->beginRecording();

		// begin the render pass
		_deviceResource->commandBuffer[i]->beginRenderPass(_deviceResource->fboOnScreen[i], pvr::Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(0.00, 0.70, 0.67, 1.f));

		// calculate the dynamic offset to use
		pvr::uint32 dynamicOffset = _deviceResource->ubo.getAlignedElementArrayOffset(0);
		// enqueue the static states which wont be changed through out the frame
		_deviceResource->commandBuffer[i]->bindPipeline(_deviceResource->pipe);
		_deviceResource->commandBuffer[i]->bindDescriptorSet(_deviceResource->pipelayout, 0, _deviceResource->texDescSet, 0);
		_deviceResource->commandBuffer[i]->bindDescriptorSet(_deviceResource->pipelayout, 1, _deviceResource->uboDescSet[i], &dynamicOffset, 1);
		drawMesh(_deviceResource->commandBuffer[i], 0);

		// record the ui renderer commands
		_uiRenderer.beginRendering(_deviceResource->commandBuffer[i]);
		_uiRenderer.getDefaultTitle()->render();
		_uiRenderer.getSdkLogo()->render();
		_uiRenderer.endRendering();

		// end the renderpass
		_deviceResource->commandBuffer[i]->endRenderPass();

		// end recording commands for the current command buffer
		_deviceResource->commandBuffer[i]->endRecording();
	}
}

/*!*********************************************************************************************************************
\return Return an auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its
    Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanBumpMap()); }
