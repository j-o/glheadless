#pragma once

#include "../AbstractImplementation.h"

#include <GL/glx.h>


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
    void createContext(GLXContext shared = nullptr);
    void setExternal(GLXDrawable drawable, GLXContext contextHandle);


private:
    GLXContext m_contextHandle;
    GLXDrawable m_drawable;
    GLXPbuffer m_pBuffer;

    bool m_owning;
    std::thread::id m_owningThread;
};


}  // namespace glheadless