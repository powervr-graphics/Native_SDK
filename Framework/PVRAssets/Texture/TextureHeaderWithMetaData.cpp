/*!*********************************************************************************************************************
\file         PVRAssets\Texture\TextureHeaderWithMetaData.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods of the TextureHeaderWithMetadata class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>
#include <algorithm>
#include "PVRAssets/Texture/TextureHeaderWithMetaData.h"
#include "PVRAssets/Texture/TextureHeader.h"
#include "PVRAssets/Texture/TextureFormats.h"
#include "PVRAssets/FileIO/FileDefinesDDS.h"
#include "PVRAssets/PixelFormat.h"

using std::string;
using std::map;
namespace pvr {
namespace assets {

TextureHeaderWithMetaData::TextureHeaderWithMetaData(const TextureHeaderWithMetaData& rhs)
	: TextureHeader(rhs)
{
	//Copy the meta data
	m_metaDataMap = rhs.m_metaDataMap;
}

TextureHeaderWithMetaData& TextureHeaderWithMetaData::operator=(const TextureHeaderWithMetaData& rhs)
{
	//If it equals itself, return early.
	if (&rhs == this)
	{
		return *this;
	}

	//Copy the header over.
	m_header = rhs.m_header;

	//Copy the meta data
	m_metaDataMap = rhs.m_metaDataMap;

	//Return
	return *this;
}

TextureHeaderWithMetaData::TextureHeaderWithMetaData(Header fileHeader, uint32 metaDataCount, TextureMetaData* metaData)
	: TextureHeader(fileHeader)
{
	if (metaData)
	{
		for (uint32 i = 0; i < metaDataCount; ++i)
		{
			addMetaData(metaData[i]);
		}
	}
}

TextureMetaData::AxisOrientation TextureHeaderWithMetaData::getOrientation(TextureMetaData::Axis axis) const
{
	//Make sure the meta block exists
	MetaDataMapType::const_iterator foundIdentifer = m_metaDataMap.find(Header::PVRv3);
	if (foundIdentifer != m_metaDataMap.end())
	{
		map<uint32, TextureMetaData>::const_iterator foundTexMetaData = foundIdentifer->second.find(
		      TextureMetaData::IdentifierTextureOrientation);
		if (foundTexMetaData != foundIdentifer->second.end())
		{
			return (TextureMetaData::AxisOrientation)
			       (foundTexMetaData->second.getData()[axis]);
		}
	}
	return (TextureMetaData::AxisOrientation)0; //Default is the flag values.
}

bool TextureHeaderWithMetaData::isBumpMap() const
{
	//Make sure the meta block exists
	MetaDataMapType::const_iterator found = m_metaDataMap.find(Header::PVRv3);
	if (found != m_metaDataMap.end())
	{
		uint32 keyToFind = TextureMetaData::IdentifierBumpData;
		return (found->second.find(keyToFind) != found->second.end());
	}
	return false;
}

float TextureHeaderWithMetaData::getBumpMapScale() const
{
	//Make sure the meta block exists
	MetaDataMapType::const_iterator foundIdentifier = m_metaDataMap.find(Header::PVRv3);
	if (foundIdentifier != m_metaDataMap.end())
	{
		uint32 keyToFind = TextureMetaData::IdentifierBumpData;
		map<uint32, TextureMetaData>::const_iterator foundMetaData = foundIdentifier->second.find(keyToFind);
		if (foundMetaData != foundIdentifier->second.end())
		{
			return ((float*)(foundMetaData->second.getData()))[0];
		}
	}

	return 0.0f;
}

const string TextureHeaderWithMetaData::getBumpMapOrder() const
{
	//Make sure the meta block exists
	MetaDataMapType::const_iterator foundIdentifier = m_metaDataMap.find(Header::PVRv3);
	if (foundIdentifier != m_metaDataMap.end())
	{
		map<uint32, TextureMetaData>::const_iterator foundMetaData = foundIdentifier->second.find(
		      TextureMetaData::IdentifierBumpData);
		if (foundMetaData != foundIdentifier->second.end())
		{
			char bumpOrder[5];
			bumpOrder[4] = 0;
			memcpy(bumpOrder, (foundMetaData->second.getData() + 4), 4);
			return string(bumpOrder);
		}
	}

	return string("");
}

int TextureHeaderWithMetaData::getNumberOfTextureAtlasMembers() const
{
	//Make sure the meta block exists
	MetaDataMapType::const_iterator foundIdentifier = m_metaDataMap.find(Header::PVRv3);
	if (foundIdentifier != m_metaDataMap.end())
	{
		map<uint32, TextureMetaData>::const_iterator foundMetaData = foundIdentifier->second.find(
		      TextureMetaData::IdentifierTextureAtlasCoords);
		if (foundMetaData != foundIdentifier->second.end())
		{
			uint32 numDimensions = getWidth() > 1 ? 1 : 0 + getHeight() > 1 ? 1 : 0 + getDepth() > 1 ? 1 : 0;
			return numDimensions * foundMetaData->second.getDataSize() / 4;
		}
	}

	return 0;
}

const float* TextureHeaderWithMetaData::getTextureAtlasData() const
{
	//Make sure the meta block exists
	MetaDataMapType::const_iterator foundIdentifier = m_metaDataMap.find(Header::PVRv3);
	if (foundIdentifier != m_metaDataMap.end())
	{
		map<uint32, TextureMetaData>::const_iterator foundMetaData = foundIdentifier->second.find(
		      TextureMetaData::IdentifierTextureAtlasCoords);
		if (foundMetaData != foundIdentifier->second.end())
		{
			return (const float*)foundMetaData->second.getData();
		}
	}

	return NULL;
}

const string TextureHeaderWithMetaData::getCubeMapOrder() const
{
	//Make sure the meta block exists
	MetaDataMapType::const_iterator foundIdentifier = m_metaDataMap.find(Header::PVRv3);
	if (getNumberOfFaces() > 1)
	{
		if (foundIdentifier != m_metaDataMap.end())
		{
			map<uint32, TextureMetaData>::const_iterator foundMetaData = foundIdentifier->second.find(
			      TextureMetaData::IdentifierCubeMapOrder);
			if (foundMetaData != foundIdentifier->second.end())
			{
				char8 cubeMapOrder[7];
				cubeMapOrder[6] = 0;
				memcpy(cubeMapOrder, (foundMetaData->second.getData()), 6);
				return string(cubeMapOrder);
			}
		}

		string defaultOrder("XxYyZz");

		// Remove characters for faces that don't exist
		defaultOrder.erase(defaultOrder.size() - 6 - getNumberOfFaces());

		return defaultOrder;
	}

	return string("");
}

void TextureHeaderWithMetaData::getBorder(uint32& uiBorderWidth, uint32& uiBorderHeight,
    uint32& uiBorderDepth) const
{
	//Default values
	uiBorderWidth = 0;
	uiBorderHeight = 0;
	uiBorderDepth = 0;

	//Make sure the meta block exists
	MetaDataMapType::const_iterator foundIdentifier = m_metaDataMap.find(Header::PVRv3);
	if (foundIdentifier != m_metaDataMap.end())
	{
		map<uint32, TextureMetaData>::const_iterator foundMetaData = foundIdentifier->second.find(
		      TextureMetaData::IdentifierBorderData);
		if (foundMetaData != foundIdentifier->second.end())
		{
			uint32* pBorderData = (uint32*)(foundMetaData->second.getData());
			uiBorderWidth = pBorderData[0];
			uiBorderHeight = pBorderData[1];
			uiBorderDepth = pBorderData[2];
		}
	}
}

const TextureMetaData TextureHeaderWithMetaData::getMetaData(uint32 fourCC, uint32 key) const
{
	//Make sure the meta block exists
	MetaDataMapType::const_iterator foundFourCC = m_metaDataMap.find(fourCC);
	if (foundFourCC != m_metaDataMap.end())
	{
		map<uint32, TextureMetaData>::const_iterator foundMetaData = foundFourCC->second.find(key);
		if (foundMetaData != foundFourCC->second.end())
		{
			return foundMetaData->second;
		}
	}

	return TextureMetaData();
}

bool TextureHeaderWithMetaData::hasMetaData(uint32 fourCC, uint32 key) const
{
	//Make sure the meta block exists
	MetaDataMapType::const_iterator foundFourCC = m_metaDataMap.find(fourCC);
	if (foundFourCC != m_metaDataMap.end())
	{
		return (foundFourCC->second.find(key) != foundFourCC->second.end());
	}

	return false;
}

const map<uint32, map<uint32, TextureMetaData>/**/>* TextureHeaderWithMetaData::getMetaDataMap() const
{
	return &m_metaDataMap;
}

