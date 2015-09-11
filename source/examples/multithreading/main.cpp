#include <iostream>
#include <thread>

#include <Windows.h>
#include <gl/GL.h>

#include <glheadless/Context.h>


using namespace glheadless;


void workerThread1(const Context* shared) {
    Context context;
    context.setVersion(4, 5);

    try {
        context.create(*shared);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    context.makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Worker 1: created shared context with version " << versionString << std::endl;

    context.doneCurrent();
}


void workerThread2(Context&& context) {
    context.makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Worker 2: created shared context with version " << versionString << std::endl;

    context.doneCurrent();
}


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


    auto worker1 = std::thread(&workerThread1, &context);
    
    Context worker2Context;
    worker2Context.setVersion(4, 5);
    try {
        worker2Context.create(context);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    auto worker2 = std::thread(&workerThread2, std::move(worker2Context));

    worker1.join();
    worker2.join();

    return EXIT_SUCCESS;
}
