# glheadless

*glheadless* is a small library to simplify the creation of headless (i.e. without any visible window) OpenGL contexts on multiple platforms.

## Features

* **RAII-style** context management.
* ***Context stealing***: Capture the current context created by any other library, especially useful for
* **Shared context** creation, e.g. for multithreaded applications.

## Example

```
```

### Supported platforms:

 * Windows (using the Windows API)
 * Max OS X (using the CGL API).

