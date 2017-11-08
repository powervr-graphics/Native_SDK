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
PaletteExpander::PaletteExpander(const uint8_t* paletteData, uint32_t paletteSize, uint32_t bytesPerEntry)
	: _paletteData(paletteData), _paletteSize(paletteSize), _bytesPerEntry(bytesPerEntry)
{
}

bool PaletteExpander::getColorFromIndex(uint32_t index, unsigned char* outputData) const
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