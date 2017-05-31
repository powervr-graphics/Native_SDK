/*!
\brief Internally used by some texture readers.
\file PVRAssets/FileIO/PaletteExpander.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/CoreIncludes.h"

namespace pvr {
/// <summary>Internally used by some texture readers.</summary>
	class PaletteExpander
{
public:
	PaletteExpander(const byte* const paletteData, uint32 paletteSize, uint32 bytesPerEntry);

	bool getColorFromIndex(uint32 index, byte* outputData) const;

private:
	const byte* const _paletteData;
	const uint32      _paletteSize;
	const uint32      _bytesPerEntry;

	// Declare this as private to avoid warnings - the compiler can't generate it because of the const members.
	const PaletteExpander& operator=(const PaletteExpander&);
};
}