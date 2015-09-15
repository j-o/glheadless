#pragma once

#include "../AbstractImplementation.h"

#include <OpenGL/CGLTypes.h>


namespace glheadless {


class Context;


class Implementation : public AbstractImplementation {
public:
    static Context currentContext();


public:
    Implementation(Context* context);
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&& other);
    ~Implementation();

    bool create();
    bool create(const Context* shared);

    bool makeCurrent() noexcept;
    bool doneCurrent() noexcept;

    bool valid() const;

    Implementation& operator=(const Implementation&) = delete;
    Implementation& operator=(Implementation&& other);


private:
    bool setPixelFormat();
    bool createContext(CGLContextObj shared = nullptr);
    void setExternal(CGLContextObj context, CGLPixelFormatObj pixelFormat);


private:
    CGLContextObj m_contextHandle;
    CGLPixelFormatObj m_pixelFormatHandle;
    bool m_owning;
};

    
}  // namespace glheadless
