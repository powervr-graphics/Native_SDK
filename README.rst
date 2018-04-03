============
Introduction
============

.. figure:: ./Documentation/SDKBrowser/images/WelcomeGraphic.png

The PowerVR SDK includes source code, example applications, tutorials
and documentation.

Developers can join the PowerVR Insider programme and interact with our
online Community at
`www.powervrinsider.com <http://www.powervrinsider.com>`__. Also visit
our `Contact <./Contact.html>`__ page for further details on how to get
in touch with us.

Release Notes
-------------

For the latest version of the Release Notes detailing what's changed in
this release of the PowerVR Tools and SDK. please go
`here <https://community.imgtec.com/download-notes/>`__.

===============
Getting Started
===============

This section describes how to handle the installation of the PowerVR SDK
on various platforms.

Platform Setup
--------------

***Note:** PVRVFrame PC Emulation is not intended to be a completely
accurate replication of the behaviour of PowerVR hardware. This tool is
a wrapper around desktop OpenGL so its performance and capabilities will
depend on the 3D acceleration present in your system.*

The installation instructions contained in this section use various
placeholders. The table below identifies the placeholders and their
corresponding description.

Placeholder
Description
[AndroidSDK]
Path to the Android PowerVR Graphics SDK.
[API]
Shorthand for the targeted graphics Application Programming Interface,
e.g., OGLES for OpenGL ES 2.0+, or Vulkan. This placeholder is regularly
used for directory and project/makefile names in the SDK.
[ExampleName]
The name of the example application that you are building.
[ExampleLevel]
The name of the folder into which the examples are sorted. These folders
are named either Beginner, Intermediate, Advanced or Navigation.
[Platform]
The targeted Application Binary Interface, e.g., x86.
[ReleaseDir]
This refers to the release directory, which is either ReleaseNullWS or
ReleaseX11 in the case of Linux.
[sdk-installation-directory]
This is the root directory where the SDK is installed.

Android
-------

The PowerVR Graphics SDK examples have been redesigned with the aim of
making it as easy as possible to use with the new Android tools,
including moving from Apache Ant to Gradle, and from Eclipse to Android
Studio.

Using Gradle scripts, the native (NDK) component has been integrated
with the Android Java component, so as to allow easier building from the
Command Line (using the Gradle wrapper), or from Android Studio.

The following instructions for building the examples in the Android
PowerVR Graphics SDK assume that you have:

-  Installed the Android SDK from Google
-  Installed the Android NDK
-  Added the Android NDK to the system path

Adding the Android NDK to the system path for Windows has changed to
Control Panel/System/Advanced System Settings/Advanced/Environment
Variables. On Linux it is dependent on the specific system. It is also
recommended to add the [Android SDK]/platform-tools sub-directory to the
path to facilitate adb and similar tools.

It is recommended to build through Android Studio, but it is also
possible to build through a Command Line.

**To build through the Android Studio:**

#. On the Startup screen or in the main Android Studio screen, select
   Import Project
#. Navigate to the /Build/Android sub-directory of the component to
   build
#. Select the build.gradle file (will normally have a green icon) and
   press OK

From that point onwards, the project can be debugged and/or built using
Android Studio as normal.

**To build through the Command Line:**

#. A file named local.properties must be present in the /Build/Android
   sub-directory. This file must contain a line pointing to the Android
   SDK. On Windows, bear in mind that backslashes (\\) must be escaped
   (\\\\), and semicolons (:) must be escaped (\\:) as well. For
   example:

   -  Windows: sdk.dir=C\\:\\\\Android\\\\AndroidSdk
   -  Linux: sdk.dir=/usr/local/android\_sdk

   In order to create this file you can open a new file in any suitable
   text editor, like Notepad on Windows or Vim, Sublime etc. on Linux.

   If you have imported the example in Android Studio, the
   local.properties file will have already been created for you.

