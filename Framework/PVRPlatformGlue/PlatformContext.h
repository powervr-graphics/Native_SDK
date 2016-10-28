/*!*********************************************************************************************************************
\file         PVRPlatformGlue\PlatformContext.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the declaration of the PlatformContext class, the main wrapper for the Platform specific part of a
              Graphics context.
***********************************************************************************************************************/
#pragma once

//CAUTION: [MY_API]/[MY_API]PlatformContext.h must be included BEFORE this file so that it defines NativeDisplay, NativeWindow symbols.
#include "PVRCore/CoreIncludes.h"
#include "PVRPlatformGlue/PlatformTypes.h"
#include "PVRCore/Stream.h"
#include "PVRCore/IPlatformContext.h"
#include "PVRCore/IGraphicsContext.h"
#include "PVRCore/OSManager.h"
#include "PVRCore/Rectangle.h"
#include "PVRCore/RefCounted.h"
#include "PVRCore/StringFunctions.h"
#include <map>

/*!*********************************************************************************************************************
\brief         Main PowerVR Framework namespace
***********************************************************************************************************************/
namespace pvr {
/*!*********************************************************************************************************************
\brief         The pvr::platform namespace contains low-level, system-communication classes and functions
***********************************************************************************************************************/
namespace platform {

/*!*********************************************************************************************************************
\brief         The platform context is the class wrapping all platform-specific objects required to power the PVRApi Graphics
              context (Displays, windows, configurations etc.).
***********************************************************************************************************************/
class PlatformContext : public IPlatformContext
{
public:
	PlatformContext(OSManager& mgr) : m_OSManager(mgr), m_platformContextHandles(), m_swapInterval(-2), m_initialized(false),
        m_preInitialized(false),m_enableDebugValidation(false), m_ContextImplementationID(static_cast<size_t>(-1)),
    m_maxApiVersion(Api::Unspecified){ }

	/*!
	   \brief Initialize this object
	   \return Result::Success on success.
	 */
	Result init();

	/*!
	   \brief Release this object
	 */
	void release();

	/*!
	   \brief Get maximum api version supported
	 */
	Api getMaxApiVersion();

	/*!
	   \brief Return true if api is supported
	   \param api
	 */
	bool isApiSupported(Api api);

	/*!
	   \brief Present back buffer
	   \return Return true if success
	 */
	bool presentBackbuffer();

	/*!
	   brief makeCurrent
	   \return
	 */
	bool makeCurrent();

	/*!
	   \brief Get number of swapchain length
	 */
	uint32 getSwapChainLength() const;

	/*!
	   \brief Get native platform handles (const)
	 */
	const NativePlatformHandles_& getNativePlatformHandles() const { return *m_platformContextHandles; }

	/*!
	   \brief Get native platform handles
	 */
	NativePlatformHandles_& getNativePlatformHandles() { return *m_platformContextHandles; }

	/*!
	   \brief Get native display handle
	 */
	const NativeDisplayHandle_& getNativeDisplayHandle() const { return *m_displayHandle; }

	/*!
	   \brief Get native display handle
	   \return
	 */
	NativeDisplayHandle_& getNativeDisplayHandle() { return *m_displayHandle; }

	/*!
	   \brief Get info
	 */
	std::string getInfo();

	/*!
	   \brief Return true if is initialized
	   \return
	 */
	bool isInitialized()  const { return (m_platformContextHandles.get() != 0) && m_initialized; }

	/*!
	   \brief getID
	   \return
	 */
	size_t getID() const {	return 	m_ContextImplementationID;	}

	/*!
	   \brief operator ==
	   \param rhs
	   \return
	 */
	bool operator==(const PlatformContext& rhs) { return m_ContextImplementationID == rhs.m_ContextImplementationID; }

	/*!
	   \brief getOsManager
	   \return
	 */
	OSManager& getOsManager() { return m_OSManager; }

	/*!
	   \brief Get os manager
	   \return
	 */
	const OSManager& getOsManager()const { return m_OSManager; }

	/*!
	   \brief Get last bound context
	   \return
	 */
	static PlatformContext* getLastBoundContext();

private:
	PlatformContext(const PlatformContext& rhs);//deleted
	OSManager& m_OSManager;
	NativePlatformHandles m_platformContextHandles;
	NativeDisplayHandle m_displayHandle;
	int8 m_swapInterval;
	bool m_initialized;
	bool m_preInitialized;
	bool m_enableDebugValidation;
	size_t m_ContextImplementationID;
	Api m_maxApiVersion;

	//Must be called after the context has been active inorder to query the driver for resource limitations.
	void populateMaxApiVersion();

	inline bool hasImplementation() { return m_ContextImplementationID != static_cast<size_t>(-1); }
};
}
}
