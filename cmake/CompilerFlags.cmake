
#some optimizations for windows
if (WIN32)
	#Get rid of the "this function is unsafe" warning in Visual Studio
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	
	#Enable Link Time Code Generation/Whole program optimization
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
	set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
	set (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /LTCG")
	set (CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
	set (CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
	set (CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL "${CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL} /LTCG")
	add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:/GL>)
else()
	if (APPLE)
		if (IOS) #Kill some annoying warnings
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type" CACHE STRING "")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-return-type" CACHE STRING "")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wno-return-type" CACHE STRING "")
		elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
			set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
			set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
			set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
		endif()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -all_load") # PREVENT THE LINKER FROM STRIPPING OUT THE FUNCTIONS THAT macOS CALLS REFLECTIVELY
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "QNX")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes")
	endif()

	#Make sure we are not getting the "reorder constructor parameters" warning
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-reorder")
	include(CheckCXXCompilerFlag)
	# workaround an issue observed when using the optimation flag "-ftree-slp-vectorize" which is enabled when using the optimisation level "-O3" with gcc versions < 4.9.
	if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
		CHECK_CXX_COMPILER_FLAG(-fno-tree-slp-vectorize COMPILER_SUPPORTS_TREE_SLP_VECTORIZE)
		if(COMPILER_SUPPORTS_TREE_SLP_VECTORIZE)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-tree-slp-vectorize")
		endif()
	endif()
	# Add the all-important fast-math flag
	CHECK_CXX_COMPILER_FLAG(-ffast-math COMPILER_SUPPORTS_FAST_MATH)
	if(COMPILER_SUPPORTS_FAST_MATH)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")
	endif()
endif()