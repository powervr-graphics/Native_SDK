/*!*********************************************************************************************************************
\file         PVRCore\CoreIncludes.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Commonly included files from other PVRCore files. To include PVRCore functionality, use PVRCore.h instead.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Defines.h"
#include "PVRCore/Maths.h"
#include "PVRCore/Log.h"
#include "PVRCore/MultiObject.h"
#include "../Builds/Include/sdkver.h"

namespace pvr {
template<typename T>
inline void assertRefcountValid(const T& t)
{
	pvr::assertion(t.isValid(), "RefcountedResource asserted not valid.");
}
}