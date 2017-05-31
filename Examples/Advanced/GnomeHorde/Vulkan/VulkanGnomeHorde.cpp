#include "PVRApi/PVRApi.h"
#include "PVRCore/Threading.h"
#include "PVRShell/PVRShell.h"
#include "PVREngineUtils/PVREngineUtils.h"

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

using namespace pvr;

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

class VulkanGnomeHorde;
//This queue is to enqueue tasks used for the "determine visibility" producer queues
//There, our "task" granularity is a "line" of tiles to process.
typedef LockedQueue<int32> LineTasksQueue;

//This queue is used to create command buffers, so its task granularity is a tile.
//It is Used for the "create command buffers for tile XXX" queues
typedef LockedQueue<glm::ivec2> TileTasksQueue;

class GnomeHordeWorkerThread
{
public:
	GnomeHordeWorkerThread() : id(-1), running(false) {}
	std::thread thread;
	VulkanGnomeHorde* app;
	volatile uint8 id;
	volatile bool running;
	void addlog(const std::string& str);
	void run();
	virtual bool doWork() = 0;
};

class GnomeHordeTileThreadData : public GnomeHordeWorkerThread
{
public:
	struct ApiObjects
	{
		std::vector<api::CommandPool> cmdPools;
		TileTasksQueue::ConsumerToken processQConsumerToken;
		TileTasksQueue::ProducerToken drawQProducerToken;
		uint8 lastSwapIndex;
		std::array<std::vector<api::SecondaryCommandBuffer>, MAX_NUMBER_OF_SWAP_IMAGES> preFreeCmdBuffers;
		std::array<std::vector<api::SecondaryCommandBuffer>, MAX_NUMBER_OF_SWAP_IMAGES> freeCmdBuffers;
		ApiObjects(TileTasksQueue& processQ, TileTasksQueue& drawQ) :
			processQConsumerToken(processQ.getConsumerToken()), drawQProducerToken(drawQ.getProducerToken()), lastSwapIndex(-1)
		{
		}
	};
	std::mutex cmdMutex;
	std::auto_ptr<ApiObjects> apiObj;

	bool doWork();

	api::SecondaryCommandBuffer getFreeCommandBuffer(uint8 swapIndex);

	void garbageCollectPreviousFrameFreeCommandBuffers(uint8 swapIndex);

	void freeCommandBuffer(const api::SecondaryCommandBuffer& cmdBuff, uint8 swapIndex);

	void generateTileBuffer(const ivec2* tiles, uint32 numTiles);

};

class GnomeHordeVisibilityThreadData : public GnomeHordeWorkerThread
{
public:
	struct ApiObjects
	{
		LineTasksQueue::ConsumerToken linesQConsumerToken;
		TileTasksQueue::ProducerToken processQproducerToken;
		TileTasksQueue::ProducerToken drawQproducerToken;
		ApiObjects(LineTasksQueue& linesQ, TileTasksQueue& processQ, TileTasksQueue& drawQ) :
			linesQConsumerToken(linesQ.getConsumerToken()), processQproducerToken(processQ.getProducerToken()), drawQproducerToken(drawQ.getProducerToken())
		{ }
	};
	std::auto_ptr<ApiObjects> apiObj;

	bool doWork();

	void determineLineVisibility(const int32* lines, uint32 numLines);
};

utils::VertexBindings attributeBindings[] =
{
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
};

struct MultiBuffering
{
	pvr::api::CommandBuffer commandBuffer;
	pvr::api::SecondaryCommandBuffer cmdBufferUI;
	utils::StructuredMemoryView uboPerFrame;
	api::DescriptorSet descSetPerFrame;
	api::Fence fence;
};
struct Mesh
{
	assets::MeshHandle mesh;
	api::Buffer vbo;
	api::Buffer ibo;
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
	void createApiObjects(GraphicsContext& ctx)
	{
		createApiMesh(gnome, ctx);
		createApiMesh(gnomeShadow, ctx);
		createApiMesh(rock, ctx);
		createApiMesh(fern, ctx);
		createApiMesh(fernShadow, ctx);
		createApiMesh(mushroom, ctx);
		createApiMesh(mushroomShadow, ctx);
		createApiMesh(bigMushroom, ctx);
		createApiMesh(bigMushroomShadow, ctx);
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
	static void createApiMesh(MeshLod& mesh, GraphicsContext& ctx)
	{
		for (MeshLod::iterator it = mesh.begin(); it != mesh.end(); ++it)
		{
			utils::createSingleBuffersFromMesh(ctx, *it->mesh, it->vbo, it->ibo);
		}
	}
};
struct DescriptorSets
{
	api::DescriptorSet gnome;
	api::DescriptorSet gnomeShadow;
	api::DescriptorSet rock;
	api::DescriptorSet fern;
	api::DescriptorSet fernShadow;
	api::DescriptorSet mushroom;
	api::DescriptorSet mushroomShadow;
	api::DescriptorSet bigMushroom;
	api::DescriptorSet bigMushroomShadow;
};

struct Pipelines
{
	api::GraphicsPipeline solid;
	api::GraphicsPipeline shadow;
	api::GraphicsPipeline alphaPremul;
};
struct TileObject
{
	MeshLod* mesh;
	api::DescriptorSet set;
	api::GraphicsPipeline pipeline;
};

struct TileInfo
{
	// Per tile info
	std::array<TileObject, NUM_OBJECTS_PER_TILE> objects;
	std::array<api::SecondaryCommandBuffer, MAX_NUMBER_OF_SWAP_IMAGES> cbs;
	math::AxisAlignedBox aabb;
	uint8 threadId;
	uint8 lod;
	uint8 oldLod;
	bool visibility;
	bool oldVisibility;
};

struct ApiObjects
{
	pvr::GraphicsContext context;
	pvr::Multi<pvr::api::Fbo> fboOnScreen;
	utils::StructuredMemoryView uboPerObject;
	pvr::utils::AssetStore assetManager;
	pvr::ui::UIRenderer uiRenderer;
	api::PipelineLayout pipeLayout;
	// OpenGL handles for shaders and VBOs

	api::Sampler trilinear;
	api::Sampler nonMipmapped;

	api::DescriptorSet descSetAllObjects;
	DescriptorSets descSets;
	Pipelines pipelines;


	std::array<GnomeHordeTileThreadData, MAX_NUMBER_OF_THREADS> tileThreadData;
	std::array<GnomeHordeVisibilityThreadData, MAX_NUMBER_OF_THREADS> visibilityThreadData;

	std::array<std::array<TileInfo, NUM_TILES_X>, NUM_TILES_Z> tileInfos;
	MultiBuffering multiBuffering[MAX_NUMBER_OF_SWAP_IMAGES];

