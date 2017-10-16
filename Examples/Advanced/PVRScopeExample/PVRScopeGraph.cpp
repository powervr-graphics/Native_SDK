/*!*********************************************************************************************************************
\File         PVRScopeGraph.cpp
\Title
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
***********************************************************************************************************************/
#include "PVRScopeStats.h"
#include "PVRScopeGraph.h"
#include "PVRCore/IO/BufferStream.h"
#include "PVRAssets/Shader.h"
#include <math.h>
#include <string.h>
using namespace pvr;

glm::vec4 ColorTable[] =
{
	{ 0.0, 0.0, 1.0, 1. }, //0
	{ 1.0, 0.0, 0.0, 1. }, //1
	{ 0.0, 1.0, 0.0, 1. }, //2
	{ .80, 0.6, 0.0, 1. }, //3
	{ .80, 0.0, 0.5, 1. }, //4
	{ .00, .50, .30, 1. }, //5
	{ .50, .00, .80, 1. }, //6
	{ .00, .00, .00, 1. }, //7
	{ .70, .00, .00, 1. }, //8
	{ .00, .80, .00, 1. }, //9
	{ .00, .00, .80, 1. }, //10
	{ .80, .30, .0,  1. }, //11
	{ .00, .50, .50, 1. }, //12
	{ .50, .00, .00, 1. }, //13
	{ .00, .50, .00, 1. }, //14
	{ .00, .00, .50, 1. }, //15
	{ .30, .60, 0.0, 1. }, //16
	{ .00, .50, .80, 1. }, //17

	{ 0.5, 0.5, 0.5, 1. }
};
enum {ColorTableSize = ARRAY_SIZE(ColorTable) };
namespace Configuration {
const char* const VertShaderFileVK = "GraphVertShader_vk.vsh.spv";
const char* const FragShaderFileVK = "GraphFragShader_vk.fsh.spv";
const char* const VertShaderFileES = "GraphVertShader.vsh";
const char* const FragShaderFileES = "GraphFragShader.fsh";
}

PVRScopeGraph::PVRScopeGraph()
	: numCounter(0)
	, scopeData(NULL)
	, counters(NULL)
	, activeGroup((uint32)0 - 2)
	, activeGroupSelect(0)
	, isActiveGroupChanged(true)
	, sizeCB(0)
	, x(0.0f)
	, y(0.0f)
	, pixelW(0.0f)
	, graphH(0.0f)
	, updateInterval(0)
	, updateIntervalCounter(0)
	, idxFPS((uint32)0 - 1)
	, idx2D((uint32)0 - 1)
	, idx3D((uint32)0 - 1)
	, idxTA((uint32)0 - 1)
	, idxCompute((uint32)0 - 1)
	, idxShaderPixel((uint32)0 - 1)
	, idxShaderVertex((uint32)0 - 1)
	, idxShaderCompute((uint32)0 - 1)
	, _isInitialzed(false)
{
	reading.pfValueBuf  = NULL;
	reading.nValueCnt   = 0;
	reading.nReadingActiveGroup = 99;
}

