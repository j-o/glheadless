#include <iostream>

#include <glheadless/Context.h>
#include <glheadless/error.h>


using namespace glheadless;


void printError(const std::error_code& code, const std::string& message) {
    std::cerr << message << ": " << code.message() << " (" << code << ")" << std::endl;
}


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
    context.setErrorCallback(throwingErrorCallback);

    try {
        context.create();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }


    //
    // Custom error callback
    //
    context.setErrorCallback(printError);

    context.create();
    if (!context.valid()) {
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
