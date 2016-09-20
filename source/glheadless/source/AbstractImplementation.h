#pragma  once

/*!
 * \file AbstractImplementation.h
 */


#include <system_error>
#include <string>

#include <glheadless/error.h>
#include <glheadless/Context.h>


namespace glheadless {


struct ContextFormat;


/*!
 * \brief Non-polymorphic base class for context implemetations.
 *
 * Implements handling of errors and ExceptionTriggers. Platform implementations are not required to inherit this class,
 * in which case they must provide the public methods (lastErrorCode() and lastErrorMessage()) themselves.
 */
class AbstractImplementation {
public:
    static AbstractImplementation* instance();


public:
    virtual std::unique_ptr<Context> getCurrent() = 0;

    virtual std::unique_ptr<Context> create(const ContextFormat& format) = 0;
    virtual std::unique_ptr<Context> create(const Context* shared, const ContextFormat& format) = 0;

    virtual bool destroy(Context* context) = 0;

    virtual bool valid(const Context* context) = 0;

    virtual bool makeCurrent(Context* context) = 0;
    virtual bool doneCurrent(Context* context) = 0;


protected:
    AbstractImplementation();
    virtual ~AbstractImplementation() = default;
};

}  // namespace glheadless
