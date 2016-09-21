#pragma once

#include "../AbstractImplementation.h"

#include <GL/glx.h>


namespace glheadless {


class Context;


namespace glx {


class Implementation : public AbstractImplementation {
public:
    Implementation();
    virtual ~Implementation();

    virtual std::unique_ptr<Context> getCurrent() override;
    virtual std::unique_ptr<Context> create(const ContextFormat &format) override;
    virtual std::unique_ptr<Context> create(const Context *shared, const ContextFormat &format) override;
    virtual bool destroy() override;
    virtual bool valid() override;
    virtual bool makeCurrent() override;
    virtual bool doneCurrent() override;


private:
    void createContext(GLXContext shared, const ContextFormat& format);


private:
    GLXDrawable m_drawable;
    GLXPbuffer m_pBuffer;
    GLXContext m_contextHandle;
    bool m_owning;
};


}  // namespace glx
}  // namespace glheadless
