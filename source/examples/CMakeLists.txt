
# Check if examples are enabled
if(NOT OPTION_BUILD_EXAMPLES)
    return()
endif()

# Example applications
add_subdirectory(basic-context)
#add_subdirectory(shared-context)
#add_subdirectory(multithreading)
#add_subdirectory(glfw-interop)
#add_subdirectory(error-handling)
