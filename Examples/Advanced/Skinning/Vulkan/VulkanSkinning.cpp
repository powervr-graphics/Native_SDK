/*!*********************************************************************************************************************
\File         VulkanSkinning.cpp
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to perform skinning combined with Dot3 (normal-mapped) lighting
***********************************************************************************************************************/
#include "PVRApi/PVRApi.h"
#include "PVRShell/PVRShell.h"
#include "PVREngineUtils/PVREngineUtils.h"

using namespace pvr;
using namespace pvr::types;

namespace Configuration {
const char EffectFile[] = "Skinning_vk.pfx";
const char SceneFile[] = "Robot.pod";// POD scene files
}

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanSkinning : public pvr::Shell
{
	//Put all API managed objects in a struct so that we can one-line free them...
	struct DeviceResource
	{
		// Rendering manager, putting together Effects with Models to render things
		utils::RenderManager mgr;
		// Print3D class used to display text
		pvr::ui::UIRenderer uiRenderer;
		// Asset loader
		pvr::utils::AssetStore assetManager;

		pvr::api::CommandBuffer cmdBuffers[4];
	};
	std::auto_ptr<DeviceResource> devObj;

	// 3D Model
	pvr::assets::ModelHandle scene;

	bool isPaused;

	// Variables to handle the animation in a time-based manner
	float currentFrame;
public:
	VulkanSkinning() : isPaused(false), currentFrame(0) {}

	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	void recordCommandBuffer(api::FboSet& fbo);
	void eventMappedInput(pvr::SimplifiedInput action);
};

