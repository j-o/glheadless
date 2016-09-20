#pragma once

#include "../AbstractImplementation.h"

#include <OpenGL/CGLTypes.h>
#include <glheadless/ContextFormat.h>


namespace glheadless {


class Implementation : public AbstractImplementation {
public:
    virtual std::unique_ptr<Context> getCurrent() override;

    virtual std::unique_ptr<Context> create(const ContextFormat& format) override;

    virtual std::unique_ptr<Context> create(const Context* shared, const ContextFormat& format) override;

    virtual bool destroy(Context* context) override;

    virtual bool valid(const Context* context) override;

    virtual bool makeCurrent(Context* context) override;

    virtual bool doneCurrent(Context* context) override;

private:
    bool setPixelFormat(Context* context, const ContextFormat& format);

    bool createContext(Context* context, CGLContextObj shared);
};

    
}  // namespace glheadless