	std::array<std::thread, 16> threads;
	LineTasksQueue::ProducerToken lineQproducerToken;
	TileTasksQueue::ConsumerToken drawQconsumerToken;

	ApiObjects(LineTasksQueue& lineQ, TileTasksQueue& drawQ)
		: lineQproducerToken(lineQ.getProducerToken()), drawQconsumerToken(drawQ.getConsumerToken())
	{ }
};

class VulkanGnomeHorde : public Shell
{
public:
	std::deque<std::string> multiThreadLog;
	std::mutex logMutex;
	std::atomic<int32> itemsRemaining;
	std::atomic<int32> itemsToDraw;
	std::atomic<int32> itemsDrawn;
	std::atomic<int32> poisonPill; //Technique used to break threads out of their waiting

	uint32 numSwapImages;
	Meshes meshes;
	std::auto_ptr<ApiObjects> apiObj;
	LineTasksQueue linesToProcessQ;
	TileTasksQueue tilesToProcessQ;
	TileTasksQueue tilesToDrawQ;

	uint32 allLines[NUM_TILES_Z]; //Stores the line #. Used to kick initial work in the visibility threads
	//as each thread will be processing one line

	volatile glm::vec3 cameraPosition;
	volatile math::ViewingFrustum frustum;
	volatile uint8 swapIndex;

	bool isPaused;
	uint8 numVisibilityThreads;
	uint8 numTileThreads;
	VulkanGnomeHorde() : isPaused(false), numVisibilityThreads(0), numTileThreads(0)
	{
		for (int i = 0; i < NUM_TILES_Z; ++i)
		{
			allLines[i] = i;
		}
	}

	Result initApplication();
	Result initView();
	Result releaseView();
	Result quitApplication();
	Result renderFrame();

	void setUpUI();

	api::DescriptorSet createDescriptorSetUtil(const api::DescriptorSetLayout&, const StringHash& texture, const api::Sampler& mipMapped, const api::Sampler& nonMipMapped);
	//// HELPERS ////
	MeshLod loadLodMesh(const StringHash& filename, const StringHash& mesh, uint32_t max_lods);
	AppModeParameter calcAnimationParameters();
	void initUboStructuredObjects();
	void createDescSetsAndTiles(const api::DescriptorSetLayout& layoutImage, const api::DescriptorSetLayout& layoutPerObject, const api::DescriptorSetLayout& layoutPerFrameUbo);
	void kickReleaseCommandBuffers();
	void updateCameraUbo(const glm::mat4& matrix);

	void printLog()
	{
		std::unique_lock<std::mutex> lock(logMutex);
		while (!multiThreadLog.empty())
		{
			Log(Log.Information, multiThreadLog.front().c_str());
			multiThreadLog.pop_front();
		}
	}