/*!*********************************************************************************************************************
\brief  init
\return true if no error occurred
***********************************************************************************************************************/
bool PVRScopeGraph::init(GraphicsContext& device, IAssetProvider& assetProvider, ui::UIRenderer& uiRenderer,
                         const api::RenderPass& renderPass, std::string& outMsg)
{
	_uiRenderer = &uiRenderer;
	_context = device;
	_assetProvider = &assetProvider;
	const EPVRScopeInitCode ePVRScopeInitCode = PVRScopeInitialise(&scopeData);
	if (ePVRScopeInitCodeOk != ePVRScopeInitCode) { scopeData = 0; }

	if (scopeData)
	{
		// create the indexbuffer
		const uint16 indexData[10] = {0, 1, 2, 3, 4, 5, 0, 4, 1, 5};
		_indexBuffer = device->createBuffer(sizeof(indexData), types::BufferBindingUse::IndexBuffer, true);
		_indexBuffer->update(indexData, 0, sizeof(indexData));
		_vertexBufferGraphBorder = device->createBuffer(sizeof(glm::vec2) * Configuration::NumVerticesGraphBorder,
		                           types::BufferBindingUse::VertexBuffer, true);
		if (PVRScopeGetCounters(scopeData, &numCounter, &counters, &reading))
		{
			graphCounters.resize(numCounter);

			position(320, 240, Rectanglei(0, 0, 320, 240));
		}
		else
		{
			numCounter = 0;
		}
	}

	if (!createPipeline(_context->getApiType(), renderPass, outMsg)) { return false; }
	_isInitialzed = true;
	// create the color descriptorset for Vulkan
	if (_context->getApiType() == Api::Vulkan)
	{
		_uboColor.addEntryPacked("color", types::GpuDatatypes::vec4);
		_uboColor.finalize(device, ColorTableSize, types::BufferBindingUse::UniformBuffer, true, false);
		_uboColor.createConnectedBuffer(0, _context);

		// fill the buffer
		_uboColor.map(0);
		for (uint32 i = 0; i < ColorTableSize; ++i)
		{
			_uboColor.setArrayValue(0, i, ColorTable[i]);
		}
		_uboColor.unmap(0);
		_uboColorDescriptor = device->createDescriptorSetOnDefaultPool(_pipeDrawLine->getPipelineLayout()->getDescriptorSetLayout(0));
		_isInitialzed = _uboColorDescriptor->update(api::DescriptorSetUpdate().setDynamicUbo(0, _uboColor.getConnectedBuffer(0)));;
	}
	return _isInitialzed;
}

/*!*********************************************************************************************************************
\brief  dtor
***********************************************************************************************************************/
PVRScopeGraph::~PVRScopeGraph()
{
	if (scopeData) {  PVRScopeDeInitialise(&scopeData, &counters, &reading); }
}


/*!*********************************************************************************************************************
\brief  ping
***********************************************************************************************************************/
void PVRScopeGraph::ping(float32 dt)
{
	if (scopeData)
	{
		SPVRScopeCounterReading*  psReading = NULL;
		if (isActiveGroupChanged)
		{
			PVRScopeSetGroup(scopeData, activeGroupSelect);

			// When the active group is changed, retrieve new indices
			idxFPS = PVRScopeFindStandardCounter(numCounter, counters, activeGroupSelect, ePVRScopeStandardCounter_FPS);
			idx2D = PVRScopeFindStandardCounter(numCounter, counters, activeGroupSelect, ePVRScopeStandardCounter_Load_2D);
			idx3D = PVRScopeFindStandardCounter(numCounter, counters, activeGroupSelect, ePVRScopeStandardCounter_Load_Renderer);
			idxTA = PVRScopeFindStandardCounter(numCounter, counters, activeGroupSelect, ePVRScopeStandardCounter_Load_Tiler);
			idxCompute = PVRScopeFindStandardCounter(numCounter, counters, activeGroupSelect, ePVRScopeStandardCounter_Load_Compute);
			idxShaderPixel = PVRScopeFindStandardCounter(numCounter, counters, activeGroupSelect, ePVRScopeStandardCounter_Load_Shader_Pixel);
			idxShaderVertex = PVRScopeFindStandardCounter(numCounter, counters, activeGroupSelect, ePVRScopeStandardCounter_Load_Shader_Vertex);
			idxShaderCompute  = PVRScopeFindStandardCounter(numCounter, counters, activeGroupSelect, ePVRScopeStandardCounter_Load_Shader_Compute);

			isActiveGroupChanged = false;
		}

		// Only recalculate counters periodically
		if (++updateIntervalCounter >= updateInterval) {  psReading = &reading; }

		//  Always call this function, but if we don't want to calculate new
		//  counters yet we set psReading to NULL.

		if (PVRScopeReadCounters(scopeData, psReading) && psReading)
		{
			updateIntervalCounter = 0;

			// Check whether the group has changed
			if (activeGroup != reading.nReadingActiveGroup)
			{
				activeGroup = reading.nReadingActiveGroup;

				// zero the buffers for all the counters becoming enabled
				for (uint32 i = 0; i < numCounter; ++i)
				{
					if (counters[i].nGroup == activeGroup || counters[i].nGroup == 0xffffffff)
					{
						graphCounters[i].writePosCB = 0;
						memset(graphCounters[i].valueCB.data(), 0, sizeof(graphCounters[i].valueCB[0]) * sizeCB);
					}
				}
			}

			// Write the counter value to the buffer
			uint32 ui32Index = 0;

			for (uint32 i = 0; i < numCounter && ui32Index < reading.nValueCnt; ++i)
			{
				if (counters[i].nGroup == activeGroup || counters[i].nGroup == 0xffffffff)
				{
					if (graphCounters[i].writePosCB >= sizeCB) { graphCounters[i].writePosCB = 0; }

					graphCounters[i].valueCB[graphCounters[i].writePosCB++] = reading.pfValueBuf[ui32Index++];
				}
			}
		}
		_context->waitIdle();
		update(dt);
	}
}

