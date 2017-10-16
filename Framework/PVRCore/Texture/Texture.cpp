/*!
\brief Implementation of methods of the Texture class.
\file PVRCore/Texture/Texture.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/Texture.h"
#include <algorithm>

namespace pvr {

TextureFileFormat getTextureFormatFromFilename(const char* assetname)
{
	std::string file(assetname);

	size_t period = file.rfind(".");
	if (period != string::npos)
	{
		string s = file.substr(period + 1);
		std::transform(s.begin(), s.end(), s.begin(), tolower);
		if (!s.compare("pvr")) { return TextureFileFormat::PVR; }
		if (!s.compare("tga")) { return TextureFileFormat::TGA; }
		if (!s.compare("ktx")) { return TextureFileFormat::KTX; }
		if (!s.compare("bmp")) { return TextureFileFormat::BMP; }
		if (!s.compare("dds")) { return TextureFileFormat::DDS; }
		if (!s.compare("ddx")) { return TextureFileFormat::DDX; }
	}
	return TextureFileFormat::UNKNOWN;
}

uint8 Texture::getPixelSize()const { return _header.pixelFormat.getBitsPerPixel() / 8; }

Texture::Texture() {	_pTextureData.resize(getDataSize()); }

Texture::Texture(const TextureHeader& sHeader, const byte* pData)
	: TextureHeader(sHeader)
{
	//Allocate new memory for the texture.
	_pTextureData.resize(getDataSize());

	//If there is data supplied, copy it into this texture.
	if (pData && getDataSize())
	{
		memcpy(&_pTextureData[0], pData, getDataSize());
	}
}

void Texture::initializeWithHeader(const TextureHeader& sHeader)
{
	*this = sHeader;
	//Get the data size from the newly attached header.
	_pTextureData.resize(getDataSize());
}

const byte* Texture::getDataPointer(uint32 mipMapLevel/*= 0*/, uint32 arrayMember/*= 0*/, uint32 face/*= 0*/) const
{
	uint32 offSet = 0;

	if ((int32)mipMapLevel == pvrTextureAllMIPMaps) {	return NULL;	}

	if (mipMapLevel >= getNumberOfMIPLevels() || arrayMember >= getNumberOfArrayMembers() || face >= getNumberOfFaces())
	{
		return NULL;
	}

	//File is organised by MIP Map levels, then surfaces, then faces.

	//Get the start of the MIP level.
	if (mipMapLevel != 0)
	{
		//Get the size for all MIP Map levels up to this one.
		for (uint32 uiCurrentMIPMap = 0; uiCurrentMIPMap < mipMapLevel; ++uiCurrentMIPMap)
		{
			offSet += getDataSize(uiCurrentMIPMap, true, true);
		}
	}

	//Get the start of the array.
	if (arrayMember != 0) {	offSet += arrayMember * getDataSize(mipMapLevel, false, true);	}

	//Get the start of the face.
	if (face != 0) {	offSet += face * getDataSize(mipMapLevel, false, false);}

	//Return the data pointer plus whatever offSet has been specified.
	return &_pTextureData[offSet];
}

byte* Texture::getDataPointer(uint32 mipMapLevel/*= 0*/, uint32 arrayMember/*= 0*/, uint32 face/*= 0*/)
{
	//Initialize the offSet value.
	uint32 offSet = 0;

	//Error checking
	if ((int32)mipMapLevel == pvrTextureAllMIPMaps) {	return NULL;}

	if (mipMapLevel >= getNumberOfMIPLevels() || arrayMember >= getNumberOfArrayMembers() || face >= getNumberOfFaces())
	{
		return NULL;
	}

	//File is organised by MIP Map levels, then surfaces, then faces.

	//Get the start of the MIP level.
	if (mipMapLevel != 0)
	{
		//Get the size for all MIP Map levels up to this one.
		for (uint32 uiCurrentMIPMap = 0; uiCurrentMIPMap < mipMapLevel; ++uiCurrentMIPMap)
		{
			offSet += getDataSize(uiCurrentMIPMap, true, true);
		}
	}

	//Get the start of the array.
	if (arrayMember != 0)
	{
		offSet += arrayMember * getDataSize(mipMapLevel, false, true);
	}

	//Get the start of the face.
	if (face != 0)
	{
		offSet += face * getDataSize(mipMapLevel, false, false);
	}

	//Return the data pointer plus whatever offSet has been specified.
	return &_pTextureData[offSet];
}

void Texture::addPaddingMetaData(uint32 paddingAlignment)
{
	//If the alignment is 0 or 1, return - as nothing is required
	if (paddingAlignment <= 1)
	{
		return;
	}

	//Set the meta data padding. The 12 is the size of an empty meta data block
	uint32 unpaddedStartOfTextureData = (Header::SizeOfHeader + getMetaDataSize() + 12);

	//Work out the value of the padding
	uint32 paddingAmount = ((uint32(-1) * unpaddedStartOfTextureData) % paddingAlignment);

	//Create the meta data
	TextureMetaData metaPadding(Header::PVRv3, TextureMetaData::IdentifierPadding, paddingAmount, NULL);

	//Add the meta data to the texture
	addMetaData(metaPadding);
}

}
//!\endcond