#include <iostream>
#include <thread>

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


void workerThread1(const Context* shared) {
    auto context = ContextFactory::create(shared);
    if (!context->valid()) {
        std::cerr << context->lastErrorCode().message() << ": " << context->lastErrorMessage() << std::endl;
        return;
    }

    context->makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Worker 1: created shared context with version " << versionString << std::endl;

    context->doneCurrent();
}


void workerThread2(Context* context) {
    context->makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Worker 2: created shared context with version " << versionString << std::endl;

    context->doneCurrent();
}


int main(int /*argc*/, char* /*argv*/[]) {
    auto context = ContextFactory::create();
    if (!context->valid()) {
        std::cerr << context->lastErrorCode().message() << ": " << context->lastErrorMessage() << std::endl;
        return EXIT_FAILURE;
    }

    context->makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created context with version " << versionString << std::endl;

    context->doneCurrent();


    //
    // Worker 1 receives a pointer to the main context and creates its own shared context
    //
    auto worker1 = std::thread(&workerThread1, context.get());


    //
    // Worker 2 receives a pointer to a shared context created on the main thread
    //
    auto worker2Context = ContextFactory::create(context.get());
    if (!worker2Context->valid()) {
        std::cerr << worker2Context->lastErrorCode().message() << ": " << worker2Context->lastErrorMessage() << std::endl;
        return EXIT_FAILURE;
    }

    auto worker2 = std::thread(&workerThread2, worker2Context.get());


    worker1.join();
    worker2.join();

    return EXIT_SUCCESS;
}
