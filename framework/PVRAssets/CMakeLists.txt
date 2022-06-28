cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PVRAssets)
include(GNUInstallDirs)
include(../FrameworkCommon.cmake)
# Copyright (c) Imagination Technologies Limited.

# Prevent PVRAssets being added multiple times
if(TARGET PVRAssets)
	return()
endif()

# Add PVRCore
add_subdirectory(../PVRCore PVRCore)

message(STATUS "Adding PVRAssets")

# PVRAssets include files
set(PVRAssets_HEADERS
	BoundingBox.h
	Geometry.h
	Helper.h
	IndexedArray.h
	Model.h
	PVRAssets.h
	ShadowVolume.h
	Volume.h
	fileio/GltfReader.h
	fileio/PODDefines.h
	fileio/PODReader.h
	model/Animation.h
	model/Camera.h
	model/Light.h
	model/Mesh.h
	model/Animation.h
	model/Camera.h
	model/Light.h
	model/Mesh.h)

# PVRAssets sources
set(PVRAssets_SRC
	fileio/GltfReader.cpp
	fileio/PODReader.cpp
	Helper.cpp
	model/Animation.cpp
	model/Camera.cpp
	model/Light.cpp
	model/Mesh.cpp
	model/Model.cpp
	ShadowVolume.cpp
	Volume.cpp)

# Create the library
add_library(PVRAssets STATIC ${PVRAssets_SRC} ${PVRAssets_HEADERS})

# Add tinygltf to the cmake build system
message(STATUS "Adding submodule tinygltf")
add_subdirectory(${SDK_ROOT_INTERNAL_DIR}/external/tinygltf tinygltf)

# Link PVRAssets to PVRCore and tinygltf
target_link_libraries(PVRAssets
	PUBLIC
		PVRCore
	PRIVATE
		tinygltf)

# Set the include directories of PVRAssets to allow it to see the rest of the framework
# As well as tinygltf
target_include_directories(PVRCore PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..
	${SDK_ROOT_INTERNAL_DIR}/external/tinygltf/source)

# Apply the common framework compile definitions to tinygltf
# This sets certain warnings on/off and adds appropriate
# debug and release flags in the compiler and linker
apply_framework_compile_options_to_target(PVRAssets)