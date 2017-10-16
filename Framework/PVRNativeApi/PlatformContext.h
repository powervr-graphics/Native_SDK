/*!
\brief Contains the declaration of the PlatformContext class, the main wrapper for the Platform specific part of a
Graphics context.
\file PVRNativeApi/PlatformContext.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

//CAUTION: [MY_API]/[MY_API]PlatformContext.h must be included BEFORE this file so that it defines NativeDisplay, NativeWindow symbols.
#include "PVRCore/CoreIncludes.h"
#include "PVRNativeApi/PlatformTypes.h"
#include "PVRCore/Stream.h"
#include "PVRCore/Interfaces/IPlatformContext.h"
#include "PVRCore/Interfaces/IGraphicsContext.h"
#include "PVRCore/Interfaces/OSManager.h"
#include "PVRCore/Math/Rectangle.h"
#include "PVRCore/Base/RefCounted.h"
#include "PVRCore/StringFunctions.h"
#include <map>

/// <summary>Main PowerVR Framework namespace</summary>
namespace pvr {
/// <summary>The pvr::platform namespace contains low-level, system-communication classes and functions</summary>
namespace platform {

class SharedPlatformContext;

/// <summary>The platform context is the class wrapping all platform-specific objects required to power the PVRApi
/// Graphics context (Displays, windows, configurations etc.).</summary>
class PlatformContext : public IPlatformContext
{
	friend class SharedPlatformContext;
public:
	PlatformContext(OSManager& mgr) : _OSManager(mgr), _platformContextHandles(), _swapInterval(-2), _initialized(false),
		_preInitialized(false), _enableDebugValidation(false), _maxApiVersion(Api::Unspecified), _supportsRayTracing(false)
	{ }

	/// <summary>Initialize this object</summary>
	/// <returns>Result::Success on success.</returns>
	Result init();

	/// <summary>Release this object</summary>
	void release();

	/// <summary>Get maximum api version supported</summary>
	/// <returns>The maximum api version supported</returns>
	Api getMaxApiVersion();

	/// <summary>Return true if the specified api is supported</summary>
	/// <param name="api">The api to check for support</param>
	/// <returns>True if supported, otherwse false</returns>
	bool isApiSupported(Api api);

	/// <summary>Present back buffer</summary>
	/// <returns>Return true if success</returns>
	bool presentBackbuffer();

	/// <summary>Make this platform context current. In non-binding contexts(e.g. vulkan) does nothing</summary>
	/// <returns>True if successful or no-op.</returns>
	bool makeCurrent();

	/// <summary>Get native platform handles (const)</summary>
	/// <returns>A platform/api specific object containing the platform handles (surface, window etc.)</returns>
	const NativePlatformHandles_& getNativePlatformHandles() const { return *_platformContextHandles; }

	/// <summary>Get native platform handles (const)</summary>
	/// <returns>A platform/api specific object containing the platform handles (context, queues, fences/semaphores
	/// etc.)</returns>
	NativePlatformHandles_& getNativePlatformHandles() { return *_platformContextHandles; }

	/// <summary>Get native display handles (const)</summary>
	/// <returns>A platform/api specific object containing the display handles (display, surface etc.)</returns>
	const NativeDisplayHandle_& getNativeDisplayHandle() const { return *_displayHandle; }

	/// <summary>Get native display handles (const)</summary>
	/// <returns>A platform/api specific object containing the display handles (display, surface etc.)</returns>
	NativeDisplayHandle_& getNativeDisplayHandle() { return *_displayHandle; }

	/// <summary>Get information on this object</summary>
	/// <returns>Information on this object, typically device name etc.</returns>
	std::string getInfo();

	/// <summary>Return true if this object is initialized</summary>
	/// <returns>true if this object is initialized</returns>
	bool isInitialized()  const { return (_platformContextHandles.get() != 0) && _initialized; }

	/// <summary>Return true if ray tracing is supported on this platform</summary>
	/// <returns>true if ray tracing is supported</returns>
	bool isRayTracingSupported() const;

	/// <summary>Return the OSManager object used by this context</summary>
	/// <returns>The OSManager used held by this context</returns>
	OSManager& getOsManager() { return _OSManager; }

	/// <summary>Return the OSManager object used by this context</summary>
	/// <returns>The OSManager used held by this context</returns>
	const OSManager& getOsManager()const { return _OSManager; }

	std::auto_ptr<ISharedPlatformContext> createSharedPlatformContext(uint32 id);
private:
	PlatformContext(const PlatformContext& rhs);//deleted
	OSManager& _OSManager;
	NativePlatformHandles _platformContextHandles;
	NativeDisplayHandle _displayHandle;
	int8 _swapInterval;
	bool _initialized;
	bool _preInitialized;
	bool _enableDebugValidation;
	Api _maxApiVersion;
	bool _supportsRayTracing;



	//Must be called after the context has been active inorder to query the driver for resource limitations.
	void populateMaxApiVersion();

	/*!
	\brief Sets whether ray tracing is supported on this platform
	\param supported
	*/
	void setRayTracingSupported(bool supported);

};


class SharedPlatformContext : public ISharedPlatformContext
{
	Result init(PlatformContext& context, uint32 contextId);
	friend class PlatformContext;
	platform::NativeSharedPlatformHandles _handles;
public:
	/// <summary>If required by the implementation, make this the shared context current for this thread.
	/// In non-binding contexts(e.g. vulkan) does nothing. Only call this from the uploading thread once.</summary>
	/// <returns>True if successful or no-op.</returns>
	bool makeSharedContextCurrent();
	platform::NativeSharedPlatformHandles_& getSharedHandles() { return *_handles; }
};

}
}
