#pragma once
#include "PVRAssets/TextureLoad.h"
#include "PVRCore/Threading.h"

namespace pvr {
namespace async {

/// <summary>A ref counted pointer to a Texture object  Used to return and pass a dynamically allocated textures</summary>
typedef RefCountedResource<Texture> TexturePtr;

/// <summary>A class wrapping the operations necessary to retrieve an asynchronously loaded texture, (e.g. querying
/// if the load is complete, or blocking-wait get the result. Is an EmbeddedRefCounted Resource, so must always be
/// instantiated with createNew()</summary>
struct TextureLoadFuture_ : public IFrameworkAsyncResult<TexturePtr>, public EmbeddedRefCount<TextureLoadFuture_>
{
private:
	friend class EmbeddedRefCount<TextureLoadFuture_>;
	TextureLoadFuture_() {}
public:
	typedef IFrameworkAsyncResult<TexturePtr> MyBase; ///< Base class
	typedef MyBase::Callback CallbackType; ///< The type of function that can be used as a completion callback
public:
	Semaphore* workSema; ///< A pointer to an externally used semaphore (normally the one used by the queue)
	std::string filename; ///< The filename from which the texture is loaded
	IAssetProvider* loader; ///< The AssetProvider to use to load the texture
	TextureFileFormat fmt; ///< The format of the texture
	mutable SemaphorePtr resultSema; ///< The semaphore that is used to wait for the result
	/// <summary> The result of the operation will be stored here </summary>
	TexturePtr result;

	/// <summary>Load the texture synchronously and signal the result semaphore. Normally called by the worker thread</summary>
	void loadNow()
	{
		Stream::ptr_type stream = loader->getAssetStream(filename);
		_successful = assets::textureLoad(stream, fmt, *result);
		resultSema->signal();
		executeCallBack(getReference());
	}
	/// <summary>Set a function to be called when the texture loading has been finished.</summary>
	/// <param name="callback">Set a function to be called when the texture loading has been finished.</param>
	void setCallBack(CallbackType callback)
	{
		setTheCallback(callback);
	}
	/// <summary>Create a new TextureLoadFuture, wrapped in an EmbeddedRefCountedResource. TextureLoadFuture
	/// can only be instantiated using this function.</summary>
	/// <returns>A new TextureLoadFuture_</summary>
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

/// <summary>A reference counted handle to a TextureLoadFuture_. A TextureLoadFuture_ can only
/// be handled using this class</summary>
typedef EmbeddedRefCountedResource<TextureLoadFuture_> TextureLoadFuture;

//!\cond NO_DOXYGEN
namespace impl {
/// <summary>internal</summary>
inline void textureLoadAsyncWorker(TextureLoadFuture params)
{
	params->loadNow();
}
}
//!\endcond

/// <summary> A class that loads Textures in a (single) different thread and provides futures to them.
/// Create an instance of it, and then just call loadTextureAsync foreach texture to load. When each texture
/// has completed loading, a callback may be called, otherwise you can use all the typical functionality
/// of futures, such as querying if loading is comlete, or using a blocking wait to get the result </summary>
class TextureAsyncLoader : public AsyncScheduler<TexturePtr, TextureLoadFuture, &impl::textureLoadAsyncWorker>
{
public:
	TextureAsyncLoader() { _myInfo = "TextureAsyncLoader"; }
	/// <summary>This function enqueues a "load texture" on a background thread, and returns an object
	/// that can be used to query and wait for the result.</summary>
	/// <param name="filename">The filename of the texture to load</param>
	/// <param name="loader">A class that provides a "getAssetStream" function to get a Stream from the filename (usually, the application class itself)</param>
	/// <param name="fmt">The texture format as which to load the texture.</param>
	/// <param name="callback">An optional callback to call immediately after texture loading is complete.</param>
	/// <returns> A future to a texture : TextureLoadFuture </returns>
	AsyncResult loadTextureAsync(const std::string& filename, IAssetProvider* loader, TextureFileFormat fmt, AsyncResult::ElementType::Callback callback = NULL)
	{
		auto future = TextureLoadFuture::ElementType::createNew();
		auto& params = *future;
		params.filename = filename;
		params.fmt = fmt;
		params.loader = loader;
		params.result.construct();
		params.resultSema.construct();
		params.workSema = &_workSemaphore;
		params.setCallBack(callback);
		_queueSemaphore.wait();
		_queue.push_back(future);
		_queueSemaphore.signal();
		_workSemaphore.signal();
		return future;
	}
};
}// namespace assets
}// namespace pvr
