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
	, activeGroup(static_cast<uint32_t>(0) - 2)
	, activeGroupSelect(0)
	, isActiveGroupChanged(true)
	, sizeCB(0)
	, x(0.0f)
	, y(0.0f)
	, pixelW(0.0f)
	, graphH(0.0f)
	, updateInterval(0)
	, updateIntervalCounter(0)
	, idxFPS(static_cast<uint32_t>(0) - 1)
	, idx2D(static_cast<uint32_t>(0) - 1)
	, idx3D(static_cast<uint32_t>(0) - 1)
	, idxTA(static_cast<uint32_t>(0) - 1)
	, idxCompute(static_cast<uint32_t>(0) - 1)
	, idxShaderPixel(static_cast<uint32_t>(0) - 1)
	, idxShaderVertex(static_cast<uint32_t>(0) - 1)
	, idxShaderCompute(static_cast<uint32_t>(0) - 1)
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
bool PVRScopeGraph::init(pvr::EglContext& context, pvr::IAssetProvider& assetProvider, pvr::ui::UIRenderer& uiRenderer
                         , std::string& outMsg)
{
	_uiRenderer = &uiRenderer;
	_assetProvider = &assetProvider;
	const EPVRScopeInitCode ePVRScopeInitCode = PVRScopeInitialise(&scopeData);
	if (ePVRScopeInitCodeOk != ePVRScopeInitCode) { scopeData = 0; }

	if (scopeData)
	{
		// create the indexbuffer
		const uint16_t indexData[10] = {0, 1, 2, 3, 4, 5, 0, 4, 1, 5};
		gl::GenBuffers(1, &_indexBuffer);
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
		gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

		gl::GenBuffers(1, &_vertexBufferGraphBorder);
		gl::BindBuffer(GL_ARRAY_BUFFER, _vertexBufferGraphBorder);
		gl::BufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * Configuration::NumVerticesGraphBorder, NULL, GL_STATIC_DRAW);

		if (PVRScopeGetCounters(scopeData, &numCounter, &counters, &reading))
		{
			graphCounters.resize(numCounter);

			position(320, 240, pvr::Rectanglei(0, 0, 320, 240));
		}
		else
		{
			numCounter = 0;
		}
	}

	if (!createProgram(context, outMsg)) { return false; }
	_isInitialzed = true;
	return _isInitialzed;
}

/*!*********************************************************************************************************************
\brief  dtor
***********************************************************************************************************************/
PVRScopeGraph::~PVRScopeGraph()
{
	if (scopeData) { PVRScopeDeInitialise(&scopeData, &counters, &reading); }
}


/*!*********************************************************************************************************************
\brief  ping
***********************************************************************************************************************/
void PVRScopeGraph::ping(float dt)
{
	if (scopeData)
	{
		SPVRScopeCounterReading* psReading = NULL;
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
				for (uint32_t i = 0; i < numCounter; ++i)
				{
					if (counters[i].nGroup == activeGroup || counters[i].nGroup == 0xffffffff)
					{
						graphCounters[i].writePosCB = 0;
						memset(graphCounters[i].valueCB.data(), 0, sizeof(graphCounters[i].valueCB[0]) * sizeCB);
					}
				}
			}

			// Write the counter value to the buffer
			uint32_t ui32Index = 0;

			for (uint32_t i = 0; i < numCounter && ui32Index < reading.nValueCnt; ++i)
			{
				if (counters[i].nGroup == activeGroup || counters[i].nGroup == 0xffffffff)
				{
					if (graphCounters[i].writePosCB >= sizeCB) { graphCounters[i].writePosCB = 0; }

					graphCounters[i].valueCB[graphCounters[i].writePosCB++] = reading.pfValueBuf[ui32Index++];
				}
			}
		}
		update(dt);
	}
}

