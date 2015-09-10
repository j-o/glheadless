#include "Window.h"

#include <mutex>

#include <Windows.h>

#include "Platform.h"


namespace glheadless {


std::weak_ptr<Window::WindowClass> Window::s_windowClass;


Window::WindowClass::WindowClass()
    : m_id(0) {
    //
    // Get module handle
    //
    auto success = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, nullptr, &m_instanceHandle);
    if (!success) {
        throw std::system_error(getLastErrorCode(), "Failed to get module handle");
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
        throw std::system_error(getLastErrorCode(), "Failed to register window class");
    }
}


Window::WindowClass::WindowClass(WindowClass&& other) {
    *this = std::move(other);
}


Window::WindowClass::~WindowClass() {
    if (m_id != 0) {
        UnregisterClass(id(), m_instanceHandle);
    }
}


LPCTSTR Window::WindowClass::id() const {
    return reinterpret_cast<LPCTSTR>(m_id);
}


Window::WindowClass& Window::WindowClass::operator=(WindowClass&& other) {
    m_instanceHandle = other.m_instanceHandle;
    m_id = other.m_id;
    other.m_id = 0;

    return *this;
}


Window::Window()
: m_windowClass(getWindowClass()) {
    //
    // Get module handle
    //
    HMODULE instance;
    auto success = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, nullptr, &instance);
    if (!success) {
        throw std::system_error(getLastErrorCode(), "Failed to get module handle");
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
        throw std::system_error(getLastErrorCode(), "Failed to create window");
    }


    //
    // Get device context
    //
    m_deviceContextHandle = GetDC(m_windowHandle);
    if (m_deviceContextHandle == nullptr) {
        throw std::system_error(getLastErrorCode(), "Failed to get device context");
    }
}


Window::Window(Window&& other) {
    *this = std::move(other);
}


Window::~Window() {
    if (m_deviceContextHandle != nullptr) {
        ReleaseDC(m_windowHandle, m_deviceContextHandle);
    }
    if (m_windowHandle != nullptr) {
        DestroyWindow(m_windowHandle);
    }
}


HDC Window::deviceContext() const {
    return m_deviceContextHandle;
}


Window& Window::operator=(Window&& other) {
    m_windowHandle = other.m_windowHandle;
    other.m_windowHandle = nullptr;

    m_deviceContextHandle = other.m_deviceContextHandle;
    other.m_deviceContextHandle = nullptr;

    m_windowClass = std::move(other.m_windowClass);

    return *this;
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


}  // namespace glheadless
