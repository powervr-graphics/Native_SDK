/*!
\brief The definition of the class used to represent Texture metadata.
\file PVRCore/Texture/MetaData.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Stream.h"

namespace pvr {
/// <summary>The TextureMetaData class contains metadata of a texture. Metadata is any information that a texture
/// could be correctly loaded from file without. In most cases, metadata may still be necessary to actually USE the
/// texture, such as winding orders, paddings, atlas information and others.</summary>
class TextureMetaData
{
public:

	//  Values for each meta data type that we know about. Texture arrays hinge on each surface being identical in all but content,
	//  including meta data. If the meta data varies even slightly then a new texture should be used. It is possible to write your own
	//  extension to get around this however.
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

	/// <summary>Load the this texture meta data from a stream</summary>
	/// <param name="stream">Stream to load the meta data from</param>
	/// <returns>Return true on success</returns>
	bool loadFromStream(Stream& stream);

	/// <summary>Write this texture meta data in to a stream</summary>
	/// <param name="stream">Stream to write in to.</param>
	/// <returns>Return true on success</returns>
	bool writeToStream(Stream& stream) const;

	/// <summary>Get the 4cc descriptor of the data type's creator. Values equating to values between 'P' 'V' 'R' 0
	/// and 'P' 'V' 'R' 255 will be used by our headers.</summary>
	/// <returns>Return 4cc descriptor of the data type's creator.</returns>
	uint32 getFourCC() const;

	/// <summary>Get the data size of this meta data</summary>
	/// <returns>Return the size of the meta data</returns>
	uint32 getDataSize() const;

	/// <summary>Get the enumeration key identifying the data type.</summary>
	/// <returns>Return the enumeration key.</returns>
	uint32 getKey() const;

	/// <summary>Get the data, can be absolutely anything, the loader needs to know how to handle it based on fourCC
	/// and key.</summary>
	/// <returns>Return the data</returns>
	const uint8* getData() const;


	/// <summary>Get the data total size in memory</summary>
	/// <returns>Return the data total size in memory</returns>
	uint32 getTotalSizeInMemory() const;

private:
	uint32 _fourCC;   // A 4cc descriptor of the data type's creator.
	// Values equating to values between 'P' 'V' 'R' 0 and 'P' 'V' 'R' 255 will be used by our headers.
	uint32  _key;      // Enumeration key identifying the data type.
	uint32  _dataSize;  // Size of attached data.
	uint8*  _data;     // Data array, can be absolutely anything, the loader needs to know how to handle it based on fourCC and key.
};
}