/*!*********************************************************************************************************************
\brief  pre-record the commands
\param  api::CommandBufferBase commandBuffer
***********************************************************************************************************************/
void PVRScopeGraph::executeCommands()
{
	if (scopeData)
	{
		gl::UseProgram(_program);

		gl::Disable(GL_DEPTH_TEST);
		gl::Disable(GL_CULL_FACE);

		gl::BindBuffer(GL_ARRAY_BUFFER, _vertexBufferGraphBorder);

		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

		uint32_t offset  = 0;
		gl::Uniform4f(_esShaderColorId, 0.5f, 0.5f, 0.5f, 1.f);

		gl::EnableVertexAttribArray(0);
		gl::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), NULL);

		// DRAW LINES
		gl::DrawElements(GL_LINES, 10, GL_UNSIGNED_SHORT, nullptr);

		// DRAW LINE_STRIP
		for (uint32_t ii = 0; ii < activeCounterIds.size(); ++ii)
		{
			uint32_t i = activeCounterIds[ii];
			if ((counters[i].nGroup == activeGroup || counters[i].nGroup == 0xffffffff) && graphCounters[i].showGraph)
			{
				gl::BindBuffer(GL_ARRAY_BUFFER, activeCounters[ii].vbo);
				gl::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), NULL);
				const glm::vec4& color = ColorTable[graphCounters[i].colorLutIdx];
				gl::Uniform4f(_esShaderColorId, color.r, color.g, color.b, color.a);

				// Render geometry
				gl::DrawArrays(GL_LINE_STRIP, 0, sizeCB);
			}
		}
	}
}

/*!*********************************************************************************************************************
\brief  pre-record the commands
\param  api::CommandBufferBase commandBuffer
***********************************************************************************************************************/
void PVRScopeGraph::executeUICommands()
{
	//Draw the visible counters.
	for (uint32_t ii = 0; ii < activeCounters.size(); ++ii)
	{
		activeCounters[ii].legendLabel->render();
		activeCounters[ii].legendValue->render();
	}
}


/*!*********************************************************************************************************************
\brief  update the graph
***********************************************************************************************************************/
void PVRScopeGraph::update(float dt)
{
	float fRatio;
	static float lastUpdate = 10000.f;
	bool mustUpdate = false;
	lastUpdate += dt;
	if (lastUpdate > 500.f)
	{
		mustUpdate = true;
		lastUpdate = 0.f;
	}

	activeCounterIds.clear();
	//Make a simple list of indexes with the counters plotted on the graph.
	for (uint32_t counterId = 0; counterId < numCounter; ++counterId)
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
	for (uint32_t ii = 0; ii < activeCounterIds.size(); ++ii)
	{
		uint32_t counterId = activeCounterIds[ii];
		{
			graphCounters[counterId].colorLutIdx = ii % ColorTableSize;

			float maximum = 0.0f;
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

			float filtering_window[3] = { .0f, .0f, .0f };
			float filtering_window_sorted[3] = { .0f, .0f, .0f };
			int32_t filter_idx = -1;

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
					activeCounters[ii].legendLabel->setText(pvr::strings::createFormatted("[%2d]  %s", counterId, counters[counterId].pszName));
					if (counters[counterId].nBoolPercentage)
					{
						activeCounters[ii].legendValue->setText(pvr::strings::createFormatted(" %8.2f%%",
						                                        graphCounters[counterId].valueCB[id]));
					}
					else if (maximum > 100000)
					{
						activeCounters[ii].legendValue->setText(pvr::strings::createFormatted(" %9.0fK",
						                                        graphCounters[counterId].valueCB[id] / 1000));
					}
					else
					{
						activeCounters[ii].legendValue->setText(pvr::strings::createFormatted(" %10.2f", graphCounters[counterId].valueCB[id]));
					}

					activeCounters[ii].legendLabel->setColor(ColorTable[graphCounters[counterId].colorLutIdx]);
					activeCounters[ii].legendValue->setColor(ColorTable[graphCounters[counterId].colorLutIdx]);
					activeCounters[ii].legendLabel->setAnchor(pvr::ui::Anchor::TopLeft, glm::vec2(-.98f, 0.5f));
					activeCounters[ii].legendValue->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(-.98f, 0.5f));
					activeCounters[ii].legendLabel->setPixelOffset(0.0f, -30.0f * ii);
					activeCounters[ii].legendValue->setPixelOffset(550.0f, -30.0f * ii);

					activeCounters[ii].legendLabel->setScale(.4, .4);
					activeCounters[ii].legendValue->setScale(.4, .4);
					activeCounters[ii].legendLabel->commitUpdates();
					activeCounters[ii].legendValue->commitUpdates();
				}
			}

			// Generate geometry
			float oneOverMax = 1.f / maximum;
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
				verticesGraphContent[iDst].y = (y + fRatio * graphH);// flip the y for Vulkan
			}
		}

		//Possible optimization: MapBuffer for ES3
		//Need reallocation?
		if (!activeCounters[ii].vbo || activeCounters[ii].bufferSize != sizeof(verticesGraphContent[0]) * sizeCB)
		{
			gl::GenBuffers(1, &activeCounters[ii].vbo);
			activeCounters[ii].bufferSize = sizeof(verticesGraphContent[0]) * sizeCB;
		}
		//Need updating anyway...
		gl::BindBuffer(GL_ARRAY_BUFFER, activeCounters[ii].vbo);
		gl::BufferData(GL_ARRAY_BUFFER, sizeof(verticesGraphContent[0]) * sizeCB,
		               verticesGraphContent.data(), GL_STATIC_DRAW);
	}
}

