#pragma once

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

#include "glxext.h"


namespace glheadless {


class Platform {
public:
    static Platform* instance();


public:
    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;


private:
    Platform();
    ~Platform();
};


}  // namespace glheadless
