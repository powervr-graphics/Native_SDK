cmake_minimum_required(VERSION 3.10)

# Apply various compile/link time options to the specified example target
function(apply_example_compile_options_to_target THETARGET)
	if(WIN32)
		if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
			if(PVR_ENABLE_EXAMPLE_RECOMMENDED_WARNINGS)
				target_compile_options(${THETARGET} PRIVATE
					/W4 
					/wd4127 # Conditional expression is constant.
					/wd4201 # nameless struct/union
					/wd4245 # assignment: signed/unsigned mismatch
					/wd4365 # assignment: signed/unsigned mismatch
					/wd4389 # equality/inequality: signed/unsigned mismatch
					/wd4018 # equality/inequality: signed/unsigned mismatch
					/wd4706 # Assignment within conditional
				)
			endif()
		
			target_compile_options(${THETARGET} PRIVATE "/MP")
			
			if(PVR_MSVC_ENABLE_EXAMPLE_FAST_LINK)
				set_target_properties(${THETARGET} PROPERTIES 
					LINK_OPTIONS "$<$<CONFIG:DEBUG>:/DEBUG:FASTLINK>")
			endif()

			if(PVR_MSVC_ENABLE_EXAMPLE_JUST_MY_CODE)
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
			if(PVR_ENABLE_EXAMPLE_FAST_MATH)
				CHECK_CXX_COMPILER_FLAG(/fp:fast COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "/fp:fast")
				endif()
			endif()
		endif()
	elseif(UNIX)	
		if(PVR_ENABLE_EXAMPLE_RECOMMENDED_WARNINGS)
			target_compile_options(${THETARGET} BEFORE PRIVATE -Wall)
		endif()
		
		if(PVR_ENABLE_EXAMPLE_FAST_MATH)
			if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
				CHECK_CXX_COMPILER_FLAG(-ffast-math COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "-ffast-math")
				endif()
			endif()
		endif()
	endif()
	
	# Use c++14
	set_target_properties(${THETARGET} PROPERTIES CXX_STANDARD 14)
	
	# Enable Debug and Release flags as appropriate
	target_compile_definitions(${THETARGET} PRIVATE $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
endfunction()

# Add various command line options that are common to the framework components
function(apply_framework_compile_options_to_target THETARGET)

	# The most important component to set is the C++ standard
	# As of right now the SDK uses C++14 Standard
	set(PVR_CXX_STANDARD 14)

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

			string(TOUPPER ${THETARGET} THETARGET_UPPERCASE)

			target_compile_definitions(${THETARGET} 
				PUBLIC
					_STL_EXTRA_DISABLED_WARNINGS=4774\ 
					_CRT_SECURE_NO_WARNINGS
					${THETARGET_UPPERCASE}_PRESENT=1
					)
				
			target_compile_options(${THETARGET} PRIVATE "/MP")
			target_link_libraries(${THETARGET} PUBLIC "$<$<CONFIG:DEBUG>:-incremental>")

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
		if(PVR_ENABLE_RECOMMENDED_WARNINGS)
			target_compile_options(${THETARGET} BEFORE PRIVATE -Wall -Wno-unused-function)
			target_compile_options(${THETARGET} PUBLIC -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-sign-compare -Wno-reorder)
		endif()
	
		# Make sure we are not getting the "reorder constructor parameters" warning
		target_compile_options(${THETARGET} PRIVATE -Wno-reorder -Wno-strict-aliasing -Wno-unknown-pragmas -Wno-sign-compare)

		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			target_compile_options(${THETARGET} PRIVATE -Wno-ignored-attributes)
		endif()
	endif()
	
	# Use c++14
	set_target_properties(${THETARGET} PROPERTIES CXX_STANDARD ${PVR_CXX_STANDARD})
	
	# Enable Debug and Release flags as appropriate
	target_compile_definitions(${THETARGET} PRIVATE $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
endfunction()