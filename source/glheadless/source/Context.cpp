#include <glheadless/Context.h>

#include <cassert>

#include "AbstractImplementation.h"


namespace glheadless {


Context::Context(AbstractImplementation* implementation)
: m_implementation(implementation)
, m_owningThread(std::this_thread::get_id()) {
    assert(implementation);
}


Context::~Context() {
    assert(m_owningThread == std::this_thread::get_id() && "a context must be destroyed on the same thread that created it");
    m_implementation->destroy();
}


bool Context::makeCurrent() {
    return m_implementation->makeCurrent();
}


bool Context::doneCurrent() {
    return m_implementation->doneCurrent();
}


bool Context::valid() const {
    return m_implementation->valid();
}


unsigned long long Context::nativeHandle() const {
    return m_implementation->nativeHandle();
}


void (*Context::getProcAddress(const char * name) const)() {
    return m_implementation->getProcAddress(name);
}


bool Context::setError(Error code, const std::string& message) {
    return setError(make_error_code(code), message);
}


bool Context::setError(const std::error_code& code, const std::string& message) {
    m_lastErrorCode = code;
    m_lastErrorMessage = message;

    return !m_lastErrorCode;
}


const std::error_code& Context::lastErrorCode() const {
    return m_lastErrorCode;
}


const std::string& Context::lastErrorMessage() const {
    return m_lastErrorMessage;
}


AbstractImplementation* Context::implementation() {
    return m_implementation.get();
}


const AbstractImplementation* Context::implementation() const {
    return m_implementation.get();
}


}  // namespace glheadless
