/*!*********************************************************************************************************************
\file         PVRPlatformGlue\PVRPlatformGlue.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains documentation generation scripts for the PVRPlatformGlue library. No direct includes.
***********************************************************************************************************************/

/*****************************************************************************/
/*! \mainpage PVRPlatformGlue
******************************************************************************

\tableofcontents 
 
\section overview Overview
*****************************

PVRPlatformGlue is designed to facilitate the communication between the API and platform, i.e. PVRShell and PVRApi, and should not be used on its own. 

PVRPlatformGlue source can be found in the <a href="../../">PVRPlatformGlue</a> folder in the SDK package.

\section usage Using PVRPlatformGlue
*****************************

To use PVRPlatformGlue:
<ul>
<li>Depending on the platform, add the module in by adding either:</li>
<ol>
<li>A project dependency (windows/osx/ios/android/...) (project file in <span class="code">Framework/PVRPlatformGlue/[Glue API]/Build/[Platform]/...</span>)</li>
<li>The library to link against directly (windows/android/linux makefiles) (.so, .dll etc. in <span class="code">Framework/Bin/[Platform]/[lib?]PVR[Glue-API][.lib/.so]</span>)</li>
</ol>
<li>No header files and no direct use of PVRPlatformGlue is necessary for platform glue: PVRPlatformGlue is used to allow the PowerVR Shell to create and use contexts for different APIs.
<li>Note: A PVRPlatformGlue library (PVREgl, PVREagl or other as available) is necessary to build and run PVRShell</li>
</ul>
*/