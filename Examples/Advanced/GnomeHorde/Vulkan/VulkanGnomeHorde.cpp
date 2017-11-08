/*!*********************************************************************************************************************
\File         VulkanGnomeHorde.cpp
\Title        GnomeHorde
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        Multithreaded command buffer generation. Requires the PVRShell.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRVk/ApiObjectsVk.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRCore/Threading.h"

/*///////////////////////////////////////////////////////////////////////////////
THE GNOME HORDE - MULTITHREADED RENDERING ON THE VULKAN API USING THE POWERVR
FRAMEWORK.

This examples shows a very efficient multithreaded rendering design using queues
for abstracted inter-thread communication.

The domain of the problem (the "game" world) is divided into a tile grid.
* Each tile has several objects, and will have its own SecondaryCommandBuffer for rendering.
* All tiles that are visible will be gathered and submitted into a primary command
buffer
* Every frame, several threads check tiles for visibility. Since all tiles need to
be checked anyway, this task is subdivided into large chunks of the game world
("Lines" of tiles). This initial work is put into a Producer-Consumer queue.
* If a tile is found to have just became visible, or had its level of detail
changed, it needs to have its command buffer (re?)generated, hence it is entered
into a "Tiles to process" queue and the thread moves to check the next one
* If a tile is found to be visible without change, it is put directly into a
"tiles to Draw" queue thread (bypassing processing entirely)
* Otherwise, it is ignored
* Another group of threads pull items from the "tiles to process" threads and for
each of them generate the command buffers, and enter them into the "tiles to draw"
* The main thread pulls the command buffers and draws them.
///////////////////////////////////////////////////////////////////////////////*/

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::mat3;
using glm::mat4;

enum CONSTANTS
{
	MAX_NUMBER_OF_SWAP_IMAGES = 4,
	MAX_NUMBER_OF_THREADS = 16,
	TILE_SIZE_X = 150,
	TILE_GAP_X = 20,
	TILE_SIZE_Y = 100,
	TILE_SIZE_Z = 150,
	TILE_GAP_Z = 20,
	NUM_TILES_X = 50,
	NUM_TILES_Z = 50,
	NUM_OBJECTS_PER_TILE = 9,
	NUM_UNIQUE_OBJECTS_PER_TILE = 5,
	TOTAL_NUMBER_OF_OBJECTS = NUM_TILES_X * NUM_TILES_Z * NUM_OBJECTS_PER_TILE,
};

// Application logic
struct AppModeParameter
{
	float speedFactor;
	float cameraHeightOffset;
	float cameraForwardOffset;
	float duration;
};

const std::array<AppModeParameter, 4> DemoModes =
{
	{
		{ 2.5f, 100.0f, 5.0f, 10.0f },
		{ 2.5f, 500.0f, 10.0f, 10.0f },
		{ 2.5f, 1000.0f, 20.0f, 10.0f },
		{ 15.0f, 1000.0f, 20.0f, 10.0f }
	}
};

struct TileProcessingResult
{
	int itemsDiscarded;
	ivec2 itemToDraw;
	TileProcessingResult() : itemToDraw(-1, -1), itemsDiscarded(0) {}
	void reset() { itemToDraw = ivec2(-1, -1); itemsDiscarded = 0; }
};

class VulkanGnomeHorde;
//This queue is to enqueue tasks used for the "determine visibility" producer queues
//There, our "task" granularity is a "line" of tiles to process.
typedef pvr::LockedQueue<int32_t> LineTasksQueue;

//This queue is used to create command buffers, so its task granularity is a tile.
//It is Used for the "create command buffers for tile XXX" queues
typedef pvr::LockedQueue<TileProcessingResult> TileResultsQueue;

class GnomeHordeWorkerThread
{
public:
	GnomeHordeWorkerThread() : id(-1), running(false) {}
	std::string myType;
	std::thread thread;
	VulkanGnomeHorde* app;
	volatile uint8_t id;
	volatile bool running;
	void addlog(const std::string& str);
	void run();
	virtual bool doWork() = 0;
};

class GnomeHordeTileThreadData : public GnomeHordeWorkerThread
{
public:
	struct ThreadApiObjects
	{
		std::vector<pvrvk::CommandPool> commandPools;
		std::mutex poolMutex;
		TileResultsQueue::ConsumerToken processQConsumerToken;
		TileResultsQueue::ProducerToken drawQProducerToken;
		uint8_t lastSwapIndex;
		std::array<std::vector<pvrvk::SecondaryCommandBuffer>, MAX_NUMBER_OF_SWAP_IMAGES> preFreeCmdBuffers;
		std::array<std::vector<pvrvk::SecondaryCommandBuffer>, MAX_NUMBER_OF_SWAP_IMAGES> freeCmdBuffers;
		ThreadApiObjects(TileResultsQueue& processQ, TileResultsQueue& drawQ) :
			processQConsumerToken(processQ.getConsumerToken()), drawQProducerToken(drawQ.getProducerToken()), lastSwapIndex(-1)
		{
		}
	};
	GnomeHordeTileThreadData() { myType = "Tile Thread"; }
	std::auto_ptr<ThreadApiObjects> threadApiObj;

	bool doWork();

	pvrvk::SecondaryCommandBuffer getFreeCommandBuffer(uint8_t swapchainIndex);

	void garbageCollectPreviousFrameFreeCommandBuffers(uint8_t swapchainIndex);

	void freeCommandBuffer(const pvrvk::SecondaryCommandBuffer& commandBuff, uint8_t swapchainIndex);

	void generateTileBuffer(const TileProcessingResult* tiles, uint32_t numTiles);

};

class GnomeHordeVisibilityThreadData : public GnomeHordeWorkerThread
{
public:
	struct DeviceResources
	{
		LineTasksQueue::ConsumerToken linesQConsumerToken;
		TileResultsQueue::ProducerToken processQproducerToken;
		TileResultsQueue::ProducerToken drawQproducerToken;
		DeviceResources(LineTasksQueue& linesQ, TileResultsQueue& processQ, TileResultsQueue& drawQ) :
			linesQConsumerToken(linesQ.getConsumerToken()), processQproducerToken(processQ.getProducerToken()), drawQproducerToken(drawQ.getProducerToken())
		{ }
	};
	GnomeHordeVisibilityThreadData() { myType = "Visibility Thread"; }

	std::unique_ptr<DeviceResources> _deviceResources;

	bool doWork();

	void determineLineVisibility(const int32_t* lines, uint32_t numLines);
};

pvr::utils::VertexBindings attributeBindings[] =
{
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
};

struct MultiBuffering
{
	pvrvk::CommandBuffer commandBuffer;
	pvrvk::SecondaryCommandBuffer commandBufferUI;
	pvrvk::DescriptorSet descSetPerFrame;
};
struct Mesh
{
	pvr::assets::MeshHandle mesh;
	pvrvk::Buffer vbo;
	pvrvk::Buffer ibo;
};
typedef std::vector<Mesh> MeshLod;

struct Meshes
{
	MeshLod gnome;
	MeshLod gnomeShadow;
	MeshLod rock;
	MeshLod fern;
	MeshLod fernShadow;
	MeshLod mushroom;
	MeshLod mushroomShadow;
	MeshLod bigMushroom;
	MeshLod bigMushroomShadow;
	void clearAll()
	{
		clearApiMesh(gnome, true);
		clearApiMesh(gnomeShadow, true);
		clearApiMesh(rock, true);
		clearApiMesh(fern, true);
		clearApiMesh(fernShadow, true);
		clearApiMesh(mushroom, true);
		clearApiMesh(mushroomShadow, true);
		clearApiMesh(bigMushroom, true);
		clearApiMesh(bigMushroomShadow, true);
	}

