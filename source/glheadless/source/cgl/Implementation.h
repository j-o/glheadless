#pragma once

#include <OpenGL/CGLTypes.h>

#include "../AbstractImplementation.h"


namespace glheadless {


class Context;


class Implementation : public AbstractImplementation {
public:
    static Context currentContext();


public:
    Implementation();
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&& other);
    ~Implementation();

    bool create(Context* context);
    bool create(Context* context, const Context* shared);

    void makeCurrent() noexcept;
    void doneCurrent() noexcept;

    bool valid() const;

    Implementation& operator=(const Implementation&) = delete;
    Implementation& operator=(Implementation&& other);


private:
    bool setPixelFormat(Context* context);
    bool createContext(CGLContextObj shared = nullptr);
    void setExternal(CGLContextObj context, CGLPixelFormatObj pixelFormat);


private:
    CGLContextObj m_context;
    CGLPixelFormatObj m_pixelFormat;
    bool m_owning;
};

    
}  // namespace glheadless
