#include "Implementation.h"

#include <system_error>
#include <vector>
#include <cassert>

#include <gl/GL.h>
#include <gl/GLU.h>

#include <glheadless/Context.h>
#include <glheadless/error.h>

#include "../InternalException.h"

#include "Window.h"
#include "Platform.h"


namespace glheadless {


namespace {


std::vector<int> createPixelFormatAttributeList(const PixelFormat& pixelFormat) {
    std::map<int, int> attributes;

    attributes[WGL_DOUBLE_BUFFER_ARB] = pixelFormat.doubleBuffer() ? GL_TRUE : GL_FALSE;
    attributes[WGL_STEREO_ARB] = pixelFormat.stereo() ? GL_TRUE : GL_FALSE;
    attributes[WGL_RED_BITS_ARB] = pixelFormat.redBits();
    attributes[WGL_GREEN_BITS_ARB] = pixelFormat.greenBits();
    attributes[WGL_BLUE_BITS_ARB] = pixelFormat.blueBits();
    attributes[WGL_ALPHA_BITS_ARB] = pixelFormat.alphaBits();
    attributes[WGL_DEPTH_BITS_ARB] = pixelFormat.depthBits();
    attributes[WGL_STENCIL_BITS_ARB] = pixelFormat.stencilBits();

    for (const auto& attribute : pixelFormat.attributes()) {
        attributes[attribute.first] = attribute.second;
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


std::vector<int> createContextAttributeList(const Context& context) {
    std::map<int, int> attributes;

    attributes[WGL_CONTEXT_MAJOR_VERSION_ARB] = context.version().first;
    attributes[WGL_CONTEXT_MINOR_VERSION_ARB] = context.version().second;
    
    if (context.debugContext()) {
        attributes[WGL_CONTEXT_FLAGS_ARB] |= WGL_CONTEXT_DEBUG_BIT_ARB;
    }

    attributes[WGL_CONTEXT_PROFILE_MASK_ARB] = context.profile() == ContextProfile::CORE ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;

    for (const auto& attribute : context.attributes()) {
        attributes[attribute.first] = attribute.second;
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


Context Implementation::currentContext() {
    Context context;

    const auto contextHandle = wglGetCurrentContext();
    if (contextHandle == nullptr) {
        context.implementation()->setError(make_error_code(Error::CONTEXT_NOT_CURRENT), "wglGetCurrentContext returned nullptr", ExceptionTrigger::CREATE);
        return std::move(context);
    }

    const auto deviceContextHandle = wglGetCurrentDC();
    if (deviceContextHandle == nullptr) {
        context.implementation()->setError(make_error_code(Error::CONTEXT_NOT_CURRENT), "wglGetCurrentDC returned nullptr", ExceptionTrigger::CREATE);
        return std::move(context);
    }

    const auto windowHandle = WindowFromDC(deviceContextHandle);
    if (windowHandle == nullptr) {
        context.implementation()->setError(make_error_code(Error::CONTEXT_NOT_CURRENT), "WindowFromDC returned nullptr", ExceptionTrigger::CREATE);
        return std::move(context);
    }

    context.implementation()->setExternal(windowHandle, deviceContextHandle, contextHandle);

    return std::move(context);
}


Implementation::Implementation(Context* context)
: AbstractImplementation(context)
, m_contextHandle(nullptr)
, m_owning(true) {
}


Implementation::Implementation(Implementation&& other)
: AbstractImplementation(std::forward<AbstractImplementation>(other))
, m_window(std::move(other.m_window))
, m_contextHandle(other.m_contextHandle)
, m_owning(other.m_owning)
, m_owningThread(other.m_owningThread) {
    other.m_contextHandle = nullptr;
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
        m_window = std::make_unique<Window>();
        setPixelFormat();
        createContext();
    } catch (InternalException& e) {
        return setError(e.code(), e.message(), e.trigger());
    }

    return true;
}


bool Implementation::create(const Context* shared) {
    m_owningThread = std::this_thread::get_id();

    try {
        m_window = std::make_unique<Window>();
        setPixelFormat();
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

    if (m_owning && m_contextHandle != nullptr) {
        const auto currentHandle = wglGetCurrentContext();
        if (currentHandle == m_contextHandle) {
            doneCurrent();
        }
        const auto success = wglDeleteContext(m_contextHandle);
        if (!success) {
            return setError(getLastErrorCode(), "wglDeleteContext failed", ExceptionTrigger::CREATE);
        }
        m_contextHandle = nullptr;
    }

    if (m_window != nullptr) {
        try {
            m_window->destroy();
            m_window = nullptr;
        } catch (InternalException& e) {
            return setError(e.code(), e.message(), e.trigger());
        }
    }

    m_owningThread = std::thread::id();

    return true;
}


void Implementation::setExternal(HWND window, HDC deviceContext, HGLRC context) {
    m_window = std::make_unique<Window>(window, deviceContext);
    m_contextHandle = context;
    m_owning = false;
}


Implementation& Implementation::operator=(Implementation&& other) {
    m_window = std::move(other.m_window);

    m_contextHandle = other.m_contextHandle;
    other.m_contextHandle = nullptr;

    m_owning = other.m_owning;
    m_owningThread = other.m_owningThread;

    return *this;
}


void Implementation::setPixelFormat() {
    const auto pixelFormatAttributes = createPixelFormatAttributeList(m_context->pixelFormat());

    int pixelFormatIndex;
    UINT numPixelFormats;
    auto success = Platform::instance()->wglChoosePixelFormatARB(m_window->deviceContext(), pixelFormatAttributes.data(), nullptr, 1, &pixelFormatIndex, &numPixelFormats);
    if (!success) {
        const auto error = glGetError();
        const auto errorString = gluErrorString(error);
        throw InternalException(getLastErrorCode(), "wglChoosePixelFormatARB failed with " + std::string(reinterpret_cast<const char*>(errorString)), ExceptionTrigger::CREATE);
    }
    if (numPixelFormats == 0) {
        throw InternalException(Error::PIXEL_FORMAT_UNAVAILABLE, "wglChoosePixelFormatARB returned zero pixel formats", ExceptionTrigger::CREATE);
    }

    PIXELFORMATDESCRIPTOR descriptor;
    success = DescribePixelFormat(m_window->deviceContext(), pixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &descriptor);
    if (!success) {
        throw InternalException(getLastErrorCode(), "DescribePixelFormat failed", ExceptionTrigger::CREATE);
    }

    success = SetPixelFormat(m_window->deviceContext(), pixelFormatIndex, &descriptor);
    if (!success) {
        throw InternalException(getLastErrorCode(), "SetPixelFormat failed", ExceptionTrigger::CREATE);
    }
}


void Implementation::createContext(HGLRC shared) {
    const auto contextAttributes = createContextAttributeList(*m_context);

    m_contextHandle = Platform::instance()->wglCreateContextAttribsARB(m_window->deviceContext(), shared, contextAttributes.data());
    if (m_contextHandle == nullptr) {
        const auto error = glGetError();
        const auto errorString = gluErrorString(error);
        throw InternalException(getLastErrorCode(), "wglCreateContextAttribsARB failed with " + std::string(reinterpret_cast<const char*>(errorString)), ExceptionTrigger::CREATE);
    }
}


bool Implementation::makeCurrent() noexcept {
    const auto success = wglMakeCurrent(m_window->deviceContext(), m_contextHandle);
    if (!success) {
        return setError(getLastErrorCode(), "wglMakeCurrent failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::doneCurrent() noexcept {
    const auto success = wglMakeCurrent(nullptr, nullptr);
    if (!success) {
        return setError(getLastErrorCode(), "wglMakeCurrent with nullptr failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::valid() const {
    return !lastErrorCode()
        && m_window != nullptr
        && m_contextHandle != nullptr;
}


}  // namespace glheadless
