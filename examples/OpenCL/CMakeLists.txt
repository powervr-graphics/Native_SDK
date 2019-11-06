cmake_minimum_required(VERSION 3.3)
project(OpenCLExamples)

set(EXAMPLES 
	MatrixMultiplication
)

foreach(EXAMPLE ${EXAMPLES})
	message ("==>CMake generation for OpenCL/${EXAMPLE}...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
endforeach()