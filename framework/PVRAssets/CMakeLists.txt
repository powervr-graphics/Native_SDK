set(source_files
    BoundingBox.h
    fileio/GltfReader.cpp
    fileio/GltfReader.h
    fileio/PODDefines.h
    fileio/PODReader.cpp
    fileio/PODReader.h
    Geometry.h
    Helper.cpp
    Helper.h
    IndexedArray.h
    Model.h
    model/Animation.cpp
    model/Animation.h
    model/Camera.cpp
    model/Camera.h
    model/Light.cpp
    model/Light.h
    model/Mesh.cpp
    model/Mesh.h
    model/Model.cpp
    PVRAssets.h
    ShadowVolume.cpp
    ShadowVolume.h
    Volume.cpp
    Volume.h
)

add_library(PVRAssets STATIC ${source_files})
target_include_directories(PVRAssets PUBLIC ..)
target_compile_definitions(PVRAssets PUBLIC
    $<$<CONFIG:Debug>:DEBUG=1>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>
)
target_link_libraries(PVRAssets PUBLIC PowerVR_SDK)
