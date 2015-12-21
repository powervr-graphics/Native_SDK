/*!*********************************************************************************************************************
\file         PVRAssets/Texture/MetaData.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        The definition of the class used to represent Texture metadata.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"

namespace pvr {
namespace assets {

/*!*********************************************************************************************************************
\brief The TextureMetaData class contains metadata of a texture. Metadata is any information that a texture could be correctly loaded from
       file without. In most cases, metadata may still be necessary to actually USE the texture, such as winding orders, paddings, atlas
       information and others.
***********************************************************************************************************************/
class TextureMetaData
{
public:

	//	Values for each meta data type that we know about. Texture arrays hinge on each surface being identical in all but content,
	//	including meta data. If the meta data varies even slightly then a new texture should be used. It is possible to write your own
	//	extension to get around this however.
	enum Identifier
	{
		IdentifierTextureAtlasCoords = 0,
		IdentifierBumpData,
		IdentifierCubeMapOrder,
		IdentifierTextureOrientation,
		IdentifierBorderData,
		IdentifierPadding,
		IdentifierNumMetaDataTypes
	};

	//Axes, used to query orientations.
	enum Axis
	{
		AxisAxisX = 0,
		AxisAxisY = 1,
		AxisAxisZ = 2
	};

	//Orientations of various axes.
	enum AxisOrientation
	{
		AxisOrientationLeft  = 1 << AxisAxisX,
		AxisOrientationRight = 0,
		AxisOrientationUp    = 1 << AxisAxisY,
		AxisOrientationDown  = 0,
		AxisOrientationOut   = 1 << AxisAxisZ,
		AxisOrientationIn    = 0
	};

public:
	TextureMetaData();
	TextureMetaData(uint32 fourCC, uint32 key, uint32 dataSize, const byte* data);
	TextureMetaData(const TextureMetaData& rhs);
	~TextureMetaData();

	TextureMetaData&      operator=(const TextureMetaData& rhs);

	/*!*******************************************************************************************************************************
	\brief	Load the this texture meta data from a stream
	\return	Return true on success
	\param	stream Stream to load the meta data from
	**********************************************************************************************************************************/
	bool loadFromStream(Stream& stream);

	/*!*******************************************************************************************************************************
	\brief	Write this texture meta data in to a stream
	\return	Return true on success
	\param	stream Stream to write in to.
	**********************************************************************************************************************************/
	bool writeToStream(Stream& stream) const;

	/*!*******************************************************************************************************************************
	\brief Get the 4cc descriptor of the data type's creator. Values equating to values between 'P' 'V' 'R' 0 and 'P' 'V' 'R' 255 will be used by our headers.
	\return	Return 4cc descriptor of the data type's creator.
	**********************************************************************************************************************************/
	uint32 getFourCC() const;

	/*!*******************************************************************************************************************************
	\brief	Get the data size of this meta data
	\return	Return the size of the meta data
	**********************************************************************************************************************************/
	uint32 getDataSize() const;

	/*!*******************************************************************************************************************************
	\brief	Get the enumeration key identifying the data type.
	\return	Return the enumeration key.
	**********************************************************************************************************************************/
	uint32 getKey() const;

	/*!*******************************************************************************************************************************
	\brief	Get the data, can be absolutely anything, the loader needs to know how to handle it based on fourCC and key.
	\return	Return the data
	**********************************************************************************************************************************/
	const uint8* getData() const;


	/*!*******************************************************************************************************************************
	\brief	Get the data total size in memory
	\return	Return the data total size in memory
	**********************************************************************************************************************************/
	uint32 getTotalSizeInMemory() const;

private:
	uint32 m_fourCC;   // A 4cc descriptor of the data type's creator.
	// Values equating to values between 'P' 'V' 'R' 0 and 'P' 'V' 'R' 255 will be used by our headers.
	uint32	m_key;      // Enumeration key identifying the data type.
	uint32	m_dataSize;	// Size of attached data.
	uint8*	m_data;     // Data array, can be absolutely anything, the loader needs to know how to handle it based on fourCC and key.
};
}
}