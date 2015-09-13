#include "Implementation.h"

#include <system_error>
#include <vector>
#include <cassert>

#include <glheadless/Context.h>

#include "Window.h"
#include "Platform.h"
#include <glheadless/error.h>


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
    const auto contextHandle = wglGetCurrentContext();
    if (contextHandle == nullptr) {
        throw std::system_error(make_error_code(Error::CONTEXT_NOT_CURRENT), "wglGetCurrentContext returned nullptr");
    }

    const auto deviceContextHandle = wglGetCurrentDC();
    if (deviceContextHandle == nullptr) {
        throw std::system_error(make_error_code(Error::CONTEXT_NOT_CURRENT), "wglGetCurrentDC returned nullptr");
    }

    const auto windowHandle = WindowFromDC(deviceContextHandle);
    if (windowHandle == nullptr) {
        throw std::system_error(make_error_code(Error::CONTEXT_NOT_CURRENT), "WindowFromDC returned nullptr");
    }

    Context context;
    context.implementation()->setExternal(windowHandle, deviceContextHandle, contextHandle);

    return std::move(context);
}


Implementation::Implementation()
: m_contextHandle(nullptr)
, m_owning(true) {
}


Implementation::Implementation(Implementation&& other) {
    *this = std::move(other);
}


Implementation::~Implementation() {
    if (m_owning) {
        if (m_contextHandle != nullptr) {
            const auto currentHandle = wglGetCurrentContext();
            if (currentHandle == m_contextHandle) {
                const auto success = wglMakeCurrent(nullptr, nullptr);
                assert(success && "wglMakeCurrent(nullptr, nullptr)");
            }
            const auto success = wglDeleteContext(m_contextHandle);
            assert(success && "wglDeleteContext");
        }
    }
}


void Implementation::create(Context* context) {
    m_window = std::make_unique<Window>();
    setPixelFormat(context);
    createContext(context);
}


void Implementation::create(Context* context, const Context* shared) {
    m_window = std::make_unique<Window>();
    setPixelFormat(context);
    createContext(context, shared->implementation()->m_contextHandle);
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

    return *this;
}


void Implementation::setPixelFormat(Context* context) {
    const auto pixelFormatAttributes = createPixelFormatAttributeList(context->pixelFormat());

    int pixelFormatIndex;
    UINT numPixelFormats;
    auto success = Platform::instance()->wglChoosePixelFormatARB(m_window->deviceContext(), pixelFormatAttributes.data(), nullptr, 1, &pixelFormatIndex, &numPixelFormats);
    if (!success || numPixelFormats == 0) {
        throw std::system_error(getLastErrorCode(), "wglChoosePixelFormatARB failed");
    }

    PIXELFORMATDESCRIPTOR descriptor;
    success = DescribePixelFormat(m_window->deviceContext(), pixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &descriptor);
    if (!success) {
        throw std::system_error(getLastErrorCode(), "DescribePixelFormat failed");
    }

    success = SetPixelFormat(m_window->deviceContext(), pixelFormatIndex, &descriptor);
    if (!success) {
        throw std::system_error(getLastErrorCode(), "SetPixelFormat failed");
    }
}


void Implementation::createContext(Context* context, HGLRC shared) {
    const auto contextAttributes = createContextAttributeList(*context);

    m_contextHandle = Platform::instance()->wglCreateContextAttribsARB(m_window->deviceContext(), shared, contextAttributes.data());
    if (m_contextHandle == nullptr) {
        throw std::system_error(getLastErrorCode(), "wglCreateContextAttribsARB failed");
    }
}


void Implementation::makeCurrent(Context* /*context*/) noexcept {
    const auto success = wglMakeCurrent(m_window->deviceContext(), m_contextHandle);
    assert(success && "wglMakeCurrent");
}


void Implementation::doneCurrent(Context* /*context*/) noexcept {
    const auto success = wglMakeCurrent(nullptr, nullptr);
    assert(success && "wglMakeCurrent(nullptr, nullptr)");
}


}  // namespace glheadless
