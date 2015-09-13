#include "Implementation.h"

#include <vector>
#include <system_error>
#include <set>
#include <cassert>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#include <glheadless/Context.h>
#include <glheadless/error.h>

#include "error.h"


namespace glheadless {


namespace {


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

const auto k_booleanAttributes = std::set<_CGLPixelFormatAttribute> {
    kCGLPFAAllRenderers,
    kCGLPFADoubleBuffer,
    kCGLPFAStereo,
    kCGLPFAMinimumPolicy,
    kCGLPFAMaximumPolicy,
    kCGLPFAOffScreen,
    kCGLPFAFullScreen,
    kCGLPFAAuxDepthStencil,
    kCGLPFAColorFloat,
    kCGLPFAMultisample,
    kCGLPFASupersample,
    kCGLPFASampleAlpha,
    kCGLPFASingleRenderer,
    kCGLPFANoRecovery,
    kCGLPFAAccelerated,
    kCGLPFAClosestPolicy,
    kCGLPFARobust,
    kCGLPFABackingStore,
    kCGLPFAMPSafe,
    kCGLPFAWindow,
    kCGLPFAMultiScreen,
    kCGLPFACompliant,
    kCGLPFAPBuffer,
    kCGLPFARemotePBuffer,
    kCGLPFAAllowOfflineRenderers,
    kCGLPFAAcceleratedCompute
};

#pragma GCC diagnostic pop


std::vector<_CGLPixelFormatAttribute> createPixelFormatAttributeList(const Context* context) {
    std::map<_CGLPixelFormatAttribute, int> attributes;

    const auto& pixelFormat = context->pixelFormat();

    attributes[kCGLPFAAccelerated] = GL_TRUE;
    attributes[kCGLPFAClosestPolicy] = GL_TRUE;
    attributes[kCGLPFADoubleBuffer] = pixelFormat.doubleBuffer() ? GL_TRUE : GL_FALSE;
    attributes[kCGLPFAStereo] = pixelFormat.stereo() ? GL_TRUE : GL_FALSE;
    attributes[kCGLPFAColorSize] = pixelFormat.redBits() + pixelFormat.greenBits() + pixelFormat.blueBits();
    attributes[kCGLPFAAlphaSize] = pixelFormat.alphaBits();
    attributes[kCGLPFADepthSize] = pixelFormat.depthBits();
    attributes[kCGLPFAStencilSize] = pixelFormat.stencilBits();

    if (context->version().first >= 4) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL4_Core;
    } else if (context->version().first >= 3) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL3_Core;
    } else {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_Legacy;
    }

    for (const auto& attribute : pixelFormat.attributes()) {
        attributes[static_cast<_CGLPixelFormatAttribute>(attribute.first)] = attribute.second;
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
        throw std::system_error(Error::CONTEXT_NOT_CURRENT, "CGLGetCurrentContext returned nullptr");
    }

    const auto pixelFormat = CGLGetPixelFormat(contextHandle);
    if (pixelFormat == nullptr) {
        throw std::system_error(Error::CONTEXT_NOT_CURRENT, "CGLGetPixelFormat returned nullptr");
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
        throw std::system_error(error, "CGLChoosePixelFormat failed");
    }
}


void Implementation::createContext(CGLContextObj shared) {
    const auto error = CGLCreateContext(m_pixelFormat, shared, &m_context);
    if (error != kCGLNoError) {
        throw std::system_error(error, "CGLCreateContext failed");
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