	void clearApiObjects()
	{
		clearApiMesh(gnome, false);
		clearApiMesh(gnomeShadow, false);
		clearApiMesh(rock, false);
		clearApiMesh(fern, false);
		clearApiMesh(fernShadow, false);
		clearApiMesh(mushroom, false);
		clearApiMesh(mushroomShadow, false);
		clearApiMesh(bigMushroom, false);
		clearApiMesh(bigMushroomShadow, false);
	}
	void createApiObjects(pvrvk::Device& device)
	{
		createApiMesh(gnome, device);
		createApiMesh(gnomeShadow, device);
		createApiMesh(rock, device);
		createApiMesh(fern, device);
		createApiMesh(fernShadow, device);
		createApiMesh(mushroom, device);
		createApiMesh(mushroomShadow, device);
		createApiMesh(bigMushroom, device);
		createApiMesh(bigMushroomShadow, device);
	}
private:
	static void clearApiMesh(MeshLod& mesh, bool deleteAll)
	{
		for (MeshLod::iterator it = mesh.begin(); it != mesh.end(); ++it)
		{
			it->vbo.reset();
			it->ibo.reset();
			if (deleteAll) { it->mesh.reset(); }
		}
	}
	static void createApiMesh(MeshLod& mesh, pvrvk::Device& device)
	{
		for (MeshLod::iterator it = mesh.begin(); it != mesh.end(); ++it)
		{
			pvr::utils::createSingleBuffersFromMesh(device, *it->mesh, it->vbo, it->ibo);
		}
	}
};
struct DescriptorSets
{
	pvrvk::DescriptorSet gnome;
	pvrvk::DescriptorSet gnomeShadow;
	pvrvk::DescriptorSet rock;
	pvrvk::DescriptorSet fern;
	pvrvk::DescriptorSet fernShadow;
	pvrvk::DescriptorSet mushroom;
	pvrvk::DescriptorSet mushroomShadow;
	pvrvk::DescriptorSet bigMushroom;
	pvrvk::DescriptorSet bigMushroomShadow;
};

struct Pipelines
{
	pvrvk::GraphicsPipeline solid;
	pvrvk::GraphicsPipeline shadow;
	pvrvk::GraphicsPipeline alphaPremul;
};
struct TileObject
{
	MeshLod* mesh;
	pvrvk::DescriptorSet set;
	pvrvk::GraphicsPipeline pipeline;
};

struct TileInfo
{
	// Per tile info
	std::array<TileObject, NUM_OBJECTS_PER_TILE> objects;
	std::array<std::pair<pvrvk::SecondaryCommandBuffer, std::mutex*>, MAX_NUMBER_OF_SWAP_IMAGES> cbs;
	pvr::math::AxisAlignedBox aabb;
	uint8_t threadId;
	uint8_t lod;
	uint8_t oldLod;
	bool visibility;
	bool oldVisibility;
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvrvk::Device device;
	pvrvk::Swapchain swapchain;
	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::Queue queue;
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;
	pvr::Multi<pvrvk::ImageView> depthStencilImages;
	pvr::utils::StructuredBufferView uboPerObjectBufferView;
	pvrvk::Buffer uboPerObject;
	pvrvk::PipelineLayout pipeLayout;

	pvrvk::Sampler trilinear;
	pvrvk::Sampler nonMipmapped;

	pvrvk::DescriptorSet descSetAllObjects;
	DescriptorSets descSets;
	Pipelines pipelines;

	std::array<GnomeHordeTileThreadData, MAX_NUMBER_OF_THREADS> tileThreadData;
	std::array<GnomeHordeVisibilityThreadData, MAX_NUMBER_OF_THREADS> visibilityThreadData;

	std::array<std::array<TileInfo, NUM_TILES_X>, NUM_TILES_Z> tileInfos;
	MultiBuffering multiBuffering[MAX_NUMBER_OF_SWAP_IMAGES];

	pvr::utils::StructuredBufferView uboBufferView;
	pvrvk::Buffer ubo;

	std::array<std::thread, 16> threads;
	LineTasksQueue::ProducerToken lineQproducerToken;
	TileResultsQueue::ConsumerToken drawQconsumerToken;

	pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	DeviceResources(LineTasksQueue& lineQ, TileResultsQueue& drawQ)
		: lineQproducerToken(lineQ.getProducerToken()), drawQconsumerToken(drawQ.getConsumerToken())
	{ }
};

class VulkanGnomeHorde : public pvr::Shell
{
public:
	std::deque<std::string> multiThreadLog;
	std::mutex logMutex;
	uint32_t numSwapImages;
	Meshes meshes;
	std::unique_ptr<DeviceResources> _deviceResources;

	LineTasksQueue linesToProcessQ;
	TileResultsQueue tilesToProcessQ;
	TileResultsQueue tilesToDrawQ;

	uint32_t allLines[NUM_TILES_Z]; //Stores the line #. Used to kick all work in the visibility threads as each thread will be processing one line

	volatile glm::vec3 cameraPosition;
	volatile pvr::math::ViewingFrustum frustum;
	volatile uint8_t swapchainIndex;
	uint32_t frameId;
	bool isPaused;
	uint8_t numVisibilityThreads;
	uint8_t numTileThreads;
	VulkanGnomeHorde() : isPaused(false), numVisibilityThreads(0), numTileThreads(0)
	{
		for (int i = 0; i < NUM_TILES_Z; ++i)
		{
			allLines[i] = i;
		}
	}

	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	void setUpUI();

	pvrvk::DescriptorSet createDescriptorSetUtil(const pvrvk::DescriptorSetLayout&,
	    const pvr::StringHash& texture, const pvrvk::Sampler& mipMapped,
	    const pvrvk::Sampler& nonMipMapped,
	    std::vector<pvr::utils::ImageUploadResults>& imageUploads,
	    pvrvk::CommandBuffer& uploadCmdBuffer);

	//// HELPERS ////
	MeshLod loadLodMesh(const pvr::StringHash& filename,
	                    const pvr::StringHash& mesh, uint32_t max_lods);

	AppModeParameter calcAnimationParameters();
	void initUboStructuredObjects();

	void createDescSetsAndTiles(const pvrvk::DescriptorSetLayout& layoutImage,
	                            const pvrvk::DescriptorSetLayout& layoutPerObject,
	                            const pvrvk::DescriptorSetLayout& layoutPerFrameUbo,
	                            std::vector<pvr::utils::ImageUploadResults>& imageUploads,
	                            pvrvk::CommandBuffer& uploadCmdBuffer);

	void updateCameraUbo(const glm::mat4& matrix);

	pvrvk::Device& getDevice()
	{
		return _deviceResources->device;
	}

	pvrvk::Queue& getQueue()
	{
		return _deviceResources->queue;
	}

	void printLog()
	{
		std::unique_lock<std::mutex> lock(logMutex);
		while (!multiThreadLog.empty())
		{
			Log(LogLevel::Information, multiThreadLog.front().c_str());
			multiThreadLog.pop_front();
		}
	}

	struct DemoDetails
	{
		// Time tracking
		float logicTime; //!< Total time that has elapsed for the application (Conceptual: Clock at start - Clock time now - Paused time)
		float gameTime; //!< Time that has elapsed for the application (Conceptual: Integration of logicTime * the demo's speed factor at each point)
		bool isManual;
		uint32_t currentMode;
		uint32_t previousMode;
		float modeSwitchTime;
		DemoDetails() : logicTime(0), gameTime(0), isManual(false), currentMode(0), previousMode(0), modeSwitchTime(0.f) {}
	} animDetails;
};


void GnomeHordeWorkerThread::addlog(const std::string& str)
{
	std::unique_lock<std::mutex> lock(app->logMutex);
	app->multiThreadLog.push_back(str);
}

void GnomeHordeWorkerThread::run()
{
	addlog(pvr::strings::createFormatted("=== [%s] [%d] ===            Starting", myType.c_str(), id));
	running = true;
	while (doWork()) { continue; } // grabs a piece of work as long as the queue is not empty.
	running = false;
	addlog(pvr::strings::createFormatted("=== [%s] [%d] ===            Exiting", myType.c_str(), id));
}

