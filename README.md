# glheadless

*glheadless* is a small library to simplify the creation of headless (i.e. without any visible window) OpenGL contexts on multiple platforms.

## Features

* **Easy to use** context management.
* ***Context stealing***: Capture the current context created by any other library, especially useful for
* **Shared context** creation, e.g. for multithreaded applications.
* Configurable **error handling** using exceptions or `std::error_code` facilities.

## Example

Here is a simple example of how to create a context:

```c++
    //! [Creating a context]
    Context context;
    context.setVersion(4, 5);
    context.create();
    //! [Creating a context]


    //! [Checking for errors]
    if (!context.valid()) {
        std::cerr << context.lastErrorMessage() << ": " << context.lastErrorCode().message() << " (" << context.lastErrorCode() << ")" << std::endl;
        return EXIT_FAILURE;
    }
    //! [Checking for errors]

    context.makeCurrent();

    const auto versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "Created context with version " << versionString << std::endl;

    context.doneCurrent();
```

Have a look at the `source/examples` folder for some more.

## Supported platforms

Currently, the following platforms are supported:

 * Windows (using the Windows API)
 * Max OS X (using the CGL API)

### Compilers

A C++14 compatible compiler is required to compile *glheadless*.
