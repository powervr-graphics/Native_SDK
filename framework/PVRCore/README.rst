PVRCore
=======

Overview
--------

PVRCore is a collection of supporting code for the PowerVR Framework. Code using other modules of the Framework should link with PVRCore.

Some examples of code that can be found in PVRCore:

- Utility classes and specialized data structures used by the Framework (``RingBuffer.h``, ``ContiguousMap.h``)
- The main Smart Pointer class used by the Framework (``RefCounted.h``)
- Data streams (e.g. ``FileStream.h``, ``BufferStream.h``)
- Logging and error reporting (``Log.h``)
- Special math (projection matrix calculations, bounding boxes, shadow volumes).

PVRCore is API agnostic, and generally either platform agnostic or actually abstracting the platform (e.g. Log).
You would usually not have to include files from here if you wish to utilize specific functionality, as most functionality is
already included by the rest of the Framework, so that - even though you can - you will normally not need to include PVRCore files.
PVRCore heavily uses the Standard Template Library.

PVRCore uses the following external modules (bundled under [SDKROOT]/external):

- GLM for linear algebra and generally math
- PugiXML for reading XML files
- ConcurrentQueue for multithreaded queues.

You can find the PVRCore source in the SDK package's ``framework/PVRCore`` folder.

Using PVRCore
-------------

To use PVRCore:

1. Depending on the platform, add the module in by adding either:

   - A project dependency (windows/macOS/ios/android gradle/...) (project file in ``Framework/PVRCore/Build/[Platform]``).
   - A library to link against (linux makefiles) (.so etc. in ``Framework/Bin/[Platform]/[lib?]PVRCore[.lib/.so]``).

2. Most commonly used files will already be included if you use the "main" modules of the PowerVR Framework (PVRShell/PVRUtils etc).
3. Otherwise, include ``Framework/PVRCore/PVRCore.h`` or the specific files containing the functionality you require.

Code Examples
-------------

.. code:: cpp

   pvr::FileStream myTexture("BodyNormalMap.pvr");
   pvr::BufferStream myTexture(myTextureInMemory);

Everything that deals with file/asset data uses streams. Also check ``WindowsResourceStream`` and ``AndroidAssetStream``.
