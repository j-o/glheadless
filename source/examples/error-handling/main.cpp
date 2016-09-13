#include <iostream>
#include <string>

#include <glheadless/Context.h>
#include <glheadless/error.h>


using namespace glheadless;


int main(int /*argc*/, char* /*argv*/[]) {
    Context context;
    context.setVersion(123, 42);


    //
    // default error handling
    //
    context.create();
    if (!context.valid()) {
        std::cerr << context.lastErrorCode().message() << ": " << context.lastErrorMessage() << std::endl;
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
