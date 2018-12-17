# gcc should be directly callable which can be done by prependeding them onto your PATH environment variable.
# The compiler should be able to infer the location of any libraries and include files which are reasonably included as part of a toolchain package.
# This means that to link against a library such as libscreen.a you can add it directly as a library to link against and the compiler will find it.

set(CMAKE_SYSTEM_NAME QNX)
set(CMAKE_SYSTEM_PROCESSOR "aarch64")

set(arch gcc_ntoaarch64)

if(NOT DEFINED QNX_HOST)
	set(QNX_HOST $ENV{QNX_HOST})
endif()

if(NOT DEFINED QNX_TARGET)
	set(QNX_TARGET $ENV{QNX_TARGET})
endif()

if(CMAKE_HOST_WIN32)
  set(HOST_EXECUTABLE_SUFFIX ".exe")
endif()

set(CMAKE_C_COMPILER   	"qcc")
set(CMAKE_C_COMPILER_TARGET ${arch})
set(CMAKE_CXX_COMPILER 	"QCC")
set(CMAKE_CXX_COMPILER_TARGET ${arch})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)