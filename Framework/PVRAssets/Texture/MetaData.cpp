/*!*********************************************************************************************************************
\file         PVRAssets\Texture\MetaData.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations of the methods of the MetaData class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRCore/Defines.h"
#include "PVRCore/Stream.h"
#include "PVRAssets/Texture/MetaData.h"
namespace pvr {
namespace assets {
TextureMetaData::TextureMetaData() : m_fourCC(0), m_key(0), m_dataSize(0), m_data(NULL)
{
}

TextureMetaData::TextureMetaData(uint32 fourCC, uint32 key, uint32 dataSize, const byte* data)
	: m_fourCC(0), m_key(0), m_dataSize(0), m_data(NULL)
{
	//Copy the data across.
	if (dataSize)
	{
		m_data = new uint8[dataSize];
		memset(m_data, 0, sizeof(uint8)*dataSize);
	}
	if (m_data)
	{
		m_fourCC = fourCC;
		m_key = key;
		m_dataSize = dataSize;
		if (data)
		{
			memcpy(m_data, data, m_dataSize);
		}
	}
}

TextureMetaData::TextureMetaData(const TextureMetaData& rhs)
	: m_fourCC(0), m_key(0), m_dataSize(0), m_data(NULL)
{
	//Copy the data across.
	if (rhs.m_dataSize)
	{
		m_data = new uint8[rhs.m_dataSize];
		memset(m_data, 0, sizeof(uint8)*rhs.m_dataSize);
	}
	if (m_data)
	{
		m_fourCC = rhs.m_fourCC;
		m_key = rhs.m_key;
		m_dataSize = rhs.m_dataSize;
		if (rhs.m_data)
		{
			memcpy(m_data, rhs.m_data, m_dataSize);
		}
	}
}

TextureMetaData::~TextureMetaData()
{
	if (m_data)
	{
		delete [] m_data;
		m_data = NULL;
	}
}

TextureMetaData& TextureMetaData::operator=(const TextureMetaData& rhs)
{
	//If it equals itself, return early.
	if (&rhs == this)
	{
		return *this;
	}

	// Initialize
	m_fourCC = m_key = m_dataSize = 0;

	// Delete any old data
	if (m_data)
	{
		delete [] m_data;
		m_data = NULL;
	}

	// Copy the data across.
	m_data = new uint8[rhs.m_dataSize];
	if (m_data)
	{
		m_fourCC = rhs.m_fourCC;
		m_key = rhs.m_key;
		m_dataSize = rhs.m_dataSize;
		if (rhs.m_data)
		{
			memcpy(m_data, rhs.m_data, m_dataSize);
		}
	}

	return *this;
}

bool TextureMetaData::loadFromStream(Stream& stream)
{
	size_t dataRead = 0;

	if (!stream.read(sizeof(m_fourCC), 1, &m_fourCC, dataRead)) { return false; }

	if (!stream.read(sizeof(m_key), 1, &m_key, dataRead)) { return false; }

	if (!stream.read(sizeof(m_dataSize), 1, &m_dataSize, dataRead)) { return false; }

	if (m_dataSize <= 0) { return false; }
	m_data = new uint8[m_dataSize];

	if (!stream.read(1, m_dataSize, m_data, dataRead)) { return false; }

	return true;
}

bool TextureMetaData::writeToStream(Stream& stream) const
{
	size_t dataRead = 0;
	if (!stream.write(sizeof(m_fourCC), 1, &m_fourCC, dataRead)) { return false; }

	if (!stream.write(sizeof(m_key), 1, &m_key, dataRead)) { return false; }


	if (!stream.write(sizeof(m_dataSize), 1, &m_dataSize, dataRead) || m_dataSize <= 0) { return false; }

	if (!stream.write(1, m_dataSize, m_data, dataRead)) { return false; }
	return true;
}

uint32 TextureMetaData::getFourCC() const
{
	return m_fourCC;
}

uint32 TextureMetaData::getDataSize() const
{
	return m_dataSize;
}

uint32 TextureMetaData::getKey() const
{
	return m_key;
}

const uint8* TextureMetaData::getData() const
{
	return m_data;
}

uint32 TextureMetaData::getTotalSizeInMemory() const
{
	return sizeof(m_fourCC) + sizeof(m_key) + sizeof(m_dataSize) + m_dataSize;
}
}
}
//!\endcond