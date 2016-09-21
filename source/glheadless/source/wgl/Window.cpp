#include "Window.h"

#include <mutex>
#include <cassert>

#include <Windows.h>

#include <glheadless/ExceptionTrigger.h>
#include <glheadless/error.h>

#include "../InternalException.h"

#include "Platform.h"


namespace glheadless {
namespace wgl {


std::weak_ptr<Window::WindowClass> Window::s_windowClass;


Window::WindowClass::WindowClass()
: m_id(0) {
    //
    // Get module handle
    //
    auto success = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, nullptr, &m_instanceHandle);
    if (!success) {
        throw InternalException(Error::INVALID_CONFIGURATION, "GetModuleHandleEx failed", ExceptionTrigger::CREATE);
    }


    //
    // Create window class
    //
    WNDCLASSEX windowClass;
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = DefWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = m_instanceHandle;
    windowClass.hIcon = nullptr;
    windowClass.hCursor = nullptr;
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = TEXT("playground");
    windowClass.hIconSm = nullptr;

    m_id = RegisterClassEx(&windowClass);
    if (m_id == 0) {
        throw InternalException(Error::INVALID_CONFIGURATION, "RegisterClassEx failed", ExceptionTrigger::CREATE);
    }
}


Window::WindowClass::~WindowClass() {
    if (m_id != 0) {
        const auto success = UnregisterClass(id(), m_instanceHandle);
        assert(success && "UnregisterClass");
    }
}


LPCTSTR Window::WindowClass::id() const {
    return reinterpret_cast<LPCTSTR>(m_id);
}


Window::Window()
: m_windowClass(getWindowClass())
, m_owning(true) {
    //
    // Get module handle
    //
    HMODULE instance;
    auto success = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, nullptr, &instance);
    if (!success) {
        throw InternalException(Error::INVALID_CONFIGURATION, "GetModuleHandleEx failed", ExceptionTrigger::CREATE);
    }


    //
    // Create window
    //
    m_windowHandle = CreateWindow(m_windowClass->id(),
                                  nullptr,
                                  WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  nullptr,
                                  nullptr,
                                  instance,
                                  nullptr);
    if (m_windowHandle == nullptr) {
        throw InternalException(Error::INVALID_CONFIGURATION, "CreateWindow failed", ExceptionTrigger::CREATE);
    }


    //
    // Get device context
    //
    m_deviceContextHandle = GetDC(m_windowHandle);
    if (m_deviceContextHandle == nullptr) {
        throw InternalException(Error::INVALID_CONFIGURATION, "GetDC failed", ExceptionTrigger::CREATE);
    }
}


Window::Window(HWND windowHandle, HDC deviceContextHandle)
: m_windowHandle(windowHandle)
, m_deviceContextHandle(deviceContextHandle)
, m_owning(false) {
}


Window::~Window() {
    if (m_owning) {
        if (m_deviceContextHandle != nullptr) {
            ReleaseDC(m_windowHandle, m_deviceContextHandle);
            m_deviceContextHandle = nullptr;
        }
        if (m_windowHandle != nullptr) {
            const auto success = DestroyWindow(m_windowHandle);
            assert(success && "DestroyWindow failed");
            m_windowHandle = nullptr;
        }
    }
}


HDC Window::deviceContext() const {
    return m_deviceContextHandle;
}


std::shared_ptr<Window::WindowClass> Window::getWindowClass() {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    auto windowClass = s_windowClass.lock();
    if (windowClass == nullptr) {
        windowClass = std::make_shared<WindowClass>();
        s_windowClass = windowClass;
    }
    return windowClass;
}


}  // namespace wgl
}  // namespace glheadless
