/*!
\brief Implementation of function to load shader files
\file PVRSuperResolution/FileIO.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <string>

namespace pvr {

/// <summary>Loads a file as a unsigned char vector.</summary>
/// <param name="fileName">Name of the file to load.</param>
/// <param name="pointerData">Vector with the file unsigned char information.</param>
/// <param name="application">Application loading the file (required on Android).</param>
void loadFile(const std::string& fileName, std::vector<unsigned char>& pointerData, void* application);

} // namespace pvr
