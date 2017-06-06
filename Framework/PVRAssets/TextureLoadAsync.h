#pragma once
#include "PVRAssets/TextureLoad.h"
#include "PVRCore/Threading.h"
#include "PVRCore/Interfaces.h"

namespace pvr {
namespace async {

typedef RefCountedResource<Texture> TexturePtr;
typedef EmbeddedRefCountedResource<IFrameworkAsyncResult<TexturePtr> >AsyncTexture;

struct TextureLoadFuture_ : public IFrameworkAsyncResult<TexturePtr>, public EmbeddedRefCount<TextureLoadFuture_>
{
public:
	typedef IFrameworkAsyncResult<TexturePtr> MyBase;
	typedef MyBase::Callback CallbackType;

	string filename;
	IAssetProvider* loader;
	TextureFileFormat fmt;
	mutable SemaphorePtr resultSema;
	Semaphore* workSema;
	TexturePtr result;

	void loadNow()
	{
		Stream::ptr_type stream = loader->getAssetStream(filename);
		Result res = assets::textureLoad(stream, fmt, *result);
		_successful = (res == Result::Success);
		resultSema->signal();
		executeCallBack(getReference());
	}
	void setCallBack(CallbackType callback)
	{
		setTheCallback(callback);
	}
	static StrongReferenceType createNew()
	{
		return MyEmbeddedType::createNew();
	}

private:
	TexturePtr get_() const
	{
		if (!_inCallback)
		{
			resultSema->wait();
			resultSema->signal();
		}
		return result;
	}
	bool isComplete_() const
	{
		if (resultSema->tryWait())
		{
			resultSema->signal();
			return true;
		}
		return false;
	}
	void cleanup_() {}
	void destroyObject() {}
};

typedef EmbeddedRefCountedResource<TextureLoadFuture_> TextureLoadFuture;

inline void textureLoadAsyncWorker(TextureLoadFuture params)
{
	params->loadNow();
}

class TextureAsyncLoader : public AsyncScheduler<TexturePtr, TextureLoadFuture, &textureLoadAsyncWorker>
{
public:
	AsyncResult loadTextureAsync(const string& filename, IAssetProvider* loader, TextureFileFormat fmt, AsyncResult::ElementType::Callback callback = NULL)
	{
		auto future = TextureLoadFuture::ElementType::createNew();
		auto& params = *future;
		params.filename = filename;
		params.fmt = fmt;
		params.loader = loader;
		params.result.construct();
		params.resultSema.construct();
		params.workSema = &_workSema;
		params.setCallBack(callback);
		_queueSema.wait();
		_queue.push_back(future);
		_queueSema.signal();
		_workSema.signal();
		return future;
	}
};

}
}