cmake_minimum_required(VERSION 3.22.1)
project(OpenCLExamples)
set(CMAKE_UNITY_BUILD ON)

set(OPENCL_EXAMPLES 
	MatrixMultiplication
	PrefixSum
)

foreach(EXAMPLE ${OPENCL_EXAMPLES})
	message ("==>CMake generation for OpenCL/${EXAMPLE}...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
endforeach()