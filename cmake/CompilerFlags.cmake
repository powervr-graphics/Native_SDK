# This file sets up optimal compiler/linker flags for the PowerVR SDK

option(USE_SANITIZER_FLAGS "Use sanitzer flags" OFF)

#some optimizations for windows
if (WIN32)
	#Get rid of the "this function is unsafe" warning in Visual Studio
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	
	#Enable Link Time Code Generation/Whole program optimization
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG" CACHE INTERNAL "")
	set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG" CACHE INTERNAL "")
	set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /LTCG" CACHE INTERNAL "")
	set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG" CACHE INTERNAL "")
	set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} /LTCG" CACHE INTERNAL "")
	set(CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL "${CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL} /LTCG" CACHE INTERNAL "")
	add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:/GL>")
	add_definitions(/MP)
	
	# Enable "Just My Code" feature introduced in MSVC 15.8 (Visual Studio 2017)
	# See https://blogs.msdn.microsoft.com/vcblog/2018/06/29/announcing-jmc-stepping-in-visual-studio/
	#
	# For MSVC version numbering used here, see:
	# https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15" )
		add_compile_options("$<$<CONFIG:DEBUG>:/JMC>")
	endif()
else()
	if (APPLE)
		if (IOS) #Kill some annoying warnings
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type" CACHE INTERNAL "")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-return-type" CACHE INTERNAL "")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wno-return-type" CACHE INTERNAL "")
		elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" CACHE INTERNAL "")
			set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" CACHE INTERNAL "")
			set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11" CACHE INTERNAL "")
			set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" CACHE INTERNAL "")
		endif()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -all_load" CACHE INTERNAL "") # PREVENT THE LINKER FROM STRIPPING OUT THE FUNCTIONS THAT macOS CALLS REFLECTIVELY
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "QNX")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes" CACHE INTERNAL "")
	endif()

	#Make sure we are not getting the "reorder constructor parameters" warning
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-reorder" CACHE INTERNAL "")
	include(CheckCXXCompilerFlag)
	# workaround an issue observed when using the optimation flag "-ftree-slp-vectorize" which is enabled when using the optimisation level "-O3" with gcc versions < 4.9.
	if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
		CHECK_CXX_COMPILER_FLAG(-fno-tree-slp-vectorize COMPILER_SUPPORTS_TREE_SLP_VECTORIZE)
		if(COMPILER_SUPPORTS_TREE_SLP_VECTORIZE)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-tree-slp-vectorize" CACHE INTERNAL "")
		endif()
	endif()
	# Add the all-important fast-math flag
	CHECK_CXX_COMPILER_FLAG(-ffast-math COMPILER_SUPPORTS_FAST_MATH)
	if(COMPILER_SUPPORTS_FAST_MATH)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math" CACHE INTERNAL "")
	endif()
	
	# Use Gold Linker by default when it is supported
	if (UNIX AND NOT APPLE)
		execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
		if ("${ld_version}" MATCHES "GNU gold")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags" CACHE INTERNAL "")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags" CACHE INTERNAL "")
		endif()
	endif()
	
	# Sanitizer Flags - set the option using "-DUSE_SANITIZER_FLAGS=ON" when calling cmake
	if (USE_SANITIZER_FLAGS)
		if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address" CACHE INTERNAL "")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer" CACHE INTERNAL "")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address" CACHE INTERNAL "")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address" CACHE INTERNAL "")
		elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak" CACHE INTERNAL "")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined" CACHE INTERNAL "")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=integer" CACHE INTERNAL "")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address" CACHE INTERNAL "")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer" CACHE INTERNAL "")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address" CACHE INTERNAL "")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address" CACHE INTERNAL "")
		elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak" CACHE INTERNAL "")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined" CACHE INTERNAL "")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer" CACHE INTERNAL "")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize-recover=all" CACHE INTERNAL "")
			if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)	
				set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address" CACHE INTERNAL "")
				set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address" CACHE INTERNAL "")
				set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address" CACHE INTERNAL "")
			endif()
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined" CACHE INTERNAL "")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=undefined" CACHE INTERNAL "")
			
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize-recover=all" CACHE INTERNAL "")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize-recover=all" CACHE INTERNAL "")
		endif()
	endif()
endif()