#. Run Gradle. We are bundling the Gradle wrapper so the required Gradle
   version will automatically be downloaded and used. On Windows, run
   gradlew [target]; on Linux, run ./gradlew [target], where [target] is
   usually:

   -  clean to remove old build and temporary files
   -  assembleDebug to create a debug version of the application
   -  assembleRelease to create the basis for a release version of the
      application. In order to create a release version of your
      application, visit the page
      http://developer.android.com/guide/index.html
	  
Windows
-------

This SDK requires a PC equipped with Windows XP, Windows Vista or
Windows 7. The required installation steps are:

#. Install the SDK on your machine as specified by Windows hardware and
   software requirements. The project and solution files provided do not
   require the SDK to be installed in a pre-defined location and are
   configured to use relative paths.

#. If you have installed our SDK with the installer, the PVRVFrame
   libraries will already be in your PATH environment variable.

***Note:** The examples can be launched directly from their project
files.*

***Note:** If the installer has not added the PVRVFrame directory to
your PATH environment variable, you must copy the PVRVFrame emulation
"drivers" to a DLL-accessible directory prior to running the SDK
applications. The driver files names are; EGL, GLES\_CM (OpenGL ES 1.x)
and GLESv2 (OpenGL ES 2.0 & 3.x). The PVRVFrame libraries are provided
as static libraries (lib\*.lib) and dynamic libraries (lib\*.dll).*

Linux
-----

***Note:** API libraries are not distributed with the PowerVR Graphics
SDK for Linux. Please ask your platform provider for these libraries if
you do not have them. You will also need to install the latest platform
toolchain on your development machine for your target platform.*

To build the code examples, follow the steps below:

#. Define the TOOLCHAIN environment variable to the toolchain directory,
   or add the path of the toolchain to the PATH environment variable
   (i.e., run export PATH="\ *path to the toolchain*:$PATH").

#. If you want an X11 build and it is available, define the environment
   variable X11ROOT to point to the freedesktop directory (i.e., export
   X11ROOT=/usr/X11R6\_SGX).

#. To build individual components go to the directory
   Examples/[ExampleLevel]/[ExampleName]/[API]/Build/LinuxGeneric and
   run the command: LinuxNullWS: "make PLATFORM=[Platform]" or LinuxX11:
   "make PLATFORM=[Platform] X11BUILD=1". [Platform] is an entry from
   one of the following supported ABIs:

   -  armv7
   -  armv7hf
   -  armv8
   -  mips\_32
   -  mips\_64
   -  x86\_32
   -  x86\_64

The executables for the examples will be under:
Examples/[ExampleLevel]/[ExampleName]/[API]/Build/[Platform]/[ReleaseDir]
where [ReleaseDir] is one of ReleaseNullWS or ReleaseX11

To run an executable, follow the steps below:

#. Ensure that the PowerVR drivers are installed on the target device
   (please refer to the DDK/driver installation instructions).

#. If the standard C++ libraries are not present on your target device,
   copy libc++ from the toolchain into /usr/lib. libdl and libgcc may
   also be required.

   ***Note:** libc++ lives at /usr/lib if you have installed the
   drivers, or can be found as part of a binary driver release package.*

#. Ensure the drivers are running (e.g., type /etc/init.d/rc/pvr start,
   then run an X session if required).

   Under X11, window sizes can be specified for the executables using
   the command-line arguments -posx=n and -posy=n to define the top
   right hand corner, and -width=n and -height=n to define width and
   height, respectively.

   For example:

   ./[API]IntroducingPOD -posx=10 -posy=10 -width=100 -height=100

#. If you attempt to run an SDK example and it fails with the message:
   "Can't open display" produced by the X client, then make sure that
   the DISPLAY variable is set with the shell command: "set \| grep -e
   DISPLAY". If this command does not yield any output then type (in
   shell): "DISPLAY=:0.0; export DISPLAY"
   
iOS
---

To proceed with the installation of the PowerVR Graphics SDK on iOS,
complete the following steps:

#. Download a version of Apple's iOS SDK from
   http://developer.apple.com/ios/. You will need to become a member of
   Apple's developer programme in order to access this page. You can
   find details of how to join at http://developer.apple.com.

