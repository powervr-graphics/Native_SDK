/*!*********************************************************************************************************************
\File         PVRScopeGraph.cpp
\Title
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
***********************************************************************************************************************/
#include "PVRScopeStats.h"
#include "PVRScopeGraph.h"
#include "PVRCore/BufferStream.h"
#include "PVRNativeApi/ShaderUtils.h"
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
	{ .80, .30, .0, 1. },  //11
	{ .00, .50, .50, 1. }, //12
	{ .50, .00, .00, 1. }, //13
	{ .00, .50, .00, 1. }, //14
	{ .00, .00, .50, 1. }, //15
	{ .30, .60, 0.0, 1. }, //16
	{ .00, .50, .80, 1. }, //17
};
enum {ColorTableSize = sizeof(ColorTable) / sizeof(ColorTable[0]) };
namespace Configuration {
const char* const VertShaderFile = "GraphVertShader.vsh";
const char* const FragShaderFile = "GraphFragShader.fsh";
}

PVRScopeGraph::PVRScopeGraph(pvr::GraphicsContext& device, pvr::IAssetProvider& assetProvider, pvr::ui::UIRenderer& uiRenderer)
	: m_numCounter(0)
	, m_scopeData(NULL)
	, m_counters(NULL)
	, m_activeGroup((pvr::uint32)0 - 2)
	, m_activeGroupSelect(0)
	, m_isActiveGroupChanged(true)
	, m_sizeCB(0)
	, m_x(0.0f)
	, m_y(0.0f)
	, m_pixelW(0.0f)
	, m_graphH(0.0f)
	, m_updateInterval(0)
	, m_updateIntervalCounter(0)
	, m_idxFPS((pvr::uint32)0 - 1)
	, m_idx2D((pvr::uint32)0 - 1)
	, m_idx3D((pvr::uint32)0 - 1)
	, m_idxTA((pvr::uint32)0 - 1)
	, m_idxCompute((pvr::uint32)0 - 1)
	, m_idxShaderPixel((pvr::uint32)0 - 1)
	, m_idxShaderVertex((pvr::uint32)0 - 1)
	, m_idxShaderCompute((pvr::uint32)0 - 1),
	  m_device(device), m_assetProvider(assetProvider), m_uiRenderer(uiRenderer)
{
	m_reading.pfValueBuf	= NULL;
	m_reading.nValueCnt		= 0;
	m_reading.nReadingActiveGroup	= 99;

	init();

	const EPVRScopeInitCode ePVRScopeInitCode = PVRScopeInitialise(&m_scopeData);
	if (ePVRScopeInitCodeOk != ePVRScopeInitCode)
	{
		m_scopeData = 0;
	}

	if (m_scopeData)
	{
		if (PVRScopeGetCounters(m_scopeData, &m_numCounter, &m_counters, &m_reading))
		{
			m_graphCounters.resize(m_numCounter);
			position(320, 240, pvr::Rectanglei(0, 0, 320, 240));
		}
		else
		{
			m_numCounter = 0;
		}
	}
}

