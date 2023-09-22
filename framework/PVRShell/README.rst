PVRShell
========

Overview
--------

PVRShell will usually be the foundation on top of which an application is written. This library abstracts the system and contains, among others, the application's entry point
(main(), android_main() or other, depending on the platform), command line arguments, events, main loop, etc., effectively abstracting the native platform.

Also, PVRShell will create and teardown the window, initialize and de-initialize the graphics system, swap the buffers at the end of every frame, search platform-specific methods
and file system storage (file system, assets, bundle, etc.).

PVRShell directly depends on PVRCore. Normally, PVRShell is used as the first step of every PowerVR SDK application.

You can find the PVRShell source in the SDK package's ``framework/PVRShell`` folder.

Using PVRShell
--------------

To use PVRShell:

1. Depending on the platform, add the module in by adding either:

   - A project dependency (windows/macOS/ios/android/…) (project file in Framework/PVRShell/Build/[Platform]/…)
   - The library to link against directly (windows/android/linux makefiles) (.so etc. in Framework/Bin/[Platform]/[lib?]PVRShell[.lib/.so])

2. In your main code file, include PVRShell/PVRShell.h
#. Write a function ``pvr::newDemo()`` that returns a ``std::unique_ptr`` to a created instance of your class (refer to the bottom of any example)
#. Implement the pure virtual functions of ``pvr::Shell`` in your own class (``initApplication``, ``initView``, ``renderFrame``, ``releaseView``, ``quitApplication``)
#. Write an application class inheriting from ``pvr::Shell``
#. Implement a simple function ``pvr::newDemo()`` returning an ``unique_ptr`` to an instance of your application class
#. Implement ``pvr::Shell::initApplication()``

   This is the first initialization step. It is called once, after PVRShell is initialized, but before the Device/Graphics Context have been initialized. Initialization steps that do not require a graphical context can go here (Model file loading, etc.). Any preferences that will influence context creation should be here.

#. Implement ``pvr::Shell::initView()``

   This is called right after a Device/Graphics Context is created and bound. Additionally, if the context is lost, this function is called when it is recreated. Implement any initialization operations that would require a graphics context initialized (texture loading, buffer creation etc.).

#. Implement ``pvr::Shell::releaseView()``

   ``releaseView()`` is called just before a Graphics Context is released at teardown, or if it is lost (for example, minimizing the application on Android). Release API objects (textures, buffers) should be here.

#. Implement ``pvr::Shell::quitApplication()``

   This is called just before application teardown, after the context is lost. Release any left-over non-API resources here.

#. Implement ``pvr::Shell::renderFrame()``

   ``renderFrame()`` is called every frame and is intended to contain the logic and actual drawing code. Backbuffer swapping is done automatically.

#. (Recommended, optional step) Implement input event handling

   Override either ``pvr::Shell::eventMappedInput(…)`` and/or any of the device-specific input functions (``pvr::Shell::onKeyDown``, ``onKeyUp``, ``onPointingDeviceDown``, etc.).

   ``eventMappedInput()`` is a call-back provided by PVRShell to handle simplified input events unified across different platforms (for example, both a swipe left of a touchscreen, and the left arrow on a keyboard would map to ``MappedInput::Left``).

The application entry point is provided by PVRShell library. Hence, application start, context creation, initialization, etc., will all be done for you.

Code Examples
-------------

.. code:: cpp

   //The five callbacks house the application
   void MyApplication::initApplication { setApiTypeRequired(pvr::api::OpenGLES3);}
   pvr::Result MyApplication::renderFrame() { float dt =this->getFrameTime(); ... }

.. code:: cpp

   this->getAssetStream("Texture.pvr"); // Will look everywhere for assets: Files, then Windows //Resources(.rc)/iOS bundled resources/Android assets

.. code:: cpp

   void MyApplication::eventMappedInput(SimplifiedInput::Actions evt){ //Abstracts/simplifies
   switch (evt){case MappedInputEvent::Action1: pauseDemo();break;	case MappedInputEvent::Left: showPreviousPage(); break;	case MappedInputEvent::Quit: if (showExitDialogue())
   exitShell(); break;}}

.. code:: cpp

   void MyApplication::eventKeyUp(Keys key){} // Or, detailed keyboard/mouse/touch input

Command-Line Arguments
----------------------

PVRShell takes a set of command-line arguments which allow items like the position and size of the example to be controlled. The table below identifies these options.

.. list-table::
   :widths: auto
   :header-rows: 1

   * - Option
     - Description
   * - -aasamples=N
     - Sets the number of samples to use for full screen anti-aliasing, e.g., 0, 2, 4, 8.
   * - -c=N
     - Save a single screenshot or a range, for a given frame or frame range, e.g., -c=14, -c=1-10.
   * - -colourbpp=N or -colorbpp=N or -cbpp=N
     - Frame buffer colour bits per pixel. When choosing an EGL config, N will be used as the value for EGL_BUFFER_SIZE.
   * - -config=N
     - Force the shell to use the EGL config with ID N.
   * - -depthbpp=N or -dbpp=N
     - Depth buffer bits per pixel. When choosing an EGL config, N will be used as the value for EGL_DEPTH_SIZE.
   * - -display
     - EGL only. Allows specifying the native display to use if the device has more than one.
   * - -forceframetime=N or -fft=N
     - Force PVRShellGetTime to report fixed frame time.
   * - -fps
     - Output frames per second.
   * - -fullscreen=[1,0]
     - Runs in full-screen mode (1) or windowed (0).
   * - -height=N
     - Sets the viewport height to N.
   * - -info
     - Output setup information to the debug output.
   * - -posx=N
     - Sets the x coordinate of the viewport.
   * - -posy=N
     - Sets the y coordinate of the viewport.
   * - -powersaving=[1,0] or -ps=[1,0]
     - Where available enable/disable power saving.
   * - -priority=N
     - Sets the priority of the EGL context.
   * - -quitafterframe=N or -qaf=N
     - Specify the frame after which to quit.
   * - -quitaftertime=N or -qat=N
     - Specify the time after which to quit.
   * - -rotatekeys=N
     - Sets the orientation of the keyboard input. N can be 0-3, where 0 is no rotation.
   * - -screenshotscale=N
     - Allows to scale up screenshots to a bigger size (pixel replication).
   * - -sw
     - Software render.
   * - -version
     - Output the SDK version to the debug output.
   * - -vsync=N
     - Where available modify the app's vsync parameters. 0: No vsync, 1: Vsync, -1: Adaptive Vsync
   * - -width=N
     - Sets the viewport width to N.
