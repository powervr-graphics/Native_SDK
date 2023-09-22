/*!
\brief Include this file if you wish to use the PVRShell functionality.
\file PVRShell/PVRShell.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRShell/Shell.h"

#if defined(__QNXNTO__)
#if not(defined(Screen) || defined(NullWS))
#error Please define a valid window system to compile PVRShell for QNX - Supported window systems are NullWS or Screen. Please pass the desired window system using -DPVR_WINDOW_SYSTEM=[NullWS,Screen].
#endif
#elif defined(__linux__)
#if not defined(__ANDROID__)
#if not(defined(X11) || defined(XCB) || defined(Wayland) || defined(NullWS))
#error Please define a valid window system to compile PVRShell for Linux - Supported window systems are X11, XCB, Wayland, or NullWS. Please pass the desired window system using -DPVR_WINDOW_SYSTEM=[NullWS,X11,XCB,Wayland].
#endif
#endif
#endif

/*****************************************************************************/
/*! \mainpage PVRShell
******************************************************************************

\tableofcontents

\section overview Overview
*****************************

PVRShell will usually be the foundation on top of which an application is written. This library abstracts the system and contains, among others, the application's entry point (main(), android_main() or other, depending on the platform), command line arguments, events, main loop, etc., effectively abstracting the native platform.

Also, PVRShell will create and teardown the window, initialize and de-initialize the graphics system, swap the buffers at the end of every frame, search platform-specific methods and file system storage (file system, assets, bundle, etc.).

PVRShell directly depends on PVRCore. Normally, PVRShell is used as the first step of every PowerVR SDK application.

*/
