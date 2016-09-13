#pragma once

#include <system_error>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <GL/glxproto.h>


enum class GLXError : int {
    SUCCESS = Success,
    BAD_REQUEST = BadRequest,
    BAD_DISPLAY = LastExtensionError + 1,
    BAD_CONTEXT = GLXBadContext,
    BAD_FB_CONFIG = GLXBadFBConfig
};


class GLXErrorCategory : public std::error_category {
public:
    GLXErrorCategory();

    virtual const char* name() const noexcept override;

    virtual std::string message(int condition) const override;
};


const std::error_category& glx_category();
std::error_condition make_error_condition(GLXError error);
std::error_code make_error_code(GLXError error);


namespace std {


template <>
struct is_error_condition_enum<GLXError> : public std::true_type {
};


template <>
struct is_error_code_enum<GLXError> : public std::true_type {
};


}  // namespace std
