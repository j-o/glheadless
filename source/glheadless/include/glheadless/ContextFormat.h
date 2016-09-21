#pragma once

/*!
* \file ContextFormat.h
* \brief Declares enum ContextProfile and struct ContextFormat.
*/


namespace glheadless {


/*!
 * \brief Describes the requested OpenGL profile (core or compatibility), if supported on the current platform.
 */
enum class ContextProfile : unsigned int {
    NONE,         //!< no profile (for OpenGL < 3.2) or default profile (for OpenGL >= 3.4)
    CORE,         //!< core profile (for OpenGL >= 3.2)
    COMPATIBILITY //!< compatibility profile (for OpenGL >= 3.2)
};


/*!
 * \brief Describes the requested context format.
 *
 * Use the default settings to obtain a default context for the current platform.
 */
struct ContextFormat {
    unsigned int   versionMajor = 0;                    //!< major version, a value of 0 indicates no preference
    unsigned int   versionMinor = 0;                    //!< minor version, a value of 0 indicates no preference
    ContextProfile profile      = ContextProfile::NONE; //!< OpenGL API profile for OpenGL >= 3.2
    bool           debug        = false;                //!< create debug context, if supported
};


} // namespace glheadless
