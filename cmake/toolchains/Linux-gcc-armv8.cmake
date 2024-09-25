# gcc and g++ should be directly callable which can be done by prepending them onto your PATH environment variable.
# The compiler should be able to infer the location of any libraries and include files which are reasonably included as part of a toolchain package.
# This means that to link against a library such as librt.a you can add it directly as a library to link against and the compiler will find it.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR "armv8_64")

# Specify the cross compiler
set(CROSS_COMPILE "aarch64-linux-gnu-")

set(CMAKE_C_COMPILER   	"${CROSS_COMPILE}gcc")
set(CMAKE_CXX_COMPILER 	"${CROSS_COMPILE}g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a" CACHE STRING "")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -march=armv8-a" CACHE STRING "")

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -march=armv8-a" CACHE STRING "")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)