/*!*********************************************************************************************************************
\brief  handle the input event
\param  action input actions to handle
***********************************************************************************************************************/
void VulkanSkinning::eventMappedInput(pvr::SimplifiedInput action)
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
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanSkinning::initApplication()
{
	pvr::assets::PODReader podReader(getAssetStream(Configuration::SceneFile));
	if ((scene = pvr::assets::Model::createWithReader(podReader)).isNull())
	{
		setExitMessage("Error: Could not create the scene file %s.", Configuration::SceneFile);
		return pvr::Result::NoData;
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by Shell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanSkinning::quitApplication()
{
	scene.reset();
	devObj.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanSkinning::initView()
{
	devObj.reset(new DeviceResource);
	devObj->assetManager.init(*this);
	api::FboSet fboOnScreen = getGraphicsContext()->createOnScreenFboSet();
	currentFrame = 0.;

	pvr::assets::pfx::PfxParser rd(Configuration::EffectFile, this);

	devObj->mgr.addEffect(*rd.getAssetHandle(), getGraphicsContext(), devObj->assetManager);
	devObj->mgr.addModelForAllPasses(scene);
	devObj->mgr.buildRenderObjects();
	scene->releaseVertexData();
	devObj->mgr.createAutomaticSemantics();

	/***************************************************************
	**** Under the hood, the above line will do the following: *****

	for (auto& pipe : devObj->mgr.toSubpass(0, 0, 0).pipelines) // actually, for all subpasses, but we only have 1
	{
	  pipe.createAutomaticModelSemantics();
	}

	for (auto& node : devObj->mgr.renderables())
	{
	  node.createAutomaticSemantics();
	}
	***************************************************************/

	pvr::Result result = pvr::Result::Success;
	result = devObj->uiRenderer.init(fboOnScreen[0]->getRenderPass(), 0);
	if (result != pvr::Result::Success) { return result; }

	devObj->uiRenderer.getDefaultTitle()->setText("Skinning");
	devObj->uiRenderer.getDefaultTitle()->commitUpdates();
	devObj->uiRenderer.getDefaultDescription()->setText("Skinning with Normal Mapped Per Pixel Lighting");

	devObj->uiRenderer.getDefaultDescription()->commitUpdates();
	devObj->uiRenderer.getDefaultControls()->setText("Any Action Key : Pause");
	devObj->uiRenderer.getDefaultControls()->commitUpdates();
	devObj->uiRenderer.getSdkLogo()->setColor(1.0f, 1.0f, 1.0f, 1.f);
	devObj->uiRenderer.getSdkLogo()->commitUpdates();
	recordCommandBuffer(fboOnScreen);
	getGraphicsContext()->waitIdle();
	return result;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanSkinning::releaseView()
{
	devObj.reset(0);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanSkinning::renderFrame()
{
	//Calculates the frame number to animate in a time-based manner.
	//Uses the shell function this->getTime() to get the time in milliseconds.

	float32 fDelta = (float32)getFrameTime();
	if (fDelta > 0.0001f)
	{
		if (!isPaused) { currentFrame += fDelta / scene->getFPS(); }

		// Wrap the Frame number back to Start
		while (currentFrame >= scene->getNumFrames() - 1) { currentFrame -= (scene->getNumFrames() - 1); }
	}
	// Set the scene animation to the current frame

	devObj->mgr.toSubpassGroupModel(0, 0, 0, 0, 0).updateFrame(currentFrame);

	devObj->mgr.updateAutomaticSemantics(getSwapChainIndex());

	/***************************************************************
	**** Under the hood, the above line will do the following: *****

	//Get a new worldview camera and light position
	auto& pipeline = devObj->mgr.toPipeline(0, 0, 0, 0);
	pipeline.updateAutomaticModelSemantics(getSwapChainIndex());

	// Update all node-specific matrices (Worldview, bone array etc).
	pvr::uint32 swapChainIndex = getGraphicsContext()->getPlatformContext().getSwapChainIndex();

	// Should be called before updating anything to optimise map/unmap. Suggest call once per frame.
	devObj->mgr.toEffect(0).beginBufferUpdates(swapChainIndex);

	for (auto& rendernode : devObj->mgr.renderables()) { rendernode.updateAutomaticSemantics(swapChainIndex); }

	// Must be called if beginBufferUpdates were previously called (performs the buffer->unmap calls)
	devObj->mgr.toEffect(0).endBufferUpdates(swapChainIndex);

	***************************************************************/

	//Update all the bones matrices
	devObj->cmdBuffers[getSwapChainIndex()]->submit();
	return pvr::Result::Success;
}

inline std::vector<StringHash> generateBonesList(const char* base, uint32 numBones)
{
	std::vector<StringHash> boneSemantics;
	char buffer[255];
	assertion(strlen(base) < 240);
	strcpy(buffer, base);
	char* ptr = buffer + strlen(base);
	int32 decades = numBones / 10;
	int32 units = numBones % 10;
	for (pvr::int32 decade = 0; decade < decades; ++decade)
	{
		for (pvr::int32 unit = 0; unit < 10; ++unit)
		{
			*ptr = '0' + unit;
			ptr[1] = '\0';
			boneSemantics.push_back(StringHash(buffer));
		}
		if (decade == 0)
		{
			ptr++;
		}
		*(ptr - 1) = '0' + decade + 1;
	}

	//0, 1, 2, 3, 4, 5, 6, 7
	for (pvr::int32 unit = 0; unit < units; ++unit)
	{
		*ptr = '0' + unit;
		ptr[1] = '\0';
		boneSemantics.push_back(StringHash(buffer));
	}

	return boneSemantics;
}



/*!*********************************************************************************************************************
\brief  pre-record the rendering commands
***********************************************************************************************************************/
void VulkanSkinning::recordCommandBuffer(api::FboSet& fbo)
{
	for (pvr::uint32 swapidx = 0; swapidx < getSwapChainLength(); ++swapidx)
	{
		devObj->cmdBuffers[swapidx] = getGraphicsContext()->createCommandBufferOnDefaultPool();
		devObj->cmdBuffers[swapidx]->beginRecording();
		// Clear the color and depth buffer automatically.
		devObj->cmdBuffers[swapidx]->beginRenderPass(fbo[swapidx], true, glm::vec4(.2f, .3f, .4f, 1.f));

		devObj->mgr.toPass(0, 0).recordRenderingCommands(devObj->cmdBuffers[swapidx], swapidx, false);

		//// PART 3 :  UIRenderer
		devObj->uiRenderer.beginRendering(devObj->cmdBuffers[swapidx]);
		devObj->uiRenderer.getDefaultDescription()->render();
		devObj->uiRenderer.getDefaultTitle()->render();
		devObj->uiRenderer.getSdkLogo()->render();
		devObj->uiRenderer.getDefaultControls()->render();
		devObj->uiRenderer.endRendering();

		///// PART 4 : End the RenderePass
		devObj->cmdBuffers[swapidx]->endRenderPass();
		devObj->cmdBuffers[swapidx]->endRecording();
	}
}

/*!*********************************************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its Shell object defining the behaviour
of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanSkinning()); }
