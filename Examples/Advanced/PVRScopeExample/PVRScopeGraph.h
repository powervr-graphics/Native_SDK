/*!*********************************************************************************************************************
\File         PVRScopeGraph.h
\Title
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description
***********************************************************************************************************************/
#pragma once

#include "PVRScopeStats.h"
#include "PVRApi/Api.h"
#include "PVRUIRenderer/UIRenderer.h"

struct PVRGraphCounter
{
	std::vector<pvr::float32> valueCB;	// Circular buffer of counter values
	pvr::uint32   writePosCB;	// Current write position of circular buffer
	bool showGraph; // Show the graph
	glm::vec4 color;
	pvr::float32 maximum;

	PVRGraphCounter() : writePosCB(0), showGraph(true), color(1.f, 0.0f, 0.0f, 1.0f), maximum(0.0f) {}
};

namespace Configuration {
enum
{
	VertexArrayBinding = 0,
	NumVerticesGraphBorder = 6,
};
}


class PVRScopeGraph
{
protected:
	std::vector<glm::vec2> verticesGraphContent;
	glm::vec2 verticesGraphBorder[Configuration::NumVerticesGraphBorder];

	SPVRScopeCounterReading	m_reading;

	pvr::uint32		m_numCounter;
	SPVRScopeImplData*		m_scopeData;
	SPVRScopeCounterDef*	m_counters;
	pvr::uint32		m_activeGroup;			// most recent group seen
	pvr::uint32		m_activeGroupSelect;	// users desired group
	bool m_isActiveGroupChanged;

	pvr::uint32		m_sizeCB;

	struct ActiveCounter
	{
		pvr::api::Buffer vbo;
		pvr::ui::Text    legendLabel;
		pvr::ui::Text    legendValue;
	};

	std::vector<PVRGraphCounter>	m_graphCounters;
	std::vector<ActiveCounter>	m_activeCounters;
	std::vector<pvr::uint16>    m_activeCounterIds;

	pvr::float32	m_x, m_y, m_pixelW, m_graphH;

	pvr::uint32		m_updateInterval, m_updateIntervalCounter;

	pvr::uint32		m_idxFPS;
	pvr::uint32		m_idx2D;
	pvr::uint32		m_idx3D;
	pvr::uint32		m_idxTA;
	pvr::uint32		m_idxCompute;
	pvr::uint32		m_idxShaderPixel;
	pvr::uint32		m_idxShaderVertex;
	pvr::uint32		m_idxShaderCompute;

	pvr::api::ParentableGraphicsPipeline m_pipeDrawLine;
	pvr::api::GraphicsPipeline m_pipeDrawLineStrip;
	pvr::api::Buffer m_vertexBufferGraphBorder;
	pvr::api::Buffer m_indexBuffer;

	pvr::uint32 colorId;
	pvr::GraphicsContext m_device;
	pvr::IAssetProvider& m_assetProvider;
	pvr::ui::UIRenderer& m_uiRenderer;

public:
	PVRScopeGraph(pvr::GraphicsContext& device, pvr::IAssetProvider& assetProvider, pvr::ui::UIRenderer& uiRenderer);
	~PVRScopeGraph();

	// Disallow copying
	PVRScopeGraph(const PVRScopeGraph&); // deleted
	PVRScopeGraph& operator=(const PVRScopeGraph&);// deleted

	void recordCommandBuffer(pvr::api::CommandBufferBase cmdBuffer);
	void recordUIElements();
	void ping(pvr::float32 dt_millis);

	void showCounter(pvr::uint32 nCounter, bool bShow);
	bool isCounterShown(const pvr::uint32 nCounter) const;
	bool isCounterBeingDrawn(pvr::uint32 nCounter) const;
	bool isCounterPercentage(const pvr::uint32 nCounter) const;
	bool setActiveGroup(const pvr::uint32 nActiveGroup);
	pvr::uint32 getActiveGroup() const { return m_activeGroup; }
	pvr::float32 getMaximumOfData(pvr::uint32 nCounter);
	pvr::float32 getMaximum(pvr::uint32 nCounter);
	void  setMaximum(pvr::uint32 nCounter, pvr::float32 fMaximum);

	pvr::float32 getStandardFPS() const;
	pvr::int32 getStandardFPSIndex() const;
	pvr::float32 getStandard2D() const;
	pvr::int32 getStandard2DIndex() const;
	pvr::float32 getStandard3D() const;
	pvr::int32 getStandard3DIndex() const;
	pvr::float32 getStandardTA() const;
	pvr::int32 getStandardTAIndex() const;
	pvr::float32 getStandardCompute() const;
	pvr::int32 getStandardComputeIndex() const;
	pvr::float32 getStandardShaderPixel() const;
	pvr::int32 getStandardShaderPixelIndex() const;
	pvr::float32 getStandardShaderVertex() const;
	pvr::int32 getStandardShaderVertexIndex() const;
	pvr::float32 getStandardShaderCompute() const;
	pvr::int32 getStandardShaderComputeIndex() const;

	pvr::uint32 getCounterNum() const { return m_numCounter; }

	const char* getCounterName(const pvr::uint32 i) const;
	int getCounterGroup(const pvr::uint32 i) const;

	void position(const pvr::uint32 nViewportW, const pvr::uint32 nViewportH, pvr::Rectanglei const& graph);

	void setUpdateInterval(const pvr::uint32 nUpdateInverval);

protected:
	bool init();
	void updateBufferLines();
	void update(pvr::float32 dt_millis);
	void filter();

};
