/*!
\brief Function implementations for the Command Pool.
\file PVRVk/CommandPoolVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/QueryPoolVk.h"
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
QueryPool_::QueryPool_(const DeviceWeakPtr& device, QueryType queryType, uint32_t queryCount, QueryPipelineStatisticFlags statisticsFlags)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_QUERY_POOL_EXT)
{
	_createInfo = {};
	_createInfo.sType = static_cast<VkStructureType>(StructureType::e_QUERY_POOL_CREATE_INFO);
	_createInfo.queryType = static_cast<VkQueryType>(queryType);
	_createInfo.queryCount = queryCount;
	_createInfo.pipelineStatistics = static_cast<VkQueryPipelineStatisticFlags>(statisticsFlags);
	vkThrowIfFailed(_device->getVkBindings().vkCreateQueryPool(_device->getVkHandle(), &_createInfo, NULL, &_vkHandle), "Create Query Pool failed");
}

bool QueryPool_::getResults(uint32_t queryIndex, size_t dataSize, void* data, QueryResultFlags flags)
{
	return getResults(queryIndex, 1, dataSize, data, 0, flags);
}

bool QueryPool_::getResults(uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* data, VkDeviceSize stride, QueryResultFlags flags)
{
	return (_device->getVkBindings().vkGetQueryPoolResults(
				_device->getVkHandle(), getVkHandle(), firstQuery, queryCount, dataSize, data, stride, static_cast<VkQueryResultFlags>(flags)) == Result::e_SUCCESS);
}
} // namespace impl
} // namespace pvrvk