void TextureHeaderWithMetaData::setOrientation(TextureMetaData::AxisOrientation eAxisOrientation)
{
	//Get a reference to the meta data block.
	TextureMetaData& orientationMetaData = m_metaDataMap[Header::PVRv3][TextureMetaData::IdentifierTextureOrientation];

	//Check if it's already been set or not.
	if (orientationMetaData.getData())
	{
		m_header.metaDataSize -= orientationMetaData.getTotalSizeInMemory();
	}

	// Set the orientation data
	byte orientationData[3];

	//Check for left/right (x-axis) orientation
	if ((eAxisOrientation & TextureMetaData::AxisOrientationLeft) > 0)
	{
		orientationData[TextureMetaData::AxisAxisX] = TextureMetaData::AxisOrientationLeft;
	}
	else
	{
		orientationData[TextureMetaData::AxisAxisX] = TextureMetaData::AxisOrientationRight;
	}

	//Check for up/down (y-axis) orientation
	if ((eAxisOrientation & TextureMetaData::AxisOrientationUp) > 0)
	{
		orientationData[TextureMetaData::AxisAxisY] = TextureMetaData::AxisOrientationUp;
	}
	else
	{
		orientationData[TextureMetaData::AxisAxisY] = TextureMetaData::AxisOrientationDown;
	}

	//Check for in/out (z-axis) orientation
	if ((eAxisOrientation & TextureMetaData::AxisOrientationOut) > 0)
	{
		orientationData[TextureMetaData::AxisAxisZ] = TextureMetaData::AxisOrientationOut;
	}
	else
	{
		orientationData[TextureMetaData::AxisAxisZ] = TextureMetaData::AxisOrientationIn;
	}

	// Update the meta data block
	orientationMetaData = TextureMetaData(Header::PVRv3, TextureMetaData::IdentifierTextureOrientation, 3, orientationData);

	// Check that the meta data was created successfully.
	if (orientationMetaData.getDataSize() != 0)
	{
		// Increment the meta data size.
		m_header.metaDataSize += orientationMetaData.getTotalSizeInMemory();
	}
	else
	{
		// Otherwise remove it.
		m_metaDataMap.erase(TextureMetaData::IdentifierTextureOrientation);
	}
}

