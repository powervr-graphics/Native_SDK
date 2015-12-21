/*!*********************************************************************************************************************
\file         PVRCore\PVRCore.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Include this file if you wish to use the PVRCore functionality
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/RefCounted.h"
#include "PVRCore/AxisAlignedBox.h"
#include "PVRCore/FileStream.h"
#include "PVRCore/BufferStream.h"
#include "PVRCore/RingBuffer.h"
#include "PVRCore/StringFunctions.h"
#include "PVRCore/Time_.h"
#include "PVRCore/UnicodeConverter.h"

/*****************************************************************************/
/*! \mainpage PVRCore
******************************************************************************

\tableofcontents 
 
\section overview Overview
*****************************

PVRCore is the supporting code of the library that you can leverage for your own use. PVRCore is also used by the rest of the Framework and because of that, all examples using any other part of the Framework should link with PVRCore. An example of code that can be found in PVRCore:
<ul>
	<li>Interfaces that bind other modules together (e.g. <span class="code">GraphicsContext.h</span>, <span class="code">OSManager.h</span>)</li>
	<li>Utility classes and specialized data structures used by the Framework (<span class="code">RingBuffer.h</span>, <span class="code">ListOfInterfaces.h</span>)</li>
	<li>The main Smart Pointer class used by the Framework (<span class="code">RefCounted.h</span>)</li>
	<li>Data streams (e.g. <span class="code">FileStream.h</span>, <span class="code">BufferStream.h</span>)</li>
	<li>Logging and error reporting (<span class="code">Log.h</span>)</li>
	<li>Special math (bounding boxes, shadow volumes)</li>
</ul>

In general, you have to include files from here only if you wish to utilise a specific functionality (most commonly Streams, Log or Refcounted). That said, most of that functionality is core to the rest of the Framework and at least the basic classes would be exposed. Unlike our old Framework, PVRCore uses the Standard Template Library and Generalized Linear Models. It is also completely API agnostic, and mostly platform agnostic (it retains some platform dependence so as to create the platform abstractions). 

PVRCore source can be found in the <a href="../../>PVRCore</a> folder in the SDK package.

\section usage Using PVRCore
*****************************

To use PVRCore:
<ul>
	<li>Depending on the platform, add the module in by adding either:</li>
	<ol>
		<li>A project dependency (windows/osx/ios/android gradle/...) (project file in <span class="code">Framework/PVRCore/Build/[Platform]</span>)</li>
		<li>A library to link against (linux makefiles) (.so etc. in <span class="code">Framework/Bin/[Platform]/[lib?]PVRCore[.lib/.so]</span>)</li>
	</ol>
	<li>Most commonly used files will already be included if you use the "main" modules of the PowerVR Framework (PVRShell/PVRApi etc)</li>
	<li>Otherwise, include <span class="code">Framework/PVRCore/PVRCore.h</span> or the specific files containing the functionality you require</li>
</ul>

\section code Code Examples
*****************************

\code
pvr::FileStream myTexture(“BodyNormalMap.pvr”); 
pvr::BufferStream myTexture(myTextureInMemory); 
\endcode

Everything that deals with file/asset data uses streams. Also includes <span class="code">WindowsResourceStream</span>, <span class="code">AndroidAssetStream</span>

\code
pvr::Logger myLog; 
myLog.setMessenger(myCustomFileLoggingMessenger);
myLog(“I am logging this in my custom logger”);
pvr::Log(Log.Verbose, “Usually I will just be using the global PowerVR Log object. Nobody likes globals, except for logging…”);
\endcode
*/