/*!*********************************************************************************************************************
\brief  pre-record the commands
\param  api::CommandBufferBase cmdBuffer
***********************************************************************************************************************/
void PVRScopeGraph::recordCommandBuffer(api::SecondaryCommandBuffer& cmdBuffer, uint32 swapChain)
{
	if (scopeData)
	{
		cmdBuffer->bindPipeline(_pipeDrawLine);
		cmdBuffer->bindVertexBuffer(_vertexBufferGraphBorder, 0, 0);
		cmdBuffer->bindIndexBuffer(_indexBuffer, 0, types::IndexType::IndexType16Bit);
		uint32 offset = _uboColor.getAlignedElementArrayOffset(ColorTableSize - 1);
		if (_context->getApiType() == Api::Vulkan)
		{
			cmdBuffer->bindDescriptorSet(_pipeDrawLine->getPipelineLayout(), 0, _uboColorDescriptor, &offset, 1);
		}
		else
		{
			cmdBuffer->setUniform(_esShaderColorId, glm::vec4(0.5, 0.5, 0.5, 1));
		}
		cmdBuffer->drawIndexed(0, 10);

		cmdBuffer->bindPipeline(_pipeDrawLineStrip);

//Draw the visible counters.
		for (uint32 ii = 0; ii < activeCounterIds.size(); ++ii)
		{
			uint32 i = activeCounterIds[ii];
			if ((counters[i].nGroup == activeGroup || counters[i].nGroup == 0xffffffff) && graphCounters[i].showGraph)
			{
				offset = _uboColor.getAlignedElementArrayOffset(graphCounters[i].colorLutIdx);
				cmdBuffer->bindVertexBuffer(activeCounters[ii].vbo, 0, 0);

				if (_context->getApiType() == Api::Vulkan)
				{
					cmdBuffer->bindDescriptorSet(_pipeDrawLineStrip->getPipelineLayout(), 0, _uboColorDescriptor,
					                             &offset, 1);
				}
				else
				{
					cmdBuffer->setUniform(_esShaderColorId, ColorTable[graphCounters[i].colorLutIdx]);
				}
				// Render geometry
				cmdBuffer->drawArrays(0, sizeCB, 0, 1);
			}
		}
	}
}

/*!*********************************************************************************************************************
\brief  pre-record the commands
\param  api::CommandBufferBase cmdBuffer
***********************************************************************************************************************/
void PVRScopeGraph::recordUIElements()
{
	//Draw the visible counters.
	for (uint32 ii = 0; ii < activeCounters.size(); ++ii)
	{
		activeCounters[ii].legendLabel->render();
		activeCounters[ii].legendValue->render();
	}
}


