#include "InternalException.h"


namespace glheadless {


InternalException::InternalException(Error code, const std::string &message)
: InternalException(make_error_code(code), message) {
}


InternalException::InternalException(const std::error_code& code, const std::string& message)
: std::system_error(code, message)
, m_message(message) {
}


const std::string& InternalException::message() const {
    return m_message;
}


}  // namespace glheadless
