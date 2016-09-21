#include "Platform.h"

#include <atomic>
#include <mutex>

#include <glheadless/ExceptionTrigger.h>
#include <glheadless/error.h>

#include "../InternalException.h"


namespace glheadless {
namespace egl {


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
: m_display(nullptr)
, m_version15(true) {
    m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    EGLint major, minor;
    eglInitialize(m_display, &major, &minor);

    if (major == 1 && minor < 5) {
        const auto extensions = std::string(eglQueryString(m_display, EGL_EXTENSIONS));
        const auto pos = extensions.find("EGL_KHR_create_context");
        if (pos == std::string::npos) {
            throw InternalException(Error::INVALID_CONFIGURATION, "OpenGL > 2 requires EGL 1.5 or EGL_KHR_create_context extension. You have version " + std::to_string(major) + "." + std::to_string(minor), ExceptionTrigger::CREATE);
        }
        m_version15 = false;
    }
}


Platform::~Platform() {
    eglTerminate(m_display);
}


EGLDisplay Platform::display() const {
    return m_display;
}


bool Platform::version15() const {
    return m_version15;
}


}  // namespace egl
}  // namespace glheadless