	struct DemoDetails
	{
		// Time tracking
		float32 logicTime; //!< Total time that has elapsed for the application (Conceptual: Clock at start - Clock time now - Paused time)
		float32 gameTime; //!< Time that has elapsed for the application (Conceptual: Integration of logicTime * the demo's speed factor at each point)
		bool isManual;
		uint32_t currentMode;
		uint32_t previousMode;
		float modeSwitchTime;
		DemoDetails() : logicTime(0), gameTime(0), isManual(false), currentMode(0), previousMode(0), modeSwitchTime(0.f) {}
	} animDetails;
};

void GnomeHordeTileThreadData::garbageCollectPreviousFrameFreeCommandBuffers(uint8 swapIndex)
{
	auto& freeCmd = apiObj->freeCmdBuffers[swapIndex];
	auto& prefreeCmd = apiObj->preFreeCmdBuffers[swapIndex];

	std::move(prefreeCmd.begin(), prefreeCmd.end(), std::back_inserter(freeCmd));
	prefreeCmd.clear();
	if (freeCmd.size() > 10)
	{
		freeCmd.clear();
	}
}

api::SecondaryCommandBuffer GnomeHordeTileThreadData::getFreeCommandBuffer(uint8 swapIndex)
{
	{
		std::unique_lock<std::mutex> lock(cmdMutex);
		if (apiObj->lastSwapIndex != app->swapIndex)
		{
			apiObj->lastSwapIndex = app->swapIndex;
			garbageCollectPreviousFrameFreeCommandBuffers(app->swapIndex);
		}
	}

	api::SecondaryCommandBuffer retval;
	{
		std::unique_lock<std::mutex> lock(cmdMutex);
		if (!apiObj->freeCmdBuffers[swapIndex].empty())
		{
			retval = apiObj->freeCmdBuffers[swapIndex].back();
			apiObj->freeCmdBuffers[swapIndex].pop_back();
		}
	}
	if (retval.isNull())
	{
		retval = apiObj->cmdPools.back()->allocateSecondaryCommandBuffer();
		if (retval.isNull())
		{
			Log(Log.Error, "[THREAD %d] Command buffer allocation failed, . Trying to create additional command buffer pool.", id);
			apiObj->cmdPools.push_back(app->getGraphicsContext()->createCommandPool());
			retval = apiObj->cmdPools.back()->allocateSecondaryCommandBuffer();
			if (retval.isNull())
			{
				Log(Log.Critical, "COMMAND BUFFER ALLOCATION FAILED ON FRESH COMMAND POOL.");
			}
		}
	}
	return retval;
}

void GnomeHordeTileThreadData::freeCommandBuffer(const api::SecondaryCommandBuffer& cmdBuff, uint8 swapIndex)
{
	std::unique_lock<std::mutex> lock(cmdMutex);
	if (apiObj->lastSwapIndex != app->swapIndex)
	{
		apiObj->lastSwapIndex = app->swapIndex;
		garbageCollectPreviousFrameFreeCommandBuffers(app->swapIndex);
	}
	apiObj->preFreeCmdBuffers[swapIndex].push_back(cmdBuff);
}

bool GnomeHordeTileThreadData::doWork()
{
	int32 batch_size = 4;
	glm::ivec2 workItem[4];
	int32 result;
	if ((result = app->tilesToProcessQ.consume(apiObj->processQConsumerToken, workItem[0])))
	{
		generateTileBuffer(workItem, result);
	}
	return result != 0;
}

void GnomeHordeTileThreadData::generateTileBuffer(const ivec2* tileIdxs, uint32 numTiles)
{
	// Lambda to create the tiles secondary cmdbs
	utils::StructuredMemoryView& uboAllObj = app->apiObj->uboPerObject;
	const api::DescriptorSet& descSetAllObj = app->apiObj->descSetAllObjects;

	for (uint32 tilenum = 0; tilenum < numTiles; ++tilenum)
	{
		ivec2 tileId2d = tileIdxs[tilenum];
		uint32 x = tileId2d.x, y = tileId2d.y;
		uint32 tileIdx = y * NUM_TILES_X + x;

		TileInfo& tile = app->apiObj->tileInfos[y][x];

		// Recreate the cmdb
		for (uint32 swapIdx = 0; swapIdx < app->numSwapImages; ++swapIdx)
		{
			MultiBuffering& multi = app->apiObj->multiBuffering[swapIdx];
			tile.cbs[swapIdx] = getFreeCommandBuffer(swapIdx);
			tile.threadId = id;

			auto& cb = tile.cbs[swapIdx];

			cb->beginRecording(app->apiObj->fboOnScreen[swapIdx]);

			for (uint32_t objId = 0; objId < NUM_OBJECTS_PER_TILE; ++objId)
			{
				TileObject& obj = tile.objects[objId];
				uint32 lod = std::min<uint32>(static_cast<pvr::uint32>(obj.mesh->size()) - 1, tile.lod);

				//Can it NOT be different than before? - Not in this demo.
				cb->bindPipeline(obj.pipeline);

				Mesh& mesh = (*obj.mesh)[lod];

				uint32 offset = uboAllObj.getDynamicOffset(0, tileIdx * NUM_OBJECTS_PER_TILE + objId);

				// Use the right texture and position - TEXTURES PER OBJECT (Can optimize to object type)
				cb->bindDescriptorSet(app->apiObj->pipeLayout, 0, obj.set);
				cb->bindDescriptorSet(app->apiObj->pipeLayout, 1, descSetAllObj, &offset, 1);
				cb->bindDescriptorSet(app->apiObj->pipeLayout, 2, multi.descSetPerFrame, 0, 0);

				//If different than before?
				cb->bindVertexBuffer(mesh.vbo, 0, 0);
				cb->bindIndexBuffer(mesh.ibo, 0, mesh.mesh->getFaces().getDataType());

				// Offset in the per-object transformation matrices UBO - these do not change frame-to-frame
				//getArrayOffset, will return the actual byte offset of item #(first param) that is in an
				//array of items, at array index #(second param).
				cb->drawIndexed(0, mesh.mesh->getNumIndices());
			}
			cb->endRecording();
		}
		app->tilesToDrawQ.produce(apiObj->drawQProducerToken, tileId2d);
		++app->itemsToDraw;
		//Add the item to the "processed" queue and mark the count. If it's the last item, mark that the main
		//thread "must unblock"
		if (--app->itemsRemaining == 0)
		{
			//For good measure... The above will unblock the main Q, but even though it appears that the main thread will
			//be able to avoid it, we must still signal the "all done" for purposes of robustness.
			++app->poisonPill;
			app->tilesToDrawQ.unblockOne();
		}
	}
}

void GnomeHordeWorkerThread::addlog(const std::string& str)
{
	std::unique_lock<std::mutex> lock(app->logMutex);
	app->multiThreadLog.push_back(str);
}

void GnomeHordeWorkerThread::run()
{
	addlog(strings::createFormatted("=== Tile Visibility Thread [%d] ===            Starting", id));
	running = true;
	while (doWork()) { continue; } // grabs a piece of work as long as the queue is not empty.
	running = false;
	addlog(strings::createFormatted("=== Tile Visibility Thread [%d] ===            Exiting", id));
}

bool GnomeHordeVisibilityThreadData::doWork()
{
	int32 batch_size = 4;
	int32 workItem[4];
	int32 result;
	if ((result = app->linesToProcessQ.consume(apiObj->linesQConsumerToken, workItem[0])))
	{
		determineLineVisibility(workItem, result);
	}
	return result != 0;
}

void GnomeHordeVisibilityThreadData::determineLineVisibility(const int32* lineIdxs, uint32 numLines)
{
	auto& tileInfos = app->apiObj->tileInfos;
	//Local temporaries of the "global volatile" visibility variables. Memcpy used as they are declared "volatile"
	//It is perfectly fine to use memcopy for these volatiles AT THIS TIME because we know certainly that MAIN thread
	//has finished writing to them some time now and no race condition is possible (the calculations happen before the threads)
	math::ViewingFrustum frustum; utils::memCopyFromVolatile(frustum, app->frustum);
	glm::vec3 camPos; utils::memCopyFromVolatile(camPos, app->cameraPosition);

	TileTasksQueue& processQ = app->tilesToProcessQ;
	TileTasksQueue& drawQ = app->tilesToDrawQ;

	uint8 numSwapImages = app->numSwapImages;
	for (uint32 line = 0; line < numLines; ++line)
	{
		glm::ivec2 id2d(0, lineIdxs[line]);
		for (id2d.x = 0; id2d.x < NUM_TILES_X; ++id2d.x)
		{
			tileInfos[id2d.y][id2d.x].visibility = math::aabbInFrustum(tileInfos[id2d.y][id2d.x].aabb, frustum);

			TileInfo& tile = tileInfos[id2d.y][id2d.x];

			// Compute tile lod
			float dist = glm::distance(tile.aabb.center(), camPos);
			float d = glm::max((dist - 400.0f) / 20.0f, 0.0f);
			float flod = glm::max<float>(sqrtf(d) - 2.0f, 0.0f);
			tile.lod = static_cast<uint32_t>(flod);

			if (tile.visibility != tile.oldVisibility || tile.lod != tile.oldLod) // The tile has some change. Will need to do something.
			{
				for (uint32 i = 0; i < numSwapImages; ++i) //First, free its pre-existing command buffers (just mark free)
				{
					if (tile.cbs[i].isValid())
					{
						app->apiObj->tileThreadData[tile.threadId].freeCommandBuffer(tile.cbs[i], i);
						tile.cbs[i].reset();
					}
				}

				//// PRODUCER CONSUMER QUEUE ////
				if (tile.visibility) //If the tile is visible, it will need to be generated.
				{
					// COMMAND BUFFER GENERATION BEGINS ** IMMEDIATELY ** on a worker thread
					processQ.produce(apiObj->processQproducerToken, id2d);
					//// PRODUCE ///
					//The producer thread must signal the unblock...
				}
				//Otherwise, no further action is required.
			}
			else if (tile.visibility) // Tile had no change, but was visible - just add it to the drawing queue.
			{
				++app->itemsToDraw;
				drawQ.produce(apiObj->drawQproducerToken, id2d);
				//Add the item to the "processed" queue and mark the count. If it's the last item, mark that the main
				//thread "must unblock"
				if (--app->itemsRemaining == 0)
				{
					//Signal that we have unblocked the main thread
					++app->poisonPill;
					drawQ.unblockOne();
				}
			}

			tile.oldVisibility = tile.visibility;
			tile.oldLod = tile.lod;

			if (!tile.visibility)
			{
				//Remove the item from the total expected number of items if it was not visible
				//If it was the last one, make sure the main thread is not blocked forever. Since there is an actual race
				//condition (but we wanted to avoid using very expensive synchronization for the exit condition), we are
				//just making sure the main thread will not block forever (In a sort of "poison pill" technique - we are
				//in a sense putting an item in the queue that will tell the main thread to stop).
				if (--app->itemsRemaining == 0)
				{
					//Make sure the main thread does not remain blocked forever
					++app->poisonPill;
					drawQ.unblockOne();
				}
			}
		}
	}

}

///// UTILS (local) /////
inline vec3 getTrackPosition(float32 time, const vec3& world_size)
{
	const float32 angle = time * 0.02f;
	const vec3 centre = world_size * 0.5f;
	const vec3 radius = world_size * 0.2f;
	// Main circle
	float a1 = time * 0.07f;
	float a2 = time * 0.1f;
	float a3 = angle;

	const float32 h = glm::sin(a1) * 15.f + 100.f;
	const float32 radius_factor = 0.95f + 0.1f * glm::sin(a2);
	const glm::vec3 circle(glm::sin(a3)*radius.x * radius_factor, h, glm::cos(a3)*radius.z * radius_factor);

	return centre + circle;
}

float32 initializeGridPosition(std::vector<float>& grid, uint32 numItemsPerRow)
{
	//| x | x | x |
	//| x | x | x |
	//| x | x | x |
	//Jittered Grid - each object is placed on the center of a normal grid, and then moved randomly around.
	const float32 MIN_DISTANCE_FACTOR = -.2f; // Minimum item distance is 1/5 th their starting distance

	grid.resize(numItemsPerRow);
	float32 distance = 1.f / numItemsPerRow;
	grid[0] = .5f * distance;
	for (uint32 i = 1; i < numItemsPerRow; ++i)
	{
		grid[i] = grid[i - 1] + distance;
	}
	float32 deviation = distance * .5f * (1.f - MIN_DISTANCE_FACTOR);
	return deviation;
}

inline void generatePositions(vec3* points, vec3 minBound, vec3 maxBound)
{
	static std::vector<float> normalGridPositions;
	static const uint32 numItemsPerRow = (uint32)(sqrtf(NUM_UNIQUE_OBJECTS_PER_TILE));
	static const float deviation = initializeGridPosition(normalGridPositions, numItemsPerRow);

	for (uint32 y = 0; y < numItemsPerRow; ++y)
	{
		for (uint32 x = 0; x < numItemsPerRow; ++x)
		{
			vec3 pos(normalGridPositions[x] + deviation * randomrange(-1.f, 1.f),
			         0.f,
			         normalGridPositions[y] + deviation * randomrange(-1.f, 1.f));
			vec3 mixed = glm::mix(minBound, maxBound, pos);

			points[y * numItemsPerRow + x] = vec3(mixed.x, mixed.y, mixed.z);
		}
	}
	for (int i = numItemsPerRow * numItemsPerRow; i < NUM_UNIQUE_OBJECTS_PER_TILE; ++i)
	{
		points[i] = glm::mix(minBound, maxBound, vec3(randomrange(-1.f, 1.f), 0, randomrange(-1.f, 1.f)));
	}
}


/////////// CLASS VulkanGnomeHorde ///////////
///// CALLBACKS - IMPLEMENTATION OF pvr::Shell /////
Result VulkanGnomeHorde::initApplication()
{
	int num_cores = (int)std::thread::hardware_concurrency();
	int THREAD_FACTOR_RELAXATION = 1;

	int thread_factor = std::max(num_cores - THREAD_FACTOR_RELAXATION, 1);

	numVisibilityThreads = std::min(thread_factor, (int)MAX_NUMBER_OF_THREADS);
	numTileThreads = std::min(thread_factor, (int)MAX_NUMBER_OF_THREADS);
	Log(Log.Information,
	    "Hardware concurreny reported: %u cores. Enabling %u visibility threads plus %u tile processing threads\n",
	    num_cores, numVisibilityThreads, numTileThreads);

	// Meshes
	meshes.gnome = loadLodMesh(StringHash("gnome"), "body", 7);
	meshes.gnomeShadow = loadLodMesh("gnome_shadow", "Plane001", 1);
	meshes.fern = loadLodMesh("fern", "Plane006", 1);
	meshes.fernShadow = loadLodMesh("fern_shadow", "Plane001", 1);
	meshes.mushroom = loadLodMesh("mushroom", "Mushroom1", 2);
	meshes.mushroomShadow = loadLodMesh("mushroom_shadow", "Plane001", 1);
	meshes.bigMushroom = loadLodMesh("bigMushroom", "Mushroom1", 1);
	meshes.bigMushroomShadow = loadLodMesh("bigMushroom_shadow", "Plane001", 1);
	meshes.rock = loadLodMesh("rocks", "rock5", 1);

	return Result::Success;
}

Result VulkanGnomeHorde::quitApplication()
{
	meshes.clearAll();
	return Result::Success;
}

void VulkanGnomeHorde::setUpUI()
{
	apiObj->uiRenderer.init(apiObj->fboOnScreen[0]->getRenderPass(), 0);

	apiObj->uiRenderer.getDefaultTitle()->setText("Gnome Horde");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	apiObj->uiRenderer.getDefaultDescription()->setText("Multithreaded command buffer generation and rendering");
	apiObj->uiRenderer.getDefaultDescription()->commitUpdates();

	for (uint32 i = 0; i < numSwapImages; ++i)
	{
		apiObj->multiBuffering[i].cmdBufferUI = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
		//UIRenderer - the easy stuff first, but we still must create one command buffer per frame.

		apiObj->uiRenderer.beginRendering(apiObj->multiBuffering[i].cmdBufferUI);
		apiObj->uiRenderer.getDefaultTitle()->render();
		apiObj->uiRenderer.getDefaultDescription()->render();
		apiObj->uiRenderer.getSdkLogo()->render();
		apiObj->uiRenderer.endRendering();
	}
}

Result VulkanGnomeHorde::initView()
{
	apiObj.reset(new ApiObjects(linesToProcessQ, tilesToDrawQ));
	apiObj->assetManager.init(*this);

	numSwapImages = getSwapChainLength();
	GraphicsContext& ctx = apiObj->context = getGraphicsContext();
	apiObj->fboOnScreen = apiObj->context->createOnScreenFboSet();

	setUpUI();

	for (uint32 i = 0; i < numSwapImages; ++i)
	{
		apiObj->multiBuffering[i].commandBuffer = ctx->createCommandBufferOnDefaultPool();
		apiObj->multiBuffering[i].fence = ctx->createFence(true);
	}

	initUboStructuredObjects();

	//Create Descriptor set layouts
	api::DescriptorSetLayoutCreateParam imageDescParam;
	imageDescParam.setBinding(0, types::DescriptorType::CombinedImageSampler, 1, types::ShaderStageFlags::Fragment);
	api::DescriptorSetLayout descLayoutImage = ctx->createDescriptorSetLayout(imageDescParam);

	api::DescriptorSetLayoutCreateParam dynamicUboDescParam;
	dynamicUboDescParam.setBinding(0, types::DescriptorType::UniformBufferDynamic, 1, types::ShaderStageFlags::Vertex);
	api::DescriptorSetLayout descLayoutUboDynamic = ctx->createDescriptorSetLayout(dynamicUboDescParam);

	api::DescriptorSetLayoutCreateParam uboDescParam;
	uboDescParam.setBinding(0, types::DescriptorType::UniformBuffer, 1, types::ShaderStageFlags::Vertex);
	api::DescriptorSetLayout descLayoutUboStatic = ctx->createDescriptorSetLayout(uboDescParam);

	//Create Pipelines
	{
		apiObj->pipeLayout = ctx->createPipelineLayout(
		                       api::PipelineLayoutCreateParam()
		                       .setDescSetLayout(0, descLayoutImage)
		                       .setDescSetLayout(1, descLayoutUboDynamic)
		                       .setDescSetLayout(2, descLayoutUboStatic));

		// Must not assume the cache will always work

		api::Shader objectVsh = ctx->createShader(*getAssetStream("Object.vsh.spv"), types::ShaderType::VertexShader);
		api::Shader shadowVsh = ctx->createShader(*getAssetStream("Shadow.vsh.spv"), types::ShaderType::VertexShader);
		api::Shader solidFsh = ctx->createShader(*getAssetStream("Solid.fsh.spv"), types::ShaderType::FragmentShader);
		api::Shader shadowFsh = ctx->createShader(*getAssetStream("Shadow.fsh.spv"), types::ShaderType::FragmentShader);
		api::Shader premulFsh = ctx->createShader(*getAssetStream("Plant.fsh.spv"), types::ShaderType::FragmentShader);

		api::GraphicsPipelineCreateParam pipeCreate;
		types::BlendingConfig cbStateNoBlend;
		types::BlendingConfig cbStateBlend(true, types::BlendFactor::OneMinusSrcAlpha, types::BlendFactor::SrcAlpha, types::BlendOp::Add);
		types::BlendingConfig cbStatePremulAlpha(true, types::BlendFactor::One, types::BlendFactor::OneMinusSrcAlpha, types::BlendOp::Add);

		utils::createInputAssemblyFromMesh(*meshes.gnome[0].mesh, &attributeBindings[0], 3, pipeCreate);
		pipeCreate.rasterizer.setFrontFaceWinding(types::PolygonWindingOrder::FrontFaceCCW);
		pipeCreate.rasterizer.setCullFace(types::Face::Back);
		pipeCreate.depthStencil.setDepthTestEnable(true);
		pipeCreate.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::Less);
		pipeCreate.depthStencil.setDepthWrite(true);
		pipeCreate.renderPass = apiObj->fboOnScreen[0]->getRenderPass();
		pipeCreate.pipelineLayout = apiObj->pipeLayout;

		// create the solid pipeline
		pipeCreate.vertexShader = objectVsh;
		pipeCreate.fragmentShader = solidFsh;
		pipeCreate.colorBlend.setAttachmentState(0, cbStateNoBlend);
		if ((apiObj->pipelines.solid = ctx->createGraphicsPipeline(pipeCreate)).isNull())
		{
			setExitMessage("Failed to create Opaque rendering pipeline");
			return Result::UnknownError;
		}

		pipeCreate.depthStencil.setDepthWrite(false);
		// create the alpha pre-multiply pipeline
		pipeCreate.vertexShader = objectVsh;
		pipeCreate.fragmentShader = premulFsh;
		pipeCreate.colorBlend.setAttachmentState(0, cbStatePremulAlpha);
		if ((apiObj->pipelines.alphaPremul = ctx->createGraphicsPipeline(pipeCreate)).isNull())
		{
			setExitMessage("Failed to create Premultiplied Alpha rendering pipeline");
			return Result::UnknownError;
		}

		// create the shadow pipeline
		pipeCreate.colorBlend.setAttachmentState(0, cbStateBlend);
		pipeCreate.vertexShader = shadowVsh;
		pipeCreate.fragmentShader = shadowFsh;
		if ((apiObj->pipelines.shadow = ctx->createGraphicsPipeline(pipeCreate)).isNull())
		{
			setExitMessage("Failed to create Shadow rendering pipeline");
			return Result::UnknownError;
		}
	}

