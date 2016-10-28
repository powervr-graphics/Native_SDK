/*!*********************************************************************************************************************
\File         OGLESBumpMap.cpp
\Title        Bump mapping
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to perform tangent space bump mapping
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"
using namespace pvr::types;
const pvr::float32 RotateY = glm::pi<pvr::float32>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

/*!*********************************************************************************************************************
 shader attributes
 ***********************************************************************************************************************/
const pvr::utils::VertexBindings_Name VertexAttribBindings[] =
{
	{ "POSITION", "inVertex" },
	{ "NORMAL", "inNormal" },
	{ "UV0",	"inTexCoord" },
	{ "TANGENT",	"inTangent" },
};

// shader uniforms
namespace Uniform {
enum Enum {	MVPMatrix, LightDir, NumUniforms };
}

const char* UniformNames[] = {  "MVPMatrix", "LightDirModel" };

/*!*********************************************************************************************************************
 Content file names
 ***********************************************************************************************************************/

// Source and binary shaders
const char FragShaderSrcFile[]		= "FragShader.fsh";
const char VertShaderSrcFile[]		= "VertShader.vsh";

// PVR texture files
const char StatueTexFile[]			= "Marble.pvr";
const char StatueNormalMapFile[]	= "MarbleNormalMap.pvr";

const char ShadowTexFile[]			= "Shadow.pvr";
const char ShadowNormalMapFile[]	= "ShadowNormalMap.pvr";

// POD scene files
const char SceneFile[]				= "scene.pod";

/*!*********************************************************************************************************************
 Class implementing the pvr::Shell functions.
 ***********************************************************************************************************************/
class OGLESBumpMap : public pvr::Shell
{
	// Print3D class used to display text
	pvr::ui::UIRenderer	uiRenderer;

	// 3D Model
	pvr::assets::ModelHandle scene;

	// Projection and view matrix
	glm::mat4 viewProj;

	struct DrawPass
	{
		glm::mat4 mvp;
		glm::vec3 lightDir;
	};

	struct DeviceResources
	{
		std::vector<pvr::api::Buffer> vbo;
		std::vector<pvr::api::Buffer> ibo;
		pvr::api::DescriptorSetLayout descSetLayout;
		pvr::api::DescriptorSet imageSamplerDescSet;
		pvr::api::GraphicsPipeline pipe;
		pvr::api::CommandBuffer commandBuffer;
		pvr::api::Fbo fboOnScreen;
	};

