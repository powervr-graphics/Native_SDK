# This file sets up optimal compiler/linker flags for the PowerVR SDK
# This file contains a set of convenient options to set for PVRVk and/or its targets.
include(CheckCXXCompilerFlag)

option(PVR_ENABLE_RECOMMENDED_WARNINGS "If enabled, pass /W4 to the compiler and disable some specific warnings." ON)
option(PVR_ENABLE_FAST_MATH "If enabled, attempt to enable fast-math." ON)

if(WIN32)
	if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		option(PVR_MSVC_ENABLE_LTCG "If enabled, use Link Time Code Generation for non-debug builds." ON)
		option(PVR_MSVC_ENABLE_STATIC_ANALYSIS "If enabled, do more complex static analysis and generate warnings appropriately." OFF)
		option(PVR_MSVC_ENABLE_JUST_MY_CODE "If enabled, enable 'Just My Code' feature." ON)
		option(PVR_MSVC_USE_STATIC_RUNTIME "If enabled, build against the static, rather than the dynamic, runtime." OFF)
	endif()
elseif(UNIX)
	option(PVR_ENABLE_ADDRESS_SANITIZER "If enabled, pass address-sanitzer flags to the compiler" OFF)
endif()

# Apply various compile/link time options to the specified target
function(apply_compile_options_to_target THETARGET)
	if(WIN32)
		if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
			if(PVR_ENABLE_RECOMMENDED_WARNINGS)
				target_compile_options(${THETARGET} 
					PRIVATE /W4 
					PUBLIC 
						/wd4127 # Conditional expression is constant.
						/wd4201 # nameless struct/union
						/wd4245 # assignment: signed/unsigned mismatch
						/wd4365 # assignment: signed/unsigned mismatch
						/wd4389 # equality/inequality: signed/unsigned mismatch
						/wd4018 # equality/inequality: signed/unsigned mismatch
						/wd4706 # Assignment within conditional
				)
			endif()
			
			target_compile_definitions(${THETARGET} 
				PUBLIC
					_STL_EXTRA_DISABLED_WARNINGS=4774\ 
					_CRT_SECURE_NO_WARNINGS)
		
			target_compile_options(${THETARGET} PRIVATE "/MP")
			target_link_libraries(${THETARGET} PUBLIC "$<$<CONFIG:DEBUG>:-incremental>")

			if(PVR_MSVC_ENABLE_LTCG)
				target_compile_options(${THETARGET} PUBLIC "$<$<NOT:$<CONFIG:DEBUG>>:/GL>")

				# Enable Link Time Code Generation/Whole program optimization
				target_link_libraries(${THETARGET} PUBLIC "$<$<NOT:$<CONFIG:DEBUG>>:-LTCG:INCREMENTAL>")
			endif()
			
			if(PVR_MSVC_ENABLE_STATIC_ANALYSIS)
				target_compile_options(${THETARGET} PRIVATE "/analyze")
			endif()

			if(PVR_MSVC_ENABLE_JUST_MY_CODE)
				# Enable "Just My Code" feature introduced in MSVC 15.8 (Visual Studio 2017)
				# See https://blogs.msdn.microsoft.com/vcblog/2018/06/29/announcing-jmc-stepping-in-visual-studio/
				#
				# For MSVC version numbering used here, see:
				# https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering
				if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15" )
					target_compile_options(${THETARGET} PRIVATE "$<$<CONFIG:DEBUG>:/JMC>")
				endif()
			endif()

			# Add the all-important fast-math flag
			if(PVR_ENABLE_FAST_MATH)
				CHECK_CXX_COMPILER_FLAG(/fp:fast COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>:/fp:fast>")
				endif()
			endif()
		endif()

		if(MINGW)
			set(_WIN32_WINNT 0x0600 CACHE INTERNAL "Setting _WIN32_WINNT to 0x0600 for Windows Vista APIs")
			set(WINVER 0x0600 CACHE INTERNAL "Setting WINVER to 0x0600 for Windows Vista APIs")
		
			target_compile_definitions(${THETARGET} PUBLIC _WIN32_WINNT=${_WIN32_WINNT})
			target_compile_definitions(${THETARGET} PUBLIC WINVER=${WINVER})
		endif()
		
		target_compile_definitions(${THETARGET}
			PUBLIC 
				WIN32_LEAN_AND_MEAN=1 
				VC_EXTRALEAN=1 
				NOMINMAX=1)
	elseif(UNIX)
		if(NOT APPLE)
			# Use Gold Linker by default when it is supported
			execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
			if("${ld_version}" MATCHES "GNU gold")
				target_link_libraries(${THETARGET} PUBLIC "-fuse-ld=gold -Wl,--disable-new-dtags")
			endif()
		endif()
		
		if(PVR_ENABLE_RECOMMENDED_WARNINGS)
			target_compile_options(${THETARGET} BEFORE PRIVATE -Wall)
			target_compile_options(${THETARGET} PUBLIC -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-sign-compare -Wno-reorder)
		endif()
	
		# Make sure we are not getting the "reorder constructor parameters" warning
		target_compile_options(${THETARGET} PRIVATE -Wno-reorder -Wno-strict-aliasing -Wno-unknown-pragmas -Wno-sign-compare)
		
		if(PVR_ENABLE_FAST_MATH)
			if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
				CHECK_CXX_COMPILER_FLAG(-ffast-math COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "$<$<CONFIG:RELEASE>:-ffast-math>")
				endif()
			endif()
		endif()
		
		# Sanitizer Flags - set the option using "-DUSE_SANITIZER_FLAGS=ON" when calling cmake
		if(PVR_ENABLE_ADDRESS_SANITIZER)
			if("${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
				target_compile_options(PowerVR_SDK PRIVATE "$<$<CONFIG:DEBUG>:-fsanitize=address -fno-omit-frame-pointer>")
				target_link_libraries(${THETARGET} PUBLIC "-fsanitize=address")
			elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
				target_compile_options(${THETARGET} PRIVATE "$<$<CONFIG:DEBUG>:-fsanitize=leak -fsanitize=undefined -fsanitize=integer -fsanitize=address -fno-omit-frame-pointer>")
				target_link_libraries(${THETARGET} PUBLIC "-fsanitize=address")
			elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
				target_compile_options(PowerVR_SDK PRIVATE "$<$<CONFIG:DEBUG>:-fsanitize=leak -fsanitize=undefined -fno-omit-frame-pointer -fsanitize-recover=all>")
				if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)	
					target_compile_options(PowerVR_SDK PRIVATE "$<$<CONFIG:DEBUG>:-fsanitize=address>")
					target_link_libraries(${THETARGET} PUBLIC "-fsanitize=address")
				endif()
				target_link_libraries(${THETARGET} PUBLIC "-fsanitize=undefined -fsanitize-recover=all")
			endif()
		endif()
		
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			target_compile_options(${THETARGET} PRIVATE -Wno-ignored-attributes)
		endif()
	endif()
	
	# Use c++14
	set_target_properties(${THETARGET} PROPERTIES CXX_STANDARD 14)
	
	# Enable Debug and Release flags as appropriate
	target_compile_definitions(${THETARGET} PRIVATE $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
endfunction()