	createDescSetsAndTiles(descLayoutImage, descLayoutUboDynamic, descLayoutUboStatic);

	animDetails.logicTime = 0.f;
	animDetails.gameTime = 0.f;
	{
		for (uint32 i = 0; i < numVisibilityThreads; ++i)
		{
			apiObj->visibilityThreadData[i].id = i;
			apiObj->visibilityThreadData[i].app = this;
			apiObj->visibilityThreadData[i].apiObj.reset(new GnomeHordeVisibilityThreadData::ApiObjects(linesToProcessQ, tilesToProcessQ, tilesToDrawQ));
			apiObj->visibilityThreadData[i].thread = std::thread(&GnomeHordeVisibilityThreadData::run, (GnomeHordeVisibilityThreadData*)&apiObj->visibilityThreadData[i]);
		}
		for (uint32 i = 0; i < numTileThreads; ++i)
		{
			apiObj->tileThreadData[i].id = i;
			apiObj->tileThreadData[i].app = this;
			apiObj->tileThreadData[i].apiObj.reset(new GnomeHordeTileThreadData::ApiObjects(tilesToProcessQ, tilesToDrawQ));
			apiObj->tileThreadData[i].apiObj->cmdPools.clear();
			apiObj->tileThreadData[i].apiObj->cmdPools.push_back(ctx->createCommandPool());
			apiObj->tileThreadData[i].thread = std::thread(&GnomeHordeTileThreadData::run, (GnomeHordeTileThreadData*)&apiObj->tileThreadData[i]);
		}
	}
	printLog();
	return Result::Success;
}

