/*!*********************************************************************************************************************
\file         PVRApi\OGLES\Sync.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the OpenGL ES implementation details of MemoryBarrier and other synchronisation objects
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/Sync.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"

namespace pvr {
namespace api {
namespace impl {
class MemoryBarrierSetImpl
{
public:
	GLenum memBarrierFlags;
	MemoryBarrierSetImpl() : memBarrierFlags(0) {}
};
}

MemoryBarrierSet::MemoryBarrierSet() { pimpl.reset(new impl::MemoryBarrierSetImpl); }
MemoryBarrierSet::~MemoryBarrierSet() {}
MemoryBarrierSet& MemoryBarrierSet::addBarrier(MemoryBarrier barrier)
{
	pimpl->memBarrierFlags |= ConvertToGles::memBarrierFlagOut((uint32)barrier.dstMask);
	return *this;
}
MemoryBarrierSet& MemoryBarrierSet::addBarrier(const BufferRangeBarrier& barrier)
{
	pimpl->memBarrierFlags |= ConvertToGles::memBarrierFlagOut((uint32)barrier.dstMask);
	return *this;
}
MemoryBarrierSet& MemoryBarrierSet::addBarrier(const ImageAreaBarrier& barrier)
{
	pimpl->memBarrierFlags |= ConvertToGles::memBarrierFlagOut((uint32)barrier.dstMask);
	return *this;
}

const void* MemoryBarrierSet::getNativeMemoryBarriers()const
{
	return (const void*)(size_t)pimpl->memBarrierFlags;
}
}
}
//!\endcond NO_DOXYGEN
