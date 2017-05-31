/*!
\brief Includes files commonly required for PVRAssets code files. Include PVRAssets.h instead to use PVRAssets
functionality.
\file PVRAssets/AssetIncludes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
<<<<<<< HEAD
#include "PVRCore/RefCounted.h"
#include "PVRCore/IndexedArray.h"
#include "PVRCore/ContiguousMap.h"
#include "PVRCore/StringHash.h"
=======
#include "PVRCore/Interfaces.h"
#include "PVRCore/Base/RefCounted.h"
#include "PVRCore/DataStructures/IndexedArray.h"
#include "PVRCore/DataStructures/ContiguousMap.h"
#include "PVRCore/Strings/StringHash.h"
>>>>>>> 1776432f... 4.3
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