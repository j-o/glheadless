#include "AbstractImplementation.h"

#include <set>
#include <vector>

#include <glheadless/Context.h>


namespace glheadless {


AbstractImplementation::AbstractImplementation(Context* context)
: m_context(context) {
}


bool AbstractImplementation::setError(const std::error_code& code, const std::string& message, ExceptionTrigger exceptionTrigger) {
    m_lastErrorCode = code;
    m_lastErrorMessage = message;

    if ((m_context->exceptionTriggers() & exceptionTrigger) != ExceptionTrigger::NONE) {
        throw std::system_error(m_lastErrorCode, m_lastErrorMessage);
    }

    return !m_lastErrorCode;
}


const std::error_code& AbstractImplementation::lastErrorCode() const {
    return m_lastErrorCode;
}


const std::string& AbstractImplementation::lastErrorMessage() const {
    return m_lastErrorMessage;
}


}  // namespace glheadless
