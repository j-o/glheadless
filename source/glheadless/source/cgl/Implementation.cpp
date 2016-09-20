#include "Implementation.h"

#include <vector>
#include <system_error>
#include <set>
#include <cassert>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#include <glheadless/Context.h>
#include <glheadless/error.h>
#include <glheadless/ContextFormat.h>

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


std::vector<_CGLPixelFormatAttribute> createPixelFormatAttributeList(const ContextFormat& format) {
    std::map<_CGLPixelFormatAttribute, int> attributes;

    attributes[kCGLPFAAccelerated] = GL_TRUE;
    attributes[kCGLPFAClosestPolicy] = GL_TRUE;

    if (format.versionMajor >= 4) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL4_Core;
    } else if (format.versionMajor >= 3) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL3_Core;
    } else if (format.versionMajor > 0) {
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


struct State {
    CGLContextObj contextHandle;
    CGLPixelFormatObj pixelFormatHandle;
    bool owning;

    State()
    : contextHandle(nullptr)
    , pixelFormatHandle(nullptr)
    , owning(true) {
    }
};


std::unique_ptr<Context> Implementation::getCurrent() {
    const auto state = new State;
    state->owning = false;
    auto context = std::make_unique<Context>(state);

    state->contextHandle = CGLGetCurrentContext();
    if (state->contextHandle == nullptr) {
        context->setError(Error::INVALID_CONTEXT, "CGLGetCurrentContext returned nullptr", ExceptionTrigger::CREATE);
        return context;
    }

    return context;
}


std::unique_ptr<Context> Implementation::create(const ContextFormat& format) {
    auto context = std::make_unique<Context>(new State);

    auto success = setPixelFormat(context.get(), format);
    if (!success) {
        return context;
    }

    createContext(context.get(), nullptr);
    return context;
}


std::unique_ptr<Context> Implementation::create(const Context* shared, const ContextFormat& format) {
    auto context = std::make_unique<Context>(new State);

    auto success = setPixelFormat(context.get(), format);
    if (!success) {
        return context;
    }

    createContext(context.get(), shared->state()->contextHandle);
    return context;
}


bool Implementation::destroy(Context* context) {
    if (context->state() == nullptr) {
        return true;
    }

    const auto state = context->state();
    if (state->contextHandle != nullptr) {
        CGLReleaseContext(state->contextHandle);
        state->contextHandle = nullptr;
    }
    if (state->pixelFormatHandle != nullptr) {
        CGLReleasePixelFormat(state->pixelFormatHandle);
        state->pixelFormatHandle = nullptr;
    }

    return true;
}


bool Implementation::valid(const Context* context) {
    return context->state()->contextHandle != nullptr
        && (!context->state()->owning || context->state()->pixelFormatHandle != nullptr);
}


bool Implementation::makeCurrent(Context* context) {
    const auto error = CGLSetCurrentContext(context->state()->contextHandle);
    if (error != kCGLNoError) {
        return context->setError(Error::INVALID_CONTEXT, "CGLSetCurrentContext failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::doneCurrent(Context* context) {
    const auto error = CGLSetCurrentContext(nullptr);
    if (error != kCGLNoError) {
        return context->setError(Error::INVALID_CONTEXT, "CGLSetCurrentContext failed", ExceptionTrigger::CHANGE_CURRENT);
    }
    return true;
}


bool Implementation::setPixelFormat(Context* context, const ContextFormat& format) {
    const auto pixelFormatAttributes = createPixelFormatAttributeList(format);

    GLint numVirtualScreens;
    const auto error = CGLChoosePixelFormat(pixelFormatAttributes.data(), &context->state()->pixelFormatHandle, &numVirtualScreens);
    if (error != kCGLNoError) {
        return context->setError(Error::INVALID_CONFIGURATION, "CGLChoosePixelFormat failed", ExceptionTrigger::CREATE);
    }

    return true;
}


bool Implementation::createContext(Context* context, CGLContextObj shared) {
    const auto error = CGLCreateContext(context->state()->pixelFormatHandle, shared, &context->state()->contextHandle);
    if (error != kCGLNoError) {
        return context->setError(Error::INVALID_CONFIGURATION, "CGLCreateContext failed", ExceptionTrigger::CREATE);
    }
    return true;
}


}  // namespace glheadless
