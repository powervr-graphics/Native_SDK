.. figure:: ./Documentation/SDKBrowser/images/WelcomeGraphic.png

===============
The PowerVR SDK
===============

The PowerVR SDK is an open source codebase that can help you develop Graphics applications for PowerVR and other platforms.
It consists of two main parts: the examples and the framework.

The framework is a collection of libraries that aim to ease development of OpenGL and Vulkan real-time applications
by removing the boilerplate code and in general enhancing productivity. The Framework can be thought of as the
lowest-level parts of a graphics engine - the main loop, platform abstraction and graphical utilities. There are some
experimental examples of how someone could build an actual engine on top of it.
Basically, the framework is there to make the uninteresting bits smaller (or even make them disappear), while focusing on the Graphics API calls
to draw something on the screen, with as little boilerplate as possible.

The examples themselves are mostly code samples intended to:

* Show the basics of OpenGL and Vulkan (e.g. HelloAPI, IntroducingPVRShell)
* Demonstrate optimal implementations for techniques relevant for PowerVR (e.g. our DeferredShading with Pixel Local 
  Storage/transient attachments, our GaussianBlur special implementation)
* Demonstrate how to use important extensions that may optimise your code (e.g. IMGFrameBufferDownsample, 
  IMGTextureFilterCubic)

The examples are built on top of the framework in such a way so that only the relevant code is left.

Developers can join the PowerVR Insider programme and interact with our
online Community at `www.powervrinsider.com <http://www.powervrinsider.com>`_. Also visit
our `Contact <./Contact.html>`_ page for further details on how to get
in touch with us.

Release Notes
-------------

For the latest version of the Release Notes detailing what's changed in
this release of the PowerVR Tools and SDK. please visit
`Release Notes <https://community.imgtec.com/download-notes/>`_.

Building
--------

The PowerVR SDK uses CMake for building for any platform, and additionally Gradle for Android.
Navigate to the root of the SDK or any example and run cmake from a folder you create.
For android, navigate to a the build-android folder of the item you wish to build and use gradle.
** For detailed instructions for building the SDK, see `BUILD.rst <BUILD.rst>`_ **

Platform Setup
--------------

Android
~~~~~~~

* Install the Android SDK
Through the Android SDK Manager (either through Android Studio or command line SDK manager), install the following packages
  + Install the Android NDK bundle
  + Install the Android 26 platform (that is the build version we are using)
  + CMake
  + LLDB (for on-device debugging)

If you are using Android Studio, it will prompt you for any missing packages when attempting to build.

The gradle build scripts can be found in the ``[path-to-sdk]``/build-android folder, and in each example's corresponding ``build-android`` folder. 

* To build from Android Studio, use the ``Import project`` dialog select the corresponding ``build-android`` the first time you open a solution.
* To build from the command line, navigate to the build-android folder and run ``gradlew assemble[Debug/Release]``
* **For detailed build instructions, see `BUILD.rst <BUILD.rst>`_ **

iOS
---

* Download a version of Apple's iOS SDK from `http://developer.apple.com/ios/ <http://developer.apple.com/ios/>`__. You will need to become a member of Apple's developer programme in order to access this page. You can find details of how to join at http://developer.apple.com.
* Install the Apple SDK on your Mac as specified by Apple's instructions. This will include Xcode and any other development tools required.
* To build for an iOS device you will need a valid Apple develeloper certificate in your machine's keychain. You may also have to change the ``Properties | Identifier`` property from ``Project | Edot Active Target...`` to match that which you have set up for yourself through Apple's Program Portal.
* If you do not have a developer certificate from Apple then you can still build and launch applications in the iOS Simulator. Choose this configuration from the dropdown menu at the top left and then choose Build and Run from the dropdown menu.
* Extract the PowerVR SDK to a location of your choosing
* Install CMake
* Run CMake and use the corresponding projects (see the detailed build instructions)
***Note:** The Scheme that you are building under may need to be set up for the SDK's projects to run.*
* **For detailed build instructions, see `BUILD.rst <BUILD.rst>`_ **

Windows
-------
***Note:** PVRVFrame PC Emulation is not intended to be a completely
accurate replication of the behaviour of PowerVR hardware. This tool is
a wrapper around desktop OpenGL so its performance and capabilities will
depend on the 3D acceleration present in your system.*

* Install the PowerVR SDK on your machine at a location of your choosing.
* If you have installed our SDK with the installer, the PVRVFrame emulation libraries will already be in your PATH environment variable.
* Otherwise, manually add the PVRVframe libraries to your path. Depending on the type of package you got, they may be pre-installed in the [path-to-sdk]/libs folder
* Run CMake to generate your project files and run the corresponding projects.
* **For detailed build instructions, see `BUILD.rst <BUILD.rst>`_ **

***Note:** If the installer has not added the PVRVFrame directory to your PATH environment variable, you must copy the PVRVFrame emulation
libraries to a DLL-accessible directory prior to running the SDK applications. This may be anywhere in the path, next to the executable, or in the windows default folders.
If you wish to install system-wide and run both 32 and 64 bit builds using the PVRVFrame libraries, you can copy the 64-bit version in %windir%\System32 and the 32-bit version in %windir%\SysWOW64
so that they are automatically selected by the corresponding applications. Otherwise, you may need to manually modify your path based on which architecture you need to run (i.e. if the 32-bit
libraries are in the path 64 bit applications cannot be run and vice versa).
The driver files names are; libEGL.dll, libGLES\_CM.dll (OpenGL ES 1.x) and libGLESv2.dll (OpenGL ES 2.0 & 3.x).

Linux
-----

***Note:** API libraries are not distributed with the PowerVR Graphics
SDK for Linux. Please ask your platform provider for these libraries if
you do not have them. You will also need to install the latest platform
toolchain on your development machine for your target platform.*

* Ensure you have the corresponding libraries to build.
* For example, you may need X11 packages, Wayland packages, libc++ and other libraries depending on the build configuration.
* For on-device compiling, it is enough to usually run CMake.
* For cross-compilation, you can use a CMake toolchain we provide in [path-to-sdk]/cmake/toolchains. We support any of the following architectures:

-  -  armv7
   -  armv7hf
   -  armv8
   -  mips\_32
   -  mips\_64
   -  x86\_32
   -  x86\_64

* Rember to pass the windowing system in the CMake command line: ``-DWS=[X11, Wayland, NullWS...]``
* Run ``cmake [path-to-sdk or path-to-example] [WS=...] [TOOLCHAIN=...]``

To run an executable on a Development machine (desktop):
* Run the binary as normal. X11 and Wayland binaries should be run from within the corresponding windowing system.

To run an executable on a PowerVR Device:

* Ensure that the PowerVR drivers are installed on the target device (please refer to the DDK/driver installation instructions).
* Ensure any libraries you are using are installed in your target device (libc, libc++, libdl etc)
* Ensure the drivers are running (e.g., type /etc/init.d/rc/pvr start, then run an X session if required).
* Run the binary. Several options can be passed, pass the -help parameter to show command line options.
  (for example,  X11 window sizes can be specified for the executables using the command-line arguments -posx=n 
  and -posy=n to define the top right hand corner, and -width=n and -height=n to define width and height, respectively.)
* If you attempt to run an SDK example and it fails with the message:
  "Can't open display" produced by the X client, then make sure that
  the DISPLAY variable is set with the shell command: "set \| grep -e
  DISPLAY". If this command does not yield any output then type (in
  shell): "DISPLAY=:0.0; export DISPLAY"
