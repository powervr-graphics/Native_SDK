cmake_minimum_required(VERSION 3.3)

include(ExternalProject)

project(external_VulkanMemoryAllocator-download NONE)

# Setup the ExternalProject_Add call for VulkanMemoryAllocator
ExternalProject_Add(external_VulkanMemoryAllocator
  PREFIX				${VulkanMemoryAllocator_PREFIX}
  SOURCE_DIR 			${VulkanMemoryAllocator_SRC_DIR}
  UPDATE_COMMAND 		""
  URL					${VulkanMemoryAllocator_URL}
  DOWNLOAD_DIR			${VulkanMemoryAllocator_DOWNLOAD_DIR}
  CONFIGURE_COMMAND 	""
  BUILD_COMMAND 		""
  INSTALL_COMMAND 		""
  TEST_COMMAND 			""
)