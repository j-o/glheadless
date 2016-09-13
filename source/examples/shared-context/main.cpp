#include <iostream>
#include <string>

#if defined(_WIN32)
#include <Windows.h>
#include <gl/GL.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#endif

#include <glheadless/Context.h>


using namespace glheadless;


int main(int /*argc*/, char* /*argv*/[]) {
    Context context;
    context.create();

    if (!context.valid()) {
        std::cerr << context.lastErrorMessage() << ": " << context.lastErrorCode().message() << " (" << context.lastErrorCode() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    context.makeCurrent();

    auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created context with version " << versionString << std::endl;

    context.doneCurrent();


    Context shared;
    shared.create(context);

    if (!shared.valid()) {
        std::cerr << shared.lastErrorMessage() << ": " << shared.lastErrorCode().message() << " (" << shared.lastErrorCode() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    shared.makeCurrent();

    versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created shared context with version " << versionString << std::endl;

    shared.doneCurrent();

    return EXIT_SUCCESS;
}
