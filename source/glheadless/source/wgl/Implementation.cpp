#include "Implementation.h"

#include <system_error>
#include <vector>
#include <cassert>

#include <glheadless/Context.h>

#include "Window.h"
#include "Platform.h"


namespace glheadless {


namespace {


std::vector<int> createPixelFormatAttributeList(const PixelFormat& pixelFormat) {
    std::map<int, int> attributes;

    attributes[WGL_DOUBLE_BUFFER_ARB] = pixelFormat.doubleBuffer() ? GL_TRUE : GL_FALSE;
    attributes[WGL_STEREO_ARB] = pixelFormat.stereo() ? GL_TRUE : GL_FALSE;

    if (pixelFormat.colorBits() > 0) {
        attributes[WGL_COLOR_BITS_ARB] = pixelFormat.colorBits();
    } else {
        attributes[WGL_RED_BITS_ARB] = pixelFormat.redBits();
        attributes[WGL_GREEN_BITS_ARB] = pixelFormat.greenBits();
        attributes[WGL_BLUE_BITS_ARB] = pixelFormat.blueBits();
        attributes[WGL_ALPHA_BITS_ARB] = pixelFormat.alphaBits();
    }

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


Implementation::Implementation()
: m_contextHandle(nullptr) {
}


Implementation::Implementation(Implementation&& other) {
    *this = std::move(other);
}


Implementation::~Implementation() {
    if (m_contextHandle != nullptr) {
        const auto currentHandle = wglGetCurrentContext();
        if (currentHandle == m_contextHandle) {
            wglMakeCurrent(nullptr, nullptr);
        }
        wglDeleteContext(m_contextHandle);
    }
}


void Implementation::create(Context* context) {
    m_window = std::make_unique<Window>();
    setPixelFormat(context);
    createContext(context);
}


Implementation& Implementation::operator=(Implementation&& other) {
    m_window = std::move(other.m_window);
    m_contextHandle = other.m_contextHandle;
    other.m_contextHandle = nullptr;

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


void Implementation::createContext(Context* context) {
    const auto contextAttributes = createContextAttributeList(*context);

    m_contextHandle = Platform::instance()->wglCreateContextAttribsARB(m_window->deviceContext(), nullptr, contextAttributes.data());
    if (m_contextHandle == nullptr) {
        throw std::system_error(getLastErrorCode(), "wglCreateContextAttribsARB failed");
    }
}


void Implementation::makeCurrent(Context* /*context*/) noexcept {
    const auto success = wglMakeCurrent(m_window->deviceContext(), m_contextHandle);
    assert(success);
}


void Implementation::doneCurrent(Context* /*context*/) noexcept {
    const auto success = wglMakeCurrent(nullptr, nullptr);
    assert(success);
}


}  // namespace glheadless
