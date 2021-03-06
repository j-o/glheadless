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
#include <glheadless/ContextFactory.h>


using namespace glheadless;


int main(int /*argc*/, char* /*argv*/[]) {
    //! [Creating a context]
    ContextFormat format;
    format.versionMajor = 3;
    format.versionMinor = 2;
    auto context = ContextFactory::create(format);
    //! [Creating a context]


    //! [Checking for errors]
    if (!context->valid()) {
        std::cerr << context->lastErrorCode().message() << ": " << context->lastErrorMessage() << std::endl;
        return EXIT_FAILURE;
    }
    //! [Checking for errors]

    context->makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created context with version " << versionString << std::endl;

    context->doneCurrent();

    return EXIT_SUCCESS;
}
