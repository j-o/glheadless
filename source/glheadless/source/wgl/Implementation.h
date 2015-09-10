#pragma once

#include <memory>

#include <Windows.h>


namespace glheadless {


class Context;
class Window;


class Implementation {
public:
    Implementation();
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&& other);
    ~Implementation();

    void create(Context* context);

    void makeCurrent(Context* context) noexcept;
    void doneCurrent(Context* context) noexcept;

    Implementation& operator=(const Implementation&) = delete;
    Implementation& operator=(Implementation&& other);


private:
    void setPixelFormat(Context* context);
    void createContext(Context* context);


private:
    std::unique_ptr<Window> m_window;
    HGLRC m_contextHandle;
};

    
}  // namespace glheadless
