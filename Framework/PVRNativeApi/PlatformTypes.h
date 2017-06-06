/*!
\brief Defines the wrappers to the native object types. Forward declared only. If you need to actually use these types
in your code, you will need to include the relevant specific file with the implementations (e.g.
EglPlatformHandles).
\file PVRNativeApi/PlatformTypes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/RefCounted.h"

namespace pvr {
namespace platform {
/// <summary>Wrapper struct containing all platform-specific handles used by a graphics context. Forward declaration.
/// </summary>
struct NativePlatformHandles_;

/// <todo_add_comment></todo_add_comment>
struct NativeSharedPlatformHandles_;

/// <summary>Pointer to a struct of platform handles. Used to pass around the undefined NativePlatformHandles_ struct.
/// </summary>
typedef RefCountedResource<NativePlatformHandles_> NativePlatformHandles;

/// <summary>Pointer to a struct of Shared context handles. Used to pass around the undefined NativeSharedPlatformHandles_
/// struct</summary>
typedef RefCountedResource<NativeSharedPlatformHandles_> NativeSharedPlatformHandles;

/// <summary>Wrapper struct containing a platform-specific Display handle. Forward declaration.</summary>
struct NativeDisplayHandle_;

/// <summary>Pointer to the display handle wrapper. Used to pass around the undefined NativeDisplayHandle_ struct.
/// </summary>
typedef RefCountedResource<NativeDisplayHandle_> NativeDisplayHandle;

/// <summary>Wrapper struct containing a platform-specific Window handle. Forward declaration.</summary>
struct NativeWindowHandle_;

/// <summary>Pointer to the window handle wrapper. Used to pass around the undefined NativeWindowHandle_ struct.
/// </summary>
typedef RefCountedResource<NativeWindowHandle_> NativeWindowHandle;
}
}