	pvr::uint32 pipeUniformLoc[Uniform::NumUniforms];
	pvr::GraphicsContext context;
	pvr::api::AssetStore assetManager;
	// The translation and Rotate parameter of Model
	pvr::float32 angleY;
	DrawPass drawPass;
	std::auto_ptr<DeviceResources> deviceResource;
public:
	OGLESBumpMap() {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool createImageSamplerDescriptor();
	bool loadPipeline();
	void loadVbos();
	void drawMesh(int i32NodeIndex);
	void recordCommandBuffer();
};

/*!*********************************************************************************************************************
\return	return true if no error occurred
\brief	Loads the textures required for this training course
***********************************************************************************************************************/
bool OGLESBumpMap::createImageSamplerDescriptor()
{
	pvr::api::TextureView texBase;
	pvr::api::TextureView texNormalMap;

	// create the bilinear sampler
	pvr::assets::SamplerCreateParam samplerInfo;
	samplerInfo.magnificationFilter = SamplerFilter::Linear;
	samplerInfo.minificationFilter = SamplerFilter::Linear;
	samplerInfo.mipMappingFilter = SamplerFilter::Nearest;
	pvr::api::Sampler samplerMipBilinear = context->createSampler(samplerInfo);

	samplerInfo.mipMappingFilter = SamplerFilter::Linear;
	pvr::api::Sampler samplerTrilinear = context->createSampler(samplerInfo);

	if (!assetManager.getTextureWithCaching(getGraphicsContext(), StatueTexFile,	&texBase, NULL) ||
	    !assetManager.getTextureWithCaching(getGraphicsContext(), StatueNormalMapFile, &texNormalMap, NULL))
	{
		setExitMessage("ERROR: Failed to load texture.");
		return false;
	}
	// create the descriptor set
	pvr::api::DescriptorSetUpdate descSetCreateInfo;
	descSetCreateInfo.setCombinedImageSampler(0, texBase, samplerMipBilinear)
	.setCombinedImageSampler(1, texNormalMap, samplerTrilinear);
	deviceResource->imageSamplerDescSet = context->createDescriptorSetOnDefaultPool(deviceResource->descSetLayout);
	if (!deviceResource->imageSamplerDescSet.isValid())
	{
		setExitMessage("ERROR: Failed to create Combined Image Sampler Descriptor set.");
		return false;
	}
	deviceResource->imageSamplerDescSet->update(descSetCreateInfo);
	return true;
}

/*!*********************************************************************************************************************
\return	 Return true if no error occurred
\brief	Loads and compiles the shaders and create a pipeline
***********************************************************************************************************************/
bool OGLESBumpMap::loadPipeline()
{
	pvr::types::BlendingConfig colorAttachemtState;
	pvr::api::GraphicsPipelineCreateParam pipeInfo;
	colorAttachemtState.blendEnable = false;

	//--- create the descriptor set layout
	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
	descSetLayoutInfo.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment).
	setBinding(1, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	deviceResource->descSetLayout = context->createDescriptorSetLayout(descSetLayoutInfo);

	//--- create the pipeline layout
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(deviceResource->descSetLayout);

	pipeInfo.colorBlend.setAttachmentState(0, colorAttachemtState);

	pvr::assets::ShaderFile fileVersioning;
	fileVersioning.populateValidVersions(VertShaderSrcFile, *this);
	pipeInfo.vertexShader = context->createShader(*fileVersioning.getBestStreamForApi(context->getApiType()),
	                        ShaderType::VertexShader);

	fileVersioning.populateValidVersions(FragShaderSrcFile, *this);
	pipeInfo.fragmentShader = context->createShader(*fileVersioning.getBestStreamForApi(context->getApiType()),
	                          ShaderType::FragmentShader);

	const pvr::assets::Mesh& mesh = scene->getMesh(0);
	pipeInfo.inputAssembler.setPrimitiveTopology(mesh.getPrimitiveType());
	pipeInfo.pipelineLayout = context->createPipelineLayout(pipeLayoutInfo);
	pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.setDepthTestEnable(true).setDepthCompareFunc(ComparisonMode::Less).setDepthWrite(true);
	pvr::utils::createInputAssemblyFromMesh(mesh, VertexAttribBindings,
	                                        sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipeInfo);

	deviceResource->pipe = context->createGraphicsPipeline(pipeInfo);
	// Store the location of uniforms for later use
	for (int i = 0; i < Uniform::NumUniforms; ++i) {	pipeUniformLoc[i] = deviceResource->pipe->getUniformLocation(UniformNames[i]); }

	deviceResource->commandBuffer->beginRecording();
	deviceResource->commandBuffer->bindPipeline(deviceResource->pipe);
	deviceResource->commandBuffer->setUniform<pvr::int32>(deviceResource->pipe->getUniformLocation("sBaseTex"), 0);
	deviceResource->commandBuffer->setUniform<pvr::int32>(deviceResource->pipe->getUniformLocation("sNormalMap"), 1);
	deviceResource->commandBuffer->endRecording();
	deviceResource->commandBuffer->submit();
	return true;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it	(e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESBumpMap::initApplication()
{
	if (isApiSupported(pvr::Api::OpenGLES3))
	{
		pvr::Log(pvr::Log.Information, "OpenGL ES 3.0 support detected. Application will run in OpenGL ES 3.0 mode");
	}
	else
	{
		pvr::Log(pvr::Log.Information, "No support for OpenGL ES 3.0 found. Application will run in OpenGL ES 2.0 mode");
	}

	// Load the scene
	assetManager.init(*this);
	if (!assetManager.loadModel(SceneFile, scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}
	angleY = 0.0f;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
		If the rendering context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result OGLESBumpMap::quitApplication() {	return pvr::Result::Success;}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESBumpMap::initView()
{
	context = getGraphicsContext();
	deviceResource.reset(new DeviceResources());
	deviceResource->commandBuffer = context->createCommandBufferOnDefaultPool();
	// load the vbo and ibo data
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, deviceResource->vbo, deviceResource->ibo);

	// load the pipeline
	if (!loadPipeline()) {	return pvr::Result::UnknownError;	}
	if (!createImageSamplerDescriptor()) { return pvr::Result::UnknownError; }

	// create OnScreen FBO
	deviceResource->fboOnScreen = context->createOnScreenFbo(0);

	//	Initialize UIRenderer
	if (uiRenderer.init(deviceResource->fboOnScreen->getRenderPass(), 0) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::UnknownError;
	}

	uiRenderer.getDefaultTitle()->setText("BumpMap");
	uiRenderer.getDefaultTitle()->commitUpdates();
	glm::vec3 from, to, up;
	pvr::float32 fov;
	scene->getCameraProperties(0, fov, from, to, up);

	// Is the screen rotated
	bool bRotate = this->isScreenRotated() && this->isFullScreen();

	//	Calculate the projection and rotate it by 90 degree if the screen is rotated.
	viewProj = (bRotate ?
	            pvr::math::perspectiveFov(getApiType(), fov, (float)this->getHeight(), (float)this->getWidth(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f) :
	            glm::perspectiveFov<pvr::float32>(fov, (float)this->getWidth(),	(float)this->getHeight(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar()));

	viewProj = viewProj * glm::lookAt(from, to, up);
	recordCommandBuffer();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Code in releaseView() will be called by PVRShell when theapplication quits or before a change in the rendering context.
\return	Return Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result OGLESBumpMap::releaseView()
{
	deviceResource.reset();
	uiRenderer.release();
	scene.reset();
	assetManager.releaseAll();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESBumpMap::renderFrame()
{
	// Calculate the model matrix
	glm::mat4 mModel = glm::rotate(angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	angleY += -RotateY * 0.05f  * getFrameTime();

	// Set light Direction in model space
	//	The inverse of a rotation matrix is the transposed matrix
	//	Because of v * M = transpose(M) * v, this means:
	//	v * R == inverse(R) * v
	//	So we don't have to actually invert or transpose the matrix
	//	to transform back from world space to model space
	drawPass.lightDir = glm::vec3(LightDir * mModel);
	drawPass.mvp = viewProj * mModel * scene->getWorldMatrix(scene->getNode(0).getObjectId());
	deviceResource->commandBuffer->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Draws a pvr::assets::Mesh after the model view matrix has been set and	the material prepared.
\param	nodeIndex	Node index of the mesh to draw
***********************************************************************************************************************/
void OGLESBumpMap::drawMesh(int nodeIndex)
{
	pvr::uint32 meshId = scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = scene->getMesh(meshId);

	// bind the VBO for the mesh
	deviceResource->commandBuffer->bindVertexBuffer(deviceResource->vbo[meshId], 0, 0);
	deviceResource->commandBuffer->bindIndexBuffer(deviceResource->ibo[meshId], 0, mesh.getFaces().getDataType());

	//	The geometry can be exported in 4 ways:
	//	- Indexed Triangle list
	//	- Non-Indexed Triangle list
	//	- Indexed Triangle strips
	//	- Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		// Indexed Triangle list
		if (deviceResource->ibo[meshId].isValid())
		{
			deviceResource->commandBuffer->bindIndexBuffer(deviceResource->ibo[meshId], 0, mesh.getFaces().getDataType());
			deviceResource->commandBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			deviceResource->commandBuffer->drawArrays(0, mesh.getNumFaces() * 3, 0, 1);
		}
	}
	else
	{
		for (int i = 0; i < (int)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (deviceResource->ibo[meshId].isValid())
			{
				// Indexed Triangle strips
				deviceResource->commandBuffer->bindIndexBuffer(deviceResource->ibo[meshId], 0,
				    mesh.getFaces().getDataType());
				deviceResource->commandBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				deviceResource->commandBuffer->drawArrays(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

/*!*********************************************************************************************************************
\brief	Pre record the commands
***********************************************************************************************************************/
void OGLESBumpMap::recordCommandBuffer()
{
	deviceResource->commandBuffer->beginRecording();
	deviceResource->commandBuffer->beginRenderPass(deviceResource->fboOnScreen, pvr::Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(0.00, 0.70, 0.67, 1.f));

	// enqueue the static states which wont be changed through out the frame
	deviceResource->commandBuffer->bindPipeline(deviceResource->pipe);
	deviceResource->commandBuffer->setUniformPtr<glm::vec3>(pipeUniformLoc[Uniform::LightDir], 1, &drawPass.lightDir);

	deviceResource->commandBuffer->bindDescriptorSet(deviceResource->pipe->getPipelineLayout(), 0, deviceResource->imageSamplerDescSet, 0);
	deviceResource->commandBuffer->setUniformPtr<glm::mat4>(pipeUniformLoc[Uniform::MVPMatrix], 1, &drawPass.mvp);
	drawMesh(0);

	pvr::api::SecondaryCommandBuffer uiCmdBuffer = context->createSecondaryCommandBufferOnDefaultPool();
	uiRenderer.beginRendering(uiCmdBuffer);
	uiRenderer.getDefaultTitle()->render();
	uiRenderer.getSdkLogo()->render();
	uiRenderer.endRendering();
	deviceResource->commandBuffer->enqueueSecondaryCmds(uiCmdBuffer);
	deviceResource->commandBuffer->endRenderPass();
	deviceResource->commandBuffer->endRecording();
}

/*!*********************************************************************************************************************
\return	Return an auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell.	The user should return its
		Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new OGLESBumpMap()); }
