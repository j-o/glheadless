
# EGL_FOUND
# EGL_INCLUDE_DIRS
# EGL_LIBRARIES

find_path(EGL_INCLUDE_DIRS
        NAMES EGL/egl.h
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        DOC "The directory where EGL/egl.h resides")

find_library(EGL_LIBRARIES
        NAMES EGL
        PATHS
        /usr/lib64
        /usr/local/lib64
        /sw/lib64
        /opt/loca/lib64
        /usr/lib
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        DOC "The EGL library")


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EGL REQUIRED_VARS EGL_LIBRARIES EGL_INCLUDE_DIRS)

mark_as_advanced(EGL_INCLUDE_DIR EGL_LIBRARY)

