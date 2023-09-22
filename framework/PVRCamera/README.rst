PVRCamera
=========

Overview
--------

PVRCamera is unique among the PowerVR Framework modules in that it does not only contain native code. PVRCamera provides an easy-to-use interface between the platform's Camera of Android or iOS and the rest of the Framework. Please refer to the relevant examples (e.g. TextureStreaming) for its use.

Except for linking to the native library, PVRCamera requires a few lines of Java code to be added (for Android applications), and to be available for linking.

You can find the PVRCamera source in the SDK package's ``framework/PVRCamera`` folder.

Using PVRCamera
---------------

To use PVRCamera:

1. Depending on the platform, add the module in by adding either:

   - A project dependency (windows/macOS/ios/android/...) (project file in ``Framework/PVRCamera/Build/[Platform]/...``).
   - The library to link against directly (windows/android/linux makefiles) (.so etc. in ``Framework/Bin/[Platform]/[lib?]PVRCamera[.lib/.so]``).

2. Include the header file (most commonly the ``PVRCamera/PVRCamera.h`` file).
3. Create and use a ``pvr::CameraInterface`` class. For environments without a built-in camera, the ``CameraInterface`` will provide a dummy static image.
