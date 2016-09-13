#pragma once

#include <system_error>
#include <string>

#include <glheadless/error.h>


namespace glheadless {


enum class ExceptionTrigger : unsigned int;


class InternalException : public std::system_error {
public:
    InternalException(Error code, const std::string& message, ExceptionTrigger trigger);
    InternalException(const std::error_code& code, const std::string& message, ExceptionTrigger trigger);

    const std::string& message() const;
    ExceptionTrigger trigger() const;


private:
    std::string m_message;
    ExceptionTrigger m_trigger;
};

    
}  // namespace glheadless
