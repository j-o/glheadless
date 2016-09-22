#include <iostream>

#include <glheadless/Context.h>
#include <glheadless/ContextFactory.h>


using namespace glheadless;


int main(int /*argc*/, char* /*argv*/[]) {
    ContextFormat format;
    format.versionMajor = 123;
    format.versionMinor = 42;

    auto context = ContextFactory::create(format);


    //
    // check for errors
    //
    if (!context->valid()) {
        std::cerr << context->lastErrorCode().message() << ": " << context->lastErrorMessage() << std::endl;
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
