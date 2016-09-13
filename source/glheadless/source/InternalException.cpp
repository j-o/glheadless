#include "InternalException.h"


namespace glheadless {


InternalException::InternalException(Error code, const std::string &message, ExceptionTrigger trigger)
: InternalException(make_error_code(code), message, trigger) {
}


InternalException::InternalException(const std::error_code& code, const std::string& message, ExceptionTrigger trigger)
: std::system_error(code, message)
, m_message(message)
, m_trigger(trigger) {
}


const std::string& InternalException::message() const {
    return m_message;
}


ExceptionTrigger InternalException::trigger() const {
    return m_trigger;
}


}  // namespace glheadless
