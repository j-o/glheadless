#include <glheadless/Context.h>

#include <cassert>

#include "AbstractImplementation.h"


namespace glheadless {


Context::Context(State* state)
: m_state(state)
, m_exceptionTriggers(ExceptionTrigger::NONE) {
    assert(state);
}


Context::Context(Context&& other) {
    *this = std::move(other);
}


Context::~Context() {
    AbstractImplementation::instance()->destroy(this);
}


bool Context::makeCurrent() {
    return AbstractImplementation::instance()->makeCurrent(this);
}


bool Context::doneCurrent() {
    return AbstractImplementation::instance()->doneCurrent(this);
}


bool Context::valid() const {
    return AbstractImplementation::instance()->valid(this);
}


bool Context::setError(Error code, const std::string& message, ExceptionTrigger exceptionTrigger) {
    return setError(make_error_code(code), message, exceptionTrigger);
}


bool Context::setError(const std::error_code& code, const std::string& message, ExceptionTrigger exceptionTrigger) {
    m_lastErrorCode = code;
    m_lastErrorMessage = message;

    if ((m_exceptionTriggers & exceptionTrigger) != ExceptionTrigger::NONE) {
        throw std::system_error(m_lastErrorCode, m_lastErrorMessage);
    }

    return !m_lastErrorCode;
}


const std::error_code& Context::lastErrorCode() const {
    return m_lastErrorCode;
}


const std::string& Context::lastErrorMessage() const {
    return m_lastErrorMessage;
}


ExceptionTrigger Context::exceptionTriggers() const {
    return m_exceptionTriggers;
}


void Context::setExceptionTriggers(ExceptionTrigger exceptions) {
    m_exceptionTriggers = exceptions;
}


State* Context::state() {
    return m_state;
}


const State* Context::state() const {
    return m_state;
}


Context& Context::operator=(Context&& other) {
    m_state = other.m_state;
    m_state = nullptr;

    m_exceptionTriggers = other.m_exceptionTriggers;

    m_lastErrorCode = std::move(other.m_lastErrorCode);
    m_lastErrorMessage = std::move(other.m_lastErrorMessage);

    return *this;
}


}  // namespace glheadless
