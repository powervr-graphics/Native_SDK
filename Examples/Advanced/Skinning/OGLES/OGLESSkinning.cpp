/*!*********************************************************************************************************************
\File         OGLESSkinning.cpp
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to perform skinning combined with Dot3 (normal-mapped) lighting
***********************************************************************************************************************/
#include "PVRApi/PVRApi.h"
#include "PVRShell/PVRShell.h"
#include "PVRUIRenderer/UIRenderer.h"
#include "PVRApi/RenderManager.h"

using namespace pvr::types;
// shader uniforms

namespace Configuration {
const char EffectFile[] = "Skinning_es.pfx";

// POD scene files
const char SceneFile[]					= "Robot.pod";
}

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class OGLESSkinning : public pvr::Shell
{
	//Put all API managed objects in a struct so that we can one-line free them...
	struct ApiObjects
	{
		pvr::utils::RenderManager mgr;

		// Print3D class used to display text
		pvr::ui::UIRenderer uiRenderer;

		pvr::api::AssetStore assetManager;

		pvr::GraphicsContext context;
		pvr::api::CommandBuffer commandBuffer;
		pvr::api::Fbo fboOnScreen;
	};
	std::auto_ptr<ApiObjects> apiObj;
	// 3D Model
	pvr::assets::ModelHandle scene;

	bool isPaused;

	// Variables to handle the animation in a time-based manner
	float currentFrame;

public:
	OGLESSkinning() : currentFrame(0), isPaused(false) {}

	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	void recordCommandBuffer();
	void eventMappedInput(pvr::SimplifiedInput action);
};

/*!*********************************************************************************************************************
\brief	handle the input event
\param	action input actions to handle
***********************************************************************************************************************/
void OGLESSkinning::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3:
		isPaused = !isPaused; break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESSkinning::initApplication()
{
	pvr::assets::PODReader podReader(getAssetStream(Configuration::SceneFile));
	if ((scene = pvr::assets::Model::createWithReader(podReader)).isNull())
	{
		setExitMessage("Error: Could not create the scene file %s.", Configuration::SceneFile);
		return pvr::Result::NoData;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (scene->getNumCameras() == 0)
	{
		setExitMessage("Error: The scene does not contain a camera.");
		return pvr::Result::NoData;
	}

	// Check the scene contains at least one light
	if (scene->getNumLights() == 0)
	{
		setExitMessage("Error: The scene does not contain a light.");
		return pvr::Result::NoData;
	}

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by Shell once per run, just before exiting the program.
		If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result OGLESSkinning::quitApplication()
{
	scene.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESSkinning::initView()
{
	apiObj.reset(new ApiObjects);
	apiObj->assetManager.init(*this);

	currentFrame = 0.;
	apiObj->context = getGraphicsContext();
	apiObj->commandBuffer = apiObj->context->createCommandBufferOnDefaultPool();

	pvr::assets::pfx::PfxParser rd(Configuration::EffectFile, this);
	pvr::utils::RenderManager& mgr = apiObj->mgr;
	mgr.addEffect(*rd.getAssetHandle(), getGraphicsContext(), apiObj->assetManager);
	mgr.addModelForAllPasses(scene);
	mgr.buildRenderObjects();

	for (auto& pipe : mgr.toSubpass(0, 0, 0).pipelines)
	{
		pipe.createAutomaticModelSemantics();
	}

	for (auto& node : mgr.renderables())
	{
		node.createAutomaticSemantics();
	}

	apiObj->fboOnScreen = apiObj->context->createOnScreenFbo(0);

	pvr::Result result = pvr::Result::Success;
	result = apiObj->uiRenderer.init(apiObj->fboOnScreen->getRenderPass(), 0);
	if (result != pvr::Result::Success) { return result; }

	apiObj->uiRenderer.getDefaultTitle()->setText("Skinning");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	apiObj->uiRenderer.getDefaultDescription()->setText("Skinning with Normal Mapped Per Pixel Lighting");
	apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
	apiObj->uiRenderer.getDefaultControls()->setText("Any Action Key : Pause");
	apiObj->uiRenderer.getDefaultControls()->commitUpdates();

	recordCommandBuffer();
	return result;
}


/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OGLESSkinning::releaseView() { apiObj.reset(0); return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESSkinning::renderFrame()
{
	//Calculates the frame number to animate in a time-based manner.
	//Uses the shell function this->getTime() to get the time in milliseconds.

	pvr::float32 fDelta = (pvr::float32)getFrameTime();
	if (fDelta > 0.0001f)
	{
		if (!isPaused)	{	currentFrame += fDelta / scene->getFPS();	}

		// Wrap the Frame number back to Start
		while (currentFrame >= scene->getNumFrames() - 1) { currentFrame -= (scene->getNumFrames() - 1); }
	}
	// Set the scene animation to the current frame

	apiObj->mgr.toSubpassModel(0, 0, 0, 0).updateFrame(currentFrame);

	apiObj->mgr.updateAutomaticSemantics(getSwapChainIndex());

	//Update all the bones matrices
	apiObj->commandBuffer->submit();
	return pvr::Result::Success;
}


/*!*********************************************************************************************************************
\brief	pre-record the rendering commands
***********************************************************************************************************************/
void OGLESSkinning::recordCommandBuffer()
{
	apiObj->commandBuffer = getGraphicsContext()->createCommandBufferOnDefaultPool();
	apiObj->commandBuffer->beginRecording();
	// Clear the color and depth buffer automatically.
	apiObj->commandBuffer->beginRenderPass(apiObj->fboOnScreen, true, glm::vec4(.2f, .3f, .4f, 1.f));

	apiObj->mgr.toPass(0, 0).recordRenderingCommands(apiObj->commandBuffer, 0, false);

	//// PART 3 :  UIRenderer
	apiObj->uiRenderer.beginRendering(apiObj->commandBuffer);
	apiObj->uiRenderer.getDefaultDescription()->render();
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->uiRenderer.getDefaultControls()->render();
	apiObj->uiRenderer.endRendering();

	///// PART 4 : End the RenderePass
	apiObj->commandBuffer->endRenderPass();
	apiObj->commandBuffer->endRecording();
}

/*!*********************************************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its Shell object defining the behaviour
		of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new OGLESSkinning()); }