Result VulkanGnomeHorde::releaseView()
{
	Log(Log.Information, "Signalling all worker threads: Signal drain empty queues...");
	//Done will allow the queue to finish its work if it has any, but then immediately
	//afterwards it will free any and all threads waiting. Any threads attempting to
	//dequeue work from the queue will immediately return "false".
	linesToProcessQ.done();
	tilesToProcessQ.done();
	tilesToDrawQ.done();

	//waitIdle is being called to make sure the command buffers we will be destroying
	//are not being referenced.
	getGraphicsContext()->waitIdle();

	Log(Log.Information, "Joining all worker threads...");

	//Finally, tear down everything.
	for (uint32 i = 0; i < numVisibilityThreads; ++i)
	{
		apiObj->visibilityThreadData[i].thread.join();
	}
	for (uint32 i = 0; i < numTileThreads; ++i)
	{
		apiObj->tileThreadData[i].thread.join();
	}

	//Clear all objects. This will also free the command buffers that were allocated
	//from the worker thread's command pools, but are currently only held by the
	//tiles.
	apiObj.reset();
	meshes.clearApiObjects();

	Log(Log.Information, "All worker threads done!");
	return Result::Success;
}

Result VulkanGnomeHorde::renderFrame()
{
	float dt = getFrameTime() * 0.001f;
	animDetails.logicTime += dt;
	if (animDetails.logicTime > 10000000) { animDetails.logicTime = 0; }

	// Get the next free swapchain image
	// We have implemented the application so that cmdb[0,1,2] points to swapchain fb[0,1,2]
	// so we must submit the cmdb that this index points to.
	// Applications that generate cmdbs on the fly may not need to do this.
	swapIndex = getSwapChainIndex();

	//Interpolate frame parameters
	AppModeParameter parameters = calcAnimationParameters();

	animDetails.gameTime += dt * parameters.speedFactor;
	if (animDetails.gameTime > 10000000) { animDetails.gameTime = 0; }

	const vec3 worldSize = vec3(TILE_SIZE_X + TILE_GAP_X, TILE_SIZE_Y, TILE_SIZE_Z + TILE_GAP_Z) * vec3(NUM_TILES_X, 1, NUM_TILES_Z);
	vec3 camPos = getTrackPosition(animDetails.gameTime, worldSize);
	//cameraPosition is also used by the visibility threads. The "volatile" variable is to make sure it is visible to the threads
	//we will be starting in a bit. For the moment, NO concurrent access happens (as the worker threads are inactive).
	utils::memCopyToVolatile(cameraPosition, camPos);
	vec3 camTarget = getTrackPosition(animDetails.gameTime + parameters.cameraForwardOffset, worldSize) + vec3(10.f, 10.f, 10.f);
	camTarget.y = 0.f;
	camPos.y += parameters.cameraHeightOffset;

	vec3 camUp = vec3(0.f, 1.f, 0.f);

	mat4 cameraMat = math::perspectiveFov(pvr::Api::Vulkan, 1.1f, (pvr::float32)getWidth(), (pvr::float32)getHeight(), 10.0f, 5000.f) *
	                 glm::lookAt(camPos, camTarget, camUp);

	mat4 cameraMat2 = math::perspectiveFov(pvr::Api::Vulkan, 1.2f, (pvr::float32)getWidth(), (pvr::float32)getHeight(), 10.0f, 5000.f) *
	                  glm::lookAt(camPos, camTarget, camUp);

	updateCameraUbo(cameraMat);

	math::ViewingFrustum frustumTmp;
	math::getFrustumPlanes(cameraMat2, frustumTmp);
	utils::memCopyToVolatile(frustum, frustumTmp);

	itemsRemaining.store(NUM_TILES_X * NUM_TILES_Z);
	itemsToDraw.store(0);
	itemsDrawn.store(0);
	poisonPill.store(0);

	linesToProcessQ.produceMultiple(apiObj->lineQproducerToken, allLines, NUM_TILES_Z);

	auto& cb = apiObj->multiBuffering[swapIndex].commandBuffer;
	//if (tilesChanged)
	{
		cb->beginRecording();
		cb->beginRenderPass(apiObj->fboOnScreen[swapIndex], Rectanglei(0, 0, getWidth(), getHeight()), false, glm::vec4(.205f, 0.3f, 0.05f, 1.f));
	}
	// Check culling and push the secondary cmdbs on to the primary cmdb
	enum ItemsTotal {itemsTotal = NUM_TILES_X * NUM_TILES_Z};

	cb->enqueueSecondaryCmds_BeginMultiple(255);
	//MAIN RENDER LOOP - Collect the work (tiles) as it is processed
	{
		// Consume extra CB's as they become ready
		uint32 result;
		glm::ivec2 tileId[256];

		uint32 loop = 0;

		//We need some rather complex safeguards to make sure this thread does not wait forever.
		//First - we must (using atomics) make sure that when we say we are done, i.e. no items are unaccounted for.
		//Second, for the case where the main thread is waiting, but all remaining items are not visible, the last thread
		//to process an item will trigger an additional "unblock" to the main thread.
		while (itemsDrawn != itemsToDraw || itemsRemaining)
		{

			if ((itemsDrawn > itemsToDraw) && !itemsRemaining)
			{
				if ((result == 0) && (loop > 0)) //NOT THE FIRST TIME?
				{
					Log(Log.Error, "Blocking is not released");
					poisonPill.store(0);
					break;
				}
			}


			result = (uint32)tilesToDrawQ.consumeMultiple(apiObj->drawQconsumerToken, tileId, 256);
			if (!result) { --poisonPill; }
			itemsDrawn += result;
			for (uint32 i = 0; i < result; ++i)
			{
				cb->enqueueSecondaryCmds_EnqueueMultiple(&(apiObj->tileInfos[tileId[i].y][tileId[i].x].cbs[swapIndex]), 1);
			}
		}

		//The uirenderer should always be drawn last as it has (and checks) no depth
		cb->enqueueSecondaryCmds_EnqueueMultiple(&apiObj->multiBuffering[swapIndex].cmdBufferUI, 1);

		cb->enqueueSecondaryCmds_SubmitMultiple(); //SUBMIT THE WORK!
		if (poisonPill >= 0)
		{
			while (poisonPill--)
			{
				tilesToDrawQ.consume(apiObj->drawQconsumerToken, tileId[255]); //Make sure it is in a consistent state
			}
		}
		else
		{
			Log(Log.Error, "poisonPill is less than 0");
		}
	}

	assertion(linesToProcessQ.isEmpty(), "Initial Line Processing Queue was not empty after work done!");
	assertion(tilesToProcessQ.isEmpty(), "Worker Tile Processing Queue was not empty after work done!");

	//We do not need any additional syncing - we know all dispatched work is done.

	cb->endRenderPass();
	cb->endRecording();
	cb->submit();
	printLog();

	return Result::Success;
}

