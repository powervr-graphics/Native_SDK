cmake_minimum_required(VERSION 3.3)
project (framework)

add_subdirectory_if_not_already_included(PVRCore PVRCore ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCore)
add_subdirectory_if_not_already_included(PVRAssets PVRAssets ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRAssets)
add_subdirectory_if_not_already_included(PVRShell PVRShell ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRShell)
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/PVRCamera/CMakeLists.txt)
	add_subdirectory_if_not_already_included(PVRCamera PVRCamera ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCamera)
endif()

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/PVRUtils/OpenGLES/CMakeLists.txt)
	add_subdirectory_if_not_already_included(PVRUtilsGles PVRUtils/OpenGLES ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRUtilsGles)
endif()

if(NOT APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
	if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/PVRVk/CMakeLists.txt)
		add_subdirectory_if_not_already_included(PVRVk PVRVk ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRVk)
	endif()

	if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/PVRUtils/Vulkan/CMakeLists.txt)
		add_subdirectory_if_not_already_included(PVRUtilsVk PVRUtils/Vulkan ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRUtilsVk)
	endif()

	if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/PVRPfx/CMakeLists.txt)
		add_subdirectory_if_not_already_included(PVRPfx PVRPfx ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRPfx)
	endif()
endif()