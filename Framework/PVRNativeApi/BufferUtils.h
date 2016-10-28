/*!*********************************************************************************************************************
\file         PVRNativeApi\BufferUtils.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contain functions for creating API Buffer object.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/ForwardDecApiObjects.h"
#include "PVRCore/IPlatformContext.h"

namespace pvr {
namespace utils {

/*!******************************************************************************************************************************
\brief create buffer
\return Return true if success
\param[in] context PlatformContext used for allocation
\param[in] usage Buffer binding use
\param[in] size Buffer size
\param[in] memHostVisible Allow Buffer memory to be host visible for map and un-map operation
\param[in] outBuffer The buffer used for memory allocation
********************************************************************************************************************************/
bool createBuffer(IPlatformContext& context, types::BufferBindingUse usage,
                  pvr::uint32 size, bool memHostVisible, native::HBuffer_& outBuffer);

}
}