void VulkanGnomeHorde::kickReleaseCommandBuffers()
{
}

void VulkanGnomeHorde::updateCameraUbo(const glm::mat4& matrix)
{
	apiObj->multiBuffering[swapIndex].uboPerFrame.map(0);
	apiObj->multiBuffering[swapIndex].uboPerFrame.setValue(0, matrix);
	apiObj->multiBuffering[swapIndex].uboPerFrame.unmap(0);
}

void VulkanGnomeHorde::createDescSetsAndTiles(const api::DescriptorSetLayout& layoutImage, const api::DescriptorSetLayout& layoutPerObject,
    const api::DescriptorSetLayout& layoutPerFrameUbo)
{
	GraphicsContext& ctx = apiObj->context;
	{
		//The objects could have been completely different - the fact that there are only a handful of different
		//objects is coincidental and does not affect the demo.
		auto& trilinear = apiObj->trilinear = ctx->createSampler(api::SamplerCreateParam(types::SamplerFilter::Linear, types::SamplerFilter::Linear, types::SamplerFilter::Linear));
		auto& nonMipmapped = apiObj->nonMipmapped = ctx->createSampler(api::SamplerCreateParam(types::SamplerFilter::Linear, types::SamplerFilter::Linear, types::SamplerFilter::None));

		apiObj->descSets.gnome = createDescriptorSetUtil(layoutImage, "gnome_texture.pvr", trilinear, nonMipmapped);
		apiObj->descSets.gnomeShadow = createDescriptorSetUtil(layoutImage, "gnome_shadow.pvr", trilinear, nonMipmapped);
		apiObj->descSets.rock = createDescriptorSetUtil(layoutImage, "rocks.pvr", trilinear, nonMipmapped);
		apiObj->descSets.fern = createDescriptorSetUtil(layoutImage, "fern.pvr", trilinear, nonMipmapped);
		apiObj->descSets.fernShadow = createDescriptorSetUtil(layoutImage, "fern_shadow.pvr", trilinear, nonMipmapped);
		apiObj->descSets.mushroom = createDescriptorSetUtil(layoutImage, "mushroom_texture.pvr", trilinear, nonMipmapped);
		apiObj->descSets.mushroomShadow = createDescriptorSetUtil(layoutImage, "mushroom_shadow.pvr", trilinear, nonMipmapped);
		apiObj->descSets.bigMushroom = createDescriptorSetUtil(layoutImage, "bigMushroom_texture.pvr", trilinear, nonMipmapped);
		apiObj->descSets.bigMushroomShadow = createDescriptorSetUtil(layoutImage, "bigMushroom_shadow.pvr", trilinear, nonMipmapped);
	}


	//The ::pvr::utils::StructuredMemoryView is a simple class that allows us easy access to update members of a buffer - it keeps track of offsets,
	//datatypes and sizes of items in the buffer, allowing us to update them very easily. The connectWithBuffer method allows us to call the map/unmap functions
	//directly on this object. In this case it will also help us with the array stride etc.

	//The uboPerObject is one huge DynamicUniformBuffer, whose data is STATIC, and contains the object Model->World matrixes
	//A different bit of this buffer is bound for each and every object.

	//CAUTION: The Range of the Buffer View for a Dynamic Uniform Buffer must be the BINDING size, not the TOTAL size, i.e. the size
	//of the part of the buffer that will be bound each time, not the total size. That is why we cannot do a one-step creation (...createBufferAndView) like
	//for static UBOs.
	apiObj->uboPerObject.connectWithBuffer(
	  0, ctx->createBufferView(
	    ctx->createBuffer(apiObj->uboPerObject.getAlignedTotalSize(), types::BufferBindingUse::UniformBuffer, true),
	    0, apiObj->uboPerObject.getAlignedElementSize()));

	apiObj->descSetAllObjects = ctx->createDescriptorSetOnDefaultPool(layoutPerObject);
	apiObj->descSetAllObjects->update(api::DescriptorSetUpdate().setDynamicUbo(0, apiObj->uboPerObject.getConnectedBuffer(0)));

	for (uint32 i = 0; i < numSwapImages; ++i)
	{
		//The uboPerFrame is a small UniformBuffer that contains the camera (World->Projection) matrix. Since it is updated every frame, it is multi-buffered to avoid
		//stalling the GPU
		auto& current = apiObj->multiBuffering[i];
		current.descSetPerFrame = ctx->createDescriptorSetOnDefaultPool(layoutPerFrameUbo);
		current.uboPerFrame.connectWithBuffer(0, ctx->createBufferAndView(current.uboPerFrame.getAlignedElementSize(),
		                                      types::BufferBindingUse::UniformBuffer, true));
		current.descSetPerFrame->update(api::DescriptorSetUpdate().setUbo(0, current.uboPerFrame.getConnectedBuffer(0)));
	}
	//Create the UBOs/VBOs for the main objects. This automatically creates the VBOs.
	meshes.createApiObjects(ctx);

	//Using the StructuredMemoryView to update the objects
	utils::StructuredMemoryView& perObj = apiObj->uboPerObject;
	uint32 mvIndex = perObj.getIndex("modelView");
	uint32 mvITIndex = perObj.getIndex("modelViewIT");

	perObj.mapMultipleArrayElements(0, 0, TOTAL_NUMBER_OF_OBJECTS, types::MapBufferFlags::Write);

	for (uint32 y = 0; y < NUM_TILES_Z; ++y)
	{
		for (uint32 x = 0; x < NUM_TILES_X; ++x)
		{
			vec3 tileBL(x * (TILE_SIZE_X + TILE_GAP_Z), TILE_SIZE_Y, y * (TILE_SIZE_Z + TILE_GAP_Z));
			vec3 tileTR = tileBL + vec3(TILE_SIZE_X, 0, TILE_SIZE_Z);

			TileInfo& thisTile = apiObj->tileInfos[y][x];

			thisTile.aabb.setMinMax(tileBL, tileTR);

			thisTile.visibility = false;
			thisTile.lod = uint8(0xFFu);
			thisTile.oldVisibility = false;
			thisTile.oldLod = (uint8)0xFFu - (uint8)1;


			thisTile.objects[0].mesh = &meshes.gnome;
			thisTile.objects[0].set = apiObj->descSets.gnome;
			thisTile.objects[0].pipeline = apiObj->pipelines.solid;

			thisTile.objects[1].mesh = &meshes.gnomeShadow;
			thisTile.objects[1].set = apiObj->descSets.gnomeShadow;
			thisTile.objects[1].pipeline = apiObj->pipelines.shadow;

			thisTile.objects[2].mesh = &meshes.mushroom;
			thisTile.objects[2].set = apiObj->descSets.mushroom;
			thisTile.objects[2].pipeline = apiObj->pipelines.solid;

			thisTile.objects[3].mesh = &meshes.mushroomShadow;
			thisTile.objects[3].set = apiObj->descSets.mushroomShadow;
			thisTile.objects[3].pipeline = apiObj->pipelines.shadow;

			thisTile.objects[4].mesh = &meshes.bigMushroom;
			thisTile.objects[4].set = apiObj->descSets.bigMushroom;
			thisTile.objects[4].pipeline = apiObj->pipelines.solid;

			thisTile.objects[5].mesh = &meshes.bigMushroomShadow;
			thisTile.objects[5].set = apiObj->descSets.bigMushroomShadow;
			thisTile.objects[5].pipeline = apiObj->pipelines.shadow;

			thisTile.objects[7].mesh = &meshes.fernShadow;
			thisTile.objects[7].set = apiObj->descSets.fernShadow;
			thisTile.objects[7].pipeline = apiObj->pipelines.shadow;

			thisTile.objects[6].mesh = &meshes.fern;
			thisTile.objects[6].set = apiObj->descSets.fern;
			thisTile.objects[6].pipeline = apiObj->pipelines.alphaPremul;

			thisTile.objects[8].mesh = &meshes.rock;
			thisTile.objects[8].set = apiObj->descSets.rock;
			thisTile.objects[8].pipeline = apiObj->pipelines.solid;

			std::array<vec3, NUM_UNIQUE_OBJECTS_PER_TILE> points;
			generatePositions(points.data(), tileBL, tileTR);
			uint32 tileBaseIndex = (y * NUM_TILES_X + x) * NUM_OBJECTS_PER_TILE;

			for (uint32 halfobj = 0; halfobj < NUM_UNIQUE_OBJECTS_PER_TILE; ++halfobj)
			{
				uint32 obj = halfobj * 2;
				uint32 objShadow = obj + 1;
				// Note: do not put these in-line with the function call because it seems that the nexus player
				// swaps around the order that the parameters are evaluated compared to desktop
				float32 rot = randomrange(-glm::pi<float32>(), glm::pi<float32>());
				float32 s = randomrange(.8, 1.3);

				vec3 position = points[halfobj];
				mat4 rotation = glm::rotate(rot, vec3(0.0f, 1.0f, 0.0f));
				mat4 scale = glm::scale(vec3(s));
				mat4 xform = glm::translate(position) * rotation * scale;
				mat4 xformIT = glm::transpose(glm::inverse(xform));

				perObj.getDynamicOffset(mvITIndex, tileBaseIndex + obj);
				perObj.setArrayValue(mvIndex, tileBaseIndex + obj, xform);
				perObj.setArrayValue(mvITIndex, tileBaseIndex + obj, xformIT);

				if (objShadow != 9)
				{
					perObj.setArrayValue(mvIndex, tileBaseIndex + objShadow, xform);
					perObj.setArrayValue(mvITIndex, tileBaseIndex + objShadow, xformIT);
				}

			}
		}
	}
	perObj.unmap(0);
}

