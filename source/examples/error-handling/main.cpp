#include <iostream>

#include <glheadless/Context.h>
#include <glheadless/error.h>


using namespace glheadless;


int main(int /*argc*/, char* /*argv*/[]) {
    PixelFormat pixelFormat;
    pixelFormat.setAttribute(0xFFFFFFFF, 0xFFFFFFFF);

    Context context;
    context.setPixelFormat(pixelFormat);


    //
    // default error handling
    //
    context.create();
    if (!context.valid()) {
        std::cerr << context.lastErrorMessage() << ": " << context.lastErrorCode().message() << " (" << context.lastErrorCode() << ")" << std::endl;
    }


    //
    // Throw exception on error
    //
    context.setExceptionTriggers(ExceptionTrigger::CREATE);

    try {
        context.create();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }


    return EXIT_SUCCESS;
}
