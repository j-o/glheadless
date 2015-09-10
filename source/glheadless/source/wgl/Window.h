#pragma once

#include  <Windows.h>

#include <memory>


namespace glheadless {


class Window {
public:
    Window();
    Window(HWND windowHandle, HDC deviceContextHandle);
    Window(const Window&) = delete;
    Window(Window&& other);
    ~Window();

    HDC deviceContext() const;

    Window& operator=(const Window&) = delete;
    Window& operator=(Window&& other);


private:
    class WindowClass {
    public:
        WindowClass();
        WindowClass(const WindowClass&) = delete;
        WindowClass(WindowClass&& other);
        ~WindowClass();

        LPCTSTR id() const;

        WindowClass& operator=(const WindowClass&) = delete;
        WindowClass& operator=(WindowClass&& other);


    private:
        HMODULE m_instanceHandle;
        ATOM m_id;
    };


private:
    HWND m_windowHandle;
    HDC m_deviceContextHandle;
    std::shared_ptr<WindowClass> m_windowClass;
    bool m_owning;


private:
    static std::shared_ptr<WindowClass> getWindowClass();
    static std::weak_ptr<WindowClass> s_windowClass;
};

    
}  // namespace glheadless
