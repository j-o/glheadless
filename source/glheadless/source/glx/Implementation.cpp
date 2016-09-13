#include "Implementation.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <vector>
#include <cassert>
#include <iostream>

#include "glxext.h"
#include "Platform.h"
#include "../InternalException.h"
#include "error.h"


namespace glheadless {


namespace {


std::vector<int> createContextAttributeList(const Context& context) {
    std::map<int, int> attributes;

    attributes[GLX_CONTEXT_MAJOR_VERSION_ARB] = context.version().first;
    attributes[GLX_CONTEXT_MINOR_VERSION_ARB] = context.version().second;

    if (context.debugContext()) {
        attributes[GLX_CONTEXT_FLAGS_ARB] |= GLX_CONTEXT_DEBUG_BIT_ARB;
    }

    attributes[GLX_CONTEXT_PROFILE_MASK_ARB] = context.profile() == ContextProfile::CORE ? GLX_CONTEXT_CORE_PROFILE_BIT_ARB : GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;

    for (const auto& attribute : context.attributes()) {
        attributes[attribute.first] = attribute.second;
    }


    std::vector<int> list;
    list.reserve(attributes.size() * 2 + 1);
    for (const auto& attribute : attributes) {
        list.push_back(attribute.first);
        list.push_back(attribute.second);
    }
    list.push_back(None); // finalize list

    return std::move(list);
}


class EnsureAtExit {
public:
    using Function = std::function<void()>;

public:
    EnsureAtExit(const Function& function);
    ~EnsureAtExit();

private:
    Function m_function;
};



EnsureAtExit::EnsureAtExit(const Function &function)
: m_function(function) {
}


EnsureAtExit::~EnsureAtExit() {
    m_function();
}


auto g_errorCode = GLXError::SUCCESS;
int errorHandler(Display* display, XErrorEvent* errorEvent) {
    g_errorCode = static_cast<GLXError>(errorEvent->error_code);
    char buffer[1024];
    XGetErrorText(display, errorEvent->error_code, buffer, 1024);
    std::cout << buffer << std::endl;
    return 0;
}


}  // unnamed namespace


Context Implementation::currentContext() {
    Context context;

    const auto contextHandle = glXGetCurrentContext();
    if (contextHandle == nullptr) {
        context.implementation()->setError(make_error_code(Error::CONTEXT_NOT_CURRENT), "glXGetCurrentContext returned nullptr", ExceptionTrigger::CREATE);
        return context;
    }

    const auto drawable = glXGetCurrentDrawable();
    if (drawable == 0) {
        context.implementation()->setError(make_error_code(Error::CONTEXT_NOT_CURRENT), "glXGetCurrentDrawable returned nullptr", ExceptionTrigger::CREATE);
        return context;
    }

    const auto display = glXGetCurrentDisplay();
    if (display == nullptr) {
        context.implementation()->setError(make_error_code(Error::CONTEXT_NOT_CURRENT), "glXGetCurrentDisplay returned nullptr", ExceptionTrigger::CREATE);
        return context;
    }

    context.implementation()->setExternal(display, drawable, contextHandle);

    return std::move(context);
}


Implementation::Implementation(Context* context)
: AbstractImplementation(context)
, m_display(nullptr)
, m_contextHandle(nullptr)
, m_drawable(0)
, m_pBuffer(0)
, m_owning(true) {
}


Implementation::Implementation(Implementation&& other)
: AbstractImplementation(std::forward<AbstractImplementation>(other))
, m_display(other.m_display)
, m_contextHandle(other.m_contextHandle)
, m_drawable(other.m_drawable)
, m_pBuffer(other.m_pBuffer)
, m_owning(other.m_owning)
, m_owningThread(other.m_owningThread) {
    other.m_display = nullptr;
    other.m_contextHandle = nullptr;
    other.m_drawable = 0;
    other.m_pBuffer = 0;
}


Implementation::~Implementation() {
    try {
        destroy();
    } catch (...) {
        assert(false && "failed to destroy context");
    }
}


bool Implementation::create() {
    m_owningThread = std::this_thread::get_id();

    try {
        createContext();
    } catch (InternalException& e) {
        return setError(e.code(), e.message(), e.trigger());
    }

    return true;
}


bool Implementation::create(const Context* shared) {
    m_owningThread = std::this_thread::get_id();

    try {
        createContext(shared->implementation()->m_contextHandle);
    } catch (InternalException& e) {
        return setError(e.code(), e.message(), e.trigger());
    }

    return true;
}


bool Implementation::destroy() {
    if (m_owningThread != std::thread::id() && m_owningThread != std::this_thread::get_id()) {
        return setError(Error::INVALID_THREAD_ACCESS, "A context must be destroyed on the same thread that created it", ExceptionTrigger::CREATE);
    }

    if (m_owning) {
        if (m_contextHandle != nullptr) {
            const auto currentContext = glXGetCurrentContext();
            if (currentContext == m_contextHandle) {
                doneCurrent();
            }
            glXDestroyContext(m_display, m_contextHandle);
        };

        if (m_pBuffer != 0) {
            glXDestroyPbuffer(m_display, m_pBuffer);
        }

        if (m_display != nullptr) {
            XCloseDisplay(m_display);
        }
    }

    m_contextHandle = nullptr;
    m_pBuffer = 0;
    m_drawable = 0;
    m_display = nullptr;

    m_owningThread = std::thread::id();
    m_owning = true;

    return true;
}


bool Implementation::makeCurrent() noexcept {
    const auto success = glXMakeContextCurrent(m_display, m_drawable, m_drawable, m_contextHandle);
    if (!success) {
        return setError(make_error_code(GLXError::BAD_CONTEXT), "glXMakeContextCurrent failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::doneCurrent() noexcept {
    const auto success = glXMakeContextCurrent(m_display, None, None, nullptr);
    if (!success) {
        return setError(make_error_code(GLXError::BAD_CONTEXT), "glXMakeContextCurrent with nullptr failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::valid() const {
    return !lastErrorCode()
        && m_display != nullptr
        && m_contextHandle != nullptr;
}


Implementation& Implementation::operator=(Implementation&& other) {
    m_display = other.m_display;
    m_contextHandle = other.m_contextHandle;
    m_drawable = other.m_drawable;
    m_pBuffer = other.m_pBuffer;
    m_owning = other.m_owning;
    m_owningThread = other.m_owningThread;

    other.m_display = nullptr;
    other.m_contextHandle = nullptr;
    other.m_drawable = 0;
    other.m_pBuffer = 0;

    return *this;
}


void Implementation::createContext(GLXContext shared) {
    //
    // Set custom error handler
    //
    const auto oldErrorHandler = XSetErrorHandler(errorHandler);
    EnsureAtExit restoreErrorHandle([oldErrorHandler] {
        XSetErrorHandler(oldErrorHandler);
    });


    //
    // Open display
    //
    m_display = XOpenDisplay(nullptr);
    if (m_display == nullptr) {
        throw InternalException(make_error_code(GLXError::BAD_DISPLAY), "Failed to open display", ExceptionTrigger::CREATE);
    }


    //
    // Select framebuffer configuration
    //
    int fbCount;
    GLXFBConfig* fbConfig = glXChooseFBConfig(m_display, DefaultScreen(m_display), { None }, &fbCount);
    if (fbConfig == nullptr) {
        throw InternalException(make_error_code(GLXError::BAD_FB_CONFIG), "Failed to get framebuffer configurations", ExceptionTrigger::CREATE);
    }
    EnsureAtExit freeFBConfig([fbConfig] {
        XFree(fbConfig);
    });


    //
    // Create context
    //
    const auto contextAttributes = createContextAttributeList(*m_context);
    m_contextHandle = Platform::instance()->glXCreateContextAttribsARB(m_display, fbConfig[0], shared, True, contextAttributes.data());
    XSync(m_display, false);
    if (m_contextHandle == nullptr || g_errorCode != GLXError::SUCCESS) {
        throw InternalException(make_error_code(g_errorCode), "Failed to create context", ExceptionTrigger::CREATE);
    }

    const int pBufferAttributes[] = {
        GLX_PBUFFER_WIDTH, 1,
        GLX_PBUFFER_HEIGHT, 1,
        None
    };
    m_pBuffer = glXCreatePbuffer(m_display, fbConfig[0], pBufferAttributes);
    XSync(m_display, false);

    // check if pbuffer is supported
    const auto success = glXMakeContextCurrent(m_display, m_pBuffer, m_pBuffer, m_contextHandle);
    if (success) {
        glXMakeContextCurrent(m_display, None, None, nullptr);
        m_drawable = m_pBuffer;
    } else {
        m_drawable = DefaultRootWindow(m_display);
    }
}

void Implementation::setExternal(Display* display, GLXDrawable drawable, GLXContext contextHandle) {
    m_display = display;
    m_drawable = drawable;
    m_contextHandle = contextHandle;
    m_owning = false;
}


}  // namespace glheadless
