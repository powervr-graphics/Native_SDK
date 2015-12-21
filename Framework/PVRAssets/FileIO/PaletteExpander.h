/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/PaletteExpander.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Internally used by some texture readers.
***********************************************************************************************************************/
#pragma once

#include "PVRCore/CoreIncludes.h"

namespace pvr {
/*!***************************************************************************************************************
\brief         Internally used by some texture readers.
*****************************************************************************************************************/
	class PaletteExpander
{
public:
	PaletteExpander(const byte* const paletteData, uint32 paletteSize, uint32 bytesPerEntry);

	bool getColorFromIndex(uint32 index, byte* outputData) const;

private:
	const byte* const m_paletteData;
	const uint32      m_paletteSize;
	const uint32      m_bytesPerEntry;

	// Declare this as private to avoid warnings - the compiler can't generate it because of the const members.
	const PaletteExpander& operator=(const PaletteExpander&);
};
}