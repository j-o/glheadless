#pragma once

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>


namespace glheadless {
namespace glx {


class Platform {
public:
    static Platform* instance();


public:
    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;


public:
    Display* display() const;


private:
    Platform();

    ~Platform();


private:
    Display* m_display;
};


}  // namespace glx
}  // namespace glheadless