/*!*********************************************************************************************************************
\brief	init
\return	true if no error occurred
***********************************************************************************************************************/
bool PVRScopeGraph::init()
{
	pvr::assets::ShaderFile shaderFile;
	shaderFile.populateValidVersions(Configuration::VertShaderFile, m_assetProvider);
	pvr::api::Shader vertexShader = m_device->createShader(*shaderFile.getBestStreamForApi(m_device->getApiType()), pvr::types::ShaderType::VertexShader);

	shaderFile.populateValidVersions(Configuration::FragShaderFile, m_assetProvider);
	pvr::api::Shader fragmentShader = m_device->createShader(*shaderFile.getBestStreamForApi(m_device->getApiType()), pvr::types::ShaderType::FragmentShader);

	if (!vertexShader.isValid() || !fragmentShader.isValid()) { return false; }

	pvr::api::GraphicsPipelineCreateParam pipeInfo;
	pipeInfo.vertexShader.setShader(vertexShader);
	pipeInfo.fragmentShader.setShader(fragmentShader);
	pipeInfo.depthStencil.setDepthTestEnable(false);
	pipeInfo.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::Lines);

	pipeInfo.vertexInput.setInputBinding(0, 0);
	pipeInfo.vertexInput.addVertexAttribute(Configuration::VertexArrayBinding, 0, pvr::assets::VertexAttributeLayout(pvr::types::DataType::Float32, 2, 0) , "myVertex");

	// create empty pipeline layout
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeInfo.pipelineLayout = m_device->createPipelineLayout(pipeLayoutInfo);
	pipeInfo.colorBlend.addAttachmentState(pvr::api::pipelineCreation::ColorBlendAttachmentState());
	m_pipeDrawLine = m_device->createParentableGraphicsPipeline(pipeInfo);
	colorId = m_pipeDrawLine->getUniformLocation("fColor");
	pipeInfo.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::LineStrip);
	m_pipeDrawLineStrip = m_device->createGraphicsPipeline(pipeInfo, m_pipeDrawLine);


	// create the indexbuffer
	const pvr::uint16 indexData[10] = {0, 1, 2, 3, 4, 5, 0, 4, 1, 5};
	m_indexBuffer = m_device->createBuffer(sizeof(indexData), pvr::types::BufferBindingUse::IndexBuffer);
	m_indexBuffer->update(indexData, 0, sizeof(indexData));
	m_vertexBufferGraphBorder = m_device->createBuffer(sizeof(glm::vec2) * Configuration::NumVerticesGraphBorder, pvr::types::BufferBindingUse::VertexBuffer);
	return true;
}

/*!*********************************************************************************************************************
\brief	dtor
***********************************************************************************************************************/
PVRScopeGraph::~PVRScopeGraph()
{

	if (m_scopeData) {	PVRScopeDeInitialise(&m_scopeData, &m_counters, &m_reading); }
}

/*!*********************************************************************************************************************
\brief	ping
***********************************************************************************************************************/
void PVRScopeGraph::ping(pvr::float32 dt)
{
	if (m_scopeData)
	{
		SPVRScopeCounterReading*	psReading;
		if (m_isActiveGroupChanged)
		{
			PVRScopeSetGroup(m_scopeData, m_activeGroupSelect);

			// When the active group is changed, retrieve new indices
			m_idxFPS		= PVRScopeFindStandardCounter(m_numCounter, m_counters, m_activeGroupSelect, ePVRScopeStandardCounter_FPS);
			m_idx2D			= PVRScopeFindStandardCounter(m_numCounter, m_counters, m_activeGroupSelect, ePVRScopeStandardCounter_Load_2D);
			m_idx3D			= PVRScopeFindStandardCounter(m_numCounter, m_counters, m_activeGroupSelect, ePVRScopeStandardCounter_Load_Renderer);
			m_idxTA			= PVRScopeFindStandardCounter(m_numCounter, m_counters, m_activeGroupSelect, ePVRScopeStandardCounter_Load_Tiler);
			m_idxCompute		= PVRScopeFindStandardCounter(m_numCounter, m_counters, m_activeGroupSelect, ePVRScopeStandardCounter_Load_Compute);
			m_idxShaderPixel	= PVRScopeFindStandardCounter(m_numCounter, m_counters, m_activeGroupSelect, ePVRScopeStandardCounter_Load_Shader_Pixel);
			m_idxShaderVertex	= PVRScopeFindStandardCounter(m_numCounter, m_counters, m_activeGroupSelect, ePVRScopeStandardCounter_Load_Shader_Vertex);
			m_idxShaderCompute	= PVRScopeFindStandardCounter(m_numCounter, m_counters, m_activeGroupSelect, ePVRScopeStandardCounter_Load_Shader_Compute);

			m_isActiveGroupChanged = false;
		}

		// Only recalculate counters periodically
		if (++m_updateIntervalCounter >= m_updateInterval)
		{
			psReading = &m_reading;
		}
		else
		{
			psReading = NULL;
		}


		//	Always call this function, but if we don't want to calculate new
		//	counters yet we set psReading to NULL.

		if (PVRScopeReadCounters(m_scopeData, psReading) && psReading)
		{
			m_updateIntervalCounter = 0;

			// Check whether the group has changed
			if (m_activeGroup != m_reading.nReadingActiveGroup)
			{
				m_activeGroup = m_reading.nReadingActiveGroup;

				// zero the buffers for all the counters becoming enabled
				for (pvr::uint32 i = 0; i < m_numCounter; ++i)
				{
					if (m_counters[i].nGroup == m_activeGroup || m_counters[i].nGroup == 0xffffffff)
					{
						m_graphCounters[i].writePosCB = 0;
						memset(m_graphCounters[i].valueCB.data(), 0, sizeof(m_graphCounters[i].valueCB[0]) * m_sizeCB);
					}
				}
			}

			// Write the counter value to the buffer
			pvr::uint32 ui32Index = 0;

			for (pvr::uint32 i = 0; i < m_numCounter && ui32Index < m_reading.nValueCnt; ++i)
			{
				if (m_counters[i].nGroup == m_activeGroup || m_counters[i].nGroup == 0xffffffff)
				{
					if (m_graphCounters[i].writePosCB >= m_sizeCB) { m_graphCounters[i].writePosCB = 0; }

					m_graphCounters[i].valueCB[m_graphCounters[i].writePosCB++] = m_reading.pfValueBuf[ui32Index++];
				}
			}
		}
		update(dt);
	}
}