void GnomeHordeTileThreadData::garbageCollectPreviousFrameFreeCommandBuffers(uint8_t swapchainIndex)
{
	auto& freeCmd = threadApiObj->freeCmdBuffers[swapchainIndex];
	auto& prefreeCmd = threadApiObj->preFreeCmdBuffers[swapchainIndex];

	std::move(prefreeCmd.begin(), prefreeCmd.end(), std::back_inserter(freeCmd));
	prefreeCmd.clear();
	if (freeCmd.size() > 10)
	{
		freeCmd.clear();
	}
}

pvrvk::SecondaryCommandBuffer GnomeHordeTileThreadData::getFreeCommandBuffer(uint8_t swapchainIndex)
{
	if (threadApiObj->lastSwapIndex != app->swapchainIndex)
	{
		threadApiObj->lastSwapIndex = app->swapchainIndex;
		std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
		garbageCollectPreviousFrameFreeCommandBuffers(app->swapchainIndex);
	}

	pvrvk::SecondaryCommandBuffer retval;
	{
		std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
		if (!threadApiObj->freeCmdBuffers[swapchainIndex].empty())
		{
			retval = threadApiObj->freeCmdBuffers[swapchainIndex].back();
			threadApiObj->freeCmdBuffers[swapchainIndex].pop_back();
		}
	}
	if (retval.isNull())
	{
		if (threadApiObj->commandPools.size() == 0)
		{
			threadApiObj->commandPools.push_back(app->getDevice()->createCommandPool(
			                                       app->getQueue()->getQueueFamilyId(),
			                                       VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
			addlog(pvr::strings::createFormatted("Created command pool %llu on thread %llu",
			                                     threadApiObj->commandPools.back()->getNativeObject(), std::this_thread::get_id()));
		}
		{
			std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
			retval = threadApiObj->commandPools.back()->allocateSecondaryCommandBuffer();
		}
		if (retval.isNull())
		{
			Log(LogLevel::Error, "[THREAD %ull] Command buffer allocation failed, "
			    ". Trying to create additional command buffer pool.", id);

			threadApiObj->commandPools.push_back(app->getDevice()->createCommandPool(
			                                       app->getQueue()->getQueueFamilyId(), VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

			addlog(pvr::strings::createFormatted(
			         "Created command pool %d on thread %d", threadApiObj->commandPools.back()->getNativeObject(),
			         std::this_thread::get_id()));
			{
				std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
				retval = threadApiObj->commandPools.back()->allocateSecondaryCommandBuffer();
			}
			if (retval.isNull())
			{
				Log(LogLevel::Critical, "COMMAND BUFFER ALLOCATION FAILED ON FRESH COMMAND POOL.");
			}
		}
	}
	return retval;
}

void GnomeHordeTileThreadData::freeCommandBuffer(const pvrvk::SecondaryCommandBuffer& commandBuff, uint8_t swapchainIndex)
{
	std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
	if (threadApiObj->lastSwapIndex != app->swapchainIndex)
	{
		threadApiObj->lastSwapIndex = app->swapchainIndex;
		garbageCollectPreviousFrameFreeCommandBuffers(app->swapchainIndex);
	}
	threadApiObj->preFreeCmdBuffers[swapchainIndex].push_back(commandBuff);
}

///// UTILS (local) /////
inline vec3 getTrackPosition(float time, const vec3& world_size)
{
	const float angle = time * 0.02f;
	const vec3 centre = world_size * 0.5f;
	const vec3 radius = world_size * 0.2f;
	// Main circle
	float a1 = time * 0.07f;
	float a2 = time * 0.1f;
	float a3 = angle;

	const float h = glm::sin(a1) * 15.f + 100.f;
	const float radius_factor = 0.95f + 0.1f * glm::sin(a2);
	const glm::vec3 circle(glm::sin(a3)*radius.x * radius_factor, h, glm::cos(a3)*radius.z * radius_factor);

	return centre + circle;
}

float initializeGridPosition(std::vector<float>& grid, uint32_t numItemsPerRow)
{
	//| x | x | x |
	//| x | x | x |
	//| x | x | x |
	//Jittered Grid - each object is placed on the center of a normal grid, and then moved randomly around.
	const float MIN_DISTANCE_FACTOR = -.2f; // Minimum item distance is 1/5 th their starting distance

	grid.resize(numItemsPerRow);
	float distance = 1.f / numItemsPerRow;
	grid[0] = .5f * distance;
	for (uint32_t i = 1; i < numItemsPerRow; ++i)
	{
		grid[i] = grid[i - 1] + distance;
	}
	float deviation = distance * .5f * (1.f - MIN_DISTANCE_FACTOR);
	return deviation;
}

inline void generatePositions(vec3* points, vec3 minBound, vec3 maxBound)
{
	static std::vector<float> normalGridPositions;
	static const uint32_t numItemsPerRow = static_cast<uint32_t>((sqrtf(NUM_UNIQUE_OBJECTS_PER_TILE)));
	static const float deviation = initializeGridPosition(normalGridPositions, numItemsPerRow);

	for (uint32_t y = 0; y < numItemsPerRow; ++y)
	{
		for (uint32_t x = 0; x < numItemsPerRow; ++x)
		{
			vec3 pos(normalGridPositions[x] + deviation * pvr::randomrange(-1.f, 1.f), 0.f,
			         normalGridPositions[y] + deviation * pvr::randomrange(-1.f, 1.f));
			vec3 mixed = glm::mix(minBound, maxBound, pos);

			points[y * numItemsPerRow + x] = vec3(mixed.x, mixed.y, mixed.z);
		}
	}
	for (int i = numItemsPerRow * numItemsPerRow; i < NUM_UNIQUE_OBJECTS_PER_TILE; ++i)
	{
		points[i] = glm::mix(minBound, maxBound, vec3(pvr::randomrange(-1.f, 1.f), 0, pvr::randomrange(-1.f, 1.f)));
	}
}


/////////// CLASS VulkanGnomeHorde ///////////
///// CALLBACKS - IMPLEMENTATION OF pvr::Shell /////
pvr::Result VulkanGnomeHorde::initApplication()
{
	setPreferredSwapChainLength(3);
	int num_cores = (int)std::thread::hardware_concurrency();
	int THREAD_FACTOR_RELAXATION = 1;

	int thread_factor = std::max(num_cores - THREAD_FACTOR_RELAXATION, 1);

	numVisibilityThreads = std::min(thread_factor, (int)MAX_NUMBER_OF_THREADS);
	numTileThreads = std::min(thread_factor, (int)MAX_NUMBER_OF_THREADS);
	Log(LogLevel::Information,
	    "Hardware concurreny reported: %u cores. Enabling %u visibility threads plus %u tile processing threads\n",
	    num_cores, numVisibilityThreads, numTileThreads);

	// Meshes
	meshes.gnome = loadLodMesh(pvr::StringHash("gnome"), "body", 7);
	meshes.gnomeShadow = loadLodMesh("gnome_shadow", "Plane001", 1);
	meshes.fern = loadLodMesh("fern", "Plane006", 1);
	meshes.fernShadow = loadLodMesh("fern_shadow", "Plane001", 1);
	meshes.mushroom = loadLodMesh("mushroom", "Mushroom1", 2);
	meshes.mushroomShadow = loadLodMesh("mushroom_shadow", "Plane001", 1);
	meshes.bigMushroom = loadLodMesh("bigMushroom", "Mushroom1", 1);
	meshes.bigMushroomShadow = loadLodMesh("bigMushroom_shadow", "Plane001", 1);
	meshes.rock = loadLodMesh("rocks", "rock5", 1);

	return pvr::Result::Success;
}

pvr::Result VulkanGnomeHorde::quitApplication()
{
	meshes.clearAll();
	return pvr::Result::Success;
}

void VulkanGnomeHorde::setUpUI()
{
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(),
	                                  _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
	                                  _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Gnome Horde");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Multithreaded command buffer generation and rendering");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	for (uint32_t i = 0; i < numSwapImages; ++i)
	{
		_deviceResources->multiBuffering[i].commandBufferUI = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		//UIRenderer - the easy stuff first, but we still must create one command buffer per frame.
		_deviceResources->multiBuffering[i].commandBufferUI->begin(_deviceResources->onScreenFramebuffer[i], 0);
		_deviceResources->uiRenderer.beginRendering(_deviceResources->multiBuffering[i].commandBufferUI);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->multiBuffering[i].commandBufferUI->end();
	}
}

pvr::Result VulkanGnomeHorde::initView()
{
	frameId = 0;
	_deviceResources.reset(new DeviceResources(linesToProcessQ, tilesToDrawQ));

	// crate the vulkan instance
	pvrvk::Surface surface;
	if (!pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(), this->getDisplay(), _deviceResources->instance, surface))
	{
		return pvr::Result::UnknownError;
	}
	// create the surface

	// create the device and the queue
	// look for  a queue that supports graphics, transfer and presentation
	pvr::utils::QueuePopulateInfo queuePopulateInfo =
	{
		VkQueueFlags::e_GRAPHICS_BIT | VkQueueFlags::e_TRANSFER_BIT, surface
	};
	pvr::utils::QueueAccessInfo queueAccessInfo;

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);

	if (_deviceResources->device.isNull())
	{
		setExitMessage("Failed to create Logical Device");
		return pvr::Result::UnknownError;
	}

	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// create the swapchain
	if (!pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device, surface,
	    getDisplayAttributes(), _deviceResources->swapchain, _deviceResources->depthStencilImages, swapchainImageUsage))
	{
		setExitMessage("failed to create Swapchain image");
		return pvr::Result::UnknownError;
	}
	numSwapImages = _deviceResources->swapchain->getSwapchainLength();

	//--------------------
	// Create the on screen framebuffer
	if (!pvr::utils::createOnscreenFramebufferAndRenderpass(
	      _deviceResources->swapchain, &_deviceResources->depthStencilImages[0], _deviceResources->onScreenFramebuffer))
	{
		return pvr::Result::UnknownError;
	}

	//--------------------
	// Create the commandpool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
	                                  _deviceResources->queue->getQueueFamilyId(), VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	//--------------------
	// Create the DescriptorPool
	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(
	                                     pvrvk::DescriptorPoolCreateInfo()
	                                     .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 32)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 32)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 32)
	                                     .setMaxDescriptorSets(32));

	if (_deviceResources->descriptorPool.isNull())
	{
		return pvr::Result::UnknownError;
	}

	setUpUI();

	//--------------------
	// create per swapchain resources
	for (uint32_t i = 0; i < numSwapImages; ++i)
	{
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);

		_deviceResources->multiBuffering[i].commandBuffer = _deviceResources->commandPool->allocateCommandBuffer();
	}

	initUboStructuredObjects();

	//Create Descriptor set layouts
	pvrvk::DescriptorSetLayoutCreateInfo imageDescParam;
	imageDescParam.setBinding(0, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);

	pvrvk::DescriptorSetLayout descLayoutImage =
	  _deviceResources->device->createDescriptorSetLayout(imageDescParam);

	pvrvk::DescriptorSetLayoutCreateInfo dynamicUboDescParam;
	dynamicUboDescParam.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, VkShaderStageFlags::e_VERTEX_BIT);

	pvrvk::DescriptorSetLayout descLayoutUboDynamic =
	  _deviceResources->device->createDescriptorSetLayout(dynamicUboDescParam);

	pvrvk::DescriptorSetLayoutCreateInfo uboDescParam;
	uboDescParam.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, VkShaderStageFlags::e_VERTEX_BIT);

	pvrvk::DescriptorSetLayout descLayoutUboStatic =
	  _deviceResources->device->createDescriptorSetLayout(uboDescParam);

	//Create Pipelines
	{
		_deviceResources->pipeLayout = _deviceResources->device->createPipelineLayout(
		                                 pvrvk::PipelineLayoutCreateInfo()
		                                 .setDescSetLayout(0, descLayoutImage)
		                                 .setDescSetLayout(1, descLayoutUboDynamic)
		                                 .setDescSetLayout(2, descLayoutUboStatic));

		// Must not assume the cache will always work

		pvrvk::Shader objectVsh = _deviceResources->device->createShader(getAssetStream("Object.vsh.spv")->readToEnd<uint32_t>());
		pvrvk::Shader shadowVsh = _deviceResources->device->createShader(getAssetStream("Shadow.vsh.spv")->readToEnd<uint32_t>());
		pvrvk::Shader solidFsh = _deviceResources->device->createShader(getAssetStream("Solid.fsh.spv")->readToEnd<uint32_t>());
		pvrvk::Shader shadowFsh = _deviceResources->device->createShader(getAssetStream("Shadow.fsh.spv")->readToEnd<uint32_t>());
		pvrvk::Shader premulFsh = _deviceResources->device->createShader(getAssetStream("Plant.fsh.spv")->readToEnd<uint32_t>());

		pvrvk::GraphicsPipelineCreateInfo pipeCreate;
		pvrvk::PipelineColorBlendAttachmentState cbStateNoBlend(false);

		pvrvk::PipelineColorBlendAttachmentState cbStateBlend(true, VkBlendFactor::e_ONE_MINUS_SRC_ALPHA,
		    VkBlendFactor::e_SRC_ALPHA, VkBlendOp::e_ADD);

		pvrvk::PipelineColorBlendAttachmentState cbStatePremulAlpha(true,
		    VkBlendFactor::e_ONE, VkBlendFactor::e_ONE_MINUS_SRC_ALPHA, VkBlendOp::e_ADD);

		pvr::utils::populateInputAssemblyFromMesh(*meshes.gnome[0].mesh,
		    &attributeBindings[0], 3, pipeCreate.vertexInput, pipeCreate.inputAssembler);

		pipeCreate.rasterizer.setFrontFaceWinding(VkFrontFace::e_COUNTER_CLOCKWISE);
		pipeCreate.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
		pipeCreate.depthStencil.enableDepthTest(true);
		pipeCreate.depthStencil.setDepthCompareFunc(VkCompareOp::e_LESS);
		pipeCreate.depthStencil.enableDepthWrite(true);
		pipeCreate.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		pipeCreate.pipelineLayout = _deviceResources->pipeLayout;

		pipeCreate.viewport.setViewportAndScissor(0,
		    pvrvk::Viewport(0, 0, _deviceResources->swapchain->getDimension().width,
		                    _deviceResources->swapchain->getDimension().height), pvrvk::Rect2Di(0, 0,
		                        _deviceResources->swapchain->getDimension().width,
		                        _deviceResources->swapchain->getDimension().height));

		// create the solid pipeline
		pipeCreate.vertexShader = objectVsh;
		pipeCreate.fragmentShader = solidFsh;
		pipeCreate.colorBlend.setAttachmentState(0, cbStateNoBlend);

		if ((_deviceResources->pipelines.solid =
		       _deviceResources->device->createGraphicsPipeline(pipeCreate)).isNull())
		{
			setExitMessage("Failed to create Opaque rendering pipeline");
			return pvr::Result::UnknownError;
		}

		pipeCreate.depthStencil.enableDepthWrite(false);
		// create the alpha pre-multiply pipeline
		pipeCreate.vertexShader = objectVsh;
		pipeCreate.fragmentShader = premulFsh;
		pipeCreate.colorBlend.setAttachmentState(0, cbStatePremulAlpha);
		if ((_deviceResources->pipelines.alphaPremul =
		       _deviceResources->device->createGraphicsPipeline(pipeCreate)).isNull())
		{
			setExitMessage("Failed to create Premultiplied Alpha rendering pipeline");
			return pvr::Result::UnknownError;
		}

		// create the shadow pipeline
		pipeCreate.colorBlend.setAttachmentState(0, cbStateBlend);
		pipeCreate.vertexShader = shadowVsh;
		pipeCreate.fragmentShader = shadowFsh;

		if ((_deviceResources->pipelines.shadow =
		       _deviceResources->device->createGraphicsPipeline(pipeCreate)).isNull())
		{
			setExitMessage("Failed to create Shadow rendering pipeline");
			return pvr::Result::UnknownError;
		}
	}

	pvrvk::CommandBuffer cb = _deviceResources->commandPool->allocateCommandBuffer();
	cb->begin();

	std::vector<pvr::utils::ImageUploadResults> imageUploads;
	createDescSetsAndTiles(descLayoutImage, descLayoutUboDynamic,
	                       descLayoutUboStatic, imageUploads, cb);

	//--------------------
	// submit the initial commandbuffer
	cb->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cb;
	submitInfo.numCommandBuffers = 1;

	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();//make sure the queue is finished

	animDetails.logicTime = 0.f;
	animDetails.gameTime = 0.f;
	{
		for (uint32_t i = 0; i < numVisibilityThreads; ++i)
		{
			_deviceResources->visibilityThreadData[i].id = i;
			_deviceResources->visibilityThreadData[i].app = this;
			_deviceResources->visibilityThreadData[i]._deviceResources.reset(
			  new GnomeHordeVisibilityThreadData::DeviceResources(
			    linesToProcessQ, tilesToProcessQ, tilesToDrawQ));

			_deviceResources->visibilityThreadData[i].thread = std::thread(
			      &GnomeHordeVisibilityThreadData::run,
			      (GnomeHordeVisibilityThreadData*)&_deviceResources->visibilityThreadData[i]);
		}
		for (uint32_t i = 0; i < numTileThreads; ++i)
		{
			_deviceResources->tileThreadData[i].id = i;
			_deviceResources->tileThreadData[i].app = this;
			_deviceResources->tileThreadData[i].threadApiObj.reset(
			  new GnomeHordeTileThreadData::ThreadApiObjects(tilesToProcessQ, tilesToDrawQ));

			_deviceResources->tileThreadData[i].thread = std::thread(
			      &GnomeHordeTileThreadData::run,
			      (GnomeHordeTileThreadData*)&_deviceResources->tileThreadData[i]);
		}
	}
	printLog();

	// Map the memory of the camera. It is HOST_COHERENT so should be fine.
	void* memory;
	_deviceResources->ubo->getDeviceMemory()->map(&memory);
	_deviceResources->uboBufferView.pointToMappedMemory(memory);


	return pvr::Result::Success;
}

