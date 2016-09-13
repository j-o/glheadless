#include "error.h"

#include <string>
#include <map>


namespace {


std::map<GLXError, std::string> g_errorStrings = {
    { GLXError::SUCCESS, "everything's okay" },
    { GLXError::BAD_REQUEST, "bad request code" }
};


}  // unnamed namespace


GLXErrorCategory::GLXErrorCategory()
: std::error_category() {
}


const char* GLXErrorCategory::name() const noexcept {
    return "GLX";
}


std::string GLXErrorCategory::message(int condition) const {
    const auto itr = g_errorStrings.find(static_cast<GLXError>(condition));
    if (itr != g_errorStrings.end()) {
        return itr->second;
    }
    return "unknown error";
}


const std::error_category& glx_category() {
    static const GLXErrorCategory s_category;
    return s_category;
}


std::error_condition make_error_condition(GLXError error) {
    return std::error_condition(static_cast<int>(error), glx_category());
}


std::error_code make_error_code(GLXError error) {
    return std::error_code(static_cast<int>(error), glx_category());
}
