
# Adapted from https://code.google.com/p/ios-cmake/

# Standard settings
set (CMAKE_SYSTEM_NAME Darwin)
set (UNIX True)
set (APPLE True)
set (IOS True)

# Required as of cmake 2.8.10
set (CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "Force unset of the deployment target for iOS" FORCE)

# Figure out where Xcode is installed
execute_process (COMMAND "xcode-select" "--print-path" OUTPUT_VARIABLE CMAKE_XCODE_ROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
set (CMAKE_XCODE_ROOT ${CMAKE_XCODE_ROOT} CACHE PATH "Location of Xcode. Normally auto-detected via 'xcode-select', but can be overridden")

# Skip the platform compiler checks for cross compiling
set (CMAKE_CXX_COMPILER_WORKS TRUE)
set (CMAKE_C_COMPILER_WORKS TRUE)

# Setup iOS platform unless specified manually with DEVICE_OR_SIMULATOR
if (NOT DEFINED DEVICE_OR_SIMULATOR)
    set (DEVICE_OR_SIMULATOR "OS")
endif ()
set (DEVICE_OR_SIMULATOR ${DEVICE_OR_SIMULATOR} CACHE STRING "Set to 'Simulator' (for simulators), or 'OS' (for devices)")

# Check the platform selection and setup for developer root
if (NOT((${DEVICE_OR_SIMULATOR} STREQUAL "OS") OR (${DEVICE_OR_SIMULATOR} STREQUAL "Simulator")))
    message (FATAL_ERROR "Unsupported DEVICE_OR_SIMULATOR value selected. Please choose 'OS' or 'Simulator'")
endif ()

set (CMAKE_IOS_DEVELOPER_ROOT "${CMAKE_XCODE_ROOT}/Platforms/iPhone${DEVICE_OR_SIMULATOR}.platform/Developer")

# Find and use the most recent iOS sdk unless specified manually with CMAKE_IOS_SDK_ROOT
if (NOT DEFINED CMAKE_IOS_SDK_ROOT)
    file (GLOB _CMAKE_IOS_SDKS "${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/*")
    if (_CMAKE_IOS_SDKS)
        list (SORT _CMAKE_IOS_SDKS)
        list (REVERSE _CMAKE_IOS_SDKS)
        list (GET _CMAKE_IOS_SDKS 0 CMAKE_IOS_SDK_ROOT)
    else ()
        message (FATAL_ERROR "No iOS SDK's found in default search path ${CMAKE_IOS_DEVELOPER_ROOT}. Manually set CMAKE_IOS_SDK_ROOT or install the iOS SDK.")
    endif ()
    message (STATUS "Toolchain using default iOS SDK: ${CMAKE_IOS_SDK_ROOT}")
endif ()
set (CMAKE_IOS_SDK_ROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Location of the selected iOS SDK")

set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")

# Set the sysroot default to the most recent SDK
# If we're generating Xcode projects, the keyword "iphoneos" is all that's needed; otherwise
# we need to specify the complete path
if (${CMAKE_GENERATOR} STREQUAL "Xcode")
    set (CMAKE_OSX_SYSROOT "iphoneos")
else ()
    # set (CMAKE_OSX_SYSROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Sysroot used for iOS support")
    # Actually, just fail - we haven't yet done the work to support any generator except Xcode for iOS
    message (FATAL_ERROR "Only the Xcode generator is supported at this time. Be sure to specify '-GXcode' on the cmd line.")
endif ()

# Set the find root to the iOS developer roots and to user defined paths
set (CMAKE_FIND_ROOT_PATH "${CMAKE_XCODE_ROOT}/Toolchains/XcodeDefault.xctoolchain/usr/bin" ${CMAKE_IOS_DEVELOPER_ROOT} ${CMAKE_IOS_SDK_ROOT} ${CMAKE_PREFIX_PATH} CACHE string  "iOS find search path root")

# default to searching for frameworks first
set (CMAKE_FIND_FRAMEWORK FIRST)

# set up the default search directories for frameworks
set (CMAKE_SYSTEM_FRAMEWORK_PATH
    ${CMAKE_IOS_SDK_ROOT}/System/Library/Frameworks
    ${CMAKE_IOS_SDK_ROOT}/System/Library/PrivateFrameworks
    ${CMAKE_IOS_SDK_ROOT}/Developer/Library/Frameworks
)
