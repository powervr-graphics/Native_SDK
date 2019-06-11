cmake_minimum_required(VERSION 3.3)

include(ExternalProject)

project(external_glslang-download NONE)

# Setup the ExternalProject_Add call for glslang
ExternalProject_Add(external_glslang
  PREFIX				${glslang_PREFIX}
  SOURCE_DIR 			${glslang_SRC_DIR}
  UPDATE_COMMAND 		""
  URL					${glslang_URL}
  DOWNLOAD_DIR			${glslang_DOWNLOAD_DIR}
  CONFIGURE_COMMAND 	""
  BUILD_COMMAND 		""
  INSTALL_COMMAND 		""
  TEST_COMMAND 			""
)