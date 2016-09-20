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

#include <glheadless/error.h>
#include <glheadless/ExceptionTrigger.h>


/*!
 * \brief Contains the public interface of the glheadless library.
 */
namespace glheadless {


struct State;


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
    Context(State* state);
    Context(const Context&) = delete;
    Context(Context&& other);
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
     * \return true if the context is ready to use
     */
    bool valid() const;

    bool setError(Error code, const std::string& message, ExceptionTrigger exceptionTrigger);
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
     * \return the enabled ::glheadless::ExceptionTrigger mask
     */
    ExceptionTrigger exceptionTriggers() const;

    /*!
     * \brief Sets the enabled ExceptionTriggers.
     *
     * By default, no exceptions are thrown.
     */
    void setExceptionTriggers(ExceptionTrigger exceptions);

    State* state();

    const State* state() const;

    Context& operator=(const Context&) = delete;
    Context& operator=(Context&& other);


private:
    State* m_state;

    ExceptionTrigger m_exceptionTriggers;
    std::error_code m_lastErrorCode;
    std::string m_lastErrorMessage;
};

    
}  // namespace glheadless
