
#
# External dependencies
#

find_package(GLFW QUIET)

if (NOT GLFW_FOUND)
    message("Example ${target} skipped: GLFW not found")
    return()
endif()


# 
# Executable name and options
# 

# Target name
set(target glfw-test)
# Exit here if required dependencies are not met
if (NOT GLFW_FOUND)
    message("Test ${target} skipped: GLFW not found")
    return()
endif()
message(STATUS "Test ${target}")


# 
# Sources
# 

set(sources
    main.cpp
    glfw_test.cpp
)


# 
# Create executable
# 

# Build executable
add_executable(${target}
    ${sources}
)

# Create namespaced alias
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})


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
    ${DEFAULT_INCLUDE_DIRECTORIES}
    ${GLFW_INCLUDE_DIR}
    ${PROJECT_BINARY_DIR}/source/include
    ${PROJECT_SOURCE_DIR}/source/glheadless/source
)


# 
# Libraries
# 

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${META_PROJECT_NAME}::glheadless
    gmock-dev
)


# 
# Compile definitions
# 

target_compile_definitions(${target}
    PRIVATE
    ${DEFAULT_COMPILE_DEFINITIONS}
)


# 
# Compile options
# 

target_compile_options(${target}
    PRIVATE
    ${DEFAULT_COMPILE_OPTIONS}
)


# 
# Linker options
# 

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LINKER_OPTIONS}
)
