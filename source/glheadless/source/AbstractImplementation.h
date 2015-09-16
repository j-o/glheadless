#pragma  once

#include <system_error>
#include <string>

#include <glheadless/error.h>
#include <glheadless/Context.h>


namespace glheadless {


class AbstractImplementation {


public:
    AbstractImplementation(const AbstractImplementation&) = delete;
    AbstractImplementation(AbstractImplementation&& other) = default;

    const std::error_code& lastErrorCode() const;
    const std::string& lastErrorMessage() const;

    AbstractImplementation& operator=(const AbstractImplementation&) = delete;
    AbstractImplementation& operator=(AbstractImplementation&& other) = default;


protected:
    AbstractImplementation(Context* context);
    ~AbstractImplementation() = default;

    bool setError(const std::error_code& code, const std::string& message, ExceptionTrigger exceptionTrigger);


protected:
    Context* m_context;
    std::error_code m_lastErrorCode;
    std::string m_lastErrorMessage;
};

}  // namespace glheadless
