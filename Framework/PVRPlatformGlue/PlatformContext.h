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
\brief         The pvr::system namespace contains low-level, system-communication classes and functions
***********************************************************************************************************************/
namespace system {

/*!*********************************************************************************************************************
\brief         The platform context is the class wrapping all platform-specific objects required to power the PVRApi Graphics
              context (Displays, windows, configurations etc.).
***********************************************************************************************************************/
class PlatformContext : public IPlatformContext
{
public:
	PlatformContext(OSManager& mgr) : m_OSManager(mgr), m_platformContextHandles(), m_swapInterval(-2), m_initialised(false),
		m_preInitialised(false), m_ContextImplementationID(static_cast<size_t>(-1)), m_maxApiVersion(Api::Unspecified) { }


	Result::Enum init();

	void release();

	Api::Enum getMaxApiVersion();

	bool isApiSupported(Api::Enum api);

	Result::Enum presentBackbuffer();

	Result::Enum makeCurrent();

	const NativePlatformHandles& getNativePlatformHandles() const;

	std::string getInfo();

	bool isInitialised() { return m_platformContextHandles.get() && m_initialised; }

	size_t getID()
	{
		return 	m_ContextImplementationID;
	}

	bool operator==(const PlatformContext& rhs) { return m_ContextImplementationID == rhs.m_ContextImplementationID; }

	static PlatformContext* getLastBoundContext();

private:
	PlatformContext(const PlatformContext& rhs);//deleted
	OSManager& m_OSManager;
	NativePlatformHandles m_platformContextHandles;
	int m_swapInterval;
	bool m_initialised;
	bool m_preInitialised;
	size_t m_ContextImplementationID;

	Api::Enum m_maxApiVersion;

	//Must be called after the context has been active inorder to query the driver for resource limitations.
	void populateMaxApiVersion();

	Result::Enum init(const NativeDisplay& nativeDisplay, const NativeWindow& nativeWindow, DisplayAttributes& attributes,
	                  const Api::Enum& api);

	inline bool hasImplementation() { return m_ContextImplementationID != static_cast<size_t>(-1); }
};
}
}
