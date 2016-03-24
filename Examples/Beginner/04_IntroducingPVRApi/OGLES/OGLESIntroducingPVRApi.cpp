/*!*********************************************************************************************************************
\File         OGLESIntroducingPVRApi.cpp
\Title        Introducing the PowerVR Framework
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to use the PVRApi library together with loading models from POD files and rendering them with effects from PFX files.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"
using namespace pvr::types;
namespace Semantics {
enum Enum
{
	WorldViewProjection,
	WorldViewIT,
	LightDirEye,
	Texture0,
	Texture1,
	Count
};
}

/*!*********************************************************************************************************************
 Content file names
***********************************************************************************************************************/
const char PfxFileName[]	= "effect.pfx"; // Effect file

const char SceneFileName[]  = "GnomeToy.pod"; // POD scene files

/*!*********************************************************************************************************************
 Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OGLESIntroducingPVRApi : public pvr::Shell
{
	// 3D Model
	pvr::assets::ModelHandle scene;

	// Projection and Model View matrices
	glm::mat4 projMtx, viewMtx;

	// Variables to handle the animation in a time-based manner
	float frame;

	typedef std::pair<pvr::int32, pvr::api::DescriptorSet> MaterialDescSet;
	struct DeviceResources
	{
		pvr::GraphicsContext context;
		// The effect file handlers
		pvr::api::EffectApi effect;

		// The Vertex buffer object handle array.
		std::vector<pvr::api::Buffer> vbos;
		std::vector<pvr::api::Buffer> ibos;
		pvr::api::Fbo fboOnScreen;
		pvr::api::CommandBuffer commandBuffer;
		std::vector<MaterialDescSet> descSet;
		pvr::api::Sampler samplerTrilinear;
		pvr::api::DescriptorSetLayout descSetLayout;
		pvr::api::PipelineLayout pipelineLayout;
	};

	std::auto_ptr<DeviceResources> deviceResource;

	pvr::int32 uniformSemanticsTable[Semantics::Count];
	pvr::ui::UIRenderer uiRenderer;
	pvr::api::AssetStore assetManager;
	struct DrawPass
	{
		std::vector<glm::mat4> worldViewProj;
		std::vector<glm::mat4> worldViewIT;
		std::vector<glm::vec3> dirLight;
		glm::mat4 scale;
	};
	DrawPass drawPass;
public:
	virtual pvr::Result::Enum initApplication();
	virtual pvr::Result::Enum initView();
	virtual pvr::Result::Enum releaseView();
	virtual pvr::Result::Enum quitApplication();
	virtual pvr::Result::Enum renderFrame();

	bool createDescriptorSet();
	void recordCommandBuffer(pvr::assets::Effect& effectAsset);
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
pvr::Result::Enum OGLESIntroducingPVRApi::initApplication()
{
	// Load the scene
	assetManager.init(*this);
	pvr::Result::Enum rslt = pvr::Result::Success;
	if ((scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(SceneFileName)))).isNull())
	{
		this->setExitMessage("ERROR: Couldn't load the %s file\n", SceneFileName);
		return rslt;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (scene->getNumCameras() == 0)
	{
		this->setExitMessage("ERROR: The scene does not contain a camera\n");
		return pvr::Result::InvalidData;
	}

	// Ensure that all meshes use an indexed triangle list
	for (unsigned int i = 0; i < scene->getNumMeshes(); ++i)
	{
		if (scene->getMesh(i).getPrimitiveType() != PrimitiveTopology::TriangleList
		        || scene->getMesh(i).getFaces().getDataSize() == 0)
		{
			this->setExitMessage("ERROR: The meshes in the scene should use an indexed triangle list\n");
			return pvr::Result::InvalidData;
		}
	}
	// Initialize variables used for the animation
	frame = 0;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
        If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result::Enum OGLESIntroducingPVRApi::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change  in the rendering context.
        Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result::Enum OGLESIntroducingPVRApi::initView()
{
	deviceResource.reset(new DeviceResources());
	deviceResource->context = getGraphicsContext();
	deviceResource->commandBuffer = deviceResource->context->createCommandBufferOnDefaultPool();
	deviceResource->fboOnScreen = deviceResource->context->createOnScreenFbo(0);

	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, deviceResource->vbos, deviceResource->ibos);

	if (uiRenderer.init(deviceResource->context, deviceResource->fboOnScreen->getRenderPass(), 0) != pvr::Result::Success) { return pvr::Result::UnknownError; }

	uiRenderer.getDefaultTitle()->setText("IntroducingPVRApi");
	uiRenderer.getDefaultTitle()->commitUpdates();

	// We check the scene contains at least one light
	if (scene->getNumLights() == 0)
	{
		pvr::Log("The scene does not contain a light\n");
		return pvr::Result::InvalidData;
	}

	pvr::api::ImageStorageFormat targetColorFormat;
	getDisplayFormat(getDisplayAttributes(), &targetColorFormat, 0);
	pvr::assets::Effect effectAsset("Effect");

	pvr::api::GraphicsPipelineCreateParam pipeDesc;
	pvr::api::pipelineCreation::ColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.blendEnable = false;

	pipeDesc.colorBlend.addAttachmentState(colorBlendAttachment);
	pipeDesc.rasterizer.setCullFace(Face::Back).setFrontFaceWinding(PolygonWindingOrder::FrontFaceCCW);
	pipeDesc.depthStencil.setDepthTestEnable(true);

	// open the pfx
	pvr::assets::PfxReader effectParser;
	pvr::assets::ShaderFile fileVersioning;
	fileVersioning.populateValidVersions(PfxFileName, *this);
	if (!effectParser.openAssetStream(fileVersioning.getBestStreamForApi(deviceResource->context->getApiType())))
	{
		this->setExitMessage("Failed to load Pfx file. %s", PfxFileName);
		return pvr::Result::UnableToOpen;
	}

	if (!effectParser.readAsset(effectAsset))
	{
		this->setExitMessage("Failed to read Pfx file");
		return pvr::Result::UnableToOpen;
	}
	pvr::string errorStr;

	uniformSemanticsTable[Semantics::WorldViewProjection] = effectAsset.getUniformSemanticId("WORLDVIEWPROJECTION");
	uniformSemanticsTable[Semantics::WorldViewIT] = effectAsset.getUniformSemanticId("WORLDVIEWIT");
	uniformSemanticsTable[Semantics::LightDirEye] = effectAsset.getUniformSemanticId("LIGHTDIREYE");
	uniformSemanticsTable[Semantics::Texture0] = effectAsset.getUniformSemanticId("TEXTURE0");


	const pvr::assets::Mesh& mesh = scene->getMesh(0);
	pvr::utils::createInputAssemblyFromMeshAndEffect(mesh, effectAsset, pipeDesc);

	// create the descriptor set layout & and pipeline layout
	pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
	descSetInfo.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	deviceResource->descSetLayout = deviceResource->context->createDescriptorSetLayout(descSetInfo);
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(deviceResource->descSetLayout);
	deviceResource->pipelineLayout = deviceResource->context->createPipelineLayout(pipeLayoutInfo);

	pipeDesc.pipelineLayout = deviceResource->pipelineLayout;
	deviceResource->effect = deviceResource->context->createEffectApi(effectAsset, pipeDesc, assetManager);
	if (!deviceResource->effect.isValid()) { return pvr::Result::UnknownError; }

	createDescriptorSet();
	recordCommandBuffer(effectAsset);
	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		projMtx = pvr::math::perspective(getApiType(), scene->getCamera(0).getFOV(),
		                                 (float)this->getHeight() / (float)this->getWidth(), scene->getCamera(0).getNear(),
		                                 scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		projMtx = glm::perspective(scene->getCamera(0).getFOV(),
		                           (float)this->getWidth() / (float)this->getHeight(), scene->getCamera(0).getNear(),
		                           scene->getCamera(0).getFar());
	}
	glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
	pvr::float32 fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;

	scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result::Enum OGLESIntroducingPVRApi::releaseView()
{
	assetManager.releaseAll();
	uiRenderer.release();
	deviceResource.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result::Enum OGLESIntroducingPVRApi::renderFrame()
{
	//	Calculates the frame number to animate in a time-based manner.
	//	get the time in milliseconds.
	frame += (float)getFrameTime() / 30.f; // design-time target fps for animation

	if (frame >= scene->getNumFrames() - 1)	{	frame = 0;	}

	// Sets the scene animation to this frame
	scene->setCurrentFrame(frame);

	//	We can build the world view matrix from the camera position, target and an up vector.
	//	A scene is composed of nodes. There are 3 types of nodes:
	//	- MeshNodes :
	//		references a mesh in the getMesh().
	//		These nodes are at the beginning of of the Nodes array.
	//		And there are nNumMeshNode number of them.
	//		This way the .pod format can instantiate several times the same mesh
	//		with different attributes.
	//	- lights
	//	- cameras
	//	To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
	glm::mat4 worldView;
	glm::vec3 lightDir3;

	scene->getLightDirection(0, lightDir3);
	for (int i = 0; i < (int)scene->getNumMeshNodes(); ++i)
	{
		// Gets the node model matrix
		worldView = viewMtx *  scene->getWorldMatrix(i);

		// Passes the world-view-projection matrix (WVP) to the shader to transform the vertices
		drawPass.worldViewProj[i] = projMtx * worldView;
		// Reads the light direction from the scene.

		drawPass.worldViewIT[i] = glm::inverseTranspose(worldView);

		//light direction in eye space
		drawPass.dirLight[i] = glm::normalize(glm::vec3(viewMtx * glm::vec4(lightDir3, 1.f)));
	}
	deviceResource->commandBuffer->submit();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Pre-record the rendering commands
***********************************************************************************************************************/
void OGLESIntroducingPVRApi::recordCommandBuffer(pvr::assets::Effect& effectAsset)
{
	deviceResource->commandBuffer->beginRecording();
	deviceResource->commandBuffer->beginRenderPass(deviceResource->fboOnScreen,
	        pvr::Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(0.00, 0.70, 0.67, 1.0f));

	deviceResource->commandBuffer->bindPipeline(deviceResource->effect->getPipeline());
	deviceResource->commandBuffer->setUniform<pvr::int32>(deviceResource->effect->getPipeline()->getUniformLocation(
	            effectAsset.uniforms[uniformSemanticsTable[Semantics::Texture0]].variableName.c_str()), 0);

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
	drawPass.dirLight.resize(scene->getNumMeshNodes());
	drawPass.worldViewIT.resize(scene->getNumMeshNodes());
	drawPass.worldViewProj.resize(scene->getNumMeshNodes());
	for (int i = 0; i < (int)scene->getNumMeshNodes(); ++i)
	{
		const pvr::assets::Model::Node* pNode = &scene->getMeshNode(i);
		// Gets pMesh referenced by the pNode
		const pvr::assets::Mesh* pMesh = &scene->getMesh(pNode->getObjectId());
		pvr::int32 matId = pNode->getMaterialIndex();
		auto found = std::find_if(deviceResource->descSet.begin(), deviceResource->descSet.end(), DescripotSetComp(matId));
		if (found != deviceResource->descSet.end())
		{
			deviceResource->commandBuffer->bindDescriptorSet(deviceResource->effect->getPipeline()->getPipelineLayout(), 0, found->second, 0);
		}

		deviceResource->commandBuffer->bindVertexBuffer(deviceResource->vbos[pNode->getObjectId()], 0, 0);
		deviceResource->commandBuffer->bindIndexBuffer(deviceResource->ibos[pNode->getObjectId()], 0, pMesh->getFaces().getDataType());
		// Passes the world-view-projection matrix (WVP) to the shader to transform the vertices
		deviceResource->commandBuffer->setUniformPtr<glm::mat4>(deviceResource->effect->getUniform(uniformSemanticsTable[Semantics::WorldViewProjection]).location, 1, &drawPass.worldViewProj[i]);
		deviceResource->commandBuffer->setUniformPtr<glm::vec3>(deviceResource->effect->getUniform(uniformSemanticsTable[Semantics::LightDirEye]).location, 1, &drawPass.dirLight[i]);
		deviceResource->commandBuffer->setUniformPtr<glm::mat4>(deviceResource->effect->getUniform(uniformSemanticsTable[Semantics::WorldViewIT]).location, 1, &drawPass.worldViewIT[i]);

		//Now that the model-view matrix is set and the materials ready,
		//call another function to actually draw the mesh.
		deviceResource->commandBuffer->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
	}

	pvr::api::SecondaryCommandBuffer cmdBuff = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();

	uiRenderer.beginRendering(cmdBuff);
	uiRenderer.getDefaultTitle()->render();
	uiRenderer.getSdkLogo()->render();
	uiRenderer.endRendering();
	deviceResource->commandBuffer->enqueueSecondaryCmds(cmdBuff);
	deviceResource->commandBuffer->endRenderPass();
	deviceResource->commandBuffer->endRecording();
}

