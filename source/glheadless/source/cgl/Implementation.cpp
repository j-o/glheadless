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

    attributes[kCGLPFAAccelerated] = GL_TRUE;
    attributes[kCGLPFAClosestPolicy] = GL_TRUE;

    if (context->version().first >= 4) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL4_Core;
    } else if (context->version().first >= 3) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL3_Core;
    } else if (context->version().first > 0) {
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

    return list;
}


} // unnamed namespace


Context Implementation::currentContext() {
    Context context;

    const auto contextHandle = CGLGetCurrentContext();
    if (contextHandle == nullptr) {
        context.implementation()->setError(Error::INVALID_CONTEXT, "CGLGetCurrentContext returned nullptr", ExceptionTrigger::CREATE);
        return std::move(context);
    }

    const auto pixelFormat = CGLGetPixelFormat(contextHandle);
    if (pixelFormat == nullptr) {
        context.implementation()->setError(Error::INVALID_CONTEXT, "CGLGetPixelFormat returned nullptr", ExceptionTrigger::CREATE);
        return std::move(context);
    }

    context.implementation()->setExternal(contextHandle, pixelFormat);

    return std::move(context);
}


Implementation::Implementation(Context* context)
: AbstractImplementation(context)
, m_contextHandle(nullptr)
, m_pixelFormatHandle(nullptr)
, m_owning(true) {
}


Implementation::Implementation(Implementation&& other)
: AbstractImplementation(std::forward<AbstractImplementation>(other))
, m_contextHandle(other.m_contextHandle)
, m_pixelFormatHandle(other.m_pixelFormatHandle)
, m_owning(other.m_owning) {
    other.m_contextHandle = nullptr;
    other.m_pixelFormatHandle = nullptr;
}


Implementation::~Implementation() {
    destroy();
}


bool Implementation::create() {
    m_owningThread = std::this_thread::get_id();
    
    return setPixelFormat()
        && createContext();
}


bool Implementation::create(const Context* shared) {
    m_owningThread = std::this_thread::get_id();
    
    return setPixelFormat()
        && createContext(shared->implementation()->m_contextHandle);
}
    
    
bool Implementation::destroy() {
    if (m_owningThread != std::thread::id() && m_owningThread != std::this_thread::get_id()) {
        return setError(Error::INVALID_THREAD_ACCESS, "A context must be destroyed on the same thread that created it", ExceptionTrigger::CREATE);
    }

    if (m_owning) {
        if (m_contextHandle != nullptr) {
            CGLReleaseContext(m_contextHandle);
            m_contextHandle = nullptr;
        }
        if (m_pixelFormatHandle != nullptr) {
            CGLReleasePixelFormat(m_pixelFormatHandle);
            m_pixelFormatHandle = nullptr;
        }
    }

    m_owningThread = std::thread::id();

    return true;
}


bool Implementation::setPixelFormat() {
    const auto pixelFormatAttributes = createPixelFormatAttributeList(m_context);

    GLint numVirtualScreens;
    const auto error = CGLChoosePixelFormat(pixelFormatAttributes.data(), &m_pixelFormatHandle, &numVirtualScreens);
    if (error != kCGLNoError) {
        return setError(Error::INVALID_CONFIGURATION, "CGLChoosePixelFormat failed", ExceptionTrigger::CREATE);
    }

    return true;
}


bool Implementation::createContext(CGLContextObj shared) {
    const auto error = CGLCreateContext(m_pixelFormatHandle, shared, &m_contextHandle);
    if (error != kCGLNoError) {
        return setError(Error::INVALID_CONFIGURATION, "CGLCreateContext failed", ExceptionTrigger::CREATE);
    }
    return true;
}


void Implementation::setExternal(CGLContextObj context, CGLPixelFormatObj pixelFormat) {
    m_contextHandle = context;
    m_pixelFormatHandle = pixelFormat;
    m_owning = false;
}


Implementation& Implementation::operator=(Implementation&& other) {
    AbstractImplementation::operator=(std::forward<Implementation>(other));

    m_contextHandle = other.m_contextHandle;
    other.m_pixelFormatHandle = nullptr;

    m_pixelFormatHandle = other.m_pixelFormatHandle;
    other.m_pixelFormatHandle = nullptr;

    m_owning = other.m_owning;

    return *this;
}


bool Implementation::makeCurrent() noexcept {
    const auto error = CGLSetCurrentContext(m_contextHandle);
    if (error != kCGLNoError) {
        return setError(Error::INVALID_CONTEXT, "CGLSetCurrentContext failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::doneCurrent() noexcept {
    const auto error = CGLSetCurrentContext(nullptr);
    if (error != kCGLNoError) {
        return setError(Error::INVALID_CONTEXT, "CGLSetCurrentContext failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::valid() const {
    return !lastErrorCode()
        && m_contextHandle != nullptr
        && m_pixelFormatHandle != nullptr;
}


}  // namespace glheadless