pvr::Result VulkanGnomeHorde::releaseView()
{
	Log(LogLevel::Information, "Signalling all worker threads: Signal drain empty queues...");
	//Done will allow the queue to finish its work if it has any, but then immediately
	//afterwards it will free any and all threads waiting. Any threads attempting to
	//dequeue work from the queue will immediately return "false".
	linesToProcessQ.done();
	tilesToProcessQ.done();
	tilesToDrawQ.done();

	//waitIdle is being called to make sure the command buffers we will be destroying
	//are not being referenced.
	_deviceResources->device->waitIdle();

	Log(LogLevel::Information, "Joining all worker threads...");

	//Finally, tear down everything.
	for (uint32_t i = 0; i < numVisibilityThreads; ++i)
	{
		_deviceResources->visibilityThreadData[i].thread.join();
	}
	for (uint32_t i = 0; i < numTileThreads; ++i)
	{
		_deviceResources->tileThreadData[i].thread.join();
	}

	//Clear all objects. This will also free the command buffers that were allocated
	//from the worker thread's command pools, but are currently only held by the
	//tiles.
	meshes.clearApiObjects();
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->perFrameAcquireFence[i]->wait();
		_deviceResources->perFrameAcquireFence[i]->reset();

		_deviceResources->perFrameCommandBufferFence[i]->wait();
		_deviceResources->perFrameCommandBufferFence[i]->reset();
	}
	_deviceResources->device->waitIdle();
	_deviceResources.reset();

	Log(LogLevel::Information, "All worker threads done!");
	return pvr::Result::Success;
}

