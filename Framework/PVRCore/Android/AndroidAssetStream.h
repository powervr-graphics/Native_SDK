/*!*********************************************************************************************************************
\file         PVRCore\Android\AndroidAssetStream.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         A Stream implementation used to access Android resources.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Stream.h"

struct AAssetManager;
struct AAsset;

namespace pvr
{
/*!*********************************************************************************************************************
\brief	      A Stream implementation that is used to access resources built in an Android package (apk).
\description  This Stream abstraction allows the user to easily access the Resources embedded in an Android .apk package.
              This is the default way resources are packed in the Android version of the PowerVR Examples.
***********************************************************************************************************************/
class AndroidAssetStream : public Stream
{
public:
	AndroidAssetStream(AAssetManager* const assetManager, const string& filename);
	~AndroidAssetStream();
	bool read(size_t size, size_t count, void* const outData, size_t& outElementsRead) const;
	bool write(size_t size, size_t count, const void* data, size_t& dataWritten);
	bool seek(long offset, SeekOrigin origin) const;
	bool open() const;
	void close();

	virtual bool isopen() const;
	virtual size_t getPosition() const;
	virtual size_t getSize() const;

private:
	AAssetManager* const m_assetManager;
	mutable AAsset* m_asset;
};
}
