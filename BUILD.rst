==================
Build Instructions
==================

All the projects in this SDK are based on `CMake <https://cmake.org/>`__, an open-source, cross-platform tool designed to create project files for a variety of compilers and IDEs.
Please download CMake from `the CMake website <https://cmake.org/download/>`__ and install it to be able to create the projects to build our SDK.

Android builds use the same CMake files, integrated in the Android gradle-based build system. To build for Android, follow the specific instructions for Android - do not run CMake manually. It is not necessary to download CMake to build for Android, as it is bundled in the Android SDK. See the `Android instructions`_.

Each example in the SDK contains:

* The CMake file
* Supporting files in a folder called ``cmake-resources``. These are files needed to generate CMake projects for some platforms. 
* Android build scripts in a ``android-build`` folder. These will build that example, including both Framework and external dependencies. 

There is also a global CMake file and ``android-build`` at the root of the SDK, which will build all the examples and Framework projects together.

CMake instructions
------------------

For building on Android, do not call CMake manually as the Android build system calls it at the correct time. See the `Android instructions`_.

* Create a directory for the generated files, and navigate to this directory. 
* Execute CMake, pointing it to the directory where the ``CMakeLists.txt`` is located.

  For example: from ``[path-to-sdk]/cmake-build/``, or from ``[path-to-sdk]/examples/[example_api]/[example_name]/`` folder:

  ``cmake ..`` (optionally ``-G`` for Unix Makefiles, Visual Studio 15, Xcode, Eclipse and so on)

Building the generated project
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The projects can be built as usual based on the types of projects selected, such as through Visual Studio or calling ``make`` for the makefiles.

Binaries are output in the ``bin`` subfolder of the CMake build folder or ``android-build`` folder.

Unix Makefiles
......................

Unix makefiles are the default way to build on on Linux, but also work anywhere where a ``make`` program exists.

Unix makefiles can also be used on macOS and potentially other platforms that support a ``make`` command.

Building the project is performed by calling ``make [-j8 , other options]``

**Note:** Always pass the ``-j[some number]`` parameter with a number close to the number of CPU cores on the system when building with makefiles. This speeds the build up *considerably*. We have found ``-j8`` to be safe and suitable for a lot of systems.

Windows Visual Studio
.....................
Microsoft Visual Studio is the default generator on Windows. CMake cannot generate multi-architecture projects (ones that support both 32-bit and 64-bit) as is conventional for those familiar with MSVC, so only one can be selected. It is recommended to use 64-bit if it is available, but both are fully supported. 

The default CMake architecture is 32-bit. It can be set to 64-bit by passing the ``-A[x64]`` parameter.

* ``cmake [path-to-CMakeLists.txt]`` - generates a solution for the installed version of Visual Studio, 32-bit
* ``cmake [path-to-CMakeLists.txt] -Ax64`` - generates a solution for the installed version of Visual Studio, 64-bit
* ``cmake [path-to-CMakeLists.txt] -G "Visual Studio 15" -Ax64`` - generates Visual Studio 2017 solution, 64-bit
* ... and so on

Xcode (when targetting macOS)
.............................

In order to generate Xcode projects, the Xcode generator must be explicitly passed:

``cmake [path-to-CMakeLists.txt] -G Xcode``

The generated project files can be opened with Xcode as normal, or built from command-line with ``xcodebuild`` or ``cmake --build .``

Xcode (when targeting iOS)
..........................

The instructions for iOS are the same as macOS except a CMake toolchain file needs to be passed, as iOS is cross-compiled. This is provided in ``[path-to-sdk]/cmake/toolchains/Darwin-gcc-ios``

Generate the Xcode projects with:

``cmake [path-to-CMakeLists.txt] -G Xcode -DCMAKE_TOOLCHAIN_FILE=[path-to-sdk]/cmake/toolchains/Darwin-gcc-ios.cmake``

CMake supported Parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~
Parameters can be passed to CMake using the ``-D[PARAM_NAME]=[PARAM_VALUE]`` syntax. There are many built-in parameters that can be passed to control aspects of the build. See the CMake documentation for more information on advanced usage.

There are a few PowerVR SDK specific parameters defined. The most important ones are as follows:

* **-DCMAKE_BUILD_TYPE** : The build variant. Supported values: Debug, **Release (Default)**, MinSizeRel, RelWithDebInfo
* **-DBUILD_OPENGLES_EXAMPLES** (Optional): Pass this parameter if both Vulkan and OpenGL examples are downloaded but, for whatever reason, only a solution for the OpenGL ES ones is required
* **-DBUILD_VULKAN_EXAMPLES** (Optional): Pass this parameter if both Vulkan and OpenGL examples are downloaded but, for whatever reason, only a solution for the Vulkan ones is required
* **-DWS=[NullWS (default), X11, Wayland, Screen]** : (Linux and QNX Only) - controls the windowing system. Usually, desktop Linux systems will be running an X11/XCB or Wayland server. Development platforms often use a NullWS system which is where the GPU renders to the screen directly without a windowing system. Screen is commonly used on QNX. Normally ``-DWS=X11`` or ``-DWS=Wayland`` is needed for typical desktop PCs running Linux, and ``-DWS=X11``, ``-DWS=Wayland`` or ``-DWS=NullWS`` for developer boards.

