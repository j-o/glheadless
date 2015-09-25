#pragma once

#include "../AbstractImplementation.h"

#include <memory>

#include <Windows.h>


namespace glheadless {


class Context;
class Window;


class Implementation : public AbstractImplementation {
public:
    static Context currentContext();


public:
    explicit Implementation(Context* context);
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&& other);
    ~Implementation();

    bool create();
    bool create(const Context* shared);

    bool destroy();

    bool makeCurrent() noexcept;
    bool doneCurrent() noexcept;

    bool valid() const;

    Implementation& operator=(const Implementation&) = delete;
    Implementation& operator=(Implementation&& other);


private:
    void setPixelFormat();
    void createContext(HGLRC shared = nullptr);
    void setExternal(HWND window, HDC deviceContext, HGLRC context);


private:
    std::unique_ptr<Window> m_window;
    HGLRC m_contextHandle;
    bool m_owning;
    std::thread::id m_owningThread;
};

    
}  // namespace glheadless
