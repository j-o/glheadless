#pragma once

#include "../AbstractImplementation.h"

#include <Windows.h>


namespace glheadless {


class Context;


namespace wgl {


class Window;


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
    void setPixelFormat() const;
    void createContext(HGLRC shared, const ContextFormat& format);


private:
    std::unique_ptr<Window> m_window;
    HGLRC m_contextHandle;
    bool m_owning;
};


}  // namespace wgl
}  // namespace glheadless