void PVRScopeGraph::setGlCommonStates()
{
	gl::CullFace(GL_NONE);
	gl::Disable(GL_DEPTH_TEST);
	gl::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

bool PVRScopeGraph::createProgram(pvr::EglContext& context, std::string& errorStr)
{
	const char* attribs = "myVertex";
	const uint16_t attribIndex = 0;
	pvr::assets::ShaderFile vertShaderFile, fragShaderFile;
	vertShaderFile.populateValidVersions(Configuration::VertShaderFileES, *_assetProvider);
	fragShaderFile.populateValidVersions(Configuration::FragShaderFileES, *_assetProvider);

	GLuint shaders[2] = { 0, 0 };
	if (!pvr::utils::loadShader(*vertShaderFile.getBestStreamForApi(context->getApiVersion()),
	                            pvr::ShaderType::VertexShader, nullptr, 0, shaders[0]))
	{
		return false;
	}

	if (!pvr::utils::loadShader(*fragShaderFile.getBestStreamForApi(context->getApiVersion()),
	                            pvr::ShaderType::FragmentShader, nullptr, 0, shaders[1]))
	{
		return false;
	}

	_program = 0;

	if (!pvr::utils::createShaderProgram(shaders, ARRAY_SIZE(shaders), &attribs,
	                                     &attribIndex, 1, _program))
	{
		return false;
	}

	// create the pipeline
	if (!_program)
	{
		errorStr = "Failed to create the Program";
		return false;
	}
	gl::UseProgram(_program);
	_esShaderColorId = gl::GetUniformLocation(_program, "fColor");
	gl::UseProgram(0);

	return true;
}

/*!*********************************************************************************************************************
\brief  show the counter
\param  uint32_t nCounter
\param  bool showGraph
***********************************************************************************************************************/
void PVRScopeGraph::showCounter(uint32_t nCounter, bool showGraph)
{
	if (nCounter < numCounter) { graphCounters[nCounter].showGraph = showGraph; }
}

/*!*********************************************************************************************************************
\brief  return true if counter is shown
\return bool
\param  uint32_t nCounter
***********************************************************************************************************************/
bool PVRScopeGraph::isCounterShown(uint32_t nCounter) const
{
	return graphCounters.size() && nCounter < numCounter ? graphCounters[nCounter].showGraph : false;
}

/*!*********************************************************************************************************************
\brief  return whether the counter is being drawn
\return bool
\param  uint32_t counter
***********************************************************************************************************************/
bool PVRScopeGraph::isCounterBeingDrawn(uint32_t counter) const
{
	if (counter < numCounter && (counters[counter].nGroup == activeGroup || counters[counter].nGroup == 0xffffffff))  { return true; }
	return false;
}

/*!*********************************************************************************************************************
\brief  return true whether the counter use percentage
\return bool
\param  uint32_t counter
***********************************************************************************************************************/
bool PVRScopeGraph::isCounterPercentage(uint32_t counter) const
{
	return counter < numCounter && counters[counter].nBoolPercentage;
}

/*!*********************************************************************************************************************
\brief  return counter's maximum data
\return float
\param  uint32_t counter
***********************************************************************************************************************/
float PVRScopeGraph::getMaximumOfData(uint32_t counter)
{
	float maximum = 0.f;
	if (counter < numCounter && graphCounters[counter].valueCB.size())
	{
		for (uint32_t i = 0; i < sizeCB; ++i)
		{
			int id_next = (i + 1 == sizeCB ? 0 : i + 1);
			int id_prev = (i == 0 ? sizeCB - 1 : i - 1);

			float prev_value = graphCounters[counter].valueCB[id_prev];
			float current_value = graphCounters[counter].valueCB[i];
			float next_value = graphCounters[counter].valueCB[id_next];
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
\return float
\param  uint32_t nCounter
***********************************************************************************************************************/
float PVRScopeGraph::getMaximum(uint32_t nCounter)
{
	if (nCounter < numCounter) {  return graphCounters[nCounter].maximum; }
	return 0.0f;
}

/*!*********************************************************************************************************************
\brief  set counter's maximum
\param  uint32_t counter
\param  float maximum
***********************************************************************************************************************/
void PVRScopeGraph::setMaximum(uint32_t counter, float maximum)
{
	if (counter < numCounter) { graphCounters[counter].maximum = maximum; }
}

/*!*********************************************************************************************************************
\brief  set the active group
\return true if no error occurred
\param  const uint32_t activeGroup
***********************************************************************************************************************/
bool PVRScopeGraph::setActiveGroup(const uint32_t activeGroup)
{
	if (activeGroupSelect == activeGroup) { return true; }

	for (uint32_t i = 0; i < numCounter; ++i)
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
const char* PVRScopeGraph::getCounterName(const uint32_t i) const
{
	if (i >= numCounter) { return ""; }
	return counters[i].pszName;
}

/*!*********************************************************************************************************************
\brief  return FPS
\return float
***********************************************************************************************************************/
float PVRScopeGraph::getStandardFPS() const
{
	return idxFPS < reading.nValueCnt ? reading.pfValueBuf[idxFPS] : -1.0f;
}

/*!*********************************************************************************************************************
\brief  return FPS
\return float
***********************************************************************************************************************/
int32_t PVRScopeGraph::getStandardFPSIndex() const
{
	return idxFPS;
}

float PVRScopeGraph::getStandard2D() const
{
	const float fRet = idx2D < reading.nValueCnt ? reading.pfValueBuf[idx2D] : -1.0f;
	return fRet;
}
int32_t PVRScopeGraph::getStandard2DIndex() const
{
	return idx2D;
}

float PVRScopeGraph::getStandard3D() const
{
	return idx3D < reading.nValueCnt ? reading.pfValueBuf[idx3D] : -1.0f;
}

int32_t PVRScopeGraph::getStandard3DIndex() const
{
	return idx3D;
}

float PVRScopeGraph::getStandardTA() const
{
	return idxTA < reading.nValueCnt ? reading.pfValueBuf[idxTA] : -1.0f;
}

int32_t PVRScopeGraph::getStandardTAIndex() const
{
	return idxTA;
}


/*!*********************************************************************************************************************
\brief  return standard compute
\return float
***********************************************************************************************************************/
float PVRScopeGraph::getStandardCompute() const
{
	return idxCompute < reading.nValueCnt ? reading.pfValueBuf[idxCompute] : -1.0f;
}

int32_t PVRScopeGraph::getStandardComputeIndex() const
{
	return idxCompute;
}

/*!*********************************************************************************************************************
\brief  return the standard pixel size
\return float
***********************************************************************************************************************/
float PVRScopeGraph::getStandardShaderPixel() const
{
	return idxShaderPixel < reading.nValueCnt ? reading.pfValueBuf[idxShaderPixel] : -1.0f;
}
int32_t PVRScopeGraph::getStandardShaderPixelIndex() const
{
	return idxShaderPixel;
}

/*!*********************************************************************************************************************
\brief  return the standard shared vertex
\return float
***********************************************************************************************************************/
float PVRScopeGraph::getStandardShaderVertex() const
{
	return idxShaderVertex < reading.nValueCnt ? reading.pfValueBuf[idxShaderVertex] : -1.0f;
}
int32_t PVRScopeGraph::getStandardShaderVertexIndex() const
{
	return idxShaderVertex;
}
/*!*********************************************************************************************************************
\brief  return the standard compute shader
\return float
***********************************************************************************************************************/
float PVRScopeGraph::getStandardShaderCompute() const
{
	return idxShaderCompute < reading.nValueCnt ? reading.pfValueBuf[idxShaderCompute] : -1.0f;
}
int32_t PVRScopeGraph::getStandardShaderComputeIndex() const
{
	return idxShaderCompute;
}
/*!*********************************************************************************************************************
\brief  return counter's number of group
\return number of group
\param  const uint32_t i
***********************************************************************************************************************/
int PVRScopeGraph::getCounterGroup(const uint32_t i) const
{
	if (i >= numCounter) { return 0xffffffff; }
	return counters[i].nGroup;
}

/*!*********************************************************************************************************************
\brief  set the position of the graph
\param  const uint32_t viewportW
\param  const uint32_t viewportH
\param  Rectanglei const & graph
***********************************************************************************************************************/
void PVRScopeGraph::position(const uint32_t viewportW, const uint32_t viewportH, pvr::Rectanglei const& graph)
{
	if (scopeData && graphCounters.size())
	{
		sizeCB = graph.width;

		float pixelW = 2 * 1.0f / viewportW;
		float graphH = 2 * (float)graph.height / viewportH;

		if (this->pixelW != pixelW || this->graphH != graphH)
		{
			this->pixelW = pixelW;
			this->graphH = graphH;

			for (uint32_t i = 0; i < numCounter; ++i)
			{
				graphCounters[i].valueCB.clear();
				graphCounters[i].valueCB.resize(sizeCB);
				memset(graphCounters[i].valueCB.data(), 0, sizeof(graphCounters[i].valueCB[0]) * sizeCB);
				graphCounters[i].writePosCB = 0;
			}
		}
		x = 2 * ((float)graph.x / viewportW) - 1;
		y = 2 * ((float)graph.y / viewportH) - 1;// flip the y for Vulkan
		updateBufferLines();
	}
}

/*!*********************************************************************************************************************
\brief  update the vertex buffer lines
\return void
***********************************************************************************************************************/
void PVRScopeGraph::updateBufferLines()
{
	verticesGraphBorder[0].x = x;
	verticesGraphBorder[0].y = y;

	verticesGraphBorder[1].x = x + sizeCB * pixelW;
	verticesGraphBorder[1].y =  y;

	verticesGraphBorder[2].x = x;
	verticesGraphBorder[2].y = (y + graphH * 0.5f);

	verticesGraphBorder[3].x = x + sizeCB * pixelW;
	verticesGraphBorder[3].y = (y + graphH * 0.5f);

	verticesGraphBorder[4].x = x;
	verticesGraphBorder[4].y = (y + graphH);

	verticesGraphBorder[5].x = x + sizeCB * pixelW;
	verticesGraphBorder[5].y = (y + graphH);

	gl::BindBuffer(GL_ARRAY_BUFFER, _vertexBufferGraphBorder);
	gl::BufferData(GL_ARRAY_BUFFER, sizeof(verticesGraphBorder), &verticesGraphBorder[0].x, GL_STATIC_DRAW);
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
}

void PVRScopeGraph::setUpdateInterval(const uint32_t updateInterval) { this->updateInterval = updateInterval; }
