.. figure:: ./docs/images/WelcomeGraphic.png

===============
The PowerVR SDK
===============

The PowerVR SDK is an open source codebase to help with the development of graphics applications for PowerVR and other platforms.
It consists of two main parts: the Framework, and our examples.

The Framework is a collection of libraries that aim to make development of OpenGL ES and Vulkan real-time applications much easier, by removing boilerplate code and enhancing productivity. It can be considered to be the lowest-level parts of a graphics engine - the main loop, platform abstraction, and graphical utilities. There are some
experimental examples of how it is possible to build an actual engine on top of the Framework.

The Framework exists to make the uninteresting bits smaller, or even make them disappear, with as little boilerplate as possible. This allows more focus on the graphics API calls for drawing something on the screen.

The examples themselves are mostly code samples intended to:

* Show the basics of OpenGL and Vulkan, with the HelloAPI and IntroducingPVRShell examples.
* Demonstrate optimal implementations for techniques relevant for PowerVR, such as our DeferredShading with Pixel Local Storage/transient attachments, or our Gaussian Blur using Sliding Average compute shader for reducing bandwidth.
* Demonstrate how to use important extensions that may improve performance, such as IMGFramebufferDownsample and IMGTextureFilterCubic.

The examples are built on top of the Framework, and optimised so that only the relevant code is left.

PVRVFrame provides a set of emulation libraries for Windows, macOS, and Linux.

Developers can interact with our online Community at `www.imgtec.com/developers <https://www.imgtec.com/developers/>`_. Visit
our `Contact <./Contact.html>`_ page for further details on how to get in touch with us.

Release notes
-------------

For the latest version of the Release Notes detailing what has changed in this release, please visit `Release Notes <https://www.imgtec.com/developers/powervr-sdk-tools/whats-new/>`_.

Building
--------

The PowerVR SDK uses CMake for building for any platform, and additionally Gradle for Android.
Navigate to the root of the SDK or any example, and run CMake from a created folder. 
For Android, navigate to the ``build-android`` folder of the item to build, and use Gradle.

For detailed instructions for building the SDK, see `here <Build.html>`_. 

Platform setup
--------------

Android
~~~~~~~

*  Install the Android SDK
*  Through the Android SDK Manager, either via Android Studio or command-line SDK manager, install the following packages:
     *  Android NDK bundle
     *  Android 26 platform - this is the build version used
     *  CMake
     *  LLDB - for on-device debugging

*  If using Android Studio, it will prompt for any missing packages when attempting to build.
*  The Gradle build scripts can be found in the ``[path-to-sdk]/build-android`` folder, and in each example's corresponding ``build-android`` folder. 
*  To build from Android Studio, use the ``Import project`` dialog, and select the corresponding ``build-android`` the first time a solution is opened.
*  To build from the command-line, navigate to the ``build-android`` folder and run ``gradlew assemble[Debug/Release]``

For detailed build instructions, see `here <Build.html>`_. 

iOS
~~~

* Download a version of Apple's iOS SDK from `http://developer.apple.com/ios/ <http://developer.apple.com/ios/>`__. It is necessary to become a member of Apple's developer program in order to access this page. Details of how to join can be found at http://developer.apple.com.
* Install the Apple SDK on the Mac as specified by Apple's instructions. This will include Xcode and any other development tools required.
* To build for an iOS device, a valid Apple developer certificate is required in the machine's keychain. The ``Properties | Identifier`` property may need to be changed from ``Project | Edit Active Target...`` to match what was set up through Apple's Program Portal.
* If you do not have a developer certificate from Apple, then it is still possible to build and launch applications in the iOS Simulator. Choose this configuration from the dropdown menu at the top left and then choose Build and Run from the dropdown menu.
* Extract the PowerVR SDK to a local location.
* Install CMake.
* Run CMake and use the corresponding projects. See the detailed build instructions.

***Note:** The Scheme being built under may need to be set up for the SDK's projects to run.

For detailed build instructions, see `here <Build.html>`_. 

Windows
~~~~~~~

***Note:** PVRVFrame is not intended to be a completely
accurate replication of the behaviour of PowerVR hardware. These libraries are
a wrapper around desktop OpenGL ES, so performance and capabilities will
depend on the 3D acceleration present in the system.

* Install the PowerVR SDK on the machine in the required location.
* If the SDK was installed with the installer, the PVRVFrame emulation libraries will already be in the PATH environment variable.
* Otherwise, manually add the PVRVFrame libraries to the path. Depending on the type of package, they may be pre-installed in the ``[path-to-sdk]/libs`` folder
* Run CMake to generate the project files and run the corresponding projects.

***Note:** If the installer has not added the PVRVFrame directory to the PATH environment variable, copy the PVRVFrame emulation
libraries to a DLL-accessible directory prior to running the SDK applications. This may be anywhere in the path, next to the executable, or in the Windows default folders.

To install system-wide and run both 32 and 64-bit builds using the PVRVFrame libraries, copy the 64-bit version to ``%windir%\System32`` and the 32-bit version to ``%windir%\SysWOW64`` so that they are automatically selected by the corresponding applications. Otherwise, it may be necessary to manually modify the path based on which architecture needs to be run. For instance, if the 32-bit libraries are in the PATH, 64-bit applications cannot be run and vice versa.

The driver filenames are:

* ``libEGL.dll``
* ``libGLES_CM.dll`` (OpenGL ES 1.x) 
* ``libGLESv2.dll`` (OpenGL ES 2.0 and 3.x)

For detailed build instructions, see `here <Build.html>`_. 

Linux
-----

***Note:** API libraries are not distributed with the PowerVR Graphics
SDK for Linux. Please ask the platform provider for these libraries if
they are not present. It will also be necessary to install the latest platform
toolchain on the development machine for the target platform.

* Ensure the corresponding libraries to build are present. These may include X11 packages, Wayland packages, libc++, and other libraries depending on the build configuration.
* For on-device compiling, it is usually enough to run CMake.
* For cross-compilation, use a CMake toolchain provided in ``[path-to-sdk]/cmake/toolchains``. The following architectures are supported:
    -  armv7
    -  armv7hf
    -  armv8
    -  mips\_32
    -  mips\_64
    -  x86\_32
    -  x86\_64

* Remember to pass the windowing system in the CMake command-line: ``-DWS=[X11, Wayland, NullWS...]``
* Run ``cmake [path-to-sdk or path-to-example] [WS=...] [TOOLCHAIN=...]``

To run an executable on a desktop development machine:

* Run the binary as normal. X11 and Wayland binaries should be run from within the corresponding windowing system.

To run an executable on a PowerVR device:

* Ensure that the PowerVR drivers are installed on the target device. Please refer to the DDK/driver installation instructions.
* Ensure any libraries being used are installed on the target device. For example: libc, libc++, or libdl.
* Ensure the drivers are running by typing  ``/etc/init.d/rc/pvr start`` then running an X session if required.
* Run the binary. Several options can be passed, pass the ``-help`` parameter to show command-line options.
  For example, X11 window sizes can be specified for the executables using the command-line arguments ``-posx=n`` 
  and ``-posy=n`` to define the top right hand corner. ``-width=n`` and ``-height=n`` are used to define width and height, respectively.
* If an SDK example fails to run with the message:
  "Can't open display" produced by the X client, then ensure that
  the DISPLAY variable is set with the shell command: "``set \| grep -e
  DISPLAY``". If this command does not yield any output then type in
  shell: "``DISPLAY=:0.0; export DISPLAY``"
