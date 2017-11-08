#pragma once
#include "PVRCore/Threading.h"
#include "PVRCore/Texture.h"
#include "PVRAssets/TextureLoadAsync.h"
#include "PVRVk/ImageVk.h"
#include "PVRUtils/Vulkan/HelperVk.h"
namespace pvr {
namespace utils {

/// <summary>Provides a reference counted pointer to a pvr::Texture which will be used for loading API agnostic texture data to disk.</summary>
typedef RefCountedResource<Texture> TexturePtr;

/// <summary>Provides a reference counted pointer to a IFrameworkAsyncResult specialised by a TexturePtr which will be used
/// as the main interface for using API agnostic asynchronous textures.</summary>
typedef EmbeddedRefCountedResource<async::IFrameworkAsyncResult<TexturePtr>> AsyncTexture;

/// <summary>Provides a reference counted pointer to a IFrameworkAsyncResult specialised by an ImageView which will be used
/// as the main interface for using API specific asynchronous textures.</summary>
typedef EmbeddedRefCountedResource<async::IFrameworkAsyncResult<pvrvk::ImageView >> AsyncApiTexture;

//!\cond NO_DOXYGEN
struct ImageUploadFuture_ : public async::IFrameworkAsyncResult<pvrvk::ImageView >, public EmbeddedRefCount<ImageUploadFuture_>
{
public:
	//!\cond NO_DOXYGEN
	typedef IFrameworkAsyncResult<pvrvk::ImageView > MyBase;
	typedef MyBase::Callback CallbackType;
	//!\endcond

	/// <summary>A queue to be used to submit image upload operations.</summary>
	pvrvk::Queue _queue;

	/// <summary>A _device to be used for creating temporary resources required for uploading an image.</summary>
	pvrvk::Device _device;

	/// <summary>A pvr::Texture to asynchronously upload to the Gpu.</summary>
	AsyncTexture _texture;

	/// <summary>A command pool from which comand buffers will be allocated to record image upload operations.</summary>
	pvrvk::CommandPool _cmdPool;

	/// <summary>A semaphore used to guard access to submitting to the CommandQueue.
	async::Mutex* _cmdQueueMutex;

	/// <summary>Specifies whether the uploaded texture can be decompressed as it is uploaded.</summary>
	bool _allowDecompress;

	/// <summary>Specifies a semaphore which will be signalled at the point the upload of the texture is finished.</summary>
	mutable async::SemaphorePtr _resultSemaphore;

	/// <summary>Specifies whether the callback should be called prior to signalling the completion of the image upload.</summary>
	bool _callbackBeforeSignal;

	/// <summary>Sets a callback which will be called after the image upload has completed.</summary>
	/// <param name="callback">Specifies a callback to call when the image upload has completed.</param>
	void setCallBack(CallbackType callback) { setTheCallback(callback); }

	/// <summary>Initiates the asynchronous image upload.</summary>
	void loadNow()
	{

		_result = customUploadImage();

		_successful = (_result.isValid());
		if (_callbackBeforeSignal)
		{
			callBack();
			_resultSemaphore->signal();
		}
		else
		{
			_resultSemaphore->signal();
			callBack();
		}
	}

	/// <summary>Returns the result of the asynchronous image upload.</summary>
	/// <returns>The result of the asynchronous image upload which is an ImageView.</returns>
	const pvrvk::ImageView& getResult() { return _result; }

	static StrongReferenceType createNew()
	{
		return MyEmbeddedType::createNew();
	}
private:

	pvrvk::ImageView customUploadImage()
	{
		Texture& assetTexture = *_texture->get();
		pvrvk::CommandBuffer cmdBuffer = _cmdPool->allocateCommandBuffer();
		cmdBuffer->begin();
		ImageUploadResults results = uploadImage(_device, assetTexture, _allowDecompress, cmdBuffer);
		cmdBuffer->end();

		if (results.getResult() == pvr::Result::Success)
		{
			pvrvk::SubmitInfo submitInfo;
			submitInfo.commandBuffers = &cmdBuffer;
			submitInfo.numCommandBuffers = 1;
			pvrvk::Fence fence = _device->createFence();
			if (_cmdQueueMutex != 0)
			{
				_cmdQueueMutex->lock();
			}
			_queue->submit(&submitInfo, 1, fence);
			if (_cmdQueueMutex != 0)
			{
				_cmdQueueMutex->unlock();
			}
			fence->wait();

			return results.getImageView();
		}
		return pvrvk::ImageView();
	}

	void callBack()
	{
		executeCallBack(getReference());
	}
	pvrvk::ImageView _result;
	pvrvk::ImageView get_() const
	{
		if (!_inCallback)
		{
			_resultSemaphore->wait();
			_resultSemaphore->signal();
		}
		return _result;
	}

	pvrvk::ImageView getNoWait() const
	{
		return _result;
	}