/*!*********************************************************************************************************************
\brief  update the graph
***********************************************************************************************************************/
void PVRScopeGraph::update(float32 dt)
{
	float32 fRatio;
	static float32 lastUpdate = 10000.f;
	bool mustUpdate = false;
	lastUpdate += dt;
	float32 flipY = static_cast<pvr::float32>(_context->getApiType() == Api::Vulkan ? -1 : 1);
	if (lastUpdate > 500.f)
	{
		mustUpdate = true;
		lastUpdate = 0.f;
	}

	activeCounterIds.clear();
	//Make a simple list of indexes with the counters plotted on the graph.
	for (uint32 counterId = 0; counterId < numCounter; ++counterId)
	{
		//Find if the counter is visible.
		if ((counters[counterId].nGroup == activeGroup ||
		     counters[counterId].nGroup == 0xffffffff) && graphCounters[counterId].showGraph)
		{
			//Add it to the list...
			activeCounterIds.push_back(counterId);
		}
	}
	//We will need one VBO per visible counter
	activeCounters.resize(activeCounterIds.size()); //Usually nop...
	verticesGraphContent.resize(sizeCB);

	//Iterate only the visible filtering_window_sorted
	for (uint32 ii = 0; ii < activeCounterIds.size(); ++ii)
	{
		uint32 counterId = activeCounterIds[ii];
		{
			graphCounters[counterId].colorLutIdx = ii % ColorTableSize;

			float32 maximum = 0.0f;
			if (graphCounters[counterId].maximum != 0.0f)
			{
				maximum = graphCounters[counterId].maximum;
			}
			else if (!counters[counterId].nBoolPercentage)
			{
				maximum = getMaximumOfData(counterId);
			}
			else
			{
				maximum = 100.0f;
			}

			float32 filtering_window[3] = { .0f, .0f, .0f };
			float32 filtering_window_sorted[3] = { .0f, .0f, .0f };
			int32 filter_idx = -1;

			if (sizeCB > 0)
			{
				filtering_window[0] = filtering_window[1] = filtering_window[2] = graphCounters[counterId].valueCB[0];
			}

			{
				bool updateThisCounter = mustUpdate;
				//Set the legend
				if (activeCounters[ii].legendLabel.isNull())
				{
					activeCounters[ii].legendLabel = _uiRenderer->createText();
					activeCounters[ii].legendValue = _uiRenderer->createText();
					updateThisCounter = true;
				}
				if (updateThisCounter)
				{
					int id = (graphCounters[counterId].writePosCB ? graphCounters[counterId].writePosCB - 1 : sizeCB - 1);
					activeCounters[ii].legendLabel->setText(strings::createFormatted("[%2d]  %s", counterId, counters[counterId].pszName));
					if (counters[counterId].nBoolPercentage)
					{
						activeCounters[ii].legendValue->setText(strings::createFormatted(" %8.2f%%",
						                                        graphCounters[counterId].valueCB[id]));
					}
					else if (maximum > 100000)
					{
						activeCounters[ii].legendValue->setText(strings::createFormatted(" %9.0fK",
						                                        graphCounters[counterId].valueCB[id] / 1000));
					}
					else
					{
						activeCounters[ii].legendValue->setText(strings::createFormatted(" %10.2f", graphCounters[counterId].valueCB[id]));
					}

					activeCounters[ii].legendLabel->setColor(ColorTable[graphCounters[counterId].colorLutIdx]);
					activeCounters[ii].legendValue->setColor(ColorTable[graphCounters[counterId].colorLutIdx]);
					activeCounters[ii].legendLabel->setAnchor(ui::Anchor::TopLeft, glm::vec2(-.98, .50));
					activeCounters[ii].legendValue->setAnchor(ui::Anchor::TopRight, glm::vec2(-.98, .50));
					activeCounters[ii].legendLabel->setPixelOffset(0, -30 * ii);
					activeCounters[ii].legendValue->setPixelOffset(550, -30 * ii);

					activeCounters[ii].legendLabel->setScale(.5, .5);
					activeCounters[ii].legendValue->setScale(.5, .5);
					activeCounters[ii].legendLabel->commitUpdates();
					activeCounters[ii].legendValue->commitUpdates();
				}
			}

			// Generate geometry
			float32 oneOverMax = 1.f / maximum;
			for (int iDst = 0, iSrc = graphCounters[counterId].writePosCB; iDst < (int)sizeCB; ++iDst, ++iSrc)
			{
				enum { FILTER = 3 };
				++filter_idx; filter_idx %= FILTER;
				// Wrap the source index when necessary
				if (iSrc >= (int)sizeCB)  { iSrc = 0;}

				//Filter the values to avoid spices. We use a rather aggressive median - of - three smoothing.
				float value = graphCounters[counterId].valueCB[iSrc];
				filtering_window[filter_idx] = value;
				filtering_window_sorted[0] = filtering_window[0];
				filtering_window_sorted[1] = filtering_window[1];
				filtering_window_sorted[2] = filtering_window[2];

				//Two-way Bubble sort that is actually not too shabby for 3 items :).
				{
					if (filtering_window_sorted[0] > filtering_window_sorted[1]) { std::swap(filtering_window_sorted[0], filtering_window_sorted[1]); }
					if (filtering_window_sorted[1] > filtering_window_sorted[2]) { std::swap(filtering_window_sorted[1], filtering_window_sorted[2]); }
					if (filtering_window_sorted[0] > filtering_window_sorted[1]) { std::swap(filtering_window_sorted[0], filtering_window_sorted[1]); }
				}

				// X
				verticesGraphContent[iDst].x = x + iDst * pixelW;

				// Y
				fRatio = .0f;
				if (filtering_window_sorted[1])
				{
					fRatio = filtering_window_sorted[1] * oneOverMax;
				}

				glm::clamp(fRatio, 0.f, 1.f);
				verticesGraphContent[iDst].y = flipY * (y + fRatio * graphH);// flip the y for Vulkan
			}

		}
		//Possible optimization: MapBuffer for ES3
		//Need reallocation?
		if (activeCounters[ii].vbo.isNull() || activeCounters[ii].vbo->getSize() != sizeof(verticesGraphContent[0]) * sizeCB)
		{
			activeCounters[ii].vbo = _context->createBuffer(sizeof(verticesGraphContent[0]) * sizeCB, types::BufferBindingUse::VertexBuffer, true);
		}
		//Need updating anyway...
		activeCounters[ii].vbo->update(verticesGraphContent.data(), 0, sizeof(verticesGraphContent[0]) * sizeCB);
	}
}

