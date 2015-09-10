#pragma once

#include <string>

#include <Windows.h>

#include <gl/GL.h>

#include "wglext.h"


namespace std {
class error_code;
}  // namespace std


namespace glheadless {


std::error_code getLastErrorCode();
std::string getLastErrorMessage();


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

    
}  // namespace glheadless
