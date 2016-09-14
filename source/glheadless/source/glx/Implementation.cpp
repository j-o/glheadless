#include "Implementation.h"

#include <cassert>
#include <vector>

#include "Platform.h"
#include "../InternalException.h"

#include <EGL/egl.h>


namespace glheadless {


namespace {


std::vector<int> createContextAttributeList(const Context& context) {
    std::map<int, int> attributes;

    if (context.version().first > 0) {
        attributes[GLX_CONTEXT_MAJOR_VERSION_ARB] = context.version().first;
        attributes[GLX_CONTEXT_MINOR_VERSION_ARB] = context.version().second;
    }

    if (context.debugContext()) {
        attributes[GLX_CONTEXT_FLAGS_ARB] = GLX_CONTEXT_DEBUG_BIT_ARB;
    }

    switch (context.profile()) {
        case ContextProfile::CORE:
            attributes[GLX_CONTEXT_PROFILE_MASK_ARB] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            break;
        case ContextProfile::COMPATIBILITY:
            attributes[GLX_CONTEXT_PROFILE_MASK_ARB] = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            break;
        default:
            break;
    }

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
    s_activeHandler->m_errorString = buffer;

    return 0;
}


}  // unnamed namespace


Context Implementation::currentContext() {
    Context context;

    const auto contextHandle = glXGetCurrentContext();
    if (contextHandle == nullptr) {
        context.implementation()->setError(Error::INVALID_CONTEXT, "glXGetCurrentContext returned nullptr", ExceptionTrigger::CREATE);
        return context;
    }

    const auto drawable = glXGetCurrentDrawable();
    if (drawable == 0) {
        context.implementation()->setError(Error::INVALID_CONTEXT, "glXGetCurrentDrawable returned nullptr", ExceptionTrigger::CREATE);
        return context;
    }

    context.implementation()->setExternal(drawable, contextHandle);

    return std::move(context);
}


Implementation::Implementation(Context* context)
: AbstractImplementation(context)
, m_contextHandle(nullptr)
, m_drawable(0)
, m_pBuffer(0)
, m_owning(true) {
}


Implementation::Implementation(Implementation&& other)
: AbstractImplementation(std::forward<AbstractImplementation>(other))
, m_contextHandle(other.m_contextHandle)
, m_drawable(other.m_drawable)
, m_pBuffer(other.m_pBuffer)
, m_owning(other.m_owning)
, m_owningThread(other.m_owningThread) {
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
        XErrorHandler xErrorHandler;

        if (m_contextHandle != nullptr) {
            const auto currentContext = glXGetCurrentContext();
            if (currentContext == m_contextHandle) {
                doneCurrent();
            }
            glXDestroyContext(Platform::instance()->display(), m_contextHandle);
            if (xErrorHandler.errorCode() != Success) {
                return setError(Error::INVALID_CONTEXT, "glXDestroyContext failed (" + xErrorHandler.errorString() + ")", ExceptionTrigger::CREATE);
            }
        };

        if (m_pBuffer != 0) {
            glXDestroyPbuffer(Platform::instance()->display(), m_pBuffer);
            if (xErrorHandler.errorCode() != Success) {
                return setError(Error::INVALID_CONTEXT, "glXDestroyPbuffer failed (" + xErrorHandler.errorString() + ")", ExceptionTrigger::CREATE);
            }
        }
    }

    m_contextHandle = nullptr;
    m_pBuffer = 0;
    m_drawable = 0;

    m_owningThread = std::thread::id();
    m_owning = true;

    return true;
}


bool Implementation::makeCurrent() noexcept {
    XErrorHandler xErrorHandler;

    const auto success = glXMakeContextCurrent(Platform::instance()->display(), m_drawable, m_drawable, m_contextHandle);
    if (!success) {
        return setError(Error::INVALID_CONTEXT, "glXMakeContextCurrent failed (" + xErrorHandler.errorString() + ")", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::doneCurrent() noexcept {
    XErrorHandler xErrorHandler;

    const auto success = glXMakeContextCurrent(Platform::instance()->display(), None, None, nullptr);
    if (!success) {
        return setError(Error::INVALID_CONTEXT, "glXMakeContextCurrent with nullptr failed (" + xErrorHandler.errorString() + ")", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::valid() const {
    return !lastErrorCode()
        && m_contextHandle != nullptr;
}


Implementation& Implementation::operator=(Implementation&& other) {
    m_contextHandle = other.m_contextHandle;
    m_drawable = other.m_drawable;
    m_pBuffer = other.m_pBuffer;
    m_owning = other.m_owning;
    m_owningThread = other.m_owningThread;

    other.m_contextHandle = nullptr;
    other.m_drawable = 0;
    other.m_pBuffer = 0;

    return *this;
}


void Implementation::createContext(GLXContext shared) {
    // set custom error handler
    XErrorHandler xErrorHandler;

    // get display
    Display* display = Platform::instance()->display();


    //
    // Select framebuffer configuration
    //
    int fbCount;
    GLXFBConfig* fbConfig = glXChooseFBConfig(display, DefaultScreen(display), { None }, &fbCount);
    if (fbConfig == nullptr) {
        throw InternalException(Error::INVALID_CONFIGURATION, "glXChooseFBConfig returned nullptr", ExceptionTrigger::CREATE);
    }
    EnsureAtExit freeFBConfigAtExit([fbConfig] { XFree(fbConfig); });


    //
    // Create context
    //
    const auto contextAttributes = createContextAttributeList(*m_context);
    m_contextHandle = Platform::instance()->glXCreateContextAttribsARB(display, fbConfig[0], shared, True, contextAttributes.data());
    XSync(display, false);
    if (m_contextHandle == nullptr || xErrorHandler.errorCode() != Success) {
        throw InternalException(Error::INVALID_CONFIGURATION, "glXCreateContextAttribsARB returned nullptr (" + xErrorHandler.errorString() + ")", ExceptionTrigger::CREATE);
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

void Implementation::setExternal(GLXDrawable drawable, GLXContext contextHandle) {
    m_drawable = drawable;
    m_contextHandle = contextHandle;
    m_owning = false;
}


}  // namespace glheadless