bool PVRScopeGraph::createPipeline(Api api, const api::RenderPass& renderPass, std::string& errorStr)
{
	api::GraphicsPipelineCreateParam pipeInfo;
	pvr::api::Shader vertexShader;
	pvr::api::Shader fragmentShader;
	pipeInfo.depthStencil.setDepthTestEnable(false);
	pipeInfo.inputAssembler.setPrimitiveTopology(types::PrimitiveTopology::LineList);
	pipeInfo.rasterizer.setCullFace(types::Face::None);
	pipeInfo.vertexInput
	.setInputBinding(0, sizeof(glm::vec2))
	.addVertexAttribute(Configuration::VertexArrayBinding, 0,
	                    assets::VertexAttributeLayout(types::DataType::Float32, 2, 0), "myVertex");
	pipeInfo.renderPass = renderPass;

	if (api == Api::Vulkan)
	{
		vertexShader = _context->createShader(*_assetProvider->getAssetStream(Configuration::VertShaderFileVK),
		                                      types::ShaderType::VertexShader);

		fragmentShader = _context->createShader(*_assetProvider->getAssetStream(Configuration::FragShaderFileVK),
		                                        types::ShaderType::FragmentShader);

		// create the pipeline
		if (!vertexShader.isValid() || !fragmentShader.isValid())
		{
			errorStr = "Failed to create the Vulkan Pipeline shaders";
			return false;
		}
		pipeInfo.vertexShader.setShader(vertexShader);
		pipeInfo.fragmentShader.setShader(fragmentShader);

		// pipeline draw line
		pipeInfo.pipelineLayout = _context->createPipelineLayout(api::PipelineLayoutCreateParam()
		                          .setDescSetLayout(0, _context->createDescriptorSetLayout(
		                                api::DescriptorSetLayoutCreateParam()
		                                .setBinding(0, types::DescriptorType::UniformBufferDynamic, 1,
		                                    types::ShaderStageFlags::Fragment))));
	}
	else if (api <= Api::OpenGLESMaxVersion)
	{
		pvr::assets::ShaderFile shaderFile;
		shaderFile.populateValidVersions(Configuration::VertShaderFileES, *_assetProvider);
		vertexShader = _context->createShader(*shaderFile.getBestStreamForApi(_context->getApiType()),
		                                      pvr::types::ShaderType::VertexShader);

		shaderFile.populateValidVersions(Configuration::FragShaderFileES, *_assetProvider);
		fragmentShader = _context->createShader(*shaderFile.getBestStreamForApi(_context->getApiType()),
		                                        pvr::types::ShaderType::FragmentShader);

		// create the pipeline
		if (!vertexShader.isValid() || !fragmentShader.isValid())
		{
			errorStr = "Failed to create the Gles Pipeline shaders";
			return false;
		}
		pipeInfo.vertexShader.setShader(vertexShader);
		pipeInfo.fragmentShader.setShader(fragmentShader);
		// create empty pipeline layout
		pipeInfo.pipelineLayout = _context->createPipelineLayout(api::PipelineLayoutCreateParam());
	}

	pipeInfo.vertexShader.setShader(vertexShader);
	pipeInfo.fragmentShader.setShader(fragmentShader);


	pipeInfo.colorBlend.setAttachmentState(0, types::BlendingConfig());
	_pipeDrawLine = _context->createParentableGraphicsPipeline(pipeInfo);
	if (!_pipeDrawLine.isValid())
	{
		errorStr = "Failed to create Draw Line pipeline";
		return false;
	}

	// pipeline line strip
	pipeInfo.inputAssembler.setPrimitiveTopology(types::PrimitiveTopology::LineStrip);
	_pipeDrawLineStrip = _context->createGraphicsPipeline(pipeInfo, _pipeDrawLine);
	if (!_pipeDrawLineStrip.isValid())
	{
		errorStr = "Failed to create Draw Line Strip pipeline";
		return false;
	}

	if (_context->getApiType() <= Api::OpenGLESMaxVersion) { _esShaderColorId = _pipeDrawLine->getUniformLocation("fColor"); }

	return true;
}

