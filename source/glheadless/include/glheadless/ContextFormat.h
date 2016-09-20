#pragma once


namespace glheadless {


/*!
 * \brief Describes the requested OpenGL profile (core or compatibility), if supported on the current platform.
 */
enum class Profile : unsigned int {
    NONE,
    CORE,
    COMPATIBILITY
};


struct ContextFormat {
    unsigned int versionMajor = 0;
    unsigned int versionMinor = 0;
    Profile profile = Profile::NONE;
    bool debug = false;
};


} // namespace glheadless
