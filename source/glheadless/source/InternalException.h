#pragma once

#include <system_error>
#include <string>

#include <glheadless/error.h>


namespace glheadless {


class InternalException : public std::system_error {
public:
    InternalException(Error code, const std::string& message);
    InternalException(const std::error_code& code, const std::string& message);

    const std::string& message() const;


private:
    std::string m_message;
};

    
}  // namespace glheadless
