/*!
\brief Interface for the class actually performing the low-level operations.
\file PVRCore/Interfaces/IPlatformContext.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Base/Types.h"
namespace pvr {

/// <summary>Enumeration of all types of DeviceQueue.</summary>
enum class DeviceQueueType
{
	Graphics = 0x01,//!<graphics operations
	Compute = 0x02,//!< compute operations
	Dma = 0x04,//!< DMA operations
	Extended    = 0x08, //!< extended operations
	MemoryManagement,//!< memory management operations
	RayTracing = 0x00000010, //!< ray tracing
	SceneGenerator = 0x00000020, //!< scene generator
	Count
};

inline DeviceQueueType operator&(DeviceQueueType a, DeviceQueueType b)
{
	return DeviceQueueType(std::underlying_type<DeviceQueueType>::type(a) &
	                       std::underlying_type<DeviceQueueType>::type(b));
}

inline DeviceQueueType operator|(DeviceQueueType a, DeviceQueueType b)
{
	return DeviceQueueType(std::underlying_type<DeviceQueueType>::type(a) |
	                       std::underlying_type<DeviceQueueType>::type(b));
}

namespace platform {
struct NativeSharedPlatformHandles_;
typedef RefCountedResource<NativeSharedPlatformHandles_> NativeSharedPlatformHandles;
struct NativePlatformHandles_;
struct NativeDisplayHandle_;
class PlatformContext;
class SharedPlatformContext;
}

class OSManager;
class IAssetProvider;
class IPlatformProvider;
class ISharedPlatformContext;

struct SharedContextCapabilities
{
private:
	uint8_t _storeBits;
	enum { _graphics = 1, _compute = 2, _transfer = 4, _present = 8, _sparse = 16, _prefer_transfer_only = 32, _prefer_different_family = 64 };
public:
	SharedContextCapabilities(bool graphics = true, bool compute = true, bool transfer = true, bool present = false, bool sparsebinding = false,
	                          bool preferTransfer = false, bool preferDifferentFamily = false)
	{
		_storeBits = ((graphics != 0) * _graphics +
		              (compute != 0) * _compute +
		              (transfer != 0) * _transfer +
		              (present != 0) * _present +
		              (sparsebinding != 0) * _sparse +
		              (preferTransfer != 0) * _prefer_transfer_only +
		              (preferDifferentFamily != 0) * _prefer_different_family);

	}
	bool graphics() const { return (_storeBits & _graphics) != 0; }
	bool compute() const { return (_storeBits & _compute) != 0; }
	bool transfer() const { return (_storeBits & _transfer) != 0; }
	bool sparseBinding() const { return (_storeBits & _sparse) != 0; }
	bool present() const { return (_storeBits & _present) != 0; }
	bool preferTransfer() const { return (_storeBits & _prefer_transfer_only) != 0; }
	bool preferDifferentFamily() const { return (_storeBits & _prefer_different_family) != 0; }
	void setGraphics(bool value)
	{
		// !! is used to "cast into bool", it is equivalent to !=0
		_storeBits |= (_graphics * !!value);
		_storeBits &= (_graphics * !!value);
	}
	void setCompute(bool value)
	{
		_storeBits |= (_compute * !!value);
		_storeBits &= (_compute * !!value);
	}
	void setTransfer(bool value)
	{
		_storeBits |= (_transfer * !!value);
		_storeBits &= (_transfer * !!value);
	}
	void setSparseBindign(bool value)
	{
		_storeBits |= (_sparse * !!value);
		_storeBits &= (_sparse * !!value);
	}
	void setPresentation(bool value)
	{
		_storeBits |= (_present * !!value);
		_storeBits &= (_present * !!value);
	}
	void setPreferTransferOnly(bool value)
	{
		_storeBits |= (_prefer_transfer_only * !!value);
		_storeBits &= (_prefer_transfer_only * !!value);
	}
	void setPreferDifferentFamily(bool value)
	{
		_storeBits |= (_prefer_different_family * !!value);
		_storeBits &= (_prefer_different_family * !!value);
	}
};


/// <summary>Interface for the Platform context. Contains all the operations that the pvr::Shell and StateMachine classes
/// needs in order to be able to function (creation, destrurction, create a window, swap the buffers, and make current).
/// Specific platform contexts (Egl, Vulkan) implement this interface.</summary>
class IPlatformContext
{
public:
	/// <summary>Swap the front and back buffers (called at the end of each frame to show the rendering).</summary>
	virtual Result init() = 0;

	/// <summary>Swap the front and back buffers (called at the end of each frame to show the rendering).</summary>
	virtual void release() = 0;

	/// <summary>Swap the front and back buffers (called at the end of each frame to show the rendering).</summary>
	virtual bool presentBackbuffer() = 0;

	/// <summary>Bind this context for using.</summary>
	virtual bool makeCurrent() = 0;

	/// <summary>Print information about this context.</summary>
	virtual std::string getInfo() = 0;

	/// <summary>Check if this context is initialized.</summary>
	virtual bool isInitialized() const = 0;

	/// <summary>Get the maximum API version supported by this context.</summary>
	virtual Api getMaxApiVersion() = 0;


	/// <summary>Query if the specified Api is supported by this context.</summary>
	virtual bool isApiSupported(Api api) = 0;

	/*!*********************************************************************************************************************
	\brief Query if the ray tracing extension is supported by this context.
	***********************************************************************************************************************/
	virtual bool isRayTracingSupported() const = 0;

	/*!*********************************************************************************************************************
	\brief Sets whether ray tracing is supported by this context.
	***********************************************************************************************************************/
	virtual void setRayTracingSupported(bool supported) = 0;

	/// <summary>Get the NativePlatformHandles wrapped by this context.</summary>
	virtual const platform::NativePlatformHandles_& getNativePlatformHandles() const = 0;

	/// <summary>Get the NativePlatformHandles wrapped by this context.</summary>
	virtual platform::NativePlatformHandles_& getNativePlatformHandles() = 0;

	/// <summary>Get the NativePlatformHandles wrapped by this context.</summary>
	virtual const platform::NativeDisplayHandle_& getNativeDisplayHandle() const = 0;

	/// <summary>Get the NativePlatformHandles wrapped by this context.</summary>
	virtual platform::NativeDisplayHandle_& getNativeDisplayHandle() = 0;

	/// <summary>Get the maximum API version supported by this context.</summary>
	Api getApiType() { return apiType; }

	/// <summary>Get the maximum API version supported by this context.</summary>
	BaseApi getBaseApi() { return baseApi; }

	/// <summary>Get the NativePlatformHandles wrapped by this context.</summary>
	uint32 getSwapChainLength() const { return swapChainLength; };

	/// <summary>Get the NativePlatformHandles wrapped by this context.</summary>
	uint32 getSwapChainIndex() const { return swapIndex; }

	uint32 getLastSwapChainIndex()const { return lastPresentedSwapIndex; }

	virtual std::auto_ptr<ISharedPlatformContext> createSharedPlatformContext(uint32 id = 0) = 0;

	void prepareContexts(const std::vector<SharedContextCapabilities>& contextList)
	{
		_contextList = contextList;
	}

	const std::vector<SharedContextCapabilities> getContextList() const
	{
		return _contextList;
	}

	virtual ~IPlatformContext() { }
protected:
	friend std::auto_ptr<IPlatformContext> createNativePlatformContext(OSManager& mgr);
	std::vector<SharedContextCapabilities> _contextList;
	uint32 swapChainLength;
	uint32 swapIndex;
	uint32 lastPresentedSwapIndex;
	Api apiType;
	BaseApi baseApi;
};

class ISharedPlatformContext
{
protected:
	platform::PlatformContext* _parentContext;
public:
	virtual bool makeSharedContextCurrent() = 0;
	platform::PlatformContext& getParentContext() { return *_parentContext; }
	virtual platform::NativeSharedPlatformHandles_& getSharedHandles() = 0;
};

/// <summary>Allocates and returns a smart pointer to the Platform context depending on the platform. Implemented in the
/// specific PVRNativeApi to return the correct type of context required.</summary>
std::auto_ptr<IPlatformContext> PVR_API_FUNC createNativePlatformContext(OSManager& osManager);

}// namespace pvr
