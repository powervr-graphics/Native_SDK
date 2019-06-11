cmake_minimum_required(VERSION 3.3)

include(ExternalProject)

# Setup the ExternalProject_Add call for glslangValidator
ExternalProject_Add(external_glslangValidator
  PREFIX				${glslangValidator_PREFIX}
  SOURCE_DIR 			${glslang_SRC_DIR}
  UPDATE_COMMAND 		""
  URL					""
  DOWNLOAD_DIR			""
  TEST_COMMAND 			""
  BUILD_BYPRODUCTS 		
	${glslangValidator_RELEASE_EXECUTABLE}
  CONFIGURE_COMMAND
	"${CMAKE_COMMAND}"
    "-H${glslang_SRC_DIR}"
	"-B${glslangValidator_PREFIX}"
	"-DCMAKE_INSTALL_PREFIX=${EXTERNAL_RELEASE_BIN_FOLDER}"
	"-DCMAKE_BUILD_TYPE=Release"
	"-DBUILD_TESTING=OFF"
	"-DENABLE_HLSL=OFF"
	"-DENABLE_OPT=OFF"
	"-DENABLE_AMD_EXTENSIONS=OFF"
	"-DENABLE_NV_EXTENSIONS=OFF"
	"-DENABLE_GLSLANG_BINARIES=ON"
  BUILD_COMMAND
	"${CMAKE_COMMAND}" --build ${glslangValidator_PREFIX} --config Release
  INSTALL_COMMAND
	"${CMAKE_COMMAND}" --build ${glslangValidator_PREFIX} --target install --config Release
)