#include <glheadless/error.h>

#include <array>
#include <string>


namespace glheadless {


namespace {


const auto NUM_ERRORS = 4;


const auto k_errorMessages = std::array<std::string, NUM_ERRORS + 1>{{
    /* SUCCESS */               "Success",
    /* CONTEXT_NOT_CURRENT */   "Context not current",
    /* UNSUPPORTED_ATTRIBTUE */ "Unsupported attribute",
    /* UNKNOWN_ERROR */         "Unknown error"
}};


}  // anonymous namespace


ErrorCategory::ErrorCategory()
: std::error_category() {
}


const char* ErrorCategory::name() const noexcept {
    return "glheadless";
}


std::string ErrorCategory::message(int condition) const {
    if (condition >= 0 && condition < NUM_ERRORS) {
        return k_errorMessages[condition];
    }

    return "Unknown error";
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


void nullErrorCallback(const std::error_code& code, const std::string& message) {
    // do nothing
}


void throwingErrorCallback(const std::error_code& code, const std::string& message) {
    throw std::system_error(code, message);
}


}  // namespace glheadless