/*!*********************************************************************************************************************
\brief	pre-record the commands
\param	pvr::api::CommandBufferBase cmdBuffer
***********************************************************************************************************************/
void PVRScopeGraph::recordCommandBuffer(pvr::api::CommandBufferBase cmdBuffer)
{
	glm::vec4 color(0.5, 0.5, 0.5, 1);
	cmdBuffer->bindVertexBuffer(m_vertexBufferGraphBorder, 0, 0);
	cmdBuffer->bindIndexBuffer(m_indexBuffer, 0, pvr::types::IndexType::IndexType16Bit);
	cmdBuffer->bindPipeline(m_pipeDrawLine);
	cmdBuffer->setUniformPtr<glm::vec4>(colorId, 1, &color);
	cmdBuffer->drawIndexed(0, 10);

	cmdBuffer->bindPipeline(m_pipeDrawLineStrip);

//Draw the visible counters.
	for (pvr::uint32 ii = 0; ii < m_activeCounterIds.size(); ++ii)
	{
		pvr::uint32 i = m_activeCounterIds[ii];
		if ((m_counters[i].nGroup == m_activeGroup || m_counters[i].nGroup == 0xffffffff) && m_graphCounters[i].showGraph)
		{
			cmdBuffer->bindVertexBuffer(m_activeCounters[ii].vbo, 0, 0);
			cmdBuffer->setUniformPtr(colorId, 1, &m_graphCounters[i].color);
			// Render geometry
			cmdBuffer->drawArrays(0, m_sizeCB, 0, 1);
		}
	}
}

/*!*********************************************************************************************************************
\brief	pre-record the commands
\param	pvr::api::CommandBufferBase cmdBuffer
***********************************************************************************************************************/
void PVRScopeGraph::recordUIElements()
{
	//Draw the visible counters.
	for (pvr::uint32 ii = 0; ii < m_activeCounters.size(); ++ii)
	{
		m_activeCounters[ii].legendLabel->render();
		m_activeCounters[ii].legendValue->render();
	}
}


