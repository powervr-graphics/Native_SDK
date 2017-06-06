/*!
\brief Implementations of the methods of the MetaData class.
\file PVRCore/Texture/MetaData.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRCore/Base/Defines.h"
#include "PVRCore/Stream.h"
#include "PVRCore/Texture/MetaData.h"
namespace pvr {

TextureMetaData::TextureMetaData() : _fourCC(0), _key(0), _dataSize(0), _data(NULL)
{
}

TextureMetaData::TextureMetaData(uint32 fourCC, uint32 key, uint32 dataSize, const byte* data)
	: _fourCC(0), _key(0), _dataSize(0), _data(NULL)
{
	//Copy the data across.
	if (dataSize)
	{
		_data = new uint8[dataSize];
		memset(_data, 0, sizeof(uint8)*dataSize);
	}
	if (_data)
	{
		_fourCC = fourCC;
		_key = key;
		_dataSize = dataSize;
		if (data)
		{
			memcpy(_data, data, _dataSize);
		}
	}
}

TextureMetaData::TextureMetaData(const TextureMetaData& rhs)
	: _fourCC(0), _key(0), _dataSize(0), _data(NULL)
{
	//Copy the data across.
	if (rhs._dataSize)
	{
		_data = new uint8[rhs._dataSize];
		memset(_data, 0, sizeof(uint8)*rhs._dataSize);
	}
	if (_data)
	{
		_fourCC = rhs._fourCC;
		_key = rhs._key;
		_dataSize = rhs._dataSize;
		if (rhs._data)
		{
			memcpy(_data, rhs._data, _dataSize);
		}
	}
}

TextureMetaData::~TextureMetaData()
{
	if (_data)
	{
		delete [] _data;
		_data = NULL;
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
	_fourCC = _key = _dataSize = 0;

	// Delete any old data
	if (_data)
	{
		delete [] _data;
		_data = NULL;
	}

	// Copy the data across.
	_data = new uint8[rhs._dataSize];
	if (_data)
	{
		_fourCC = rhs._fourCC;
		_key = rhs._key;
		_dataSize = rhs._dataSize;
		if (rhs._data)
		{
			memcpy(_data, rhs._data, _dataSize);
		}
	}

	return *this;
}

bool TextureMetaData::loadFromStream(Stream& stream)
{
	size_t dataRead = 0;

	if (!stream.read(sizeof(_fourCC), 1, &_fourCC, dataRead)) { return false; }

	if (!stream.read(sizeof(_key), 1, &_key, dataRead)) { return false; }

	if (!stream.read(sizeof(_dataSize), 1, &_dataSize, dataRead)) { return false; }

	if (_dataSize <= 0) { return false; }
	_data = new uint8[_dataSize];

	if (!stream.read(1, _dataSize, _data, dataRead)) { return false; }

	return true;
}

bool TextureMetaData::writeToStream(Stream& stream) const
{
	size_t dataRead = 0;
	if (!stream.write(sizeof(_fourCC), 1, &_fourCC, dataRead)) { return false; }

	if (!stream.write(sizeof(_key), 1, &_key, dataRead)) { return false; }


	if (!stream.write(sizeof(_dataSize), 1, &_dataSize, dataRead) || _dataSize <= 0) { return false; }

	if (!stream.write(1, _dataSize, _data, dataRead)) { return false; }
	return true;
}

uint32 TextureMetaData::getFourCC() const
{
	return _fourCC;
}

uint32 TextureMetaData::getDataSize() const
{
	return _dataSize;
}

uint32 TextureMetaData::getKey() const
{
	return _key;
}

const uint8* TextureMetaData::getData() const
{
	return _data;
}

uint32 TextureMetaData::getTotalSizeInMemory() const
{
	return sizeof(_fourCC) + sizeof(_key) + sizeof(_dataSize) + _dataSize;
}
}
//!\endcond