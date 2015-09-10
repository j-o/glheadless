#include <iostream> 

#include <Windows.h>
#include <gl/GL.h>

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

    auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created context with version " << versionString << std::endl;

    context.doneCurrent();


    Context shared;
    shared.setVersion(4, 5);

    try {
        shared.create(context);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    shared.makeCurrent();

    versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created shared context with version " << versionString << std::endl;

    shared.doneCurrent();

    return EXIT_SUCCESS;
}
