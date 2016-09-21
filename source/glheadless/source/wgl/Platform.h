#pragma once

#include <Windows.h>

#include <gl/GL.h>

#include "wglext.h"


namespace glheadless {
namespace wgl {


class Platform {
public:
    static Platform* instance();


public:
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;


private:
    Platform();
    ~Platform();
};


}  // namespace wgl
}  // namespace glheadless