/*!*********************************************************************************************************************
\brief	Create combined texture and sampler descriptor set for the materials in the scene
\return	Return true on success
***********************************************************************************************************************/
bool OGLESIntroducingPVRApi::createDescriptorSet()
{
	// create the sampler object
	pvr::assets::SamplerCreateParam samplerInfo;
	samplerInfo.minificationFilter = samplerInfo.magnificationFilter = samplerInfo.mipMappingFilter = SamplerFilter::Linear;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = SamplerWrap::Repeat;
	deviceResource->samplerTrilinear = deviceResource->context->createSampler(samplerInfo);

	if (!deviceResource->samplerTrilinear.isValid())
	{
		pvr::Log("Failed to create Sampler Object");
		return false;
	}

	const pvr::api::PipelineLayoutCreateParam& pipeLayoutInfo = deviceResource->effect->getPipeline()->getPipelineLayout()->getCreateParam();
	pvr::uint32 i = 0;
	while (i < scene->getNumMaterials() && scene->getMaterial(i).getDiffuseTextureIndex() != -1)
	{
		pvr::api::DescriptorSetUpdate descSetInfo;
		pvr::api::TextureView diffuseMap;
		const pvr::assets::Model::Material& material = scene->getMaterial(i);

		// Load the diffuse texture map
		if (!assetManager.getTextureWithCaching(getGraphicsContext(),
		                                        scene->getTexture(material.getDiffuseTextureIndex()).getName(), &(diffuseMap), NULL))
		{
			setExitMessage("ERROR: Failed to load texture %s", scene->getTexture(material.getDiffuseTextureIndex()).getName().c_str());
			return false;
		}
		descSetInfo.setCombinedImageSampler(0, diffuseMap, deviceResource->samplerTrilinear);

		MaterialDescSet matDescSet = std::make_pair(i, deviceResource->context->createDescriptorSetOnDefaultPool(pipeLayoutInfo.getDescriptorSetLayout(0)));
		matDescSet.second->update(descSetInfo);
		deviceResource->descSet.push_back(matDescSet);
		++i;
	}
	return true;
}

/*!*********************************************************************************************************************
\brief	This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.
\return Return an auto ptr to the demo supplied by the user
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new OGLESIntroducingPVRApi()); }
