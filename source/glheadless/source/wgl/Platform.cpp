#include "Platform.h"

#include <system_error>
#include <functional>
#include <mutex>
#include <atomic>
#include <codecvt>

#include "Window.h"


namespace glheadless {


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
std::mutex g_platformInstanceMutex;


}  // unnamed namespace


std::error_code getLastErrorCode() {
    return std::error_code(GetLastError(), std::system_category());
}


std::string getLastErrorMessage() {
    LPTSTR lpMsgBuf;
    const auto error = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPTSTR>(&lpMsgBuf),
        0,
        nullptr);

#ifdef UNICODE
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    const auto message = converter.to_bytes(lpMsgBuf);
#else
    const auto message = std::string(lpMsgBuf);
#endif

    LocalFree(lpMsgBuf);

    return message;
}


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
: wglChoosePixelFormatARB(nullptr)
, wglCreateContextAttribsARB(nullptr) {
    Window window;


    //
    // Set pixel format
    //
    PIXELFORMATDESCRIPTOR pixelFormatParams;
    pixelFormatParams.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatParams.iPixelType = PFD_TYPE_RGBA;
    pixelFormatParams.cColorBits = 32;
    pixelFormatParams.cDepthBits = 24;
    pixelFormatParams.cStencilBits = 8;
    pixelFormatParams.iLayerType = PFD_MAIN_PLANE;

    const auto pixelFormat = ChoosePixelFormat(window.deviceContext(), &pixelFormatParams);
    if (pixelFormat == 0) {
        throw std::system_error(getLastErrorCode(), "ChoosePixelFormat failed on temporary context");
    }

    auto success = SetPixelFormat(window.deviceContext(), pixelFormat, &pixelFormatParams);
    if (!success) {
        throw std::system_error(getLastErrorCode(), "SetPixelFormat failed on temporary context");
    }


    //
    // Create dummy context
    //
    const auto dummyContext = wglCreateContext(window.deviceContext());
    if (dummyContext == nullptr) {
        throw std::system_error(getLastErrorCode(), "wglCreateContext failed on temporary context");
    }
    EnsureAtExit wglDeleteContextAtExit([dummyContext] { wglDeleteContext(dummyContext); });

    success = wglMakeCurrent(window.deviceContext(), dummyContext);
    if (!success) {
        throw std::system_error(getLastErrorCode(), "wglMakeCurrent failed on temporary context");
    }
    EnsureAtExit wglDoneCurrentAtExit([] { wglMakeCurrent(nullptr, nullptr); });


    //
    // Resolve functions
    //
    wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
    if (wglChoosePixelFormatARB == nullptr) {
        throw std::system_error(getLastErrorCode(), "wglGetProcAddress failed on wglChoosePixelFormatARB");
    }

    wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
    if (wglCreateContextAttribsARB == nullptr) {
        throw std::system_error(getLastErrorCode(), "wglGetProcAddress failed on wglCreateContextAttribsARB");
    }
}


Platform::~Platform() {
}


}  // namespace glheadless
