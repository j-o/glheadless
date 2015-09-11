#include "error.h"

#include <string>


CGLErrorCategory::CGLErrorCategory()
: std::error_category() {
}


const char* CGLErrorCategory::name() const noexcept {
    return "CGL";
}


std::string CGLErrorCategory::message(int condition) const {
    return CGLErrorString(static_cast<_CGLError>(condition));
}


const std::error_category& cgl_category() {
    static const CGLErrorCategory s_category;
    return s_category;
}


std::error_condition make_error_condition(_CGLError error) {
    return std::error_condition(static_cast<int>(error), cgl_category());
}


std::error_code make_error_code(_CGLError error) {
    return std::error_code(static_cast<int>(error), cgl_category());
}
