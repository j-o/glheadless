#include "Implementation.h"

#include <vector>
#include <cassert>
#include <map>

#include <gl/GL.h>
#include <gl/GLU.h>

#include <glheadless/Context.h>
#include <glheadless/ContextFormat.h>

#include "../InternalException.h"

#include "Window.h"
#include "Platform.h"


using namespace glheadless;


namespace glheadless {
namespace wgl {


GLHEADLESS_REGISTER_IMPLEMENTATION(WGL, wgl::Implementation)


namespace {


std::vector<int> createContextAttributeList(const ContextFormat& format) {
    std::map<int, int> attributes;

    if (format.versionMajor > 0) {
        attributes[WGL_CONTEXT_MAJOR_VERSION_ARB] = format.versionMajor;
        attributes[WGL_CONTEXT_MINOR_VERSION_ARB] = format.versionMinor;
    }
    
    if (format.debug) {
        attributes[WGL_CONTEXT_FLAGS_ARB] = WGL_CONTEXT_DEBUG_BIT_ARB;
    }

    switch (format.profile) {
        case ContextProfile::CORE:
            attributes[WGL_CONTEXT_PROFILE_MASK_ARB] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
            break;
        case ContextProfile::COMPATIBILITY:
            attributes[WGL_CONTEXT_PROFILE_MASK_ARB] = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
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
    list.push_back(0); // finalize list

    return std::move(list);
}


} // unnamed namespace


Implementation::Implementation()
: m_contextHandle(nullptr)
, m_owning(true) {
}


Implementation::~Implementation() {
}


std::unique_ptr<Context> Implementation::getCurrent() {
    auto context = std::make_unique<Context>(this);
    m_context = context.get();
    m_owning = false;

    try {
        m_contextHandle = wglGetCurrentContext();
        if (m_contextHandle == nullptr) {
            throw InternalException(Error::INVALID_CONTEXT, "wglGetCurrentContext returned nullptr", ExceptionTrigger::CREATE);
        }

        const auto deviceContextHandle = wglGetCurrentDC();
        if (deviceContextHandle == nullptr) {
            throw InternalException(Error::INVALID_CONTEXT, "wglGetCurrentDC returned nullptr", ExceptionTrigger::CREATE);
        }

        const auto windowHandle = WindowFromDC(deviceContextHandle);
        if (windowHandle == nullptr) {
            throw InternalException(Error::INVALID_CONTEXT, "WindowFromDC returned nullptr", ExceptionTrigger::CREATE);
        }

        m_window = std::make_unique<Window>(windowHandle, deviceContextHandle);
    } catch (InternalException& e) {
        context->setError(e.code(), e.message(), e.trigger());
    }

    return context;
}


std::unique_ptr<Context> Implementation::create(const ContextFormat& format) {
    auto context = std::make_unique<Context>(this);
    m_context = context.get();

    try {
        m_window = std::make_unique<Window>();
        setPixelFormat();
        createContext(nullptr, format);
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
        m_window = std::make_unique<Window>();
        setPixelFormat();
        createContext(sharedImplementation->m_contextHandle, format);
    } catch (InternalException& e) {
        context->setError(e.code(), e.message(), e.trigger());
    }
    return context;
}


bool Implementation::destroy() {
    if (m_owning && m_contextHandle != nullptr) {
        const auto currentHandle = wglGetCurrentContext();
        if (currentHandle == m_contextHandle) {
            doneCurrent();
        }
        const auto success = wglDeleteContext(m_contextHandle);
        assert(success && "wglDeleteContext failed");
    }

    m_contextHandle = nullptr;
    m_window = nullptr;

    return true;
}


bool Implementation::valid() {
    return m_contextHandle != nullptr
        && m_window != nullptr;
}


bool Implementation::makeCurrent() {
    const auto success = wglMakeCurrent(m_window->deviceContext(), m_contextHandle);
    if (!success) {
        return m_context->setError(Error::INVALID_CONTEXT, "wglMakeCurrent failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::doneCurrent() {
    const auto success = wglMakeCurrent(nullptr, nullptr);
    if (!success) {
        return m_context->setError(Error::INVALID_CONTEXT, "wglMakeCurrent with nullptr failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


void Implementation::setPixelFormat() const {
    const auto deviceContext = m_window->deviceContext();

    static const int attributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        0
    };

    int pixelFormatIndex;
    UINT numPixelFormats;
    auto success = Platform::instance()->wglChoosePixelFormatARB(deviceContext, attributes, nullptr, 1, &pixelFormatIndex, &numPixelFormats);
    if (!success) {
        const auto error = glGetError();
        const auto errorString = gluErrorString(error);
        throw InternalException(Error::INVALID_CONFIGURATION, "wglChoosePixelFormatARB failed with " + std::string(reinterpret_cast<const char*>(errorString)), ExceptionTrigger::CREATE);
    }
    if (numPixelFormats == 0) {
        throw InternalException(Error::INVALID_CONFIGURATION, "wglChoosePixelFormatARB returned zero pixel formats", ExceptionTrigger::CREATE);
    }

    PIXELFORMATDESCRIPTOR descriptor;
    success = DescribePixelFormat(deviceContext, pixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &descriptor);
    if (!success) {
        throw InternalException(Error::INVALID_CONFIGURATION, "DescribePixelFormat failed", ExceptionTrigger::CREATE);
    }

    success = SetPixelFormat(deviceContext, pixelFormatIndex, &descriptor);
    if (!success) {
        throw InternalException(Error::INVALID_CONFIGURATION, "SetPixelFormat failed", ExceptionTrigger::CREATE);
    }
}


void Implementation::createContext(HGLRC shared, const ContextFormat& format) {
    const auto contextAttributes = createContextAttributeList(format);

    m_contextHandle = Platform::instance()->wglCreateContextAttribsARB(m_window->deviceContext(), shared, contextAttributes.data());
    if (m_contextHandle == nullptr) {
        const auto error = glGetError();
        const auto errorString = gluErrorString(error);
        throw InternalException(Error::INVALID_CONFIGURATION, "wglCreateContextAttribsARB failed with " + std::string(reinterpret_cast<const char*>(errorString)), ExceptionTrigger::CREATE);
    }
}


}  // namespace wgl
}  // namespace glheadless
