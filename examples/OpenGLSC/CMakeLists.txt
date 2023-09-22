cmake_minimum_required(VERSION 3.18)
project(OpenGLSCExamples)

set(WIN_EXAMPLES
	OpenGLSCHelloAPI
	)

if(WIN32)
	message ("==>CMake generation for OpenGLSC/BinaryShaderCache...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/BinaryShaderCache")
	
	foreach(EXAMPLE ${WIN_EXAMPLES})
		message ("==>CMake generation for OpenGLSC/${EXAMPLE}...")
		add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
	endforeach()
endif()

add_definitions(-DSC_ENABLED=0)
if(WIN32 OR UNIX)
	message ("==>CMake generation for OpenGLSC/OpenGLSCSurroundView...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/OpenGLSCSurroundView")
endif()