Cross-compilation
~~~~~~~~~~~~~~~~~

CMake uses toolchain files for cross-compiling. These are usually not necessary when targeting the machine that is being built on, also known as native or host compilation.

For cross-compiling, there are several different CMake toolchain files for a number of different architectures on Linux (armv7, armv7hf, armv8, x86_32, x86_64, mips32, mips64), QNX (aarch64le, armle-v7, x86_32, x86_64) and iOS. These can be found in ``[path-to-sdk]/cmake/toolchains``, and they can be used as guidance to make a different build setup. 

Toolchains are passed directly to the CMake command-line: ``cmake ../.. -DCMAKE_TOOLCHAIN_FILE=[path-to-sdk]/cmake/toolchains/Linux-gcc-armv8`` 

Android instructions
--------------------

Setup
~~~~~

Download the `Android SDK <https://developer.android.com/studio/index.html>`__ and NDK, and configure it with the Android platforms required. The examples currently use Android 26 but can easily be changed. We recommend using Android Studio, but the command-line tools work as well.

If using Android Studio:

* Use the dialog: ``Import project``, and import the ``build-android`` folder for building.
* Go to Tools>Android>SDK Manager and download the NDK, CMake, LLDB, Android platform 26, and Android build tools.
* Follow the IDE prompts to download and configure any additional packages required.
   
If using command-line tools:

* Use the Android SDK Manager and download the NDK, CMake, LLDB, Android platform 26, and Android build tools.
* Navigate to the ``build-android`` folder either in the root of the SDK, or in the folder of the example for building.
* Create a ``local.properties`` file variable, and add the line ``sdk-dir=[path-to-the-ANDROID-sdk]``, or add an environment variable ``ANDROID_HOME=[path-to-the-ANDROID-sdk]``.

Gradle basics
~~~~~~~~~~~~~

Android uses its own build system, which uses CMake internally. It is not necessary to call CMake directly, instead, run Gradle or the Gradle Wrapper and it will call CMake when necessary.

Each example has a ``build-android`` folder which contains the necessary Gradle project files for only that example, and a ``build-android`` folder in the root of the SDK for building the entire SDK.

The easiest way to build, run, and debug with Gradle is to download and use Android Studio from Google. This is highly recommended, if nothing else for the easy on-device debugging that it offers.

***Note:** Use Google's documentation for general instructions on using Android Studio. The UI is intuitive and we do not require special steps. Android Studio may prompt for downloading and installing various packages, or updating the examples when newer versions of plugins are available.

Building from the command-line is very easy. The ``gradle wrapper`` is used to avoid downloading and installing ``gradle``. The wrapper is a tiny script located in the corresponding ``build-android`` folder. The wrapper will automatically download (if not present) the required Gradle version and run it. Using the Gradle wrapper is optional, it can still be downloaded and installed manually.

To use the Gradle wrapper:

* Run ``gradlew assemble[Debug/Release] [parameters]`` from the ``build-android`` folder

To use Gradle:

* Download, install, and add Gradle to the path
* Run ``gradle assemble[Debug/Release] [parameters]`` from the ``build-android`` folder

Gradle properties
~~~~~~~~~~~~~~~~~

There are a few different properties that can/need to be configured. These can be set up in different places:

* A ``gradle.properties`` file in each example or Framework module configures properties for that project.
* A global ``gradle.properties`` file in the ``GRADLE_USER_HOME`` directory. This is not provided, but it is very convenient to globally override all the SDK options. For example - key signing, or for changing the target Android ABI for the whole SDK.
* The properties can be passed as command-line parameters, by passing ``-P[PARAM_NAME]=[PARAM_VALUE]`` to the command-line.

Android ABIs
............
By default, every example's ``gradle.properties`` file has an ``ANDROID_ABIS=x86,x86_64,armeabi-v7a,arm64-v8a`` entry. This creates an apk that targets those architectures.

During development it is usually preferable to build only for your platform's architecture to decrease build times. To change the architectures which are built, there are several options:

* Change the properties in each required project 
* Add a corresponding line to the global ``gradle.properties`` file. This overrides per-project properties.
* Build with, for example, ``gradlew assembleDebug -PANDROID_ABIS=armeabi-v7a``. This overrides both ``gradle.properties`` files.

APK Signing
...........

The provided Gradle scripts have provision for signing the release apks. This is achieved by setting properties in your apks. We recommend that if you set up your own keystore, add your usernames and key aliases to a global ``gradle.properties``, and pass the passwords through the command-line. 

The following properties must be set either per project in per-project ``gradle.properties``, or globally in system-wide ``gradle.properties`` or through the command-line with ``-PNOSIGN``:

* ``KEYSTORE=[Path-to-keystore-file]``
* ``KEYSTORE_PASSWORD=[Password-to-keystore]``
* ``KEY_ALIAS=[Alias-to-signing-key]``
* ``KEY_PASSWORD=[Password-to-signing]``

If the release apks do not need to be signed, pass the parameter ``NOSIGN`` with any value to disable signing:

* ``NOSIGN=[anything]``

