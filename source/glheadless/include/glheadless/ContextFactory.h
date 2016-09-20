#pragma once

#include <glheadless/glheadless_api.h>

#include <glheadless/ContextFormat.h>
#include <memory>


namespace glheadless {


class Context;


class GLHEADLESS_API ContextFactory {
public:
    /*!
     * \brief Captures the current context.
     *
     * The current context does not have be created through glheadless, any context can be captured, e.g., contexts
     * created through Qt or GLFW. The only requirement is that is was made current for the calling thread.
     *
     * The returned context will be "non-owning", i.e., it is only valid as long as the original context exists and will
     * not be destroyed at the end of the object lifetime.
     * Be careful when calling makeCurrent() on this context, as internal state of the original creating library might
     * be invalidated. Capturing the current context is only intended for creating a shared context via
     * create(const Context& shared).
     *
     * \exception std::system_error if any error occurs and exception ExceptionTrigger::CREATE is enabled.
     *
     * \return the current Context.
     */
    static std::unique_ptr<Context> getCurrent();

    /*!
     * \brief Creates a context with the specified format.
     */
    static std::unique_ptr<Context> create(const ContextFormat& format = ContextFormat());
    static std::unique_ptr<Context> create(const Context* shared, const ContextFormat& format = ContextFormat());
};


}  // namespace glheadless

