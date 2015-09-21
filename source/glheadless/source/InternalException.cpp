#include "InternalException.h"


namespace glheadless {


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
