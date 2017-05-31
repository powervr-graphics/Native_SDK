/*!
\brief Include this file if you wish to use the PVRShell functionality, but you will NOT be linking with a PVRApi
library. Only include this file once, in one of your CPP files. If you need to include PVRShell in multiple
locations, include this file in one of your main cpp files, and PVRShell.h in the rest.
\file PVRShell/PVRShellNoPVRApi.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//If you do not use this file, some platforms will give linker errors if using PVRShell without PVRApi.
//If you include this file from multiple source files, some platforms will emit multiply defined symbol errors.
//This file provides a stub implementation for the pvr::createGraphicsContext and other required libraries.
#pragma once
#include "PVRShell/PVRShell.h"

namespace pvr {
/// <summary>This is an empty implementation of an external PVRApi function call that PVRShell will normally use.
/// Used when PVRApi will not be linked into the application</summary>
/// <returns>An empty Graphics Context.</returns>
pvr::GraphicsContextStrongReference PVR_API_FUNC createGraphicsContext()
{
	return GraphicsContextStrongReference();
}
}