PVRAssets
=========

Overview
--------

PVRAssets provides the necessary classes to work with assets and resources, like Model, Mesh, Texture, Effect, Animation, Camera and others. PVRAssets provides built-in support for
reading all PowerVR deployment formats (PVR textures, POD models and PFX effects) into these classes, usually with a single line of code. PVRAssets classes are very flexible and
can be adapted for a wide variety of uses.

The Model and Mesh classes along with Texture will very commonly be encountered in user code. The Model and Mesh codes are especially very well suited for extracting data to
simplify usual graphics tasks such as building Vertex Buffer Objects (VBOs), automating rendering, etc. PVRAssets is heavily used by PVRUtils and is additionally very commonly used
directly.

You can find the PVRAssets source in the SDK package's ``framework/PVRAssets`` folder.

Using PVRAssets
---------------

To use PVRAssets:

1. Depending on the platform, add the module in by adding either:

   - A project dependency (windows/macOS/ios/android/...) (project file in ``Framework/PVRCamera/Build/[Platform]/...``).
   - The library to link against directly (windows/android/linux makefiles) (.so etc. in ``Framework/Bin/[Platform]/[lib?]PVRCamera[.lib/.so]``).

2. Include the relevant header files (usually ``PVRAssets/PVRAssets.h``).
3. Use the code (load .pod files into models, modify meshes, load .pvr textures, inspect texture metadata, extract attribute information from meshes, etc.).

Code Examples
-------------

.. code:: cpp

   pvr::assets::PODReader podReader(myPODFileStream);
   pvr::assets::Model::createWithReader(podReader, mySceneSmartPointer);

.. code:: cpp

   pvr:::assets::WaveFrontReader objReader(myObjFileStream);
   pvr::assets::Model::createWithReader(objReader, mySceneSmartPointer);