MeshLod VulkanGnomeHorde::loadLodMesh(const StringHash& filename, const StringHash& mesh, uint32_t num_lods)
{
	MeshLod meshLod;
	meshLod.resize(num_lods);

	for (uint32_t i = 0; i < num_lods; ++i)
	{
		std::stringstream ss;
		ss << i;
		ss << ".pod";

		std::string path = filename.str() + ss.str();
		Log(Log.Information, "Loading model:%s mesh:%s\n", path.c_str(), mesh.c_str());
		pvr::assets::ModelHandle model;
		Stream::ptr_type str = getAssetStream(path);

		if ((model = pvr::assets::Model::createWithReader(pvr::assets::PODReader(str))).isNull())
		{
			assertion(false, strings::createFormatted("Failed to load model file %s", path.c_str()).c_str());
		}
		for (uint32 j = 0; j < model->getNumMeshNodes(); ++j)
		{
			if (model->getMeshNode(j).getName() == mesh)
			{
				uint32 meshId = model->getMeshNode(j).getObjectId();
				meshLod[i].mesh = assets::getMeshHandle(model, meshId);
				break;
			}
			if (j == model->getNumMeshNodes()) { assertion(false, strings::createFormatted("Could not find mesh %s in model file %s", mesh.c_str(), path.c_str()).c_str()); }
		}
	}
	return meshLod;
}

