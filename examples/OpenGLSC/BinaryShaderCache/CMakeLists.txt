cmake_minimum_required(VERSION 3.18)
project(BinaryShaderCache)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SDK_ROOT}/framework/cmake/modules")  #for find_package

if(NOT CMAKE_BUILD_TYPE) #Default to release if the user passes nothing.
	set(CMAKE_BUILD_TYPE "Release")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BinaryShaderCache)

set(BinaryShaderCache_INCLUDE_DIRECTORIES ${SDK_ROOT}/include/)

set(PLATFORM_LIBS ${CMAKE_DL_LIBS})

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

if(WIN32)
	set(SRC_FILES BinaryShaderCache_Windows.cpp)
	add_executable(BinaryShaderCache ${SRC_FILES})
	# Get rid of the "this function is unsafe" warning in Visual Studio
	target_compile_definitions(BinaryShaderCache PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

set_target_properties(BinaryShaderCache PROPERTIES
    CXX_STANDARD 14)

target_link_libraries(BinaryShaderCache PUBLIC
	PVRUtilsGles
)
target_include_directories(BinaryShaderCache PUBLIC ${BinaryShaderCache_INCLUDE_DIRECTORIES})
target_compile_definitions(BinaryShaderCache PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1
