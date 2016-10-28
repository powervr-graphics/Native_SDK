/*!*********************************************************************************************************************
\file         PVRAssets/AssetIncludes.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Includes files commonly required for PVRAssets code files. Include PVRAssets.h instead to use PVRAssets functionality.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/RefCounted.h"
#include "PVRCore/IndexedArray.h"
#include "PVRCore/ContiguousMap.h"
#include "PVRCore/StringHash.h"
#include "PVRCore/Stream.h"
#include <functional>
#include <map>

namespace pvr {
namespace assets {
typedef ::pvr::types::VertexAttributeLayout  VertexAttributeLayout;
typedef ::pvr::types::SamplerCreateParam SamplerCreateParam;
}
}