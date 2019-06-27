cmake_minimum_required(VERSION 3.3)

include(ExternalProject)

project(external_glslangValidator-download NONE)

# Setup the ExternalProject_Add call for glslangValidator
ExternalProject_Add(external_glslangValidator
  PREFIX				${glslangValidator_PREFIX}
  SOURCE_DIR 			${EXTERNAL_RELEASE_BIN_FOLDER}
  UPDATE_COMMAND 		""
  URL					${glslangValidator_URL}
  DOWNLOAD_DIR			${glslang_DOWNLOAD_DIR}
  CONFIGURE_COMMAND 	""
  BUILD_COMMAND 		""
  INSTALL_COMMAND 		""
  TEST_COMMAND 			""
)