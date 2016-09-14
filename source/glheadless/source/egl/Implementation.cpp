#include "Implementation.h"

#include <cassert>
#include <vector>

#include <EGL/eglext.h>

#include "Platform.h"
#include "../InternalException.h"


namespace glheadless {


namespace {


std::vector<int> createContextAttributeList(const Context& context) {
    std::map<int, int> attributes;

    attributes[EGL_CONTEXT_MAJOR_VERSION] = context.version().first;
    attributes[EGL_CONTEXT_MINOR_VERSION] = context.version().second;

    if (context.debugContext()) {
        if (Platform::instance()->version15()) {
            attributes[EGL_CONTEXT_OPENGL_DEBUG] = EGL_TRUE;
        } else {
            attributes[EGL_CONTEXT_FLAGS_KHR] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
        }
    }

    attributes[EGL_CONTEXT_OPENGL_PROFILE_MASK] = context.profile() == ContextProfile::CORE ? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT : EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT;

    for (const auto& attribute : context.attributes()) {
        attributes[attribute.first] = attribute.second;
    }


    std::vector<int> list;
    list.reserve(attributes.size() * 2 + 1);
    for (const auto& attribute : attributes) {
        list.push_back(attribute.first);
        list.push_back(attribute.second);
    }
    list.push_back(EGL_NONE); // finalize list

    return std::move(list);
}

const auto k_errorStrings = std::map<EGLint, std::string>{
    { EGL_SUCCESS, "success" },
    { EGL_NOT_INITIALIZED, "not initialized" },
    { EGL_BAD_ACCESS, "bad access" },
    { EGL_BAD_ALLOC, "bad alloc" },
    { EGL_BAD_ATTRIBUTE, "bad attribute" },
    { EGL_BAD_CONTEXT, "bad context" },
    { EGL_BAD_CONFIG, "bad config" },
    { EGL_BAD_CURRENT_SURFACE, "bad current surface" },
    { EGL_BAD_DISPLAY, "bad display" },
    { EGL_BAD_SURFACE, "bad surface" },
    { EGL_BAD_MATCH, "bad match" },
    { EGL_BAD_PARAMETER, "bad parameter" },
    { EGL_BAD_NATIVE_PIXMAP, "bad native pixmap" },
    { EGL_BAD_NATIVE_WINDOW, "bad native windoe" },
    { EGL_CONTEXT_LOST, "context lost" }
};


std::string errorString(EGLint error) {
    const auto itr = k_errorStrings.find(error);
    if (itr != k_errorStrings.end()) {
        return itr->second;
    }

    return "unkown error";
}

std::string getErrorString() {
    return errorString(eglGetError());
}


thread_local auto t_apiBound = false;
void bindApi() {
    if (!t_apiBound) {
        eglBindAPI(EGL_OPENGL_API);
        t_apiBound = true;
    }
}


}  // unnamed namespace


Context Implementation::currentContext() {
    bindApi();

    Context context;

    const auto contextHandle = eglGetCurrentContext();
    if (contextHandle == EGL_NO_CONTEXT) {
        context.implementation()->setError(Error::INVALID_CONTEXT, "glXGetCurrentContext returned EGL_NO_CONTEXT", ExceptionTrigger::CREATE);
        return context;
    }

    context.implementation()->setExternal(contextHandle);

    return std::move(context);
}


Implementation::Implementation(Context* context)
: AbstractImplementation(context)
, m_contextHandle(EGL_NO_CONTEXT)
, m_owning(true) {
}


Implementation::Implementation(Implementation&& other)
: AbstractImplementation(std::forward<AbstractImplementation>(other))
, m_contextHandle(other.m_contextHandle)
, m_owning(other.m_owning)
, m_owningThread(other.m_owningThread) {
    other.m_contextHandle = EGL_NO_CONTEXT;
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

    bindApi();

    if (m_owning && m_contextHandle != EGL_NO_CONTEXT) {
        const auto success = eglDestroyContext(Platform::instance()->display(), m_contextHandle);
        if (!success) {
            return setError(Error::INVALID_CONTEXT, "eglDestroyContext failed: " + getErrorString(), ExceptionTrigger::CREATE);
        }
    }

    m_contextHandle = EGL_NO_CONTEXT;

    m_owningThread = std::thread::id();
    m_owning = true;

    return true;
}


bool Implementation::makeCurrent() noexcept {
    bindApi();

    const auto success = eglMakeCurrent(Platform::instance()->display(), EGL_NO_SURFACE, EGL_NO_SURFACE, m_contextHandle);
    if (!success) {
        return setError(Error::INVALID_CONTEXT, "glXMakeContextCurrent failed: " + getErrorString(), ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::doneCurrent() noexcept {
    bindApi();

    const auto success = eglMakeCurrent(Platform::instance()->display(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (!success) {
        return setError(Error::INVALID_CONTEXT, "glXMakeContextCurrent with EGL_NO_CONTEXT failed: " + getErrorString(), ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::valid() const {
    return !lastErrorCode()
        && m_contextHandle != EGL_NO_CONTEXT;
}


Implementation& Implementation::operator=(Implementation&& other) {
    m_contextHandle = other.m_contextHandle;
    m_owning = other.m_owning;
    m_owningThread = other.m_owningThread;

    other.m_contextHandle = EGL_NO_CONTEXT;

    return *this;
}


void Implementation::createContext(EGLContext shared) {
    // get display
    EGLDisplay display = Platform::instance()->display();


    //
    // Select configuration
    //
    static EGLint configAttributes[] = {
        EGL_SURFACE_TYPE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    EGLint numConfigs;
    EGLConfig config;
    auto success = eglChooseConfig(display, configAttributes, &config, 1, &numConfigs);
    if (!success) {
        throw InternalException(Error::INVALID_CONFIGURATION, "eglChooseConfig failed: " + getErrorString(), ExceptionTrigger::CREATE);
    }


    bindApi();


    //
    // Create context
    //
    const auto contextAttributes = createContextAttributeList(*m_context);
    m_contextHandle = eglCreateContext(display, config, shared, contextAttributes.data());
    if (m_contextHandle == EGL_NO_CONTEXT) {
        throw InternalException(Error::INVALID_CONFIGURATION, "eglCreateContext failed: " + getErrorString(), ExceptionTrigger::CREATE);
    }
}

void Implementation::setExternal(EGLContext contextHandle) {
    m_contextHandle = contextHandle;
    m_owning = false;
}


}  // namespace glheadless
