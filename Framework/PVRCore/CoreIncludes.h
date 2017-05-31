/*!
\brief Commonly included files from other PVRCore files. To include PVRCore functionality, use PVRCore.h instead.
\file PVRCore/CoreIncludes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Defines.h"
#include "PVRCore/Maths.h"
#include "PVRCore/Log.h"
<<<<<<< HEAD
#include "PVRCore/RefCounted.h"
#include "PVRCore/MultiObject.h"
#include "PVRCore/SortedArray.h"
#include "PVRCore/IndexedArray.h"
#include "PVRCore/ContiguousMap.h"
#include "PVRCore/FreeValue.h"
#include "../Builds/Include/sdkver.h"

namespace pvr {
template<typename T>
inline void assertRefcountValid(const T& t)
{
	pvr::assertion(t.isValid(), "RefcountedResource asserted not valid.");
}
}
=======
#include "PVRCore/Base/ComplexTypes.h"
#include "PVRCore/Base/RefCounted.h"
#include "PVRCore/DataStructures/MultiObject.h"
#include "PVRCore/DataStructures/SortedArray.h"
#include "PVRCore/DataStructures/IndexedArray.h"
#include "PVRCore/DataStructures/ContiguousMap.h"
#include "PVRCore/DataStructures/FreeValue.h"
#include "../Builds/Include/sdkver.h"
>>>>>>> 1776432f... 4.3
