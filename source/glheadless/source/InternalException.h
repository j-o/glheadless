#pragma once

#include <system_error>


namespace glheadless {


enum class ExceptionTrigger : unsigned int;


class InternalException : public std::system_error {
public:
    InternalException(const std::error_code& code, const std::string& message, ExceptionTrigger trigger);

    const std::string& message() const;
    ExceptionTrigger trigger() const;


private:
    std::string m_message;
    ExceptionTrigger m_trigger;
};

    
}  // namespace glheadless