#. Install the Apple SDK on your Mac as specified by Apple's hardware
   and software requirements. This should also install Xcode and the
   other development tools required.

#. Expand the PowerVR Graphics SDK for iOS to a location for which you
   have both read and write access.

#. To build the examples and other projects from the SDK, find the
   various [API]\*\*\*.xcode projects available within the
   SDKPackage\_[API] directory and double click these to launch them in
   Xcode.

#. To build for an iOS device you will need a valid developer
   certificate in your machine's keychain. You may also have to change
   the Properties \| Identifier property from Project \| Edit Active
   Target... to match that which you have set up for yourself through
   Apple's Program Portal.

#. If you do not have a developer certificate from Apple then you can
   still build and launch applications in the iOS Simulator. Choose this
   configuration from the dropdown menu at the top left and then choose
   Build and Run from the dropdown menu.

***Note:** The Scheme that you are building under may need to be set up
for the SDK's projects to run.*

XCode
-----

.. rubric:: XCode Project Setup for OpenGL ES Using PVRVFrame
   :name: xcode-project-setup-for-opengl-es-using-pvrvframe

The following steps detail how to set up, from scratch, an XCode project
for OpenGL ES using PVRVFrame:

#. Open the XCode application and then click File -> New -> Project from
   the menu. Select Cocoa Application from the template OS X or
   Application. Select Next and fill the product name, e.g.,
   IntroducingPOD, and the company identifier. Other fields can be left
   as is. Then select the project destination directory.

#. To organise the project, right-click on it from the Project Navigator
   and select New Group. Following this, create the following groups
   (which will contain the mentioned application contents):

   -  Sources (Application source files)
   -  Content/Models (.pod files)
   -  Content/Textures (.pvr textures)
   -  Content/Shaders (shaders)
   -  Libraries (dependencies)

#. Add the following frameworks to the project:

   -  PVRCore (from
      [sdk-installation-directory]/PVRCore/Builds/OSX/PVRCore.xcodeproj)
   -  PVRShell (from
      [sdk-installation-directory]/PVRShell/Builds/OSX/PVRShell.xcodeproj)
   -  PVRUtils (from
      [sdk-installation-directory]/PVRUtils/OGLES/Builds/OSX/PVRUtilsGles.xcodeproj)
   -  (Optional)PVRCamera (from
      [sdk-installation-directory]/PVRCamera/Builds/OSX/PVRCamera.xcodeproj)

#. Under the Sources group create a new .cpp file for your application
   code.

#. Go to your project’s Targets Build Phases settings and add Your
   application source file under the Compile Sources section:

   Add the library files under LinkBinary With Libraries section:

   -  Lib[API]Tools.a
   -  Quartzcore.framework
   -  libEGL.dylib located at [sdk-installation-directory]/
      /Builds/OSX/x86/Lib
   -  libGLESv2.dylib located at [sdk-installation-directory]/
      /Builds/OSX/x86/Lib

   Then add all content files (such as PVR files, POD files, shaders,
   icons, etc.) under Copy Bundle Resources.

   Following this, add a new build phase called Copy Files, and select
   Destination Frameworks and add these following library files:

   -  libEGL.dylib
   -  libGLESv2.dylib

#. Go to your project’s Search paths section select Yes for Always
   Search User Paths and add these header search paths to User Header
   Search Paths:

   -  [sdk-installation-directory]/Builds/Include
   -  [sdk-installation-directory]/Framework

   Add these following paths to the Library Search Paths:

   -  [sdk-installation-directory]/Builds/OSX/x86/Lib
   
=========
Framework
=========

For a detailed explanation of how the Framework functions, please refer
to the `PowerVR Framework Development
Guide. <./Documents/RELEASE/PowerVR%20Framework.Development%20Guide.pdf>`__

Overview
--------