bool GnomeHordeTileThreadData::doWork()
{
	TileProcessingResult workItem[4];
	int32_t result;
	if ((result = app->tilesToProcessQ.consume(threadApiObj->processQConsumerToken, workItem[0])))
	{
		generateTileBuffer(workItem, result);
	}
	return result != 0;
}

void GnomeHordeTileThreadData::generateTileBuffer(const TileProcessingResult* tileIdxs, uint32_t numTiles)
{
	// Lambda to create the tiles secondary commandbs
	pvr::utils::StructuredBufferView& uboAllObj = app->_deviceResources->uboPerObjectBufferView;
	const pvrvk::DescriptorSet& descSetAllObj = app->_deviceResources->descSetAllObjects;
	pvr::utils::StructuredBufferView& uboCamera = app->_deviceResources->uboBufferView;

	for (uint32_t tilenum = 0; tilenum < numTiles; ++tilenum)
	{
		const TileProcessingResult& tileInfo = tileIdxs[tilenum];
		ivec2 tileId2d = tileInfo.itemToDraw;
		if (tileId2d != ivec2(-1, -1))
		{
			uint32_t x = tileId2d.x, y = tileId2d.y;
			uint32_t tileIdx = y * NUM_TILES_X + x;

			TileInfo& tile = app->_deviceResources->tileInfos[y][x];

			// Recreate the command buffer
			for (uint32_t swapIdx = 0; swapIdx < app->numSwapImages; ++swapIdx)
			{
				MultiBuffering& multi = app->_deviceResources->multiBuffering[swapIdx];
				tile.cbs[swapIdx].first = getFreeCommandBuffer(swapIdx);
				tile.cbs[swapIdx].second = &threadApiObj->poolMutex;
				tile.threadId = id;

				auto& cb = tile.cbs[swapIdx];
				std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
				cb.first->begin(app->_deviceResources->onScreenFramebuffer[swapIdx], 0, VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

				for (uint32_t objId = 0; objId < NUM_OBJECTS_PER_TILE; ++objId)
				{
					TileObject& obj = tile.objects[objId];
					uint32_t lod = std::min<uint32_t>(static_cast<uint32_t>(obj.mesh->size()) - 1, tile.lod);

					//Can it NOT be different than before? - Not in this demo.
					cb.first->bindPipeline(obj.pipeline);

					Mesh& mesh = (*obj.mesh)[lod];

					uint32_t offset = uboAllObj.getDynamicSliceOffset(tileIdx * NUM_OBJECTS_PER_TILE + objId);
					uint32_t uboCameraOffset = uboCamera.getDynamicSliceOffset(swapIdx);

					// Use the right texture and position - TEXTURES PER OBJECT (Can optimize to object type)
					cb.first->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
					                            app->_deviceResources->pipeLayout, 0, obj.set);

					cb.first->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
					                            app->_deviceResources->pipeLayout, 1, descSetAllObj, &offset, 1);

					cb.first->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
					                            app->_deviceResources->pipeLayout, 2, multi.descSetPerFrame, &uboCameraOffset, 1);

					//If different than before?
					cb.first->bindVertexBuffer(mesh.vbo, 0, 0);
					cb.first->bindIndexBuffer(mesh.ibo, 0, pvr::utils::convertToVk(mesh.mesh->getFaces().getDataType()));

					// Offset in the per-object transformation matrices UBO - these do not change frame-to-frame
					//getArrayOffset, will return the actual byte offset of item #(first param) that is in an
					//array of items, at array index #(second param).
					cb.first->drawIndexed(0, mesh.mesh->getNumIndices());
				}
				cb.first->end();
			}
		}
		app->tilesToDrawQ.produce(threadApiObj->drawQProducerToken, tileInfo);
	}
}

bool GnomeHordeVisibilityThreadData::doWork()
{
	int32_t workItem[4];
	int32_t result;
	if ((result = app->linesToProcessQ.consume(_deviceResources->linesQConsumerToken, workItem[0])))
	{
		determineLineVisibility(workItem, result);
	}
	return result != 0;
}

