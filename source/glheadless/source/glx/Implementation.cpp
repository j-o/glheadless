#include "Implementation.h"

#include <cassert>
#include <vector>
#include <map>

#include <glheadless/Context.h>
#include <glheadless/ContextFormat.h>

#include "../InternalException.h"

#include "Platform.h"


namespace glheadless {
namespace glx {


GLHEADLESS_REGISTER_IMPLEMENTATION(GLX, Implementation)


namespace {


std::vector<int> createContextAttributeList(const ContextFormat& format) {
    std::map<int, int> attributes;

    if (format.versionMajor > 0) {
        attributes[GLX_CONTEXT_MAJOR_VERSION_ARB] = format.versionMajor;
        attributes[GLX_CONTEXT_MINOR_VERSION_ARB] = format.versionMinor;
    }

    if (format.debug) {
        attributes[GLX_CONTEXT_FLAGS_ARB] = GLX_CONTEXT_DEBUG_BIT_ARB;
    }

    switch (format.profile) {
        case ContextProfile::CORE:
            attributes[GLX_CONTEXT_PROFILE_MASK_ARB] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            break;
        case ContextProfile::COMPATIBILITY:
            attributes[GLX_CONTEXT_PROFILE_MASK_ARB] = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            break;
        default:
            break;
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


class XErrorHandler {
public:
    XErrorHandler();
    ~XErrorHandler();

    int errorCode() const;
    const std::string& errorString() const;


private:
    static XErrorHandler* s_activeHandler;
    static int errorHandler(Display* display, XErrorEvent* errorEvent);


private:
    int (*m_oldHandler)(Display*, XErrorEvent*);
    int m_errorCode;
    std::string m_errorString;
};


XErrorHandler* XErrorHandler::s_activeHandler = nullptr;


XErrorHandler::XErrorHandler()
: m_oldHandler(nullptr)
, m_errorCode(Success) {
    s_activeHandler = this;
    m_oldHandler = XSetErrorHandler(XErrorHandler::errorHandler);
}


XErrorHandler::~XErrorHandler() {
    s_activeHandler = nullptr;
    XSetErrorHandler(m_oldHandler);
}


int XErrorHandler::errorCode() const {
    return m_errorCode;
}


const std::string &XErrorHandler::errorString() const {
    return m_errorString;
}


int XErrorHandler::errorHandler(Display* display, XErrorEvent* errorEvent) {
    char buffer[1024];
    XGetErrorText(display, errorEvent->error_code, buffer, 1024);

    s_activeHandler->m_errorCode = errorEvent->error_code;
    s_activeHandler->m_errorString = std::string(buffer);

    return 0;
}


}  // unnamed namespace


Implementation::Implementation()
: m_drawable(0)
, m_pBuffer(0)
, m_contextHandle(nullptr)
, m_owning(true) {
}


Implementation::~Implementation() {
}


std::unique_ptr<Context> Implementation::getCurrent() {
    auto context = std::unique_ptr<Context>(new Context(this));
    m_context = context.get();
    m_owning = false;

    m_contextHandle = glXGetCurrentContext();
    if (m_contextHandle == nullptr) {
        context->setError(Error::INVALID_CONTEXT, "glXGetCurrentContext returned nullptr");
        return context;
    }

    m_drawable = glXGetCurrentDrawable();
    if (m_drawable == 0) {
        context->setError(Error::INVALID_CONTEXT, "glXGetCurrentDrawable returned nullptr");
        return context;
    }

    return context;
}


std::unique_ptr<Context> Implementation::create(const ContextFormat& format) {
    auto context = std::unique_ptr<Context>(new Context(this));
    m_context = context.get();

    try {
        createContext(nullptr, format);
    } catch (InternalException& e) {
        m_context->setError(e.code(), e.message());
    }

    return context;
}


std::unique_ptr<Context> Implementation::create(const Context* shared, const ContextFormat& format) {
    auto sharedImplementation = static_cast<const Implementation*>(shared->implementation());
    auto context = std::unique_ptr<Context>(new Context(this));
    m_context = context.get();

    try {
        createContext(sharedImplementation->m_contextHandle, format);
    } catch (InternalException& e) {
        m_context->setError(e.code(), e.message());
    }

    return context;
}


bool Implementation::destroy() {
    if (m_owning) {
        XErrorHandler xErrorHandler;

        if (m_contextHandle != nullptr) {
            const auto currentContext = glXGetCurrentContext();
            if (currentContext == m_contextHandle) {
                doneCurrent();
            }
            glXDestroyContext(Platform::instance()->display(), m_contextHandle);
            assert(xErrorHandler.errorCode() == Success && "glXDestroyContext failed");
        };

        if (m_pBuffer != 0) {
            glXDestroyPbuffer(Platform::instance()->display(), m_pBuffer);
            assert(xErrorHandler.errorCode() == Success && "glXDestroyPbuffer failed");
        }
    }

    m_contextHandle = nullptr;
    m_pBuffer = 0;
    m_drawable = 0;

    return true;
}


bool Implementation::valid() {
    return m_contextHandle != nullptr
        && m_drawable != 0;
}


bool Implementation::makeCurrent() {
    XErrorHandler xErrorHandler;

    if (m_contextHandle == nullptr) {
        return m_context->setError(Error::INVALID_CONTEXT, "Context not set up");
    }

    const auto success = glXMakeContextCurrent(Platform::instance()->display(), m_drawable, m_drawable, m_contextHandle);
    if (!success) {
        return m_context->setError(Error::INVALID_CONTEXT, "glXMakeContextCurrent failed (" + xErrorHandler.errorString() + ")");
    }
    return true;
}


bool Implementation::doneCurrent() {
    XErrorHandler xErrorHandler;

    const auto success = glXMakeContextCurrent(Platform::instance()->display(), None, None, nullptr);
    if (!success) {
        return m_context->setError(Error::INVALID_CONTEXT, "glXMakeContextCurrent with nullptr failed (" + xErrorHandler.errorString() + ")");
    }
    return true;
}


void Implementation::createContext(GLXContext shared, const ContextFormat& format) {
    // set custom error handler
    XErrorHandler xErrorHandler;

    // get display
    Display* display = Platform::instance()->display();


    //
    // Select framebuffer configuration
    //
    int fbCount;
    GLXFBConfig* fbConfig = glXChooseFBConfig(display, DefaultScreen(display), nullptr, &fbCount);
    if (fbConfig == nullptr) {
        throw InternalException(Error::INVALID_CONFIGURATION, "glXChooseFBConfig returned nullptr");
    }
    EnsureAtExit freeFBConfigAtExit([fbConfig] { XFree(fbConfig); });


    //
    // Create context
    //
    const auto contextAttributes = createContextAttributeList(format);
    m_contextHandle = Platform::instance()->glXCreateContextAttribsARB(display, fbConfig[0], shared, True, contextAttributes.data());
    XSync(display, false);
    if (m_contextHandle == nullptr || xErrorHandler.errorCode() != Success) {
        throw InternalException(Error::INVALID_CONFIGURATION, "glXCreateContextAttribsARB returned nullptr (" + xErrorHandler.errorString() + ")");
    }

    const int pBufferAttributes[] = {
        GLX_PBUFFER_WIDTH, 1,
        GLX_PBUFFER_HEIGHT, 1,
        None
    };
    m_pBuffer = glXCreatePbuffer(display, fbConfig[0], pBufferAttributes);
    XSync(display, false);

    // check if pbuffer is supported
    const auto success = glXMakeContextCurrent(display, m_pBuffer, m_pBuffer, m_contextHandle);
    if (success) {
        glXMakeContextCurrent(display, None, None, nullptr);
        m_drawable = m_pBuffer;
    } else {
        m_drawable = DefaultRootWindow(display);
    }
}


}  // namespace glx
}  // namespace glheadless
