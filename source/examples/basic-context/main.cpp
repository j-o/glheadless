#include <iostream> 
#include <string>

#if defined(_WIN32)
#include <Windows.h>
#include <gl/GL.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#include <glheadless/Context.h>


using namespace glheadless;


int main(int /*argc*/, char* /*argv*/[]) {
    Context context;
    context.setVersion(4, 5);
    context.create();

    if (!context.valid()) {
        std::cerr << context.lastErrorMessage() << ": " << context.lastErrorCode().message() << " (" << context.lastErrorCode() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    context.makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created context with version " << versionString << std::endl;

    context.doneCurrent();

    return EXIT_SUCCESS;
}
