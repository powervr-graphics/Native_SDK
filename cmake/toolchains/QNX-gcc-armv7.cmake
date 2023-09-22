# qcc should be directly callable which can be done by prependeding them onto your PATH environment variable.
# The compiler should be able to infer the location of any libraries and include files which are reasonably included as part of a toolchain package.
# This means that to link against a library such as libscreen.a you can add it directly as a library to link against and the compiler will find it.

set(CMAKE_SYSTEM_NAME QNX)
set(CMAKE_SYSTEM_PROCESSOR "armv7")
set(ARCH 32)

set(arch gcc_ntoarmv7)

if(NOT DEFINED QNX_HOST)
	set(QNX_HOST $ENV{QNX_HOST})
endif()

if(NOT DEFINED QNX_TARGET)
	set(QNX_TARGET $ENV{QNX_TARGET})
endif()

if(CMAKE_HOST_WIN32)
  set(HOST_EXECUTABLE_SUFFIX ".exe")
endif()

set(CMAKE_C_COMPILER    "${QNX_HOST}/usr/bin/ntoarmv7-gcc${HOST_EXECUTABLE_SUFFIX}")
set(CMAKE_CXX_COMPILER  "${QNX_HOST}/usr/bin/ntoarmv7-g++${HOST_EXECUTABLE_SUFFIX}")
set(CMAKE_AR_COMPILER   "${QNX_HOST}/usr/bin/ntoarmv7-ar${HOST_EXECUTABLE_SUFFIX}")
set(CMAKE_STRIP         "${QNX_HOST}/usr/bin/ntoarmv7-strip${HOST_EXECUTABLE_SUFFIX}")

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(CMAKE_CXX_COMPILER_TARGET ${ARCH})

SET(CMAKE_FIND_ROOT_PATH "${QNX_TARGET}" "${QNX_TARGET}/armv7")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -march=armv7-a" CACHE STRING "")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