void GnomeHordeVisibilityThreadData::determineLineVisibility(const int32_t* lineIdxs, uint32_t numLines)
{
	auto& tileInfos = app->_deviceResources->tileInfos;
	//Local temporaries of the "global volatile" visibility variables. Memcpy used as they are declared "volatile"
	//It is perfectly fine to use memcopy for these volatiles AT THIS TIME because we know certainly that MAIN thread
	//has finished writing to them some time now and no race condition is possible (the calculations happen before the threads)
	pvr::math::ViewingFrustum frustum;
	pvr::utils::memCopyFromVolatile(frustum, app->frustum);
	glm::vec3 camPos;
	pvr::utils::memCopyFromVolatile(camPos, app->cameraPosition);

	TileResultsQueue& processQ = app->tilesToProcessQ;
	TileResultsQueue& drawQ = app->tilesToDrawQ;

	uint8_t numSwapImages = app->numSwapImages;
	TileProcessingResult retval;
	int numItems = 0;
	int numItems2 = 0;
	int numItemsProcessed = 0;
	int numItemsDrawn = 0;
	int numItemsDiscarded = 0;
	for (uint32_t line = 0; line < numLines; ++line)
	{
		glm::ivec2 id2d(0, lineIdxs[line]);
		for (id2d.x = 0; id2d.x < NUM_TILES_X; ++id2d.x)
		{
			++numItems2;
			tileInfos[id2d.y][id2d.x].visibility = pvr::math::aabbInFrustum(tileInfos[id2d.y][id2d.x].aabb, frustum);

			TileInfo& tile = tileInfos[id2d.y][id2d.x];

			// Compute tile lod
			float dist = glm::distance(tile.aabb.center(), camPos);
			float d = glm::max((dist - 400.0f) / 20.0f, 0.0f);
			float flod = glm::max<float>(sqrtf(d) - 2.0f, 0.0f);
			tile.lod = static_cast<uint32_t>(flod);

			if (tile.visibility != tile.oldVisibility || tile.lod != tile.oldLod) // The tile has some change. Will need to do something.
			{
				for (uint32_t i = 0; i < numSwapImages; ++i) //First, free its pre-existing command buffers (just mark free)
				{
					if (tile.cbs[i].first.isValid())
					{
						app->_deviceResources->tileThreadData[tile.threadId].freeCommandBuffer(tile.cbs[i].first, i);
						tile.cbs[i].first.reset();
					}
				}
				if (tile.visibility) //Item is visible, so must be recreated and drawn
				{
					retval.itemToDraw = id2d;
					processQ.produce(_deviceResources->processQproducerToken, retval);
					retval.reset();
					numItems++;
					numItemsProcessed++;
				} //Otherwise, see below
			}
			else if (tile.visibility) // Tile had no change, but was visible - just add it to the drawing queue.
			{
				retval.itemToDraw = id2d;
				drawQ.produce(_deviceResources->drawQproducerToken, retval);
				retval.reset();
				numItemsDrawn++;
				numItems++;
			}
			if (!tile.visibility) // THIS IS NOT AN ELSE. All invisible items end up here
			{
				++retval.itemsDiscarded;
				numItems++;
				numItemsDiscarded++;
			}

			tile.oldVisibility = tile.visibility;
			tile.oldLod = tile.lod;
		}
	}
	if (retval.itemsDiscarded != 0)
	{
		drawQ.produce(_deviceResources->drawQproducerToken, retval);
		retval.reset();
	}
}

pvr::Result VulkanGnomeHorde::renderFrame()
{
	const pvrvk::ClearValue clearVals[] =
	{
		pvrvk::ClearValue(.205f, 0.3f, 0.05f, 1.f),
		pvrvk::ClearValue(1.f, 0u)
	};

	_deviceResources->perFrameAcquireFence[frameId]->wait();
	_deviceResources->perFrameAcquireFence[frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[frameId], _deviceResources->perFrameAcquireFence[frameId]);

	swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	float dt = getFrameTime() * 0.001f;
	animDetails.logicTime += dt;
	if (animDetails.logicTime > 10000000)
	{
		animDetails.logicTime = 0;
	}

	// Get the next free swapchain image
	// We have implemented the application so that commandb[0,1,2] points to swapchain fb[0,1,2]
	// so we must submit the commandb that this index points to.
	// Applications that generate commandbs on the fly may not need to do this.

	//Interpolate frame parameters
	AppModeParameter parameters = calcAnimationParameters();

	animDetails.gameTime += dt * parameters.speedFactor;
	if (animDetails.gameTime > 10000000) { animDetails.gameTime = 0; }

	const vec3 worldSize = vec3(TILE_SIZE_X + TILE_GAP_X, TILE_SIZE_Y, TILE_SIZE_Z + TILE_GAP_Z) * vec3(NUM_TILES_X, 1, NUM_TILES_Z);
	vec3 camPos = getTrackPosition(animDetails.gameTime, worldSize);
	// cameraPosition is also used by the visibility threads. The "volatile"
	// variable is to make sure it is visible to the threads
	// we will be starting in a bit. For the moment, NO concurrent access happens
	// (as the worker threads are inactive).
	pvr::utils::memCopyToVolatile(cameraPosition, camPos);
	vec3 camTarget = getTrackPosition(animDetails.gameTime +
	                                  parameters.cameraForwardOffset, worldSize) + vec3(10.f, 10.f, 10.f);
	camTarget.y = 0.f;
	camPos.y += parameters.cameraHeightOffset;

	vec3 camUp = vec3(0.f, 1.f, 0.f);

	mat4 cameraMat = pvr::math::perspectiveFov(pvr::Api::Vulkan, 1.1f,
	                 (float)getWidth(), (float)getHeight(), 10.0f, 5000.f) *
	                 glm::lookAt(camPos, camTarget, camUp);

	mat4 cameraMat2 = pvr::math::perspectiveFov(pvr::Api::Vulkan, 1.2f,
	                  (float)getWidth(), (float)getHeight(), 10.0f, 5000.f) *
	                  glm::lookAt(camPos, camTarget, camUp);

	updateCameraUbo(cameraMat);

	pvr::math::ViewingFrustum frustumTmp;
	pvr::math::getFrustumPlanes(cameraMat2, frustumTmp);
	pvr::utils::memCopyToVolatile(frustum, frustumTmp);
	linesToProcessQ.produceMultiple(_deviceResources->lineQproducerToken, allLines, NUM_TILES_Z);

	auto& cb = _deviceResources->multiBuffering[swapchainIndex].commandBuffer;
	cb->begin();

	cb->beginRenderPass(_deviceResources->onScreenFramebuffer[swapchainIndex], false, clearVals, ARRAY_SIZE(clearVals));

	// Check culling and push the secondary command buffers on to the primary command buffer
	enum ItemsTotal {itemsTotal = NUM_TILES_X * NUM_TILES_Z};
	//MAIN RENDER LOOP - Collect the work (tiles) as it is processed
	{
		// Consume extra command buffers as they become ready
		uint32_t numItems;
		TileProcessingResult results[256];

		//We need some rather complex safeguards to make sure this thread does not wait forever.
		//First - we must (using atomics) make sure that when we say we are done, i.e. no items are unaccounted for.
		//Second, for the case where the main thread is waiting, but all remaining items are not visible, the last thread
		//to process an item will trigger an additional "unblock" to the main thread.

		uint32_t numItemsToDraw = itemsTotal;

		while (numItemsToDraw > 0)
		{
			numItems = static_cast<uint32_t>(tilesToDrawQ.consumeMultiple(_deviceResources->drawQconsumerToken, results, 256));
			for (uint32_t i = 0; i < numItems; ++i)
			{
				numItemsToDraw -= results[i].itemsDiscarded;

				ivec2 tileId = results[i].itemToDraw;
				if (tileId != ivec2(-1, -1))
				{
					--numItemsToDraw;
					std::unique_lock<std::mutex> lock(*_deviceResources->tileInfos[tileId.y][tileId.x].cbs[swapchainIndex].second);
					cb->executeCommands(&(_deviceResources->tileInfos[tileId.y][tileId.x].cbs[swapchainIndex].first), 1);
				}
			}
		}
	}

	//The uirenderer should always be drawn last as it has (and checks) no depth
	cb->executeCommands(_deviceResources->multiBuffering[swapchainIndex].commandBufferUI);

	assertion(linesToProcessQ.isEmpty(), "Initial Line Processing Queue was not empty after work done!");
	assertion(tilesToProcessQ.isEmpty(), "Worker Tile Processing Queue was not empty after work done!");

	//We do not need any additional syncing - we know all dispatched work is done.

	cb->endRenderPass();
	cb->end();
	pvrvk::SubmitInfo submitInfo;
	VkPipelineStageFlags waitStage = VkPipelineStageFlags::e_ALL_GRAPHICS_BIT;
	submitInfo.commandBuffers = &cb;
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDestStages = &waitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		if (_deviceResources->swapchain->supportsUsage(VkImageUsageFlags::e_TRANSFER_SRC_BIT))
		{
			pvr::utils::takeScreenshot(_deviceResources->swapchain, swapchainIndex, _deviceResources->commandPool, _deviceResources->queue, this->getScreenshotFileName());
		}
		else
		{
			Log(LogLevel::Warning, "Could not take screenshot as the swapchain does not support TRANSFER_SRC_BIT");
		}
	}


	uint32_t swapIdx = swapchainIndex;
	//--------------------
	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &swapIdx;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[frameId];
	_deviceResources->queue->present(presentInfo);
	printLog();
	frameId = (frameId + 1) % _deviceResources->swapchain->getSwapchainLength();
	return pvr::Result::Success;
}

