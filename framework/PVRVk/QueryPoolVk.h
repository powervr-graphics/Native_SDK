/*!
\brief The PVRVk QueryPool, a pool that can create queries.
\file PVRVk/QueryPoolVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Vulkan implementation of the Query Pool class.
/// Destroying the query pool will also destroy the queries allocated from this pool
/// </summary>
class QueryPool_ : public EmbeddedRefCount<QueryPool_>, public DeviceObjectHandle<VkQueryPool>, public DeviceObjectDebugMarker<QueryPool_>
{
	// Implementing EmbeddedRefCount
	template<typename>
	friend class ::pvrvk::EmbeddedRefCount;

public:
	DECLARE_NO_COPY_SEMANTICS(QueryPool_)

	/// <summary>Retrieves the status and results for a particular query</summary>
	/// <param name="queryIndex">The initial query index</param>
	/// <param name="dataSize">The size of bytes of the buffer pointed to by data</param>
	/// <param name="data">A pointer to a user allocated buffer where the results will be written</param>
	/// <param name="flags">Specifies how and when results are returned</param>
	/// <returns>True if the results or status' for the set of queries were successfully retrieved</returns>
	bool getResults(uint32_t queryIndex, size_t dataSize, void* data, QueryResultFlags flags);

	/// <summary>Retrieves the status and results for a set of queries</summary>
	/// <param name="firstQuery">The initial query index</param>
	/// <param name="queryCount">The number of queries</param>
	/// <param name="dataSize">The size of bytes of the buffer pointed to by data</param>
	/// <param name="data">A pointer to a user allocated buffer where the results will be written</param>
	/// <param name="stride">The stide in bytes between results for individual queries within data</param>
	/// <param name="flags">Specifies how and when results are returned</param>
	/// <returns>True if the results or statuses for the set of queries were successfully retrieved</returns>
	bool getResults(uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* data, VkDeviceSize stride, QueryResultFlags flags);

	/// <summary>Returns the number of queries in the QueryPool</summary>
	/// <returns>The number of queries being managed by the QueryPool</returns>
	uint32_t getNumQueries() const
	{
		return _createInfo.queryCount;
	}

private:
	friend class ::pvrvk::impl::Device_;

	/// <summary>Construct a QueryPool</summary>
	/// <param name="device">The GpuDevice this query pool will be constructed from.</param>
	QueryPool_(const DeviceWeakPtr& device, pvrvk::QueryType queryType, uint32_t queryCount, pvrvk::QueryPipelineStatisticFlags statisticsFlags);

	/* IMPLEMENTING EmbeddedResource */
	void destroyObject()
	{
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			if (_device.isValid())
			{
				_device->getVkBindings().vkDestroyQueryPool(_device->getVkHandle(), getVkHandle(), nullptr);
				_vkHandle = VK_NULL_HANDLE;
				_device.reset();
			}
			else
			{
				reportDestroyedAfterDevice("QueryPool");
			}
		}
	}

	VkQueryPoolCreateInfo _createInfo;
};
} // namespace impl
} // namespace pvrvk
