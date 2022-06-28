cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(VulkanMemoryAllocator)

# Ensure The Vulkan Memory Allocator isn't added multiple times
if(TARGET VulkanMemoryAllocator)
	return()
endif() 

# This release of Vulkan Memory allocator doesn't have a CMakeLists.txt, so we cant just add it as
# a subdirectory. Instead we have to create an interface library. We use an interface library 
# because the library is a header only implementation
add_library(VulkanMemoryAllocator INTERFACE)

target_include_directories(VulkanMemoryAllocator INTERFACE ${CMAKE_CURRENT_LIST_DIR}/source/src)