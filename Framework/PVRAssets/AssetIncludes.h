/*!
\brief Includes files commonly required for PVRAssets code files. Include PVRAssets.h instead to use PVRAssets
functionality.
\file PVRAssets/AssetIncludes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/Interfaces.h"
#include "PVRCore/Base/RefCounted.h"
#include "PVRCore/DataStructures/IndexedArray.h"
#include "PVRCore/DataStructures/ContiguousMap.h"
#include "PVRCore/Strings/StringHash.h"
#include "PVRCore/Stream.h"
#include "PVRCore/IO/Asset.h"
#include <functional>
#include <map>

namespace pvr {
namespace assets {
typedef ::pvr::types::VertexAttributeLayout  VertexAttributeLayout;
typedef ::pvr::types::SamplerCreateParam SamplerCreateParam;

}
}