/*!
\brief Include this file if you wish to use the PVRCore functionality
\file PVRCore/PVRCore.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#ifndef PVRCORE_NO_GLM
#include "PVRCore/math/MathUtils.h"
#endif
#include "PVRCore/stream/BufferStream.h"
#include "PVRCore/types/Types.h"
#include "PVRCore/strings/StringFunctions.h"
#include "PVRCore/strings/StringHash.h"
#include "PVRCore/IAssetProvider.h"
#include "PVRCore/commandline/CommandLine.h"
#include "PVRCore/textureio/TextureIO.h"
#include "PVRCore/texture/TextureLoad.h"
#include "PVRCore/stream/FilePath.h"
#include "PVRCore/Time_.h"

#include <iterator>
// RefCounted.h has been made deprecated and is now unused throughout the PowerVR SDK
#include "PVRCore/RefCounted.h"

/*****************************************************************************/
/*! \mainpage PVRCore
******************************************************************************

\tableofcontents

\section overview Overview
*****************************

PVRCore is a collection of supporting code for the PowerVR Framework. Code using other modules of the Framework should link with PVRCore.

Some examples of code that can be found in PVRCore:

- Utility classes and specialized data structures used by the Framework (RingBuffer.h, ContiguousMap.h)
- The main Smart Pointer class used by the Framework (RefCounted.h)
- Data streams (e.g. FileStream.h, BufferStream.h)
- Logging and error reporting (Log.h)
- Special math (projection matrix calculations, bounding boxes, shadow volumes).

*/
