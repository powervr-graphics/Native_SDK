cmake_minimum_required(VERSION 3.3)
project(OpenGLESNavigation2D)
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESNavigation2D)

set(SRC_FILES OpenGLESNavigation2D.cpp ../../common/NavDataProcess.cpp ../../common/NavDataProcess2D.cpp ../../common/NavDataProcess.h)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESNavigation2D)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set(ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/ATM.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/bank.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/bar.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/butcher.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/cafe.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/casino.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/cinema.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/college.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/courthouse.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/dentist.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/doctors.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/embassy.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/fastfood.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/fire.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/florist.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/font.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/hairdresser.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/hospital.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/library.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/optician.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/other.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/parking.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/petrol.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/pharmacy.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/police.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/postbox.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/postoffice.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/pub.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/restaurant.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/Road.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/shop.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/theatre.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/toilets.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/university.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/vets.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/worship.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/map.osm
	)
list(APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# TEXTUAL SHADER FILES (OpenGL ES) - Packaged with the executable for runtime use
set(SHADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/AA_VertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/AA_FragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader.fsh
	)
list(APPEND SRC_FILES ${SHADER_FILES})
source_group("shaders" FILES ${SHADER_FILES})

set(RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

# Adds a platform specific executable
add_platform_specific_executable(OpenGLESNavigation2D "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(OpenGLESNavigation2D
	PVRShell
	PVRUtilsGles
)