/*!*********************************************************************************************************************
\brief  show the counter
\param  uint32 nCounter
\param  bool showGraph
***********************************************************************************************************************/
void PVRScopeGraph::showCounter(uint32 nCounter, bool showGraph)
{
	if (nCounter < numCounter) { graphCounters[nCounter].showGraph = showGraph; }
}

/*!*********************************************************************************************************************
\brief  return true if counter is shown
\return bool
\param  uint32 nCounter
***********************************************************************************************************************/
bool PVRScopeGraph::isCounterShown(uint32 nCounter) const
{
	return graphCounters.size() && nCounter < numCounter ? graphCounters[nCounter].showGraph : false;
}

/*!*********************************************************************************************************************
\brief  return whether the counter is being drawn
\return bool
\param  uint32 counter
***********************************************************************************************************************/
bool PVRScopeGraph::isCounterBeingDrawn(uint32 counter) const
{
	if (counter < numCounter && (counters[counter].nGroup == activeGroup || counters[counter].nGroup == 0xffffffff))  { return true; }
	return false;
}

/*!*********************************************************************************************************************
\brief  return true whether the counter use percentage
\return bool
\param  uint32 counter
***********************************************************************************************************************/
bool PVRScopeGraph::isCounterPercentage(uint32 counter) const
{
	return counter < numCounter && counters[counter].nBoolPercentage;
}

