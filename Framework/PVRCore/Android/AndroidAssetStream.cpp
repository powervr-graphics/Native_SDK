/*!*********************************************************************************************************************
\file         PVRCore\Android\AndroidAssetStream.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation file for the AndroidAssetStream.
***********************************************************************************************************************/
#include "PVRCore/Android/AndroidAssetStream.h"
#include <android/asset_manager.h>

namespace pvr {
AndroidAssetStream::AndroidAssetStream(AAssetManager* const assetManager, const std::string& filename)
	: Stream(filename), m_assetManager(assetManager), m_asset(NULL)
{
	m_isReadable = true;
}

AndroidAssetStream::~AndroidAssetStream()
{
	close();
}

bool AndroidAssetStream::read(size_t size, size_t count, void* outData, size_t& outElementsRead) const
{
	if (m_asset)
	{
		int dataRead = (size_t)AAsset_read(m_asset, outData, size * count);

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
	if (m_asset)
	{
		off_t newPos = AAsset_seek(m_asset, offset, (int)origin);

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
	if (m_asset == NULL)
	{
		m_asset = AAssetManager_open(m_assetManager, m_fileName.c_str(), AASSET_MODE_RANDOM);
		return !!m_asset;
	}
	else
	{
		return seek(0, SeekOriginFromStart);
	}
}

void AndroidAssetStream::close()
{
	if (m_asset)
	{
		AAsset_close(m_asset);
		m_asset = NULL;
	}
}

bool AndroidAssetStream::isopen() const
{
	return m_asset != NULL;
}

size_t	AndroidAssetStream::getPosition() const
{
	if (m_asset)
	{
		return static_cast<pvr::uint64>(AAsset_getLength(m_asset) - AAsset_getRemainingLength(m_asset));
	}

	return 0;
}

size_t AndroidAssetStream::getSize() const
{
	if (m_asset)
	{
		return static_cast<size_t>(AAsset_getLength(m_asset));
	}
	return 0;
}
}