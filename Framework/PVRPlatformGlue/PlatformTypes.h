/*!*********************************************************************************************************************
\file         PVRPlatformGlue\PlatformTypes.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Defines the wrappers to the native object types. Forward declared only. If you need to actually use these types in
              your code, you will need to include the relevant specific file with the implementations (e.g. EglPlatformHandles).
***********************************************************************************************************************/
#pragma once
#include "PVRCore/RefCounted.h"

namespace pvr {
namespace platform {
/*!*********************************************************************************************************************
\brief         Wrapper struct containing all platform-specific handles used by a graphics context. Forward declaration.
***********************************************************************************************************************/
struct NativePlatformHandles_;

/*!*********************************************************************************************************************
\brief         Pointer to a struct of platform handles. Used to pass around the undefined NativePlatformHandles_ struct.
***********************************************************************************************************************/
typedef std::auto_ptr<NativePlatformHandles_> NativePlatformHandles;

/*!*********************************************************************************************************************
\brief         Wrapper struct containing a platform-specific Display handle. Forward declaration.
***********************************************************************************************************************/
struct NativeDisplayHandle_;

/*!*********************************************************************************************************************
\brief         Pointer to the display handle wrapper. Used to pass around the undefined NativeDisplayHandle_ struct.
***********************************************************************************************************************/
typedef std::auto_ptr<NativeDisplayHandle_> NativeDisplayHandle;

/*!*********************************************************************************************************************
\brief         Wrapper struct containing a platform-specific Window handle. Forward declaration.
***********************************************************************************************************************/
struct NativeWindowHandle_;

/*!*********************************************************************************************************************
\brief         Pointer to the window handle wrapper. Used to pass around the undefined NativeWindowHandle_ struct.
***********************************************************************************************************************/
typedef std::auto_ptr<NativeWindowHandle_> NativeWindowHandle;
}
}