void VulkanGnomeHorde::updateCameraUbo(const glm::mat4& matrix)
{
	_deviceResources->uboBufferView.getElement(0, 0, swapchainIndex).setValue(matrix);
}

void VulkanGnomeHorde::createDescSetsAndTiles(
  const pvrvk::DescriptorSetLayout& layoutImage,
  const pvrvk::DescriptorSetLayout& layoutPerObject,
  const pvrvk::DescriptorSetLayout& layoutPerFrameUbo,
  std::vector<pvr::utils::ImageUploadResults>& imageUploads,
  pvrvk::CommandBuffer& uploadCmdBuffer)
{
	pvrvk::Device& device = _deviceResources->device;
	{
		//The objects could have been completely different -
		//the fact that there are only a handful of different
		//objects is coincidental and does not affect the demo.
		auto& trilinear = _deviceResources->trilinear = device->createSampler(
		                    pvrvk::SamplerCreateInfo(VkFilter::e_LINEAR,
		                        VkFilter::e_LINEAR, VkSamplerMipmapMode::e_LINEAR));

		auto& nonMipmapped = _deviceResources->nonMipmapped = device->createSampler(
		                       pvrvk::SamplerCreateInfo(VkFilter::e_LINEAR,
		                           VkFilter::e_LINEAR, VkSamplerMipmapMode::e_NEAREST));

		_deviceResources->descSets.gnome = createDescriptorSetUtil(
		                                     layoutImage, "gnome_texture.pvr", trilinear, nonMipmapped, imageUploads, uploadCmdBuffer);

		_deviceResources->descSets.gnomeShadow = createDescriptorSetUtil(
		      layoutImage, "gnome_shadow.pvr", trilinear, nonMipmapped, imageUploads, uploadCmdBuffer);

		_deviceResources->descSets.rock = createDescriptorSetUtil(
		                                    layoutImage, "rocks.pvr", trilinear, nonMipmapped, imageUploads, uploadCmdBuffer);

		_deviceResources->descSets.fern = createDescriptorSetUtil(
		                                    layoutImage, "fern.pvr", trilinear, nonMipmapped, imageUploads, uploadCmdBuffer);

		_deviceResources->descSets.fernShadow = createDescriptorSetUtil(
		    layoutImage, "fern_shadow.pvr", trilinear, nonMipmapped, imageUploads, uploadCmdBuffer);

		_deviceResources->descSets.mushroom = createDescriptorSetUtil(
		                                        layoutImage, "mushroom_texture.pvr", trilinear, nonMipmapped, imageUploads, uploadCmdBuffer);

		_deviceResources->descSets.mushroomShadow = createDescriptorSetUtil(
		      layoutImage, "mushroom_shadow.pvr", trilinear, nonMipmapped, imageUploads, uploadCmdBuffer);

		_deviceResources->descSets.bigMushroom = createDescriptorSetUtil(
		      layoutImage, "bigMushroom_texture.pvr", trilinear, nonMipmapped, imageUploads, uploadCmdBuffer);

		_deviceResources->descSets.bigMushroomShadow = createDescriptorSetUtil(
		      layoutImage, "bigMushroom_shadow.pvr", trilinear, nonMipmapped, imageUploads, uploadCmdBuffer);

	}

	//The ::pvr::utils::StructuredMemoryView is a simple class that allows us easy access to update members of a buffer - it keeps track of offsets,
	//datatypes and sizes of items in the buffer, allowing us to update them very easily. The connectWithBuffer method allows us to call the map/unmap functions
	//directly on this object. In this case it will also help us with the array stride etc.

	//The uboPerObject is one huge DynamicUniformBuffer, whose data is STATIC, and contains the object Model->World matrixes
	//A different bit of this buffer is bound for each and every object.

	//CAUTION: The Range of the Buffer View for a Dynamic Uniform Buffer must be the BINDING size, not the TOTAL size, i.e. the size
	//of the part of the buffer that will be bound each time, not the total size. That is why we cannot do a one-step creation (...createBufferAndView) like
	//for static UBOs.
	pvrvk::WriteDescriptorSet descSetWrites[pvrvk::FrameworkCaps::MaxSwapChains + 1];
	_deviceResources->descSetAllObjects = _deviceResources->descriptorPool->allocateDescriptorSet(layoutPerObject);
	descSetWrites[0].set(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->descSetAllObjects, 0)
	.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboPerObject, 0, _deviceResources->uboPerObjectBufferView.getDynamicSliceSize()));

	for (uint32_t i = 0; i < numSwapImages; ++i)
	{
		//The uboPerFrame is a small UniformBuffer that contains the camera (World->Projection) matrix.
		//Since it is updated every frame, it is multi-buffered to avoid stalling the GPU
		auto& current = _deviceResources->multiBuffering[i];
		current.descSetPerFrame = _deviceResources->descriptorPool->allocateDescriptorSet(layoutPerFrameUbo);

		descSetWrites[i + 1] .set(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, current.descSetPerFrame, 0)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->ubo, 0, _deviceResources->uboBufferView.getDynamicSliceSize()));
	}
	_deviceResources->device->updateDescriptorSets(descSetWrites, numSwapImages + 1, nullptr, 0);
	//Create the UBOs/VBOs for the main objects. This automatically creates the VBOs.
	meshes.createApiObjects(device);

	//Using the StructuredMemoryView to update the objects
	pvr::utils::StructuredBufferView& perObj = _deviceResources->uboPerObjectBufferView;
	pvrvk::Buffer& perObjBuffer = _deviceResources->uboPerObject;
	uint32_t mvIndex = perObj.getIndex("modelView");
	uint32_t mvITIndex = perObj.getIndex("modelViewIT");

	void* memory;
	perObjBuffer->getDeviceMemory()->map(&memory);
	perObj.pointToMappedMemory(memory);

	for (uint32_t y = 0; y < NUM_TILES_Z; ++y)
	{
		for (uint32_t x = 0; x < NUM_TILES_X; ++x)
		{
			vec3 tileBL(x * (TILE_SIZE_X + TILE_GAP_Z), TILE_SIZE_Y, y * (TILE_SIZE_Z + TILE_GAP_Z));
			vec3 tileTR = tileBL + vec3(TILE_SIZE_X, 0, TILE_SIZE_Z);

			TileInfo& thisTile = _deviceResources->tileInfos[y][x];

			thisTile.aabb.setMinMax(tileBL, tileTR);

			thisTile.visibility = false;
			thisTile.lod = uint8_t(0xFFu);
			thisTile.oldVisibility = false;
			thisTile.oldLod = (uint8_t)0xFFu - (uint8_t)1;

			thisTile.objects[0].mesh = &meshes.gnome;
			thisTile.objects[0].set = _deviceResources->descSets.gnome;
			thisTile.objects[0].pipeline = _deviceResources->pipelines.solid;

			thisTile.objects[1].mesh = &meshes.gnomeShadow;
			thisTile.objects[1].set = _deviceResources->descSets.gnomeShadow;
			thisTile.objects[1].pipeline = _deviceResources->pipelines.shadow;

			thisTile.objects[2].mesh = &meshes.mushroom;
			thisTile.objects[2].set = _deviceResources->descSets.mushroom;
			thisTile.objects[2].pipeline = _deviceResources->pipelines.solid;

			thisTile.objects[3].mesh = &meshes.mushroomShadow;
			thisTile.objects[3].set = _deviceResources->descSets.mushroomShadow;
			thisTile.objects[3].pipeline = _deviceResources->pipelines.shadow;

			thisTile.objects[4].mesh = &meshes.bigMushroom;
			thisTile.objects[4].set = _deviceResources->descSets.bigMushroom;
			thisTile.objects[4].pipeline = _deviceResources->pipelines.solid;

			thisTile.objects[5].mesh = &meshes.bigMushroomShadow;
			thisTile.objects[5].set = _deviceResources->descSets.bigMushroomShadow;
			thisTile.objects[5].pipeline = _deviceResources->pipelines.shadow;

			thisTile.objects[7].mesh = &meshes.fernShadow;
			thisTile.objects[7].set = _deviceResources->descSets.fernShadow;
			thisTile.objects[7].pipeline = _deviceResources->pipelines.shadow;

			thisTile.objects[6].mesh = &meshes.fern;
			thisTile.objects[6].set = _deviceResources->descSets.fern;
			thisTile.objects[6].pipeline = _deviceResources->pipelines.alphaPremul;

			thisTile.objects[8].mesh = &meshes.rock;
			thisTile.objects[8].set = _deviceResources->descSets.rock;
			thisTile.objects[8].pipeline = _deviceResources->pipelines.solid;

			std::array<vec3, NUM_UNIQUE_OBJECTS_PER_TILE> points;
			generatePositions(points.data(), tileBL, tileTR);
			uint32_t tileBaseIndex = (y * NUM_TILES_X + x) * NUM_OBJECTS_PER_TILE;

			for (uint32_t halfobj = 0; halfobj < NUM_UNIQUE_OBJECTS_PER_TILE; ++halfobj)
			{
				uint32_t obj = halfobj * 2;
				uint32_t objShadow = obj + 1;
				// Note: do not put these in-line with the function call because it seems that the nexus player
				// swaps around the order that the parameters are evaluated compared to desktop
				float rot = pvr::randomrange(-glm::pi<float>(), glm::pi<float>());
				float s = pvr::randomrange(.8, 1.3);

				vec3 position = points[halfobj];
				mat4 rotation = glm::rotate(rot, vec3(0.0f, 1.0f, 0.0f));
				mat4 scale = glm::scale(vec3(s));
				mat4 xform = glm::translate(position) * rotation * scale;
				mat4 xformIT = glm::transpose(glm::inverse(xform));

				perObj.getElement(mvIndex, 0, tileBaseIndex + obj).setValue(xform);
				perObj.getElement(mvITIndex, 0, tileBaseIndex + obj).setValue(xformIT);

				if (objShadow != 9)
				{
					perObj.getElement(mvIndex, 0, tileBaseIndex + objShadow).setValue(xform);
					perObj.getElement(mvITIndex, 0, tileBaseIndex + objShadow).setValue(xformIT);
				}
			}
		}
	}
	perObjBuffer->getDeviceMemory()->unmap();
}

