==================
Build Instructions
==================

All the projects in this SDK are based on `CMake <https://cmake.org/>`__, an open-source, cross-platform tool designed to create project files for a variety of Compilers and IDEs.
Please download CMake from `the CMake website <https://cmake.org/download/>`__ and install it to be able to create the projects to build our SDK.

Android builds use the same CMake files, integrated in the Android gradle-based build system. In order to build for Android, follow the specific instructions for Android; you should not run CMake manually.
Also, you do not need to download CMake to build for Android, as it is bundled in the Android SDK. See the `Android instructions`_.

Each example in the SDK contains its own CMake file, a few supporting files in a folder called ``cmake-resources`` (these are files needed to generate CMake projects for some platforms) and Android build 
scripts in a ``android-build`` folder. These will build that example, including all (framework and external) dependencies. There is also a global CMake file and ``android-build`` at the root of the SDK, which 
will build all the Examples and Framework projects in one go.

CMake instructions
------------------

For building on Android, you should not call CMake manually as the Android build system calls that at the correct time. See the `Android instructions`_.

Create and navigate to a directory where to put all the generated files, and execute CMake pointing it to the directory where the CMakeLists.txt is.

e.g. from ``[path-to-sdk]/cmake-build/``, or from ``[path-to-sdk]/examples/[example_api]/[example_name]/``

