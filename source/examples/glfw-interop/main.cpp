#include <iostream> 
#include <string>
#include <thread>

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


void workerThread1(const Context* shared) {
    Context context;
    context.setVersion(4, 5);
    context.create(*shared);

    if (!context.valid()) {
        std::cerr << context.lastErrorMessage() << ": " << context.lastErrorCode().message() << " (" << context.lastErrorCode() << ")" << std::endl;
        return;
    }

    context.makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Worker 1: created shared context with version " << versionString << std::endl;

    context.doneCurrent();
}


int main(int /*argc*/, char* /*argv*/[]) {
    glfwInit();
    glfwDefaultWindowHints();
    const auto window = glfwCreateWindow(320, 240, "", nullptr, nullptr);


    glfwMakeContextCurrent(window);

    auto context = Context::currentContext();
    if (!context.valid()) {
        std::cerr << context.lastErrorMessage() << ": " << context.lastErrorCode().message() << " (" << context.lastErrorCode() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(nullptr);


    auto worker = std::thread(workerThread1, &context);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));


    glfwMakeContextCurrent(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    glfwMakeContextCurrent(nullptr);


    worker.join();


    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
