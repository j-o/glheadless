#pragma once

/*!
 * \file Context.h
 * \brief Declares class Context.
 */


#include <memory>
#include <thread>

#include <glheadless/glheadless_api.h>
#include <glheadless/error.h>
#include <glheadless/ExceptionTrigger.h>


/*!
 * \brief Contains the public interface of the glheadless library.
 */
namespace glheadless {


/*!
 * \brief Opaque base class for platform-depedent implementations.
 */
class AbstractImplementation;


/*!
 * \brief Platform-independent headless OpenGL context representation.
 *
 * This class is a common abstraction for different context creation implementations based on various OS-specific APIs.
 * Currently supported: WGL (Windows), CGL (Mac OS X), GLX (Linux), EGL (Linux)
 *
 * To set up an OpenGL context, configure a ContextFormat (or use the default) and call ContextFactory::create()
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
     * \brief Do not call directly, use ContextFactory::create() instead.
     */
    explicit Context(AbstractImplementation* implementation);
    Context(const Context&) = delete;
    Context(Context&& other) = delete;
    ~Context();

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
     * \return true if the context is ready to use.
     */
    bool valid() const;

    /*!
     * \brief For internal use.
     *
     * Sets lastErrorCode and lastErrorMessage and triggers exception, if enabled.
     *
     * \return false, if code identifies an error.
     */
    bool setError(Error code, const std::string& message, ExceptionTrigger exceptionTrigger);

    /*!
    * \brief For internal use.
    *
    * Sets lastErrorCode and lastErrorMessage and triggers exception, if enabled.
    *
    * \return false, if code identifies an error.
    */
    bool setError(const std::error_code& code, const std::string& message, ExceptionTrigger exceptionTrigger);

    /*!
     * \return an std::error_code describing the last error.
     */
    const std::error_code& lastErrorCode() const;

    /*!
     * \return a detailed message describing the last error.
     */
    const std::string& lastErrorMessage() const;

    /*!
     * \return the enabled glheadless::ExceptionTrigger mask.
     */
    ExceptionTrigger exceptionTriggers() const;

    /*!
     * \brief Sets the enabled ExceptionTriggers.
     *
     * By default, no exceptions are thrown.
     */
    void setExceptionTriggers(ExceptionTrigger exceptions);

    /*!
     * \return the platform-dependent implementation (opaque).
     */
    AbstractImplementation* implementation();

    /*!
    * \return the platform-dependent implementation (opaque).
    */
    const AbstractImplementation* implementation() const;

    Context& operator=(const Context&) = delete;
    Context& operator=(Context&& other) = delete;


private:
    std::unique_ptr<AbstractImplementation> m_implementation; //!< platform-dependent implementation
    std::thread::id                         m_owningThread;   //!< id of the thread that created this context

    ExceptionTrigger m_exceptionTriggers; //!< enabled exception trigger mask
    std::error_code  m_lastErrorCode;     //!< last error code that occured, default: 0 (success)
    std::string      m_lastErrorMessage;  //!< detailed message of the last error, default: empty
};

    
}  // namespace glheadless
