/*!
\brief Include this file if you wish to use the PVRCore functionality
\file PVRCore/PVRCore.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/Base/RefCounted.h"
#include "PVRCore/Maths.h"
#include "PVRCore/Base/ComplexTypes.h"
#include "PVRCore/IO.h"
#include "PVRCore/DataStructures.h"
#include "PVRCore/StringFunctions.h"
#include "PVRCore/Strings/StringHash.h"
#include "PVRCore/Base/Time_.h"
#include "PVRCore/Interfaces/IAssetProvider.h"
#include <iterator>


/*****************************************************************************/
/*! \mainpage PVRCore
******************************************************************************

\tableofcontents

\section overview Overview
*****************************

PVRCore is a collection of supporting code for the PowerVR Framework. Code using other modules of the Framework should link with PVRCore. 
An example of code that can be found in PVRCore:
<ul>
	<li>Utility classes and specialized data structures used by the Framework (<span class="code">RingBuffer.h</span>, <span class="code">ContiguousMap.h</span>)</li>
	<li>The main Smart Pointer class used by the Framework (<span class="code">RefCounted.h</span>)</li>
	<li>Data streams (e.g. <span class="code">FileStream.h</span>, <span class="code">BufferStream.h</span>)</li>
	<li>Logging and error reporting (<span class="code">Log.h</span>)</li>
	<li>Special math (projection matrix calculations, bounding boxes, shadow volumes)</li>
</ul>

PVRCore is API agnostic, and generally either platform agnostic or actually abstracting the platform (e.g. Log).
You would usually not have to include files from here if you wish to utilize specific functionality, as most functionality is 
already included by the rest of the Framework, so that - even though you can - you will normally not need to include PVRCore files.
PVRCore heavily uses the Standard Template Library.
PVRCore uses the following external modules (bundled under [SDKROOT]/External):
<ul>
	<li>GLM for linear algebra and generally math</li>
	<li>PugiXML for reading XML files</li>
	<li>ConcurrentQueue for multithreaded queues</li>
</ul>

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
	<li>Most commonly used files will already be included if you use the "main" modules of the PowerVR Framework (PVRShell/PVRUtils etc)</li>
	<li>Otherwise, include <span class="code">Framework/PVRCore/PVRCore.h</span> or the specific files containing the functionality you require</li>
</ul>

\section code Code Examples
*****************************

\code
pvr::FileStream myTexture(“BodyNormalMap.pvr”);
pvr::BufferStream myTexture(myTextureInMemory);
\endcode

Everything that deals with file/asset data uses streams. Also check <span class="code">WindowsResourceStream</span>, <span class="code">AndroidAssetStream</span>

\code
pvr::Logger myLog;
myLog.setMessenger(myCustomFileLoggingMessenger);
myLog(“I am logging this in my custom logger”);
Log(Log::Verbose, “Usually I will just be using the global PowerVR Log function. Nobody likes globals, except for logging…”);
\endcode
*/