/*!*********************************************************************************************************************
\brief  return counter's maximum data
\return float32
\param  uint32 counter
***********************************************************************************************************************/
float32 PVRScopeGraph::getMaximumOfData(uint32 counter)
{
	float32 maximum = 0.f;
	if (counter < numCounter && graphCounters[counter].valueCB.size())
	{
		for (uint32 i = 0; i < sizeCB; ++i)
		{
			int id_next = (i + 1 == sizeCB ? 0 : i + 1);
			int id_prev = (i == 0 ? sizeCB - 1 : i - 1);

			float32 prev_value = graphCounters[counter].valueCB[id_prev];
			float32 current_value = graphCounters[counter].valueCB[i];
			float32 next_value = graphCounters[counter].valueCB[id_next];
			if (prev_value > current_value) { std::swap(prev_value, current_value); }
			if (current_value > next_value) { std::swap(current_value, next_value); }
			if (prev_value > current_value) { std::swap(prev_value, current_value); }
			//CURRENT_VALUE CONTAINS THE MEDIAN.

			maximum = std::max(current_value, maximum);
		}
		return maximum;
	}
	else
	{
		return 0.f;
	}
}

/*!*********************************************************************************************************************
\brief  return counter's maximum
\return float32
\param  uint32 nCounter
***********************************************************************************************************************/
float32 PVRScopeGraph::getMaximum(uint32 nCounter)
{
	if (nCounter < numCounter) {  return graphCounters[nCounter].maximum; }
	return 0.0f;
}

/*!*********************************************************************************************************************
\brief  set counter's maximum
\param  uint32 counter
\param  float32 maximum
***********************************************************************************************************************/
void PVRScopeGraph::setMaximum(uint32 counter, float32 maximum)
{
	if (counter < numCounter) { graphCounters[counter].maximum = maximum; }
}

/*!*********************************************************************************************************************
\brief  set the active group
\return true if no error occurred
\param  const uint32 activeGroup
***********************************************************************************************************************/
bool PVRScopeGraph::setActiveGroup(const uint32 activeGroup)
{
	if (activeGroupSelect == activeGroup) { return true; }

	for (uint32 i = 0; i < numCounter; ++i)
	{
		// Is it a valid group
		if (counters[i].nGroup != 0xffffffff && counters[i].nGroup >= activeGroup)
		{
			activeGroupSelect = activeGroup;
			isActiveGroupChanged = true;
			return true;
		}
	}
	return false;
}

/*!*********************************************************************************************************************
\brief  return the counter name
\return const char*
\param  i counter index
***********************************************************************************************************************/
const char* PVRScopeGraph::getCounterName(const uint32 i) const
{
	if (i >= numCounter) { return ""; }
	return counters[i].pszName;
}

/*!*********************************************************************************************************************
\brief  return FPS
\return float32
***********************************************************************************************************************/
float32 PVRScopeGraph::getStandardFPS() const
{
	return idxFPS < reading.nValueCnt ? reading.pfValueBuf[idxFPS] : -1.0f;
}

/*!*********************************************************************************************************************
\brief  return FPS
\return float32
***********************************************************************************************************************/
int32 PVRScopeGraph::getStandardFPSIndex() const
{
	return idxFPS;
}

float32 PVRScopeGraph::getStandard2D() const
{
	const float32 fRet = idx2D < reading.nValueCnt ? reading.pfValueBuf[idx2D] : -1.0f;
	return fRet;
}
int32 PVRScopeGraph::getStandard2DIndex() const
{
	return idx2D;
}

float32 PVRScopeGraph::getStandard3D() const
{
	return idx3D < reading.nValueCnt ? reading.pfValueBuf[idx3D] : -1.0f;
}
int32 PVRScopeGraph::getStandard3DIndex() const
{
	return idx3D;
}

float32 PVRScopeGraph::getStandardTA() const
{
	return idxTA < reading.nValueCnt ? reading.pfValueBuf[idxTA] : -1.0f;
}

int32 PVRScopeGraph::getStandardTAIndex() const
{
	return idxTA;
}


/*!*********************************************************************************************************************
\brief  return standard compute
\return float32
***********************************************************************************************************************/
float32 PVRScopeGraph::getStandardCompute() const
{
	return idxCompute < reading.nValueCnt ? reading.pfValueBuf[idxCompute] : -1.0f;
}

int32 PVRScopeGraph::getStandardComputeIndex() const
{
	return idxCompute;
}

