# - Find X11_XCB
#
# Copyright (C) 2015 Valve Corporation

find_package(PkgConfig)

#if(NOT X11_XCB_FIND_COMPONENTS)
    set(X11_XCB_FIND_COMPONENTS X11-xcb)
#endif()

include(FindPackageHandleStandardArgs)
set(X11_XCB_FOUND true)
set(X11_XCB_INCLUDE_DIRS "")
set(X11_XCB_LIBRARIES "")

foreach(comp ${X11_XCB_FIND_COMPONENTS})
    # component name
    string(TOUPPER ${comp} compname)
    string(REPLACE "-" "_" compname ${compname})
    # header name
    string(REPLACE "X11-xcb" "" headername X11-xcb/${comp}.h)
    # library name
    set(libname ${comp})

    pkg_check_modules(PC_${comp} QUIET ${comp})

#    find_path(${compname}_INCLUDE_DIR NAMES ${headername}
#        HINTS
#        ${PC_${comp}_INCLUDEDIR}
#        ${PC_${comp}_INCLUDE_DIRS}
#        )
    find_library(${compname}_LIBRARY NAMES ${libname}
        HINTS
        ${PC_${comp}_LIBDIR}
        ${PC_${comp}_LIBRARY_DIRS}
        )

    find_package_handle_standard_args(${comp}
        FOUND_VAR ${comp}_FOUND
        REQUIRED_VARS
       # ${compname}_INCLUDE_DIR
        ${compname}_LIBRARY)
    mark_as_advanced(
       # ${compname}_INCLUDE_DIR
        ${compname}_LIBRARY)

    #list(APPEND X11_XCB_INCLUDE_DIRS ${${compname}_INCLUDE_DIR})
    list(APPEND X11_XCB_LIBRARIES ${${compname}_LIBRARY})



    if(NOT ${comp}_FOUND)
        set(X11_XCB_FOUND false)
    endif()
endforeach()

if(X11_XCB_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES X11_XCB_INCLUDE_DIRS)
endif()