MeshLod VulkanGnomeHorde::loadLodMesh(const pvr::StringHash& filename, const pvr::StringHash& mesh, uint32_t num_lods)
{
	MeshLod meshLod;
	meshLod.resize(num_lods);

	for (uint32_t i = 0; i < num_lods; ++i)
	{
		std::stringstream ss;
		ss << i;
		ss << ".pod";

		std::string path = filename.str() + ss.str();
		Log(LogLevel::Information, "Loading model:%s mesh:%s\n", path.c_str(), mesh.c_str());
		pvr::assets::ModelHandle model =  pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(path)));

		if (model.isNull())
		{
			assertion(false, pvr::strings::createFormatted("Failed to load model file %s", path.c_str()).c_str());
		}
		for (uint32_t j = 0; j < model->getNumMeshNodes(); ++j)
		{
			if (model->getMeshNode(j).getName() == mesh)
			{
				uint32_t meshId = model->getMeshNode(j).getObjectId();
				meshLod[i].mesh = pvr::assets::getMeshHandle(model, meshId);
				break;
			}
			if (j == model->getNumMeshNodes()) { assertion(false, pvr::strings::createFormatted("Could not find mesh %s in model file %s", mesh.c_str(), path.c_str()).c_str()); }
		}
	}
	return meshLod;
}

pvrvk::DescriptorSet VulkanGnomeHorde::createDescriptorSetUtil(
  const pvrvk::DescriptorSetLayout& layout,
  const pvr::StringHash& texture, const pvrvk::Sampler& mipMapped,
  const pvrvk::Sampler& nonMipMapped,
  std::vector<pvr::utils::ImageUploadResults>& imageUploads,
  pvrvk::CommandBuffer& uploadCmdBuffer)
{
	//--------------------
	// Load the texture
	imageUploads.push_back(pvr::utils::loadAndUploadImage(_deviceResources->device,
	                       texture.c_str(), true, uploadCmdBuffer, *this));

	pvrvk::ImageView tex = imageUploads.back().getImageView();
	pvrvk::DescriptorSet tmp = _deviceResources->descriptorPool->allocateDescriptorSet(layout);
	bool hasMipmaps = tex->getImage()->getNumMipMapLevels() > 1;
	pvrvk::WriteDescriptorSet write(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, tmp, 0);
	write.setImageInfo(0, pvrvk::DescriptorImageInfo(tex, hasMipmaps ? mipMapped : nonMipMapped,
	                   VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	_deviceResources->device->updateDescriptorSets(&write, 1, nullptr, 0);
	return tmp;
}

void VulkanGnomeHorde::initUboStructuredObjects()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("projectionMat", pvr::GpuDatatypes::mat4x4);

		_deviceResources->uboBufferView.initDynamic(desc, numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));

		_deviceResources->ubo = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->uboBufferView.getSize(),
		                        VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("modelView", pvr::GpuDatatypes::mat4x4);
		desc.addElement("modelViewIT", pvr::GpuDatatypes::mat4x4);

		_deviceResources->uboPerObjectBufferView.initDynamic(desc, TOTAL_NUMBER_OF_OBJECTS, pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));

		_deviceResources->uboPerObject = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->uboPerObjectBufferView.getSize(),
		                                 VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}
}

AppModeParameter VulkanGnomeHorde::calcAnimationParameters()
{
	bool needsTransition = false;
	if (!animDetails.isManual)
	{
		if (animDetails.logicTime > animDetails.modeSwitchTime + DemoModes[animDetails.currentMode].duration)
		{
			animDetails.previousMode = animDetails.currentMode;
			animDetails.currentMode = (animDetails.currentMode + 1) % DemoModes.size();
			Log(LogLevel::Information, "Switching to mode: [%d]", animDetails.currentMode);
			needsTransition = true;
		}
	}
	if (needsTransition)
	{
		animDetails.modeSwitchTime = animDetails.logicTime;
		needsTransition = false;
	}

	// Generate camera position
	float iterp = glm::clamp((animDetails.logicTime - animDetails.modeSwitchTime) * 1.25f, 0.0f, 1.0f);
	float factor = (1.0f - glm::cos(iterp * 3.14159f)) / 2.0f;
	const AppModeParameter& current = DemoModes[animDetails.currentMode];
	const AppModeParameter& prev = DemoModes[animDetails.previousMode];

	// Interpolate
	AppModeParameter result;
	result.cameraForwardOffset = glm::mix(prev.cameraForwardOffset, current.cameraForwardOffset, factor);
	result.cameraHeightOffset = glm::mix(prev.cameraHeightOffset, current.cameraHeightOffset, factor);
	result.speedFactor = glm::mix(prev.speedFactor, current.speedFactor, factor);
	return result;
}

std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanGnomeHorde);}
