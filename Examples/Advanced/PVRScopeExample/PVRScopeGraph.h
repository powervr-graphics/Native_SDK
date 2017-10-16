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
#include "PVREngineUtils/UIRenderer.h"

struct PVRGraphCounter
{
	std::vector<pvr::float32> valueCB;	// Circular buffer of counter values
	pvr::uint32   writePosCB;	// Current write position of circular buffer
	bool showGraph; // Show the graph
	pvr::uint32 colorLutIdx;// color lookup table index
	pvr::float32 maximum;

	PVRGraphCounter() : writePosCB(0), showGraph(true), maximum(0.0f) {}
};

namespace Configuration {
enum
{
	VertexArrayBinding = 0,
	NumVerticesGraphBorder = 6,
	MaxSwapChains = 8
};
}


class PVRScopeGraph
{
protected:
	std::vector<glm::vec2> verticesGraphContent;
	glm::vec2 verticesGraphBorder[Configuration::NumVerticesGraphBorder];

	SPVRScopeCounterReading	reading;

	pvr::uint32		numCounter;
	SPVRScopeImplData*		scopeData;
	SPVRScopeCounterDef*	counters;
	pvr::uint32		activeGroup;			// most recent group seen
	pvr::uint32		activeGroupSelect;	// users desired group
	bool isActiveGroupChanged;

	pvr::uint32		sizeCB;

	struct ActiveCounter
	{
		pvr::api::Buffer vbo;
		pvr::ui::Text    legendLabel;
		pvr::ui::Text    legendValue;
	};

	std::vector<PVRGraphCounter>	graphCounters;
	std::vector<ActiveCounter>	activeCounters;
	std::vector<pvr::uint16>    activeCounterIds;

	pvr::float32	x, y, pixelW, graphH;

	pvr::uint32		updateInterval, updateIntervalCounter;

	pvr::uint32		idxFPS;
	pvr::uint32		idx2D;
	pvr::uint32		idx3D;
	pvr::uint32		idxTA;
	pvr::uint32		idxCompute;
	pvr::uint32		idxShaderPixel;
	pvr::uint32		idxShaderVertex;
	pvr::uint32		idxShaderCompute;

	pvr::api::ParentableGraphicsPipeline _pipeDrawLine;
	pvr::api::GraphicsPipeline _pipeDrawLineStrip;
	pvr::api::Buffer _vertexBufferGraphBorder;
	pvr::api::Buffer _indexBuffer;
	pvr::utils::StructuredMemoryView _uboColor;
	pvr::api::DescriptorSet	 _uboColorDescriptor;
	pvr::ui::UIRenderer* _uiRenderer;
	pvr::IAssetProvider* _assetProvider;
	pvr::GraphicsContext _context;
	pvr::uint32	_esShaderColorId;
	bool _isInitialzed;
public:
	PVRScopeGraph();

	~PVRScopeGraph();

	// Disallow copying
	PVRScopeGraph(const PVRScopeGraph&); // deleted
	PVRScopeGraph& operator=(const PVRScopeGraph&);// deleted

	void recordCommandBuffer(pvr::api::SecondaryCommandBuffer &cmdBuffer, pvr::uint32 swapChain);
	void recordUIElements();
	void ping(pvr::float32 dt_millis);

	void showCounter(pvr::uint32 nCounter, bool bShow);
	bool isCounterShown(const pvr::uint32 nCounter) const;
	bool isCounterBeingDrawn(pvr::uint32 nCounter) const;
	bool isCounterPercentage(const pvr::uint32 nCounter) const;
	bool setActiveGroup(const pvr::uint32 nActiveGroup);
	pvr::uint32 getActiveGroup() const { return activeGroup; }
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

	pvr::uint32 getCounterNum() const { return numCounter; }

	const char* getCounterName(const pvr::uint32 i) const;
	int getCounterGroup(const pvr::uint32 i) const;

	void position(const pvr::uint32 nViewportW, const pvr::uint32 nViewportH, pvr::Rectanglei const& graph);

	void setUpdateInterval(const pvr::uint32 nUpdateInverval);
	bool isInitialized()const{ return _isInitialzed; }

	bool init(pvr::GraphicsContext &device, pvr::IAssetProvider& assetProvider, pvr::ui::UIRenderer& uiRenderer,
			  const pvr::api::RenderPass& renderPass, std::string& outMsg);
protected:
	void updateBufferLines();
	void update(pvr::float32 dt_millis);
	void filter();
	bool createPipeline(pvr::Api api, const pvr::api::RenderPass &renderPass, std::string &errorStr);
};