The PowerVR Framework is a complete framework source code, targeted for
all major platforms, such as Windows, Linux, and OS X, as well as
Android and iOS mobile platforms. A key strength of the PowerVR
Framework is that it is platform-agnostic, meaning that with the same
code, it is possible to compile for different platforms without changing
source code.

The majority of the code is written in C++ and tested across different
compilers (namely Visual Studio 2013, GNU Compiler Collection and Clang)
using modern style and provides a complete framework for application
development. There is also supporting per-platform code (Objective-C
code for iOS and OS X, some Java code for Android, etc.), and project
files.

The Framework consists of libraries divided by functionality, as shown
in the figure below. These modules are provided to be compiled as static
libraries, but you can choose to use them differently, if desired.

.. figure:: ./Documentation/SDKBrowser/images/PowerVRFrameworkComponents.png

PVRCore
-------

`View source code <./Framework/PVRCore/>`__

This is the supporting code of the library to leverage for your own use.
PVRCore is also used by the rest of the Framework and because of that,
all examples using any other part of the Framework should link with
PVRCore.

PVRAssets
---------

`View source code <./Framework/PVRAssets/>`__

This is the Framework’s asset code. It includes classes and helpers for
scene representation, asset loading code, etc. PVRAssets supports the
loading of POD files, PVR and PFX materials format, as well as limited
support for a number of texture formats.

PVRShell
--------

`View source code <./Framework/PVRShell/>`__

This is the native system abstraction (event loops, surfaces, windows,
etc.) which greatly simplifies cross-platform compatibility.
Essentially, PVRShell provides you with useful scaffolding for
cross-platform development.

PVRVk
-----

`View source code <./Framework/PVRVk/>`__

This is a Vulkan C++ wrapper sporting reference-counted objects with
lifetime management, strongly typed enums and other niceties.

PVRUtils
--------

`View source code <./Framework/PVRUtils/>`__

This is actually two libraries (the OpenGL ES version and the Vulkan
version) providing very convenient helpers and wrappers, simplifying
common OpenGL and Vulkan tasks such as context creation and texture
loading. The Vulkan version is written agains PVRVk. Both also contain
each a version of the UIRenderer, a 2D/3D printing library that can be
used for text or sprite renderering. Its interface is very similar
between OpenGL ES and Vulkan, of course taking into account the core
differences of the two APIs.

PVRCamera
---------

`View source code <./Framework/PVRCamera/>`__

This is the code for interfacing with the camera of mobile platforms. A
"dummy" desktop version is provided to ease development.

Building
--------

All PowerVR examples for all platforms will build the PowerVR Framework
libraries they require. If you use them, or base your own code on them,
you should not need to have to build the Framework separately - just add
the relevant project files for your platforms as dependencies. That
said, the PowerVR SDK normally also ships with pre-built versions of the
libraries in the folder [SDK]/Framework/Bin/[Platform], where [SDK] is
the SDK root and [Platform] is the name of your platform of interest -
this is where you would normally link.

All modules can be built separately, by navigating to
[SDK]/Framework/[ModuleName]/Build/[Platform], where [ModuleName] is the
name of the specific module of the PowerVR Framework. You can then run a
build command as normal for that platform, although this is not
required, as building the examples automatically builds the Framework.

Creating an Application
-----------------------

To create a typical application, please follow these steps:

Firstly, either build the Framework components previously mentioned (by
moving to the corresponding /Builds/[Platform] or add their build
projects/scripts to your own as dependencies.
 
Create a project/build script (Visual Studio, Makefile, etc.) for your
platform. We suggest taking one of the scripts from the following
location as base:
Examples/[Intermediate/Advanced]/[API]/Builds/[Platform], where
[Intermediate/Advanced] is the folder for either the intermediate or
advanced example applications supplied with the SDK. In more detail:

Add include directories:

-  /Framework
-  /Builds/Include

Add library directories:

-  Framework/Bin/[Platform]
-  (Optional) /Builds/[Platform]/Lib

Link against static libraries:

-  (Optional) PVRUtilsGles/PVRUtilsVk
-  (If vulkan) PVRVk
-  PVRShell
-  PVRAssets
-  PVRCore

 
Create your application files. For a single CPP file, your includes will
usually be:

-  PVRShell/PVRShell.h
-  PVRUtils/GLES/PVRUtilsGles.h or PVRUtils/Vulkan/PVRUtilsVk.h

 
Write the skeleton of your application (see description of
`PVRShell <#PVRShell>`__)

Guidelines and Recommendations
------------------------------

Below are a set of guidelines and recommendations you might want to
consider when using PowerVR Framework:

-  Use PVRUtils to simplify common suprisingly complex tasks, making
   them easy, concise and understandable: Context creation,
   backbuffer/swapchain setup, texture uploading. Then, simply step
   through the code to understand the actual mechanics implemented.
   Especially important for Vulkan tasks that are surprisingly involved,
   like Texture uploading.
-  The pvr::assets::Model class contains all the information you need
   for drawing, including cameras, lights, and effects. Follow a typical
   PowerVR SDK examples (e.g. IntroducingPVRUtils) to understand its
   basic use, including getting out of a Model the information about the
   data layouts of its Meshes, etc.
-  There are many utility functions that simplify complex tasks between
   Assets and the underlying API - for example, the
   pvr::utils::createInputAssemblyFromXXXXXX functions will make sure to
   populate a Vulkan pipeline's vertex configuration with the correct
   vertex configuration of a mesh. Similarly, the
   createXXXXBufferFromXXXX functions will auto-generate and upload VBOs
   for a mesh. The browse pvr::utils namespace for such helpers
-  Make sure you understand the StructuredMemoryView: It is a class that
   basically allows you to precisely describe a Shader Interface Block
   (i.e. a UBO/SSBO definition in the shader) and then automatically
   calculates all its information of every single one of its members
   (sizes, offsets etc), if needed aligns to dynamic slices and STD140,
   provides helpers for setting values to mapped memory, and in general
   makes working with buffers a breeze. All examples that use UBOs or
   SSBOs use the StructuredMemoryView to define and set values.
   
========
Examples
========

This section of the PowerVR SDK Browser provides a range of example
applications and tutorials that are implemented across multiple APIs.
These examples include optimized and thoroughly commented code. They
also make consistent use of our Framework and provide a wealth of
techniques for the novice user to the advanced developer.

The examples are classified into Beginner, Intermediate and Advanced
categories. Browse to the desired example to view its details. Check the
example description to know which APIs are supported by any specific
example. Additionally, use the `Examples <../../Examples>`__ folder
supplied with this SDK to explore them.

***Note:** Some of the examples do not handle screen rotation, in order
to keep the code as simple as possible. On devices with a portrait
display the example images may appear stretched.*

| Controls are defined as follows for mouse, touch screen and keyboard:
| Action1: *Click/Touch center of screen, Space, Enter.*
| Action2: *Click/Touch left 30% of screen, Key "1".*
| Action3: *Click/Touch right 30% of screen, Key "2".*
| Left/Right/Up/Down: *Swipe/Drag Left/Right/Up/Down, Cursor keys.*
| Quit: *Home/Back button,Close window, Escape/Q key*
   
===============
Contact Details
===============

Forum
-----

For further support, please visit our
`forum <http://forum.imgtec.com>`__.

Support and Ticketing System
----------------------------

Alternatively, file a ticket in our `support
system <https://pvrsupport.imgtec.com>`__.

PowerVR Insider
---------------

To learn more about our PowerVR Graphics SDK and Insider programme,
please visit our `community webpages <http://www.powervrinsider.com>`__.

General Enquiries
-----------------

For general enquiries, please visit our `corporate
website <https://www.imgtec.com/about/contact-us/>`__.

Further Contact Info
--------------------

Imagination Technologies Ltd.
Home Park Estate
Kings Langley
Hertfordshire, WD4 8LZ
United Kingdom
Tel: +44 (0)1923 260511
Fax: +44 (0)1923 277463
