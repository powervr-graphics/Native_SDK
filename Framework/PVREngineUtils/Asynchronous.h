#pragma once
#include "PVRCore/Interfaces/IGraphicsContext.h"
#include "PVRCore/Threading.h"
#include "PVRCore/Texture.h"
#include "PVRAssets/TextureLoadAsync.h"

namespace pvr {
namespace async {

typedef RefCountedResource<Texture> TexturePtr;
typedef EmbeddedRefCountedResource<IFrameworkAsyncResult<TexturePtr> >AsyncTexture;

typedef EmbeddedRefCountedResource<IFrameworkAsyncResult<api::TextureView > > AsyncApiTexture;

struct TextureUploadFuture_ : public IFrameworkAsyncResult<api::TextureView >, public EmbeddedRefCount<TextureUploadFuture_>
{
public:
	typedef IFrameworkAsyncResult<api::TextureView > MyBase;
	typedef MyBase::Callback CallbackType;
	SharedContext context;
	AsyncTexture texture;

	TexturePtr textureSync;
	bool allowDecompress;
	mutable SemaphorePtr resultSema;
	Semaphore* workSema;
	bool callbackBeforeSignal;
	void setCallBack(CallbackType callback) { setTheCallback(callback); }
	void loadNow()
	{
		context->getSharedPlatformContext().makeSharedContextCurrent();
		auto res = context->uploadTextureDeferred(*texture->get(), allowDecompress);
		res->fence->wait();
		_result = res->texture;
		_successful = (_result.isValid());
		if (callbackBeforeSignal)
		{
			callBack();
			resultSema->signal();
		}
		else
		{
			resultSema->signal();
			callBack();
		}
	}
	
	const api::TextureView& getResult() { return _result; }

	static StrongReferenceType createNew()
	{
		return MyEmbeddedType::createNew();
	}
private:
	void callBack()
	{
		executeCallBack(getReference());
	}
	api::TextureView _result;
	api::TextureView get_() const
	{
		if (!_inCallback)
		{
			resultSema->wait();
			resultSema->signal();
		}
		return _result;
	}

	api::TextureView getNoWait() const
	{
		return _result;
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

typedef EmbeddedRefCountedResource<TextureUploadFuture_> TextureUploadFuture;

inline void textureUploadAsyncWorker(TextureUploadFuture params)
{
	params->loadNow();
}


class TextureApiAsyncUploader : public async::AsyncScheduler<api::TextureView, TextureUploadFuture, textureUploadAsyncWorker>
{
	SharedContext _ctx;
public:
	typedef IFrameworkAsyncResult<api::TextureView > MyBase;
	typedef MyBase::Callback CallbackType;
	void init(GraphicsContext ctx, uint32 contextId)
	{
		_ctx = ctx->createSharedContext(contextId);
	}
	AsyncApiTexture uploadTextureAsync(const AsyncTexture& texture, bool allowDecompress = true,
	                                   CallbackType callback = NULL, bool callbackBeforeSignalling = false)
	{
		assertion(_ctx.isValid(), "Context has not been initialized");
		auto future = TextureUploadFuture::ElementType::createNew();
		auto& params = *future;
		params.allowDecompress = allowDecompress;
		params.context = _ctx;
		params.texture = texture;
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

typedef RefCountedResource<TextureUploadFuture> ApiTextureFuturePtr;

}

}

