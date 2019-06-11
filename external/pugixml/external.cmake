cmake_minimum_required(VERSION 3.3)

include(ExternalProject)

project(external_pugixml-download NONE)

# Setup the ExternalProject_Add call for pugixml
ExternalProject_Add(external_pugixml
  PREFIX				${pugixml_PREFIX}
  SOURCE_DIR 			${pugixml_SRC_DIR}
  UPDATE_COMMAND 		""
  URL					${pugixml_URL}
  DOWNLOAD_DIR			${pugixml_DOWNLOAD_DIR}
  CONFIGURE_COMMAND 	""
  BUILD_COMMAND 		""
  INSTALL_COMMAND 		""
  TEST_COMMAND 			""
)