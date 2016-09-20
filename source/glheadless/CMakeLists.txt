
# 
# External dependencies
# 

find_package(OpenGL REQUIRED)

if(UNIX AND NOT APPLE)
    find_package(EGL)
endif()


# 
# Library name and options
# 

# Target name
set(target glheadless)

if(UNIX AND NOT APPLE)
    if(EGL_FOUND)
        message(STATUS "Defaulting to EGL implementation. Unset OPTION_LINUX_USE_EGL to use GLX.")
        option(OPTION_LINUX_USE_EGL "Use EGL instead of GLX implementation on Linux" ON)
    else()
        message(STATUS "EGL not available, using GLX")
    endif()
endif()

# Exit here if required dependencies are not met
message(STATUS "Lib ${target}")

# Set API export file and macro
string(TOUPPER ${target} target_upper)
set(feature_file "include/${target}/${target}_features.h")
set(export_file  "include/${target}/${target}_api.h")
set(export_macro "${target_upper}_API")


# 
# Sources
# 

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(headers
    ${include_path}/Context.h
    ${include_path}/ContextFactory.h
    ${include_path}/ContextFormat.h
    ${include_path}/error.h
    ${include_path}/ExceptionTrigger.h
    ${include_path}/ExceptionTrigger.hpp
)

set(sources
    ${source_path}/AbstractImplementation.h
    ${source_path}/AbstractImplementation.cpp
    ${source_path}/Context.cpp
    ${source_path}/ContextFactory.cpp
    ${source_path}/error.cpp
    ${source_path}/InternalException.h
    ${source_path}/InternalException.cpp
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
elseif(UNIX)
    if (EGL_FOUND AND OPTION_LINUX_USE_EGL)
        set(sources ${sources}
            ${source_path}/egl/Implementation.h
            ${source_path}/egl/Implementation.cpp
            ${source_path}/egl/Platform.h
            ${source_path}/egl/Platform.cpp
        )
    else()
        set(sources ${sources}
            ${source_path}/glx/Implementation.h
            ${source_path}/glx/Implementation.cpp
            ${source_path}/glx/Platform.h
            ${source_path}/glx/Platform.cpp
        )
    endif()
endif()

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$" 
    ${header_group} ${headers})
source_group_by_path(${source_path}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$" 
    ${source_group} ${sources})


# 
# Create library
# 

# Build library
add_library(${target}
    ${sources}
    ${headers}
)

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Export library for downstream projects
export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

# Create feature detection header
# Compilers: https://cmake.org/cmake/help/v3.1/variable/CMAKE_LANG_COMPILER_ID.html#variable:CMAKE_%3CLANG%3E_COMPILER_ID
# Feature: https://cmake.org/cmake/help/v3.1/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html
write_compiler_detection_header(
    FILE ${feature_file}
    PREFIX ${target_upper}
    COMPILERS AppleClang Clang GNU MSVC
    FEATURES cxx_alignas cxx_alignof cxx_constexpr cxx_final cxx_noexcept cxx_nullptr cxx_sizeof_member cxx_thread_local
)

# Create API export header
generate_export_header(${target}
    EXPORT_FILE_NAME  ${export_file}
    EXPORT_MACRO_NAME ${export_macro}
)


# 
# Project options
# 

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
)


# 
# Include directories
# 

target_include_directories(${target}
    PRIVATE
    ${PROJECT_BINARY_DIR}/source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include

    PUBLIC
    ${DEFAULT_INCLUDE_DIRECTORIES}

    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


# 
# Libraries
# 

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${EGL_LIBRARIES}

    INTERFACE
)


# 
# Compile definitions
# 

target_compile_definitions(${target}
    PRIVATE

    PUBLIC
    $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_upper}_STATIC_DEFINE>
    ${DEFAULT_COMPILE_DEFINITIONS}
    $<$<BOOL:${OPTION_LINUX_USE_EGL}>:${target_upper}_LINUX_USE_EGL>

    INTERFACE
)


# 
# Compile options
# 

target_compile_options(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
)


# 
# Linker options
# 

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LINKER_OPTIONS}

    INTERFACE
)


# 
# Deployment
# 

# Library
install(TARGETS ${target}
    EXPORT  "${target}-export"            COMPONENT dev
    RUNTIME DESTINATION ${INSTALL_BIN}    COMPONENT runtime
    LIBRARY DESTINATION ${INSTALL_SHARED} COMPONENT runtime
    ARCHIVE DESTINATION ${INSTALL_LIB}    COMPONENT dev
)

# Header files
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
    COMPONENT dev
)

# Generated header files
install(DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
    COMPONENT dev
)

# CMake config
install(EXPORT ${target}-export
    NAMESPACE   ${META_PROJECT_NAME}::
    DESTINATION ${INSTALL_CMAKE}/${target}
    COMPONENT   dev
)
