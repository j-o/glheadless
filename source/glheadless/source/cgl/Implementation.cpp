#include "Implementation.h"

#include <vector>
#include <system_error>
#include <set>
#include <cassert>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#include <glheadless/Context.h>
#include <glheadless/error.h>


namespace glheadless {


namespace {


const auto k_booleanAttributes = std::set<_CGLPixelFormatAttribute> {
    kCGLPFAAllRenderers,
    kCGLPFADoubleBuffer,
    kCGLPFAStereo,
    kCGLPFAMinimumPolicy,
    kCGLPFAMaximumPolicy,
    kCGLPFAColorFloat,
    kCGLPFAMultisample,
    kCGLPFASupersample,
    kCGLPFASampleAlpha,
    kCGLPFANoRecovery,
    kCGLPFAAccelerated,
    kCGLPFAClosestPolicy,
    kCGLPFABackingStore,
    kCGLPFAAllowOfflineRenderers,
    kCGLPFAAcceleratedCompute
};


std::vector<_CGLPixelFormatAttribute> createPixelFormatAttributeList(const Context* context) {
    std::map<_CGLPixelFormatAttribute, int> attributes;

    const auto& pixelFormat = context->pixelFormat();

    attributes[kCGLPFADoubleBuffer] = pixelFormat.doubleBuffer() ? GL_TRUE : GL_FALSE;
    attributes[kCGLPFAStereo] = pixelFormat.stereo() ? GL_TRUE : GL_FALSE;

    if (pixelFormat.colorBits() > 0) {
        attributes[kCGLPFAColorSize] = pixelFormat.colorBits() / 4 * 3; // factor out alpha size
        attributes[kCGLPFAAlphaSize] = pixelFormat.colorBits() / 4;
    } else {
        throw std::system_error(Error::UNSUPPORTED_ATTRIBUTE, "Separate RED, GREEN, BLUE buffer sizes are not supported on OS X");
    }

    attributes[kCGLPFADepthSize] = pixelFormat.depthBits();
    attributes[kCGLPFAStencilSize] = pixelFormat.stencilBits();

    for (const auto& attribute : pixelFormat.attributes()) {
        attributes[static_cast<_CGLPixelFormatAttribute>(attribute.first)] = attribute.second;
    }


    if (context->version().first >= 4) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL4_Core;
    } else if (context->version().first >= 3) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL3_Core;
    } else {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_Legacy;
    }


    std::vector<_CGLPixelFormatAttribute> list;
    list.reserve(attributes.size() * 2 + 1);
    for (const auto& attribute : attributes) {
        if (k_booleanAttributes.find(static_cast<_CGLPixelFormatAttribute>(attribute.first)) != k_booleanAttributes.end()) {
            if (attribute.second == GL_TRUE) {
                list.push_back(attribute.first);
            }
        } else {
            list.push_back(attribute.first);
            list.push_back(static_cast<_CGLPixelFormatAttribute>(attribute.second));
        }
    }
    list.push_back(static_cast<_CGLPixelFormatAttribute>(0)); // finalize list

    return std::move(list);
}


} // unnamed namespace


Context Implementation::currentContext() {
    const auto contextHandle = CGLGetCurrentContext();
    if (contextHandle == nullptr) {
        throw std::system_error(Error::CONTEXT_NOT_CURRENT, "CGLGetCurrentContext return nullptr");
    }

    const auto pixelFormat = CGLGetPixelFormat(contextHandle);
    if (pixelFormat == nullptr) {
        throw std::system_error(Error::CONTEXT_NOT_CURRENT, "CGLGetPixelFormat return nullptr");
    }

    Context context;
    context.implementation()->setExternal(contextHandle, pixelFormat);

    return std::move(context);
}


Implementation::Implementation()
: m_context(nullptr)
, m_pixelFormat(nullptr)
, m_owning(true) {
}


Implementation::Implementation(Implementation&& other) {
    *this = std::move(other);
}


Implementation::~Implementation() {
    if (m_owning) {
        if (m_context != nullptr) {
            CGLReleaseContext(m_context);
        }
        if (m_pixelFormat != nullptr) {
            CGLReleasePixelFormat(m_pixelFormat);
        }
    }
}


void Implementation::create(Context* context) {
    setPixelFormat(context);
    createContext();
}


void Implementation::create(Context* context, const Context* shared) {
    setPixelFormat(context);
    createContext(shared->implementation()->m_context);
}


void Implementation::setPixelFormat(Context *context) {
    const auto pixelFormatAttributes = createPixelFormatAttributeList(context);

    GLint numVirtualScreens;
    const auto error = CGLChoosePixelFormat(pixelFormatAttributes.data(), &m_pixelFormat, &numVirtualScreens);
    if (error != kCGLNoError) {
        throw std::system_error(Error::UNKNOWN_ERROR, CGLErrorString(error));
    }
}


void Implementation::createContext(CGLContextObj shared) {
    const auto error = CGLCreateContext(m_pixelFormat, shared, &m_context);
    if (error != kCGLNoError) {
        throw std::system_error(Error::UNKNOWN_ERROR, CGLErrorString(error));
    }
}


void Implementation::setExternal(CGLContextObj context, CGLPixelFormatObj pixelFormat) {
    m_context = context;
    m_pixelFormat = pixelFormat;
    m_owning = false;
}


Implementation& Implementation::operator=(Implementation&& other) {
    m_context = other.m_context;
    other.m_pixelFormat = nullptr;

    m_pixelFormat = other.m_pixelFormat;
    other.m_pixelFormat = nullptr;

    m_owning = other.m_owning;

    return *this;
}


void Implementation::makeCurrent(Context* context) noexcept {
    const auto error = CGLSetCurrentContext(m_context);
    assert(error == kCGLNoError);
}


void Implementation::doneCurrent(Context* context) noexcept {
    const auto error = CGLSetCurrentContext(nullptr);
    assert(error == kCGLNoError);
}


}  // namespace glheadless
