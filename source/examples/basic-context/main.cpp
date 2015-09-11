#include <iostream> 

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

    try {
        context.create();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    context.makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created context with version " << versionString << std::endl;

    context.doneCurrent();

    return EXIT_SUCCESS;
}
