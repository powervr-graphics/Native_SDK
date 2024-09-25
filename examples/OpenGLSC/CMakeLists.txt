cmake_minimum_required(VERSION 3.3)
project(OpenGLSCExamples)

set(OPENGLES_AUTOMOTIVE_EXAMPLES
	OpenGLSCSurroundView
	)

if(WIN32)
	foreach(EXAMPLE ${OPENGLES_AUTOMOTIVE_EXAMPLES})
		message ("==>CMake generation for OpenGLSC/${EXAMPLE}...")
		add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
	endforeach()
endif()

add_definitions(-DSC_ENABLED=0)

if(UNIX)
	foreach(EXAMPLE ${OPENGLES_AUTOMOTIVE_EXAMPLES})
		message ("==>CMake generation for OpenGLSC/${EXAMPLE}...")
		add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
	endforeach()
endif()