/*!*********************************************************************************************************************
\brief  return the standard pixel size
\return float32
***********************************************************************************************************************/
float32 PVRScopeGraph::getStandardShaderPixel() const
{
	return idxShaderPixel < reading.nValueCnt ? reading.pfValueBuf[idxShaderPixel] : -1.0f;
}
int32 PVRScopeGraph::getStandardShaderPixelIndex() const
{
	return idxShaderPixel;
}

/*!*********************************************************************************************************************
\brief  return the standard shared vertex
\return float32
***********************************************************************************************************************/
float32 PVRScopeGraph::getStandardShaderVertex() const
{
	return idxShaderVertex < reading.nValueCnt ? reading.pfValueBuf[idxShaderVertex] : -1.0f;
}
int32 PVRScopeGraph::getStandardShaderVertexIndex() const
{
	return idxShaderVertex;
}
/*!*********************************************************************************************************************
\brief  return the standard compute shader
\return float32
***********************************************************************************************************************/
float32 PVRScopeGraph::getStandardShaderCompute() const
{
	return idxShaderCompute < reading.nValueCnt ? reading.pfValueBuf[idxShaderCompute] : -1.0f;
}
int32 PVRScopeGraph::getStandardShaderComputeIndex() const
{
	return idxShaderCompute;
}
/*!*********************************************************************************************************************
\brief  return counter's number of group
\return number of group
\param  const uint32 i
***********************************************************************************************************************/
int PVRScopeGraph::getCounterGroup(const uint32 i) const
{
	if (i >= numCounter) { return 0xffffffff; }
	return counters[i].nGroup;
}

/*!*********************************************************************************************************************
\brief  set the position of the graph
\param  const uint32 viewportW
\param  const uint32 viewportH
\param  Rectanglei const & graph
***********************************************************************************************************************/
void PVRScopeGraph::position(const uint32 viewportW, const uint32 viewportH, Rectanglei const& graph)
{

	if (scopeData && graphCounters.size())
	{
		sizeCB = graph.width;

		float32 pixelW = 2 * 1.0f / viewportW;
		float32 graphH = 2 * (float32)graph.height / viewportH;

		if (this->pixelW != pixelW || this->graphH != graphH)
		{
			this->pixelW = pixelW;
			this->graphH = graphH;

			for (uint32 i = 0; i < numCounter; ++i)
			{
				graphCounters[i].valueCB.clear();
				graphCounters[i].valueCB.resize(sizeCB);
				memset(graphCounters[i].valueCB.data(), 0, sizeof(graphCounters[i].valueCB[0]) * sizeCB);
				graphCounters[i].writePosCB = 0;
			}
		}
		x = 2 * ((float32)graph.x / viewportW) - 1;
		y = 2 * ((float32)graph.y / viewportH) - 1;// flip the y for Vulkan
		updateBufferLines();
	}
}

/*!*********************************************************************************************************************
\brief  update the vertex buffer lines
\return void
***********************************************************************************************************************/
void PVRScopeGraph::updateBufferLines()
{
	const float32 flipY = static_cast<pvr::float32>(_context->getApiType() == Api::Vulkan ? -1 : 1);
	verticesGraphBorder[0].x = x;
	verticesGraphBorder[0].y = flipY * y;

	verticesGraphBorder[1].x = x + sizeCB * pixelW;
	verticesGraphBorder[1].y = flipY * y;

	verticesGraphBorder[2].x = x;
	verticesGraphBorder[2].y = flipY * (y + graphH * 0.5f);

	verticesGraphBorder[3].x = x + sizeCB * pixelW;
	verticesGraphBorder[3].y = flipY * (y + graphH * 0.5f);

	verticesGraphBorder[4].x = x;
	verticesGraphBorder[4].y = flipY * (y + graphH);

	verticesGraphBorder[5].x = x + sizeCB * pixelW;
	verticesGraphBorder[5].y = flipY * (y + graphH);

	_vertexBufferGraphBorder->update(&verticesGraphBorder[0].x, 0, sizeof(verticesGraphBorder));
}

void PVRScopeGraph::setUpdateInterval(const uint32 updateInterval) { this->updateInterval = updateInterval; }
