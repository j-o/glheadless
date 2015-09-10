
# Generate meta-header
configure_file(glheadless-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/glheadless-version.h)

# We do NOT want to deploy this header (at least not for binary packages)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/glheadless/glheadless-version.h DESTINATION ${INSTALL_INCLUDE}/glheadless)

# Include directories
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Libraries
set(IDE_FOLDER "")
add_subdirectory(glheadless)

# Applications
set(IDE_FOLDER "Examples")
add_subdirectory(examples)

# Tests
set(IDE_FOLDER "Tests")
add_subdirectory(tests)
