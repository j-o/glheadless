#pragma once

#include "../AbstractImplementation.h"

#include <EGL/egl.h>


namespace glheadless {


class Context;


class Implementation : public AbstractImplementation {
public:
    static Context currentContext();


public:
    explicit Implementation(Context* context);
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&& other);
    virtual ~Implementation();

    bool create();
    bool create(const Context* shared);

    bool destroy();

    bool makeCurrent() noexcept;
    bool doneCurrent() noexcept;

    bool valid() const;

    Implementation& operator=(const Implementation&) = delete;
    Implementation& operator=(Implementation&& other);


private:
    void createContext(EGLContext shared = EGL_NO_CONTEXT);
    void setExternal(EGLContext contextHandle);


private:
    EGLContext m_contextHandle;

    bool m_owning;
    std::thread::id m_owningThread;
};


}  // namespace glheadless