#pragma once

/*!
 * \file Context.h
 * \brief Declares class Context.
 */


#include <glheadless/glheadless_api.h>

#include <memory>
#include <system_error>
#include <thread>
#include <map>

#include <glheadless/ExceptionTrigger.h>


/*!
 * \brief Contains the public interface of the glheadless library.
 */
namespace glheadless {


class Implementation;


/*!
 * \brief Describes the requested OpenGL profile (core or compatibility), if supported on the current platform.
 */
enum class ContextProfile : unsigned int {
    CORE,
    COMPATIBILITY
};


/*!
 * \brief Platform-independent headless OpenGL context representation.
 *
 * This class is a common abstraction for different context creation implementations based on various OS-specific APIs.
 * Currently supported: WGL (Windows), CGL (Mac OS X)
 *
 * To set up an OpenGL context, instantiate a Context object, configure its pixel format, version, profile and debug
 * context (or use the defaults) and then call create().
 *
 * \snippet basic-context/main.cpp Creating a context
 *
 * Then check the success by calling valid(), any errors can be retrieved using lastErrorCode() and lastErrorMessage().
 *
 * \snippet basic-context/main.cpp Checking for errors
 *
 * After successful creation, the new context can be made current using makeCurrent() and doneCurrent(). The context is
 * automatically destroyed at the end of the object lifetime.
 */
class GLHEADLESS_API Context {
public:
    /*!
     * \brief OpenGL version imformation (major, minor)
     */
    using Version = std::pair<int, int>;


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
    static Context currentContext();


public:
    /*!
     * \brief Creates a context with default configuration.
     *
     * The default configuration is: version 3.2, core profile, no debugging and using the default PixelFormat.
     */
    Context();
    Context(const Context&) = delete;
    Context(Context&& other);
    ~Context();

    /*!
     * \return the requested Version.
     */
    const Version& version() const;

    /*!
     * \brief Sets the requested version.
     *
     * Only takes effect before create() succeeds.
     */
    void setVersion(int major, int minor);

    /*!
     * \return the requested ContextProfile.
     */
    ContextProfile profile() const;

    /*!
     * \brief Sets the requested ContextProfile.
     *
     * Only takes effect before create() succeeds.
     */
    void setProfile(const ContextProfile profile);

    /*!
     * \return whether a debug context is requested.
     */
    bool debugContext() const;

    /*!
     * \brief Sets whether a debug context is requested.
     *
     * Only takes effect before create() succeeds.
     */
    void setDebugContext(const bool debugContext);

    /*!
     * \see setAttribute
     *
     * \return the raw attributes
     */
    const std::map<int, int>& attributes() const;

    /*!
     * \brief Sets a raw attribute.
     *
     * A raw attribute is a platform-dependent attribute that should be passed to the context creation function (e.g.,
     * wglCreateContextAttribs and glxCreateContextAttribs). Any attributes specified here override attributes derived
     * from the settings above.
     * Boolean attributes are specified with their name and GL_FALSE/GL_TRUE as value, regardless of the actual
     * operating system API.
     *
     * Only takes effect before create() succeeds.
     */
    void setAttribute(int name, int value);

    /*!
     * \brief Tries to create a context according to the settings configured above.
     *
     * \exception std::system_error if any error occurs and exception ExceptionTrigger::CREATE is enabled.
     *
     * \return true on success.
     */
    bool create();

    /*!
     * \brief Tries to create a shared context according to the settings configured above.
     *
     * Make sure `shared` is not current on any thread at the time of calling.
     * Use Context:currentContext() to get a handle to the current context for sharing.
     *
     * \param shared The context whose resources should be shared with the new context.
     *
     * \exception std::system_error if any error occurs and exception ExceptionTrigger::CREATE is enabled.
     *
     * \return true on success.
     */
    bool create(const Context& shared);

    /*!
     * \brief Destroys the context.
     *
     * Must be called from the same thread as create().
     *
     * \exception std::system_error if any error occurs and exception ExceptionTrigger::CREATE is enabled.
     *
     * \return true on success
     */
    bool destroy();

    /*!
     * \brief Makes this context current for the calling thread.
     *
     * \exception std::system_error if any error occurs and exception ExceptionTrigger::CHANGE_CURRENT is enabled.
     *
     * \return true on success.
     */
    bool makeCurrent();

    /*!
     * \brief Resets the current context for the calling thread.
     *
     * \exception std::system_error if any error occurs and exception ExceptionTrigger::CHANGE_CURRENT is enabled.
     *
     * \return true on success.
     */
    bool doneCurrent();

    /*!
     * \brief Checks if the context has been created successfully and is ready to use.
     *
     * If the context is not valid, use lastErrorCode() and lastErrorMessage() to check for any error that occured
     * during context creation.
     *
     * \return true if the context is ready to use
     */
    bool valid() const;

    /*!
     * \return an std::error_code describing the last error.
     */
    const std::error_code& lastErrorCode() const;

    /*!
     * \return a detailed message describing the last error.
     */
    const std::string& lastErrorMessage() const;

    /*!
     * \return the enabled ::glheadless::ExceptionTrigger mask
     */
    ExceptionTrigger exceptionTriggers() const;

    /*!
     * \brief Sets the enabled ExceptionTriggers.
     *
     * By default, no exceptions are thrown.
     */
    void setExceptionTriggers(ExceptionTrigger exceptions);

    /*!
     * \brief For internal use.
     *
     * \return The concrete implementation.
     */
    Implementation* implementation();

    /*!
     * \brief For internal use.
     *
     * \return The concrete implementation.
     */
    const Implementation* implementation() const;

    Context& operator=(const Context&) = delete;
    Context& operator=(Context&& other);


private:
    Version m_version;
    ContextProfile m_profile;
    bool m_debugContext;
    std::map<int, int> m_attributes;
    ExceptionTrigger m_exceptionTriggers;

    std::unique_ptr<Implementation> m_implementation;
};

    
}  // namespace glheadless
