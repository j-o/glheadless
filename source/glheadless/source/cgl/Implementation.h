#pragma once

#include "../AbstractImplementation.h"

#include <OpenGL/CGLTypes.h>


namespace glheadless {
namespace cgl {


class Implementation : public AbstractImplementation {
public:
    Implementation();
    virtual ~Implementation();

    virtual std::unique_ptr<Context> getCurrent() override;
    virtual std::unique_ptr<Context> create(const ContextFormat& format) override;
    virtual std::unique_ptr<Context> create(const Context* shared, const ContextFormat& format) override;
    virtual bool destroy() override;
    virtual bool valid() override;
    virtual bool makeCurrent() override;
    virtual bool doneCurrent() override;


private:
    void setPixelFormat(const ContextFormat& format);
    void createContext(CGLContextObj shared);


private:
    CGLContextObj m_contextHandle;
    CGLPixelFormatObj m_pixelFormatHandle;
    bool m_owning;
};


}  // namespace cgl
}  // namespace glheadless