/*!*********************************************************************************************************************
\brief	update the graph
***********************************************************************************************************************/
void PVRScopeGraph::update(pvr::float32 dt)
{
	pvr::float32 fRatio;
	static pvr::float32 lastUpdate = 10000.f;
	bool mustUpdate = false;
	lastUpdate += dt;
	if (lastUpdate > 500.f)
	{
		mustUpdate = true;
		lastUpdate = 0.f;
	}

	m_activeCounterIds.clear();
	//Make a simple list of indexes with the counters plotted on the graph.
	for (pvr::uint32 counterId = 0; counterId < m_numCounter; ++counterId)
	{
		//Find if the counter is visible.
		if ((m_counters[counterId].nGroup == m_activeGroup || m_counters[counterId].nGroup == 0xffffffff) && m_graphCounters[counterId].showGraph)
		{
			//Add it to the list...
			m_activeCounterIds.push_back(counterId);
		}
	}
	//We will need one VBO per visible counter
	m_activeCounters.resize(m_activeCounterIds.size()); //Usually nop...
	verticesGraphContent.resize(m_sizeCB);

	//Iterate only the visible filtering_window_sorted
	for (pvr::uint32 ii = 0; ii < m_activeCounterIds.size(); ++ii)
	{
		pvr::uint32 counterId = m_activeCounterIds[ii];
		{
			m_graphCounters[counterId].color = ColorTable[ii % ColorTableSize];

			pvr::float32 maximum = 0.0f;
			if (m_graphCounters[counterId].maximum != 0.0f)
			{
				maximum = m_graphCounters[counterId].maximum;
			}
			else if (!m_counters[counterId].nBoolPercentage)
			{
				maximum = getMaximumOfData(counterId);
			}
			else
			{
				maximum = 100.0f;
			}

			pvr::float32 filtering_window[3] = { .0f, .0f, .0f };
			pvr::float32 filtering_window_sorted[3] = { .0f, .0f, .0f };
			pvr::int32 filter_idx = -1;

			if (m_sizeCB > 0)
			{
				filtering_window[0] = filtering_window[1] = filtering_window[2] = m_graphCounters[counterId].valueCB[0];
			}

			static float lastUpdate = 0;
			{
				bool updateThisCounter = mustUpdate;
				//Set the legend
				if (m_activeCounters[ii].legendLabel.isNull())
				{
					m_activeCounters[ii].legendLabel = m_uiRenderer.createText();
					m_activeCounters[ii].legendValue = m_uiRenderer.createText();
					updateThisCounter = true;
				}
				if (updateThisCounter)
				{
					int id = (m_graphCounters[counterId].writePosCB ? m_graphCounters[counterId].writePosCB - 1 : m_sizeCB - 1);
					m_activeCounters[ii].legendLabel->setText(pvr::strings::createFormatted("[%2d]  %s", counterId, m_counters[counterId].pszName));
					if (m_counters[counterId].nBoolPercentage)
					{
						m_activeCounters[ii].legendValue->setText(pvr::strings::createFormatted(" %8.2f%%", m_graphCounters[counterId].valueCB[id]));
					}
					else if (maximum > 100000)
					{
						m_activeCounters[ii].legendValue->setText(pvr::strings::createFormatted(" %9.0fK", m_graphCounters[counterId].valueCB[id] / 1000));
					}
					else
					{
						m_activeCounters[ii].legendValue->setText(pvr::strings::createFormatted(" %10.2f", m_graphCounters[counterId].valueCB[id]));
					}

					m_activeCounters[ii].legendLabel->setColor(m_graphCounters[counterId].color);
					m_activeCounters[ii].legendValue->setColor(m_graphCounters[counterId].color);
					m_activeCounters[ii].legendLabel->setAnchor(pvr::ui::Anchor::TopLeft, glm::vec2(-.98, .55));
					m_activeCounters[ii].legendValue->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(-.98, .55));
					m_activeCounters[ii].legendLabel->setPixelOffset(0, -30 * ii);
					m_activeCounters[ii].legendValue->setPixelOffset(550, -30 * ii);

					m_activeCounters[ii].legendLabel->setScale(.5, .5);
					m_activeCounters[ii].legendValue->setScale(.5, .5);
					m_activeCounters[ii].legendLabel->commitUpdates();
					m_activeCounters[ii].legendValue->commitUpdates();
				}
			}

			// Generate geometry
			for (int iDst = 0, iSrc = m_graphCounters[counterId].writePosCB; iDst < (int)m_sizeCB; ++iDst, ++iSrc)
			{
				enum { FILTER = 3 };
				++filter_idx; filter_idx %= FILTER;
				// Wrap the source index when necessary
				if (iSrc >= (int)m_sizeCB)	{	iSrc = 0;}

				//Filter the values to avoid spices. We use a rather aggressive median - of - three smoothing.
				float value = m_graphCounters[counterId].valueCB[iSrc];
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
				verticesGraphContent[iDst].x = m_x + iDst * m_pixelW;

				// Y
				if (filtering_window_sorted[1])
				{
					fRatio = filtering_window_sorted[1] / maximum;
				}
				else
				{
					fRatio = 0;
				}

				if (fRatio < 0)
				{
					fRatio = 0;
				}
				else if (fRatio > 1)
				{
					fRatio = 1;
				}

				verticesGraphContent[iDst].y = m_y + fRatio * m_graphH;
			}

		}
		//Possible optimization: MapBuffer for ES3
		//Need reallocation?
		if (m_activeCounters[ii].vbo.isNull() || m_activeCounters[ii].vbo->getSize() != sizeof(verticesGraphContent[0]) * m_sizeCB)
		{
			m_activeCounters[ii].vbo = m_device->createBuffer(sizeof(verticesGraphContent[0]) * m_sizeCB, pvr::types::BufferBindingUse::VertexBuffer);
		}
		//Need updating anyway...
		m_activeCounters[ii].vbo->update(verticesGraphContent.data(), 0, sizeof(verticesGraphContent[0]) * m_sizeCB);
	}
}

