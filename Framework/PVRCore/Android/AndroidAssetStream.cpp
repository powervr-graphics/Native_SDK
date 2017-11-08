/*!
\brief Implementation file for the AndroidAssetStream.
\file PVRCore/Android/AndroidAssetStream.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCore/Android/AndroidAssetStream.h"
#include <android/asset_manager.h>

namespace pvr {
AndroidAssetStream::AndroidAssetStream(AAssetManager* assetManager, const std::string& filename)
	: Stream(filename), assetManager(assetManager), _asset(NULL)
{
	_isReadable = true;
}

AndroidAssetStream::~AndroidAssetStream()
{
	close();
}

bool AndroidAssetStream::read(size_t size, size_t count, void* outData, size_t& outElementsRead) const
{
	if (_asset)
	{
		int dataRead = (size_t)AAsset_read(_asset, outData, size * count);

		if (dataRead == 0)
		{
			outElementsRead = dataRead;
			return false;
		}
		else if (dataRead < 0)
		{
			outElementsRead = 0;
			return false;
		}
		// AssetManager returns number of bytes. We want number of items.
		outElementsRead = (size_t)dataRead / size;
		return true;
	}

	return false;
}

bool AndroidAssetStream::write(size_t size, size_t count, const void* data, size_t& dataWritten)
{
	return false;
}

bool AndroidAssetStream::seek(long offset, SeekOrigin origin) const
{
	if (_asset)
	{
		off_t newPos = AAsset_seek(_asset, offset, (int)origin);

		if (newPos == (off_t) - 1)
		{
			return false;
		}

		return true;
	}

	return false;
}

bool AndroidAssetStream::open() const
{
	if (_asset == NULL)
	{
		_asset = AAssetManager_open(assetManager, _fileName.c_str(), AASSET_MODE_RANDOM);
		return !!_asset;
	}
	else
	{
		return seek(0, SeekOriginFromStart);
	}
}

void AndroidAssetStream::close()
{
	if (_asset)
	{
		AAsset_close(_asset);
		_asset = NULL;
	}
}

bool AndroidAssetStream::isopen() const
{
	return _asset != NULL;
}

size_t	AndroidAssetStream::getPosition() const
{
	if (_asset)
	{
		return static_cast<uint64_t>(AAsset_getLength(_asset) - AAsset_getRemainingLength(_asset));
	}

	return 0;
}

size_t AndroidAssetStream::getSize() const
{
	if (_asset)
	{
		return static_cast<size_t>(AAsset_getLength(_asset));
	}
	return 0;
}
}