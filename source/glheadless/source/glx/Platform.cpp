#include "Platform.h"

#include <atomic>
#include <mutex>

#include <glheadless/ExceptionTrigger.h>
#include <glheadless/error.h>

#include "../InternalException.h"


namespace glheadless {
namespace glx {


namespace {


std::atomic<Platform*> g_platformInstance;
std::mutex g_platformInstanceMutex;


}  // unnamed namespace


Platform* Platform::instance() {
    // double-checked locking according to http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/
    auto tmp = g_platformInstance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (tmp == nullptr) {
        std::lock_guard<std::mutex> __attribute__((unused)) lock(g_platformInstanceMutex);
        tmp = g_platformInstance.load(std::memory_order_relaxed);
        if (tmp == nullptr) {
            tmp = new Platform();
            std::atomic_thread_fence(std::memory_order_release);
            g_platformInstance.store(tmp, std::memory_order_relaxed);
        }
    }
    return tmp;
}


Platform::Platform()
: glXCreateContextAttribsARB(nullptr) {
    // resolve function
    glXCreateContextAttribsARB = reinterpret_cast<PFNGLXCREATECONTEXTATTRIBSARBPROC>(
        glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB")));
    if (glXCreateContextAttribsARB == nullptr) {
        throw InternalException(Error::INVALID_CONFIGURATION, "glXGetProcAddress failed on glXCreateContextAttribs", ExceptionTrigger::CREATE);
    }

    // connect to x server
    m_display = XOpenDisplay(nullptr);
    if (m_display == nullptr) {
        throw InternalException(Error::INVALID_CONFIGURATION, "XOpenDisplay returned nullptr", ExceptionTrigger::CREATE);
    }
}


Platform::~Platform() {
    XCloseDisplay(m_display);
}


Display* Platform::display() const {
    return m_display;
}


}  // namespace glx
}  // namespace glheadless
