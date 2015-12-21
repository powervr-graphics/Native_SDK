/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\PaletteExpander.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods of the PaletteExpander class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>
#include "PaletteExpander.h"
namespace pvr {
PaletteExpander::PaletteExpander(const byte* const paletteData, uint32 paletteSize, uint32 bytesPerEntry)
	: m_paletteData(paletteData), m_paletteSize(paletteSize), m_bytesPerEntry(bytesPerEntry)
{
}

bool PaletteExpander::getColorFromIndex(uint32 index, byte* outputData) const
{
	if (m_paletteData != 0 && m_paletteSize != 0 && m_bytesPerEntry != 0)
	{
		if (index < (m_paletteSize / m_bytesPerEntry))
		{
			memcpy(outputData, &(m_paletteData[index * m_bytesPerEntry]), m_bytesPerEntry);
			return true;
		}
	}

	return false;
}
}
//!\endcond