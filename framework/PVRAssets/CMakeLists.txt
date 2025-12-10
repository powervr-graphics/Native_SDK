cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(PVRAssets)
include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../FrameworkCommon.cmake)

# Prevent PVRAssets being added multiple times
if(TARGET PVRAssets)
	return()
endif()

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
	model/Mesh.h
	model/FormattedUserData.h)

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

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${PVR_SDK_ROOT}/cmake/modules")
	find_dependency(PVRCore MODULE REQUIRED)
	find_dependency(tinygltf MODULE REQUIRED)
    find_dependency(glm MODULE REQUIRED)
endif()

# Apply the common framework compile definitions to tinygltf
apply_framework_compile_options_to_target(PVRAssets)

# Link PVRAssets to PVRCore and tinygltf
target_link_libraries(PVRAssets
	PUBLIC
		PVRCore
        glm
	PRIVATE
		tinygltf)

# Set the include directories of PVRAssets
target_include_directories(PVRAssets 
	PUBLIC 
		$<BUILD_INTERFACE:${PVR_FRAMEWORK_ROOT}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		"${PVR_EXTERNAL_ROOT}/glm/source")

# Use the consolidated install function
pvr_install_target(TARGET PVRAssets HEADERS ${PVRAssets_HEADERS})
