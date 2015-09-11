
set(target glheadless)
message(STATUS "Lib ${target}")


# External libraries

find_package(OpenGL REQUIRED)


# Includes

include_directories(
    # e.g., ${TRDP_INCLUDE_DIR}
)

include_directories(
    BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# Libraries

set(libs
    ${OPENGL_LIBRARIES}
)


# Compiler definitions

if (OPTION_BUILD_STATIC)
    add_definitions("-D${META_PROJECT_NAME_UPPER}_STATIC")
else()
    add_definitions("-DGLHEADLESS_EXPORTS")
endif()


# Sources

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(api_includes
    ${include_path}/glheadless_api.h
    ${include_path}/Context.h
    ${include_path}/error.h
    ${include_path}/PixelFormat.h
)

set(sources
    ${source_path}/Context.cpp
    ${source_path}/error.cpp
    ${source_path}/PixelFormat.cpp
)

if(WIN32)
    set(sources ${sources}
        ${source_path}/wgl/Implementation.h
        ${source_path}/wgl/Implementation.cpp
        ${source_path}/wgl/Platform.h
        ${source_path}/wgl/Platform.cpp
        ${source_path}/wgl/Window.h
        ${source_path}/wgl/Window.cpp
    )
elseif(APPLE)
    set(sources ${sources}
        ${source_path}/cgl/error.h
        ${source_path}/cgl/error.cpp
        ${source_path}/cgl/Implementation.h
        ${source_path}/cgl/Implementation.cpp
    )
endif()

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$" 
    ${header_group} ${api_includes})
source_group_by_path(${source_path} "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$" 
    ${source_group} ${sources})


# Build library

add_library(${target} ${api_includes} ${sources})

target_link_libraries(${target} ${libs})

target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_FLAGS})

set_target_properties(${target}
    PROPERTIES
    LINKER_LANGUAGE              CXX
    FOLDER                      "${IDE_FOLDER}"
    COMPILE_DEFINITIONS_DEBUG   "${DEFAULT_COMPILE_DEFS_DEBUG}"
    COMPILE_DEFINITIONS_RELEASE "${DEFAULT_COMPILE_DEFS_RELEASE}"
    LINK_FLAGS_DEBUG            "${DEFAULT_LINKER_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE          "${DEFAULT_LINKER_FLAGS_RELEASE}"
    DEBUG_POSTFIX               "d${DEBUG_POSTFIX}"
    INCLUDE_PATH	            ${include_path})


# Deployment

# Library
install(TARGETS ${target}
    RUNTIME DESTINATION ${INSTALL_BIN}
    LIBRARY DESTINATION ${INSTALL_SHARED}
    ARCHIVE DESTINATION ${INSTALL_LIB}
)

# Header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE})
