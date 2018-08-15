#pragma once

#include "../AbstractImplementation.h"

#include <EGL/egl.h>


namespace glheadless {


class Context;


namespace egl {


class Implementation : public AbstractImplementation {
public:
    Implementation();
    virtual ~Implementation();

    virtual std::unique_ptr<Context> getCurrent() override;
    virtual std::unique_ptr<Context> create(const ContextFormat& format) override;
    virtual std::unique_ptr<Context> create(const Context* shared, const ContextFormat& format) override;
    virtual bool destroy() override;
    virtual long long nativeHandle() override;
    virtual bool valid() override;
    virtual bool makeCurrent() override;
    virtual bool doneCurrent() override;
    virtual void(*getProcAddress(const char* name))() override;


private:
    void createContext(EGLContext shared, const ContextFormat& format);


private:
    EGLContext m_contextHandle;
    bool m_owning;
};


}  // namespace egl
}  // namespace glheadless
