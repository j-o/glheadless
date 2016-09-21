#include "Platform.h"

#include <functional>
#include <mutex>
#include <atomic>
#include <codecvt>

#include <glheadless/ExceptionTrigger.h> 
#include <glheadless/error.h>

#include "../InternalException.h"

#include "Window.h"


namespace glheadless {
namespace wgl {


namespace {


class EnsureAtExit {
public:
    using Callback = std::function<void()>;

public:
    explicit EnsureAtExit(const Callback& callback)
        : m_callback(callback) {
    }
    ~EnsureAtExit() {
        if (m_callback != nullptr) {
            m_callback();
        }
    }

    EnsureAtExit(const EnsureAtExit&) = delete;
    EnsureAtExit& operator=(const EnsureAtExit&) = delete;

private:
    Callback m_callback;
};


std::atomic<Platform*> g_platformInstance;
std::mutex g_instanceMutex;


}  // unnamed namespace


Platform* Platform::instance() {
    // double-checked locking according to http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/
    auto tmp = g_platformInstance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (tmp == nullptr) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
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
: wglChoosePixelFormatARB(nullptr)
, wglCreateContextAttribsARB(nullptr) {
    Window window;


    //
    // Set pixel format
    //
    PIXELFORMATDESCRIPTOR pixelFormatDesc;
    ZeroMemory(&pixelFormatDesc, sizeof(PIXELFORMATDESCRIPTOR));
    pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixelFormatDesc.nVersion = 1;
    pixelFormatDesc.dwFlags = PFD_SUPPORT_OPENGL;
    pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
    pixelFormatDesc.iLayerType = PFD_MAIN_PLANE;

    const auto pixelFormat = ChoosePixelFormat(window.deviceContext(), &pixelFormatDesc);
    if (pixelFormat == 0) {
        throw InternalException(Error::INVALID_CONFIGURATION, "ChoosePixelFormat failed on temporary context", ExceptionTrigger::CREATE);
    }

    auto success = SetPixelFormat(window.deviceContext(), pixelFormat, &pixelFormatDesc);
    if (!success) {
        throw InternalException(Error::INVALID_CONFIGURATION, "SetPixelFormat failed on temporary context", ExceptionTrigger::CREATE);
    }


    //
    // Create dummy context
    //
    const auto dummyContext = wglCreateContext(window.deviceContext());
    if (dummyContext == nullptr) {
        throw InternalException(Error::INVALID_CONFIGURATION, "wglCreateContext failed on temporary context", ExceptionTrigger::CREATE);
    }
    EnsureAtExit wglDeleteContextAtExit([dummyContext] { wglDeleteContext(dummyContext); });

    success = wglMakeCurrent(window.deviceContext(), dummyContext);
    if (!success) {
        throw InternalException(Error::INVALID_CONFIGURATION, "wglMakeCurrent failed on temporary context", ExceptionTrigger::CREATE);
    }
    EnsureAtExit wglDoneCurrentAtExit([] { wglMakeCurrent(nullptr, nullptr); });


    //
    // Resolve functions
    //
    wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
    if (wglChoosePixelFormatARB == nullptr) {
        throw InternalException(Error::INVALID_CONFIGURATION, "wglGetProcAddress failed on wglChoosePixelFormatARB", ExceptionTrigger::CREATE);
    }

    wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
    if (wglCreateContextAttribsARB == nullptr) {
        throw InternalException(Error::INVALID_CONFIGURATION, "wglGetProcAddress failed on wglCreateContextAttribsARB", ExceptionTrigger::CREATE);
    }
}


Platform::~Platform() {
}


}  // namespace wgl
}  // namespace glheadless
