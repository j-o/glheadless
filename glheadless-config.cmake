# GLHEADLESS_DIR
# GLHEADLESS_FOUND
# GLHEADLESS_LIBRARIES
# GLHEADLESS_INCLUDE_DIRS

set(GLHEADLESS_INCLUDE_DIRS "")
set(GLHEADLESS_LIBRARIES "")

# Definition of function "find" with two mandatory arguments, "LIB_NAME" and "HEADER".
macro (find LIB_NAME HEADER)

    set(HINT_PATHS ${ARGN})

    if (${LIB_NAME} STREQUAL "glheadless")
        set(LIB_NAME_UPPER GLHEADLESS)
        set(LIBNAME glheadless)
    else()
        string(TOUPPER GLHEADLESS_${LIB_NAME} LIB_NAME_UPPER)
        set(LIBNAME ${LIB_NAME})
    endif()

    find_path(
        ${LIB_NAME_UPPER}_INCLUDE_DIR
        ${HEADER}
        ${ENV_GLHEADLESS_DIR}/include
        ${ENV_GLHEADLESS_DIR}/source/${LIB_NAME}/include
        ${GLHEADLESS_DIR}/include
        ${GLHEADLESS_DIR}/source/${LIB_NAME}/include
        ${ENV_PROGRAMFILES}/glheadless/include
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        DOC "The directory where ${HEADER} resides"
    )


    find_library(
        ${LIB_NAME_UPPER}_LIBRARY_RELEASE
        NAMES ${LIBNAME}
        PATHS ${HINT_PATHS}
        DOC "The ${LIB_NAME} library"
    )
    find_library(
        ${LIB_NAME_UPPER}_LIBRARY_DEBUG
        NAMES ${LIBNAME}d
        PATHS ${HINT_PATHS}
        DOC "The ${LIB_NAME} debug library"
    )
    

    if(${LIB_NAME_UPPER}_LIBRARY_RELEASE AND ${LIB_NAME_UPPER}_LIBRARY_DEBUG)
        set(${LIB_NAME_UPPER}_LIBRARY "optimized" ${${LIB_NAME_UPPER}_LIBRARY_RELEASE} "debug" ${${LIB_NAME_UPPER}_LIBRARY_DEBUG})
    elseif(${LIB_NAME_UPPER}_LIBRARY_RELEASE)
        set(${LIB_NAME_UPPER}_LIBRARY ${${LIB_NAME_UPPER}_LIBRARY_RELEASE})
    elseif(${LIB_NAME_UPPER}_LIBRARY_DEBUG)
        set(${LIB_NAME_UPPER}_LIBRARY ${${LIB_NAME_UPPER}_LIBRARY_DEBUG})
    endif()

    list(APPEND GLHEADLESS_INCLUDE_DIRS ${${LIB_NAME_UPPER}_INCLUDE_DIR})
    list(APPEND GLHEADLESS_LIBRARIES ${${LIB_NAME_UPPER}_LIBRARY})

    # DEBUG MESSAGES
    # message("${LIB_NAME_UPPER}_INCLUDE_DIR     = ${${LIB_NAME_UPPER}_INCLUDE_DIR}")
    # message("${LIB_NAME_UPPER}_LIBRARY_RELEASE = ${${LIB_NAME_UPPER}_LIBRARY_RELEASE}")
    # message("${LIB_NAME_UPPER}_LIBRARY_DEBUG   = ${${LIB_NAME_UPPER}_LIBRARY_DEBUG}")
    # message("${LIB_NAME_UPPER}_LIBRARY         = ${${LIB_NAME_UPPER}_LIBRARY}")

endmacro(find)


# load standard CMake arguments (c.f. http://stackoverflow.com/questions/7005782/cmake-include-findpackagehandlestandardargs-cmake)
include(FindPackageHandleStandardArgs)

if(CMAKE_CURRENT_LIST_FILE)
    get_filename_component(GLHEADLESS_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
endif()

file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" ENV_PROGRAMFILES)
file(TO_CMAKE_PATH "$ENV{GLHEADLESS_DIR}" ENV_GLHEADLESS_DIR)

set(LIB_PATHS
    ${GLHEADLESS_DIR}/build
    ${GLHEADLESS_DIR}/build/Release
    ${GLHEADLESS_DIR}/build/Debug
    ${GLHEADLESS_DIR}/build-release
    ${GLHEADLESS_DIR}/build-debug
    ${GLHEADLESS_DIR}/lib
    ${ENV_GLHEADLESS_DIR}/lib
    ${ENV_PROGRAMFILES}/glheadless/lib
    /usr/lib
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    /usr/lib64
    /usr/local/lib64
    /sw/lib64
    /opt/local/lib64
)

# Find libraries
find(glheadless glheadless/glheadless_api.h ${LIB_PATHS})

if(GLHEADLESS_LIBRARY)
  # add dependencies
endif()


# DEBUG
# message("GLHEADLESS_INCLUDE_DIRS  = ${GLHEADLESS_INCLUDE_DIRS}")
# message("GLHEADLESS_LIBRARIES = ${GLHEADLESS_LIBRARIES}")

find_package_handle_standard_args(GLHEADLESS DEFAULT_MSG GLHEADLESS_LIBRARIES GLHEADLESS_INCLUDE_DIRS)
mark_as_advanced(GLHEADLESS_FOUND)
