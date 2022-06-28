cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PVRUtilsCL)
include(GNUInstallDirs)
# Copyright (c) Imagination Technologies Limited.

message(STATUS "Adding PVRUtilsOpenCL")

set(PVRUtilsCL_HEADERS
	../../../include/DynamicOCL.h
	OpenCLUtils.h)

add_library(PVRUtilsCL INTERFACE)


# Allow PVRUtilsCL library to see the rest of the framework 
target_include_directories(PVRUtilsCL INTERFACE 
	${CMAKE_CURRENT_LIST_DIR}/../../../include
	${CMAKE_CURRENT_LIST_DIR}/../../)

# Link to PVRCore
target_link_libraries(PVRUtilsCL INTERFACE PVRCore)