``[path-to-sdk]/cmake-build>cmake .. (optionally -G"[Unix Makefiles, Visual Studio 15, Xcode, eclipse etc])``


Building the generated project
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The projects can be built "as usual" based on the types of projects you have selected (through Visual Studio, calling ``make`` for the makefiles etc.)

Binaries are output in the ``bin`` subfolder of your cmake build folder or android-build folder.

Unix Makefiles
......................

Unix makefiles are the default way to build on on Linux, but also work anywhere where a make program exists.

Unix Makefiles can also be used on macOS and potentially other platforms that support a ``make`` command.

Building the project is done by calling ``make [-j8 , other options]``

**Note:** We recommend to always pass the -j[some number] parameter with a number close to the number of CPU cores to your system when building with Makefiles - it speeds the build up *dramatically*. We have found -j8 to be safe and suitable for a lot of systems.

Windows Visual Studio
.....................
Microsoft Visual Studio is default generator on Windows. CMake cannot generate multi-architecture projects (ones that support both 32-bit and 64-bit) as is conventional on most familiar with MSVC, so you need to select one. We recommend 64 bit if available, but both are fully supported. The default CMake architecture is 32-bit, you can set it to 64-bit by passing the -A[x64] parameter.

* ``cmake [path-to-CMakeLists.txt]`` //Generates a solution for the installed version of Visual Studio, 32-bit
* ``cmake [path-to-CMakeLists.txt] -Ax64`` //Generates a solution for the installed version of Visual Studio, 64-bit
* ``cmake [path-to-CMakeLists.txt] -G "Visual Studio 15" -Ax64`` //Generates Visual Studio 2017 solution, 64-bit
* ... etc

Xcode (when targetting macOS)
.............................

In order to generate Xcode projects, the Xcode generator must be explicitly passed:

``cmake [path-to-CMakeLists.txt] -G Xcode``

Then, you can open the generate project files with Xcode as normal, or build from command line with ``xcodebuild`` or ``cmake --build .``

Xcode (when targeting iOS)
..........................

The instructions for iOS are same as macOS but, additionally, a CMake Toolchain file needs to be passed (as iOS is cross-compiled). We provide it in ``[path-to-sdk]/cmake/toolchains/Darwin-gcc-ios``

So generate the Xcode projects with:

``cmake [path-to-CMakeLists.txt] -G Xcode -DCMAKE_TOOLCHAIN_FILE=[path-to-sdk]/cmake/toolchains/Darwin-gcc-ios.cmake``

CMake supported Parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~
You can pass parameters to CMake using the -D[PARAM_NAME]=[PARAM_VALUE] syntax. There are a lot of built-in parameters that can be passed to control aspects of the build (see the CMake documentation for more information on advanced usage). Additionally there are a few PowerVR SDK specific parameters we define. The most important ones are as follows:

* **-DCMAKE_BUILD_TYPE** : The build variant. Supported values: Debug, **Release (Default)**, MinSizeRel, RelWithDebInfo
* **-DBUILD_OPENGLES_EXAMPLES** (Optional): Pass this parameters if you have downloaded both Vulkan and OpenGL examples but, for whatever reason, you want to create a solution only for the OpenGL ES ones
* **-DBUILD_VULKAN_EXAMPLES** (Optional): Pass this parameters if you have downloaded both Vulkan and OpenGL examples but, for whatever reason, you want to create a solution only for the OpenGL ES ones
* **-DWS=**[**NullWS(Default)**, X11, Wayland, Screen] : **(Linux and QNX Only)**. Controls the Windowing System. Usually, desktop linux systems will be running an X11/XCB or Wayland server. Development platform very commonly use a NullWS system (the GPU renders to the screen directly without a windowing system). Screen is commonly used on QNX. Usually, you need -DWS=X11 or -DWS=Wayland for typical desktop PCs running Linux, and -DWS=X11, -DWS=Wayland or -DWS=NullWS for developer boards.

Cross-compilation
~~~~~~~~~~~~~~~~~

CMake uses toolchain files for cross-compiling. These are usually not necessary when you are targeting the machine you are building on (native or host compilation).

For cross-compiling, we are already providing a number of different CMake toolchain files for a number of different architectures on Linux (armv7,armv7hf,armv8,x86_32,x86_64, mips32, mips64), QNX(aarch64le, armle-v7, x86_32, x86_64) and iOS. You can find these in ``[path-to-sdk]/cmake/toolchains``, and you can also use these as guidance to make a different build setup. Toolchains are of course passed directly to the cmake command line: ``cmake ../.. -DCMAKE_TOOLCHAIN_FILE=[path-to-sdk]/cmake/toolchains/Linux-gcc-armv8`` 

Android instructions
--------------------

Setup
~~~~~

Download the `Android SDK <https://developer.android.com/studio/index.html>`__ , NDK and configure it with the Android Platforms you require (the examples currently use Android 26 but can easily be changed). We recommend using Android Studio, but the command line tools work as well.

* If using Android Studio
  + Use the dialog: ``Import project``, and import the ``build-android`` folder you wish to build
  + Go to Tools>Android>SDK Manager and download the NDK, CMake, LLDB, Android platform 26, Android build tools
  + In general, follow the IDE's prompts to download and configure any additional packages required.
* If using command line tools
  + Use the Android SDK Manager and download the NDK, CMake, LLDB, Android platform 26, Android build tools
  + Navigate to the ``build-android`` folder either in the root of the SDK, or in the folder of the Example you wish to build.
  + Create a local.properties file variable and add the line ``sdk-dir=[path-to-the-ANDROID-sdk]``, or add an environment variable ``ANDROID_HOME=[path-to-the-ANDROID-sdk]``

Gradle basics
~~~~~~~~~~~~~

Android uses its own build system, which uses CMake internally: you do not call CMake at all, you run Gradle or the Gradle Wrapper and it will call CMake when necessary.

Each example has a ``build-android`` folder which contains the necessary Gradle project files that example only, and a ``build-android`` folder in the root of the SDK for building the entire SDK.

The easiest way to build, run and debug with gradle is to download and use Android Studio from Google. This is highly recommended, if nothing else for the easy on-device debugging that it offers.

* Use Google's documentation for general instructions on using Android Studio. The UI is intuitive and we do not require special steps. Additionally, android studio may prompt you to download and install various packages, update several aspects of the examples (such as when newer versions of plugins are available) etc., you may do that if you wish.

Otherwise, building from the command line is also very easy. We are using the ``gradle wrapper`` to avoid the need to download and install ``gradle``. The wrapper is a tiny script located in the corresponding ``build-android`` folder. This means you do not need to install gradle. The wrapper will automatically download (if not present) the required gradle version and run it. Using the gradle wrapper is optional, if you prefer you can download, install and use Gradle.
To use the gradle wrapper:

* Run ``gradlew assemble[Debug/Release] [parameters]`` from the build android folder

To use gradle:

* Download, install and add to the path ``gradle``
* Run ``gradle assemble[Debug/Release] [parameters]`` from the build-android folder

Gradle properties
~~~~~~~~~~~~~~~~~

There are a few different properties that can/need to be configured. These can be set up in different places:

* A ``gradle.properties`` file in each example or framework module configures properties for that project.
* Additionally, a global ``gradle.properties`` file in the GRADLE_USER_HOME directory. We do not provide that, but it is very convenient to globally override all the SDK options, for example for key signing or for changing the target android ABI for the whole SDK
* Finally, you can pass those properties as command-line parameters, by passing -P[PARAM_NAME]=[PARAM_VALUE] in the command line.

Android ABIs
............
By default, each example's ``gradle.properties`` file has an ``ANDROID_ABIS=x86,x86_64,armeabi-v7a,arm64-v8a`` entry. This creates an apk that targets those architectures.

To build for more or less architectures than (for example, when developing you would probably only build for your platform's architecture, to decrease build times), either change the properties in each project you want, or add a corresponding line to the global ``gradle.properties`` file (this overrides per-project properties), or build with, for example, ``gradlew assembleDebug -PANDROID_ABIS=armeabi-v7a`` (this overrides both ``gradle.properties`` files).

APK Signing
...........

Our gradle scripts have provision for signing the release apks. In order to do that, all you need to do is set up some properties in your apks. We recommend that if you set up your own keystore, add your usernames and key aliases to a global ``gradle.properties``, and pass the passwords through the command line. In any case, the following properties must be set either per project (in per-project gradle.properties) or globally (in systemwide gradle.properties) or through command line (-PNOSIGN):

* ``KEYSTORE=[Path-to-keystore-file]``
* ``KEYSTORE_PASSWORD=[Password-to-keystore]``
* ``KEY_ALIAS=[Alias-to-signing-key]``
* ``KEY_PASSWORD=[Password-to-signing]``

If you don't wish to sign your release apks, you must instead pass the parameter NOSIGN(with any value) to disable signing

* ``NOSIGN=[anything]``

