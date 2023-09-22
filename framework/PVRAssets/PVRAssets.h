/*!
\brief Include this file if you wish to use the PVRAssets functionality.
\file PVRAssets/PVRAssets.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/Model.h"
#include "PVRAssets/fileio/PODReader.h"
#include "PVRAssets/fileio/GltfReader.h"
#include "PVRAssets/BoundingBox.h"
#include "PVRAssets/Geometry.h"
#include "PVRAssets/Helper.h"

/*****************************************************************************/
/*! \mainpage PVRAssets
******************************************************************************

\tableofcontents

\section overview Overview
*****************************

PVRAssets provides the necessary classes to work with assets and resources, like Model, Mesh, Texture, Effect, Animation, Camera and others. PVRAssets provides built-in support for reading all PowerVR deployment formats (PVR textures, POD models and PFX effects) into these classes, usually with a single line of code. PVRAssets classes are very flexible and can be adapted for a wide variety of uses.

*/
