#include "AbstractImplementation.h"

#include <set>
#include <vector>

#include <glheadless/Context.h>


namespace glheadless {


AbstractImplementation::AbstractImplementation()
: m_errorCallback(nullErrorCallback) {
}


bool AbstractImplementation::setError(const std::error_code& code, const std::string& message) {
    m_lastErrorCode = code;
    m_lastErrorMessage = message;

    m_errorCallback(m_lastErrorCode, m_lastErrorMessage);

    return !m_lastErrorCode;
}


const std::error_code& AbstractImplementation::lastErrorCode() const {
    return m_lastErrorCode;
}


const std::string& AbstractImplementation::lastErrorMessage() const {
    return m_lastErrorMessage;
}


void AbstractImplementation::setErrorCallback(const ErrorCallback& callback) {
    m_errorCallback = callback;
}


}  // namespace glheadless
