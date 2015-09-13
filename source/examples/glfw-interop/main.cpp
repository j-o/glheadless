#include <iostream> 

#if defined(_WIN32)
#include <Windows.h>
#include <gl/GL.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glheadless/Context.h>


using namespace glheadless;


int main(int /*argc*/, char* /*argv*/[]) {
    glfwInit();
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, false);
    const auto window = glfwCreateWindow(320, 240, "", nullptr, nullptr);
    glfwHideWindow(window);


    glfwMakeContextCurrent(window);

    Context context = Context::currentContext();
    if (!context.valid()) {
        std::cerr << context.lastErrorMessage() << ": " << context.lastErrorCode().message() << " (" << context.lastErrorCode() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(nullptr);


    context.makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Captured context with version " << versionString << std::endl;

    context.doneCurrent();


    glfwDestroyWindow(window);

    return EXIT_SUCCESS;
}
