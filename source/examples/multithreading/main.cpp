#include <iostream>
#include <thread>
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


void workerThread1(const Context* shared) {
    Context context;
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


void workerThread2(Context* context) {
    context->makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Worker 2: created shared context with version " << versionString << std::endl;

    context->doneCurrent();
}


int main(int /*argc*/, char* /*argv*/[]) {
    Context context;
    context.create();

    if (!context.valid()) {
        std::cerr << context.lastErrorMessage() << ": " << context.lastErrorCode().message() << " (" << context.lastErrorCode() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    context.makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created context with version " << versionString << std::endl;

    context.doneCurrent();


    auto worker1 = std::thread(&workerThread1, &context);
    
    Context worker2Context;
    worker2Context.create();

    if (!worker2Context.valid()) {
        std::cerr << worker2Context.lastErrorMessage() << ": " << worker2Context.lastErrorCode().message() << " (" << worker2Context.lastErrorCode() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    auto worker2 = std::thread(&workerThread2, &worker2Context);

    worker1.join();
    worker2.join();

    return EXIT_SUCCESS;
}
