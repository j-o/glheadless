#pragma once

#include <EGL/egl.h>


namespace glheadless {
namespace egl {


class Platform {
public:
    static Platform* instance();


public:
    EGLDisplay display() const;
    bool version15() const;


private:
    Platform();
    ~Platform();


private:
    EGLDisplay m_display;
    bool m_version15;
};


}  // namespace egl
}  // namespace glheadless
