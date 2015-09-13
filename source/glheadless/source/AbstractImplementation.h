#pragma  once

#include <system_error>
#include <string>

#include <glheadless/error.h>


namespace glheadless {


class AbstractImplementation {


public:
    AbstractImplementation(const AbstractImplementation&) = delete;
    AbstractImplementation(AbstractImplementation&& other) = default;

    const std::error_code& lastErrorCode() const;
    const std::string& lastErrorMessage() const;
    void setErrorCallback(const ErrorCallback& callback);

    AbstractImplementation& operator=(const AbstractImplementation&) = delete;
    AbstractImplementation& operator=(AbstractImplementation&& other) = default;


protected:
    AbstractImplementation();
    ~AbstractImplementation() = default;

    bool setError(const std::error_code& code, const std::string& message);


private:
    std::error_code m_lastErrorCode;
    std::string m_lastErrorMessage;
    ErrorCallback m_errorCallback;
};

}  // namespace glheadless
