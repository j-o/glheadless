#include "Platform.h"

#include <GL/glx.h>

#include <atomic>
#include <mutex>
#include <glheadless/ExceptionTrigger.h>

#include "../InternalException.h"


namespace glheadless {


namespace {


std::atomic<Platform*> g_platformInstance;
std::mutex g_platformInstanceMutex;


}  // unnamed namespace


Platform* Platform::instance() {
    // double-checked locking according to http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/
    auto tmp = g_platformInstance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (tmp == nullptr) {
        std::lock_guard<std::mutex> lock(g_platformInstanceMutex);
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
        throw InternalException(std::error_code(0, std::system_category()), "glXGetProcAddress failed on glXCreateContextAttribs", ExceptionTrigger::CREATE);
    }
}


Platform::~Platform() {
}


}  // namespace glheadless
