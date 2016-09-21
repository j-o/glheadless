#include "Implementation.h"

#include <cassert>
#include <vector>
#include <map>

#include <EGL/eglext.h>

#include <glheadless/ContextFormat.h>

#include "../InternalException.h"

#include "Platform.h"


namespace glheadless {
namespace egl {


GLHEADLESS_REGISTER_IMPLEMENTATION(EGL, Implementation)


namespace {


std::vector<int> createContextAttributeList(const ContextFormat& format) {
    std::map<int, int> attributes;

    if (format.versionMajor > 0) {
        attributes[EGL_CONTEXT_MAJOR_VERSION] = format.versionMajor;
        attributes[EGL_CONTEXT_MINOR_VERSION] = format.versionMinor;
    }

    if (format.debug) {
        if (Platform::instance()->version15()) {
            attributes[EGL_CONTEXT_OPENGL_DEBUG] = EGL_TRUE;
        } else {
            attributes[EGL_CONTEXT_FLAGS_KHR] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
        }
    }

    switch (format.profile) {
        case ContextProfile::CORE:
            attributes[EGL_CONTEXT_OPENGL_PROFILE_MASK] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
            break;
        case ContextProfile::COMPATIBILITY:
            attributes[EGL_CONTEXT_OPENGL_PROFILE_MASK] = EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT;
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
    list.push_back(EGL_NONE); // finalize list

    return std::move(list);
}

const auto k_errorStrings = std::map<EGLint, std::string>{
    {EGL_SUCCESS,             "success"},
    {EGL_NOT_INITIALIZED,     "not initialized"},
    {EGL_BAD_ACCESS,          "bad access"},
    {EGL_BAD_ALLOC,           "bad alloc"},
    {EGL_BAD_ATTRIBUTE,       "bad attribute"},
    {EGL_BAD_CONTEXT,         "bad context"},
    {EGL_BAD_CONFIG,          "bad config"},
    {EGL_BAD_CURRENT_SURFACE, "bad current surface"},
    {EGL_BAD_DISPLAY,         "bad display"},
    {EGL_BAD_SURFACE,         "bad surface"},
    {EGL_BAD_MATCH,           "bad match"},
    {EGL_BAD_PARAMETER,       "bad parameter"},
    {EGL_BAD_NATIVE_PIXMAP,   "bad native pixmap"},
    {EGL_BAD_NATIVE_WINDOW,   "bad native windoe"},
    {EGL_CONTEXT_LOST,        "context lost"}
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


Implementation::Implementation()
: m_contextHandle(EGL_NO_CONTEXT)
, m_owning(true) {
}



Implementation::~Implementation() {
}


std::unique_ptr<Context> Implementation::getCurrent() {
    auto context = std::make_unique<Context>(this);
    m_context = context.get();
    m_owning = false;

    m_contextHandle = eglGetCurrentContext();
    if (m_contextHandle == EGL_NO_CONTEXT) {
        context->setError(Error::INVALID_CONTEXT, "glXGetCurrentContext returned EGL_NO_CONTEXT", ExceptionTrigger::CREATE);
        return context;
    }

    return context;
}


std::unique_ptr<Context> Implementation::create(const ContextFormat& format) {
    auto context = std::make_unique<Context>(this);
    m_context = context.get();

    try {
        createContext(EGL_NO_CONTEXT, format);
    } catch (InternalException& e) {
        context->setError(e.code(), e.message(), e.trigger());
    }

    return context;
}


std::unique_ptr<Context> Implementation::create(const Context* shared, const ContextFormat& format) {
    auto sharedImplementation = static_cast<const Implementation*>(shared->implementation());
    auto context = std::make_unique<Context>(this);
    m_context = context.get();

    try {
        createContext(sharedImplementation->m_contextHandle, format);
    } catch (InternalException& e) {
        context->setError(e.code(), e.message(), e.trigger());
    }

    return context;
}


bool Implementation::destroy() {
    bindApi();

    if (m_owning && m_contextHandle != EGL_NO_CONTEXT) {
        const auto success = eglDestroyContext(Platform::instance()->display(), m_contextHandle);
        assert(success && "eglDestroyContext failed");
    }

    m_contextHandle = EGL_NO_CONTEXT;

    return true;
}


bool Implementation::valid() {
    return m_contextHandle != EGL_NO_CONTEXT;
}


bool Implementation::makeCurrent() {
    bindApi();

    const auto success = eglMakeCurrent(Platform::instance()->display(), EGL_NO_SURFACE, EGL_NO_SURFACE, m_contextHandle);
    if (!success) {
        return m_context->setError(Error::INVALID_CONTEXT, "glXMakeContextCurrent failed: " + getErrorString(), ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::doneCurrent() {
    bindApi();

    const auto success = eglMakeCurrent(Platform::instance()->display(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (!success) {
        return m_context->setError(Error::INVALID_CONTEXT, "glXMakeContextCurrent with EGL_NO_CONTEXT failed: " + getErrorString(), ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


void Implementation::createContext(EGLContext shared, const ContextFormat& format) {
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
    const auto contextAttributes = createContextAttributeList(format);
    m_contextHandle = eglCreateContext(display, config, shared, contextAttributes.data());
    if (m_contextHandle == EGL_NO_CONTEXT) {
        throw InternalException(Error::INVALID_CONFIGURATION, "eglCreateContext failed: " + getErrorString(), ExceptionTrigger::CREATE);
    }
}


}  // namespace egl
}  // namespace glheadless