/*!*********************************************************************************************************************
\brief	show the counter
\param	pvr::uint32 nCounter
\param	bool showGraph
***********************************************************************************************************************/
void PVRScopeGraph::showCounter(pvr::uint32 nCounter, bool showGraph)
{
	if (nCounter < m_numCounter) { m_graphCounters[nCounter].showGraph = showGraph; }
}

/*!*********************************************************************************************************************
\brief	return true if counter is shown
\return	bool
\param	pvr::uint32 nCounter
***********************************************************************************************************************/
bool PVRScopeGraph::isCounterShown(pvr::uint32 nCounter) const
{
	return m_graphCounters.size() && nCounter < m_numCounter ? m_graphCounters[nCounter].showGraph : false;
}

/*!*********************************************************************************************************************
\brief	return whether the counter is being drawn
\return	bool
\param	pvr::uint32 counter
***********************************************************************************************************************/
bool PVRScopeGraph::isCounterBeingDrawn(pvr::uint32 counter) const
{
	if (counter < m_numCounter && (m_counters[counter].nGroup == m_activeGroup || m_counters[counter].nGroup == 0xffffffff))	{ return true; }
	return false;
}

/*!*********************************************************************************************************************
\brief	return true whether the counter use percentage
\return	bool
\param	pvr::uint32 counter
***********************************************************************************************************************/
bool PVRScopeGraph::isCounterPercentage(pvr::uint32 counter) const
{
	return counter < m_numCounter && m_counters[counter].nBoolPercentage;
}

