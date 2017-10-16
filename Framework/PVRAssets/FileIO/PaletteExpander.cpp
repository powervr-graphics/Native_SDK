/*!
\brief Implementation of methods of the PaletteExpander class.
\file PVRAssets/FileIO/PaletteExpander.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>
#include "PaletteExpander.h"
namespace pvr {
PaletteExpander::PaletteExpander(const byte* const paletteData, uint32 paletteSize, uint32 bytesPerEntry)
	: _paletteData(paletteData), _paletteSize(paletteSize), _bytesPerEntry(bytesPerEntry)
{
}

bool PaletteExpander::getColorFromIndex(uint32 index, byte* outputData) const
{
	if (_paletteData != 0 && _paletteSize != 0 && _bytesPerEntry != 0)
	{
		if (index < (_paletteSize / _bytesPerEntry))
		{
			memcpy(outputData, &(_paletteData[index * _bytesPerEntry]), _bytesPerEntry);
			return true;
		}
	}

	return false;
}
}
//!\endcond