	bool isComplete_() const
	{
		if (_resultSemaphore->tryWait())
		{
			_resultSemaphore->signal();
			return true;
		}
		return false;
	}
	void cleanup_() {}
	void destroyObject() {}
};
//!\endcond

/// <summary> A ref-counted pointer to a Future of an Image Upload: A class that wraps the texture that
/// "is being uploaded on a separate thread", together with functions to "query if the upload is yet complete"
/// and to "block until the upload is complete, if necessary, and return the result"
typedef EmbeddedRefCountedResource<ImageUploadFuture_> ImageUploadFuture;

//!\cond NO_DOXYGEN
inline void imageUploadAsyncWorker(ImageUploadFuture params)
{
	params->loadNow();
}
//!\endcond

/// <summary> This class wraps a worker thread that uploads texture to the GPU asynchronously and returns
/// futures to them. This class would normally be used with Texture Futures as well, in order to do both
/// of the operations asynchronously.
class ImageApiAsyncUploader : public async::AsyncScheduler<pvrvk::ImageView, ImageUploadFuture, imageUploadAsyncWorker>
{
private:
	pvrvk::Device _device;
	pvrvk::Queue _queueVk;
	pvrvk::CommandPool _cmdPool;
	async::Mutex* _cmdQueueMutex;
public:
	ImageApiAsyncUploader() { _myInfo = "ImageApiAsyncUploader"; }
	/// <summary>Base class of the framework future.</summary>
	typedef async::IFrameworkAsyncResult<pvrvk::ImageView > MyBase;
	/// <summary>The type of the optional callback that is called at the end of the operation</summary>
	typedef MyBase::Callback CallbackType;

	/// <summary>Initialize this AsyncUploader. Do not use the queue and pool unguarded aftewards, as
	/// they will be accessed from an indeterminate thread at indeterminate times. It is ideal that
	/// the queue is only used by this Uploader, but if it is not, a (CPU) Semaphore must be passed
	/// to guard access to the queue.</summary>
	/// <param name="device">A Vulkan _device that will be used to create the textures. A Command pool
	/// will be created on this device for the queue family of the queue</param>
	/// <param name="queue">A Vulkan command queue that will be used to upload the textures</param>
	/// <param name="queueSemaphore">Use the Semaphore as a mutex: Initial count 1, call wait() before
	/// all accesses to the Vulkan queue, then signal() when finished accessing. If the queue does
	/// not need external synchronization (i.e. it is only used by this object), leave the
	/// queueSemaphore at its default value of NULL</param>
	void init(pvrvk::Device& device, pvrvk::Queue& queue,  async::Mutex* queueSemaphore = nullptr)
	{
		_device = device;
		_queueVk = queue;
		_cmdPool = device->createCommandPool(queue->getQueueFamilyId(),
		                                     VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);
		_cmdQueueMutex = queueSemaphore;
	}

	/// <summary>Begin a texture uploading task and return the future to the Vulkan Texture. Use the returned
	/// future to query completion and get the result. Takes asynchronous textures as input so that the loading
	/// and uploading tasks can be chained and done on different threads.</summary>
	/// <param name="texture">A PVR Texture future (can be gotten from the Async Texture Loader or another async source)</param>
	/// <param name="allowDecompress">If the texture is compressed to an unsupported format, allow it to be decompressed to
	/// RGBA8 if a software decompressor is available. Default true</param>
	/// <param name="callback">An optional function pointer that will be invoked when the uploading is complete. IF you use the
	/// callback AND the you set the flag "callbackBeforeSignal" to TRUE, do NOT call the "get" function of the future in the
	/// callback. Default NULL.</param>
	/// <param name="callbackBeforeSignal">A flag signifying if the callback should be called BEFORE or AFTER the semaphore
	/// of the Result future (the Texture future) is signalled as complete. Defaults to false, so as to avoid the deadlock that
	/// will happen if the user attempts to call "get" on the future while the signal will happen just after return of the callback.
	/// Set to "true" if you want to do something WITHOUT calling "get" on the future, but before the texture is  used.</param>
	/// <returns> A texture upload Future which you can use to query or get the uploaded texture</returns>
	AsyncApiTexture uploadTextureAsync(const AsyncTexture& texture, bool allowDecompress = true,
	                                   CallbackType callback = NULL, bool callbackBeforeSignal = false)
	{
		assertion(_queueVk.isValid(), "Context has not been initialized");
		auto future = ImageUploadFuture::ElementType::createNew();
		auto& params = *future;
		params._allowDecompress = allowDecompress;
		params._queue = _queueVk;
		params._device = _device;
		params._texture = texture;
		params._resultSemaphore.construct();
		params._cmdPool = _cmdPool;
		params.setCallBack(callback);
		params._callbackBeforeSignal = callbackBeforeSignal;
		params._cmdQueueMutex = _cmdQueueMutex;
		_queueSemaphore.wait();
		_queue.push_back(future);
		_queueSemaphore.signal();
		_workSemaphore.signal();
		return future;
	}
};
}
}