api::DescriptorSet VulkanGnomeHorde::createDescriptorSetUtil(const api::DescriptorSetLayout& layout, const StringHash& texture, const api::Sampler& mipMapped, const api::Sampler& nonMipMapped)
{
	api::DescriptorSet tmp = apiObj->context->createDescriptorSetOnDefaultPool(layout);
	api::TextureView tex;
	apiObj->assetManager.getTextureWithCaching(apiObj->context, texture, &tex, NULL);
	bool hasMipmaps = tex->getResource()->getFormat().mipmapLevels > 1;
	api::DescriptorSetUpdate write;
	write.setCombinedImageSampler(0, tex, hasMipmaps ? mipMapped : nonMipMapped);
	tmp->update(write);
	return tmp;
}

void VulkanGnomeHorde::initUboStructuredObjects()
{
	for (uint32 i = 0; i < numSwapImages; ++i)
	{
		apiObj->multiBuffering[i].uboPerFrame.addEntryPacked("projectionMat", types::GpuDatatypes::mat4x4);
		apiObj->multiBuffering[i].uboPerFrame.finalize(getGraphicsContext(), 1, types::BufferBindingUse::UniformBuffer);
	}
	apiObj->uboPerObject.addEntryPacked("modelView", types::GpuDatatypes::mat4x4);
	apiObj->uboPerObject.addEntryPacked("modelViewIT", types::GpuDatatypes::mat4x4);
	apiObj->uboPerObject.finalize(getGraphicsContext(), TOTAL_NUMBER_OF_OBJECTS, types::BufferBindingUse::UniformBuffer, true, false);
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
			Log(Log.Information, "Switching to mode: [%d]", animDetails.currentMode);
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

std::auto_ptr<pvr::Shell> pvr::newDemo()
{
	return std::auto_ptr<pvr::Shell>(new VulkanGnomeHorde);
}