/*!*********************************************************************************************************************
\brief	return counter's maximum data
\return	pvr::float32
\param	pvr::uint32 counter
***********************************************************************************************************************/
pvr::float32 PVRScopeGraph::getMaximumOfData(pvr::uint32 counter)
{
	pvr::float32 maximum = 0.f;
	if (counter < m_numCounter && m_graphCounters[counter].valueCB.size())
	{
		for (pvr::uint32 i = 0; i < m_sizeCB; ++i)
		{
			int id_next = (i + 1 == m_sizeCB ? 0 : i + 1);
			int id_prev = (i == 0 ? m_sizeCB - 1 : i - 1);

			pvr::float32 prev_value = m_graphCounters[counter].valueCB[id_prev];
			pvr::float32 current_value = m_graphCounters[counter].valueCB[i];
			pvr::float32 next_value = m_graphCounters[counter].valueCB[id_next];
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
\brief	return counter's maximum
\return	pvr::float32
\param	pvr::uint32 nCounter
***********************************************************************************************************************/
pvr::float32 PVRScopeGraph::getMaximum(pvr::uint32 nCounter)
{
	if (nCounter < m_numCounter) {	return m_graphCounters[nCounter].maximum;	}
	return 0.0f;
}

/*!*********************************************************************************************************************
\brief	set counter's maximum
\param	pvr::uint32 counter
\param	pvr::float32 maximum
***********************************************************************************************************************/
void PVRScopeGraph::setMaximum(pvr::uint32 counter, pvr::float32 maximum)
{
	if (counter < m_numCounter) { m_graphCounters[counter].maximum = maximum; }
}

/*!*********************************************************************************************************************
\brief	set the active group
\return	true if no error occurred
\param	const pvr::uint32 activeGroup
***********************************************************************************************************************/
bool PVRScopeGraph::setActiveGroup(const pvr::uint32 activeGroup)
{
	if (m_activeGroupSelect == activeGroup) { return true; }

	for (pvr::uint32 i = 0; i < m_numCounter; ++i)
	{
		// Is it a valid group
		if (m_counters[i].nGroup != 0xffffffff && m_counters[i].nGroup >= activeGroup)
		{
			m_activeGroupSelect = activeGroup;
			m_isActiveGroupChanged = true;
			return true;
		}
	}
	return false;
}

/*!*********************************************************************************************************************
\brief	return the counter name
\return	const char*
\param	i counter index
***********************************************************************************************************************/
const char* PVRScopeGraph::getCounterName(const pvr::uint32 i) const
{
	if (i >= m_numCounter) { return ""; }
	return m_counters[i].pszName;
}

/*!*********************************************************************************************************************
\brief	return FPS
\return	pvr::float32
***********************************************************************************************************************/
pvr::float32 PVRScopeGraph::getStandardFPS() const
{
	return m_idxFPS < m_reading.nValueCnt ? m_reading.pfValueBuf[m_idxFPS] : -1.0f;
}

/*!*********************************************************************************************************************
\brief	return FPS
\return	pvr::float32
***********************************************************************************************************************/
pvr::int32 PVRScopeGraph::getStandardFPSIndex() const
{
	return m_idxFPS;
}

pvr::float32 PVRScopeGraph::getStandard2D() const
{
	const pvr::float32 fRet = m_idx2D < m_reading.nValueCnt ? m_reading.pfValueBuf[m_idx2D] : -1.0f;
	return fRet;
}
pvr::int32 PVRScopeGraph::getStandard2DIndex() const
{
	return m_idx2D;
}

pvr::float32 PVRScopeGraph::getStandard3D() const
{
	return m_idx3D < m_reading.nValueCnt ? m_reading.pfValueBuf[m_idx3D] : -1.0f;
}
pvr::int32 PVRScopeGraph::getStandard3DIndex() const
{
	return m_idx3D;
}

pvr::float32 PVRScopeGraph::getStandardTA() const
{
	return m_idxTA < m_reading.nValueCnt ? m_reading.pfValueBuf[m_idxTA] : -1.0f;
}

pvr::int32 PVRScopeGraph::getStandardTAIndex() const
{
	return m_idxTA;
}


/*!*********************************************************************************************************************
\brief	return standard compute
\return	pvr::float32
***********************************************************************************************************************/
pvr::float32 PVRScopeGraph::getStandardCompute() const
{
	return m_idxCompute < m_reading.nValueCnt ? m_reading.pfValueBuf[m_idxCompute] : -1.0f;
}

pvr::int32 PVRScopeGraph::getStandardComputeIndex() const
{
	return m_idxCompute;
}

/*!*********************************************************************************************************************
\brief	return the standard pixel size
\return	pvr::float32
***********************************************************************************************************************/
pvr::float32 PVRScopeGraph::getStandardShaderPixel() const
{
	return m_idxShaderPixel < m_reading.nValueCnt ? m_reading.pfValueBuf[m_idxShaderPixel] : -1.0f;
}
pvr::int32 PVRScopeGraph::getStandardShaderPixelIndex() const
{
	return m_idxShaderPixel;
}

/*!*********************************************************************************************************************
\brief	return the standard shared vertex
\return	pvr::float32
***********************************************************************************************************************/
pvr::float32 PVRScopeGraph::getStandardShaderVertex() const
{
	return m_idxShaderVertex < m_reading.nValueCnt ? m_reading.pfValueBuf[m_idxShaderVertex] : -1.0f;
}
pvr::int32 PVRScopeGraph::getStandardShaderVertexIndex() const
{
	return m_idxShaderVertex;
}
/*!*********************************************************************************************************************
\brief	return the standard compute shader
\return	pvr::float32
***********************************************************************************************************************/
pvr::float32 PVRScopeGraph::getStandardShaderCompute() const
{
	return m_idxShaderCompute < m_reading.nValueCnt ? m_reading.pfValueBuf[m_idxShaderCompute] : -1.0f;
}
pvr::int32 PVRScopeGraph::getStandardShaderComputeIndex() const
{
	return m_idxShaderCompute;
}
/*!*********************************************************************************************************************
\brief	return counter's number of group
\return	number of group
\param	const pvr::uint32 i
***********************************************************************************************************************/
int PVRScopeGraph::getCounterGroup(const pvr::uint32 i) const
{
	if (i >= m_numCounter) { return 0xffffffff; }
	return m_counters[i].nGroup;
}

/*!*********************************************************************************************************************
\brief	set the position of the graph
\param	const pvr::uint32 viewportW
\param	const pvr::uint32 viewportH
\param	pvr::Rectanglei const & graph
***********************************************************************************************************************/
void PVRScopeGraph::position(const pvr::uint32 viewportW, const pvr::uint32 viewportH, pvr::Rectanglei const& graph)
{
	if (m_scopeData && m_graphCounters.size())
	{
		m_sizeCB = graph.width;

		pvr::float32 pixelW = 2 * 1.0f / viewportW;
		pvr::float32 graphH = 2 * (pvr::float32)graph.height / viewportH;

		if (pixelW != m_pixelW || graphH != m_graphH)
		{
			m_pixelW = pixelW;
			m_graphH = graphH;

			for (pvr::uint32 i = 0; i < m_numCounter; ++i)
			{
				m_graphCounters[i].valueCB.clear();
				m_graphCounters[i].valueCB.resize(m_sizeCB);
				memset(m_graphCounters[i].valueCB.data(), 0, sizeof(m_graphCounters[i].valueCB[0]) * m_sizeCB);
				m_graphCounters[i].writePosCB = 0;
			}
		}
		m_x = 2 * ((pvr::float32)graph.x / viewportW) - 1;
		m_y = 2 * ((pvr::float32)graph.y / viewportH) - 1;
		updateBufferLines();
	}
}

/*!*********************************************************************************************************************
\brief	update the vertex buffer lines
\return	void
***********************************************************************************************************************/
void PVRScopeGraph::updateBufferLines()
{
	verticesGraphBorder[0].x = m_x;
	verticesGraphBorder[0].y = m_y;

	verticesGraphBorder[1].x = m_x + m_sizeCB * m_pixelW;
	verticesGraphBorder[1].y = m_y;

	verticesGraphBorder[2].x = m_x;
	verticesGraphBorder[2].y = m_y + m_graphH * 0.5f;

	verticesGraphBorder[3].x = m_x + m_sizeCB * m_pixelW;
	verticesGraphBorder[3].y = m_y + m_graphH * 0.5f;

	verticesGraphBorder[4].x = m_x;
	verticesGraphBorder[4].y = m_y + m_graphH;

	verticesGraphBorder[5].x = m_x + m_sizeCB * m_pixelW;
	verticesGraphBorder[5].y = m_y + m_graphH;

	m_vertexBufferGraphBorder->update(&verticesGraphBorder[0].x, 0, sizeof(verticesGraphBorder));
}

void PVRScopeGraph::setUpdateInterval(const pvr::uint32 updateInterval) { m_updateInterval = updateInterval; }