void TextureHeaderWithMetaData::setBumpMap(float bumpScale, string bumpOrder)
{
	if (bumpOrder.find_first_not_of("xyzh") != std::string::npos)
	{
		pvr::Log("Invalid bumpmap order string");
		assertion(false ,  "Invalid bumpmap order string");
		return;
	}

	//Get a reference to the meta data block.
	TextureMetaData& bumpMetaData = m_metaDataMap[Header::PVRv3][TextureMetaData::IdentifierBumpData];

	//Check if it's already been set or not.
	if (bumpMetaData.getData())
	{
		m_header.metaDataSize -= bumpMetaData.getTotalSizeInMemory();
	}


	// Initialize and clear the bump map data
	byte bumpData[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	//Copy the floating point scale and character order into the bumpmap data
	memcpy(bumpData, &bumpScale, 4);
	memcpy(bumpData + 4, bumpOrder.data(), (std::min)(bumpOrder.length(), (size_t)4));

	bumpMetaData = TextureMetaData(Header::PVRv3, TextureMetaData::IdentifierBumpData, 8, bumpData);

	//Increment the meta data size.
	m_header.metaDataSize += bumpMetaData.getTotalSizeInMemory();
}

void TextureHeaderWithMetaData::setTextureAtlas(const float32* const textureAtlasData, uint32 dataSize)
{
	//Get a reference to the meta data block.
	TextureMetaData& textureAtlasMetaData = m_metaDataMap[Header::PVRv3][TextureMetaData::IdentifierTextureAtlasCoords];

	//Check if it's already been set or not.
	if (textureAtlasMetaData.getData())
	{
		m_header.metaDataSize -= textureAtlasMetaData.getTotalSizeInMemory();
	}

	textureAtlasMetaData = TextureMetaData(Header::PVRv3, TextureMetaData::IdentifierTextureAtlasCoords,
	                                       dataSize * sizeof(float32), (byte*)textureAtlasData);

	//Increment the meta data size.
	m_header.metaDataSize += textureAtlasMetaData.getTotalSizeInMemory();
}

void TextureHeaderWithMetaData::setCubeMapOrder(string cubeMapOrder)
{
	//Get a reference to the meta data block.
	if (cubeMapOrder.find_first_not_of("xXyYzZ") != std::string::npos)
	{
		pvr::Log("Invalid cubemap order string");

		return;
	}
	TextureMetaData& cubeOrderMetaData = m_metaDataMap[Header::PVRv3][TextureMetaData::IdentifierCubeMapOrder];

	//Check if it's already been set or not.
	if (cubeOrderMetaData.getData())
	{
		m_header.metaDataSize -= cubeOrderMetaData.getTotalSizeInMemory();
	}

	cubeOrderMetaData = TextureMetaData(Header::PVRv3, TextureMetaData::IdentifierCubeMapOrder,
	                                    (std::min)((uint32)cubeMapOrder.length(), 6u), reinterpret_cast<const byte*>(cubeMapOrder.data()));

	//Increment the meta data size.
	m_header.metaDataSize += cubeOrderMetaData.getTotalSizeInMemory();
}

void TextureHeaderWithMetaData::setBorder(uint32 borderWidth, uint32 borderHeight, uint32 borderDepth)
{
	//Get a reference to the meta data block.
	TextureMetaData& borderMetaData = m_metaDataMap[Header::PVRv3][TextureMetaData::IdentifierBorderData];

	//Check if it's already been set or not.
	if (borderMetaData.getData())
	{
		m_header.metaDataSize -= borderMetaData.getTotalSizeInMemory();
	}

	//Setup an array of the dimensions for memcpy.
	uint32 borderDimensions[3] = { borderWidth, borderHeight, borderDepth };

	borderMetaData = TextureMetaData(Header::PVRv3, TextureMetaData::IdentifierBorderData, sizeof(uint32) * 3,
	                                 (byte*)borderDimensions);

	//Increment the meta data size.
	m_header.metaDataSize += borderMetaData.getTotalSizeInMemory();
}

void TextureHeaderWithMetaData::addMetaData(const TextureMetaData& metaData)
{
	// Get a reference to the meta data block.
	TextureMetaData& currentMetaData = m_metaDataMap[metaData.getFourCC()][metaData.getKey()];

	// Check if it's already been set or not.
	if (currentMetaData.getData())
	{
		m_header.metaDataSize -= currentMetaData.getTotalSizeInMemory();
	}

	// Set the meta data block
	currentMetaData = metaData;

	// Increment the meta data size.
	m_header.metaDataSize += currentMetaData.getTotalSizeInMemory();
}

void TextureHeaderWithMetaData::removeMetaData(uint32 fourCC, uint32 key)
{
	// Get a reference to the meta data block.
	TextureMetaData& currentMetaData = m_metaDataMap[fourCC][key];

	// Check if it's already been set or not.
	if (currentMetaData.getData())
	{
		m_header.metaDataSize -= currentMetaData.getTotalSizeInMemory();
	}

	//Remove the meta data.
	m_metaDataMap[fourCC].erase(key);
}

}
}
//!\endcond