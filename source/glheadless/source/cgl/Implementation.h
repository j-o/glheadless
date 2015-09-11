#pragma once

#include <OpenGL/CGLTypes.h>


namespace glheadless {


class Context;


class Implementation {
public:
    static Context currentContext();


public:
    Implementation();
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&& other);
    ~Implementation();

    void create(Context* context);
    void create(Context* context, const Context* shared);

    void makeCurrent(Context* context) noexcept;
    void doneCurrent(Context* context) noexcept;

    Implementation& operator=(const Implementation&) = delete;
    Implementation& operator=(Implementation&& other);


private:
    void setPixelFormat(Context* context);
    void createContext(CGLContextObj shared = nullptr);
    void setExternal(CGLContextObj context, CGLPixelFormatObj pixelFormat);


private:
    CGLContextObj m_context;
    CGLPixelFormatObj m_pixelFormat;
    bool m_owning;
};

    
}  // namespace glheadless
