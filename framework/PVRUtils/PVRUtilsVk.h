/*!
\brief Include this file if you wish to use the PVRUtils functionality.
\file PVRUtils/PVRUtilsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/PVRCore.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/Vulkan/UIRendererVk.h"
#include "PVRUtils/Vulkan/HelperVk.h"
#include "PVRUtils/Vulkan/ShaderUtilsVk.h"
#include "PVRUtils/Vulkan/AsynchronousVk.h"
#include "PVRUtils/StructuredMemory.h"

/*****************************************************************************/
/*! \mainpage PVRUtils
******************************************************************************

\tableofcontents

\section Overview
*****************************

PVRUtilsVk provides several useful utilities for rendering with Vulkan through PVRVk.
PVRUtilsGles provides several useful utilities for rendering with OpenGL ES.

UIRenderer is a simple, platform agnostic library for rendering 2D elements in a 3D environment, written with text and sprites in mind, providing very useful capabilities.
A wealth of tools is provided to import fonts and create glyphs for specific or full parts of them.
For PVRUtilsVk, UIRenderer will use a Command Buffer provided to it to queue up the "recipe" for a render, allowing the user to defer it for later for using it inside complex
engines. For PVRUtilsGles, UIRenderer will execute the rendering commands immediately at the point of calling "render".

Other utilities include:

- A ton of helpers like instance/devices/swapchain creation for Vulkan and EGLContexts for OpenGLES, eliminating the overhead of configuring everything every time
- Uploading textures (easier said than done on Vulkan)
- For Vulkan, Multithreading tools, like the Asynchronous Texture Uploader
- For Vulkan, the very cool Render Manager rendering engine (you can see it in action in the Skinning example).

*/
