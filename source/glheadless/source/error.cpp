#include <glheadless/error.h>

#include <array>
#include <map>


namespace glheadless {


namespace {


const auto k_errorMessages = std::map<Error, std::string>{
    { Error::INVALID_THREAD_ACCESS, "invalid thread access" },
    { Error::INVALID_CONFIGURATION, "invalid configuration" },
    { Error::INVALID_CONTEXT, "invalid context" }
};


}  // anonymous namespace


ErrorCategory::ErrorCategory()
: std::error_category() {
}


const char* ErrorCategory::name() const noexcept {
    return "glheadless";
}


std::string ErrorCategory::message(int condition) const {
    const auto itr = k_errorMessages.find(static_cast<Error>(condition));
    if (itr != k_errorMessages.end()) {
        return itr->second;
    }

    return "unknown error";
}


const std::error_category& glheadless_category() {
    static const ErrorCategory s_category;
    return s_category;
}


std::error_condition make_error_condition(Error error) {
    return std::error_condition(static_cast<int>(error), glheadless_category());
}


std::error_code make_error_code(Error error) {
    return std::error_code(static_cast<int>(error), glheadless_category());
}


}  // namespace glheadless

