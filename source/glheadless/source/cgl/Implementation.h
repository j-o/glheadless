#pragma once

#include <system_error>
#include <string>

#include <OpenGL/CGLTypes.h>


namespace glheadless {


class Context;


class Implementation {
public:
    static Context currentContext();


public:
    Implementation();
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&& other);
    ~Implementation();

    bool create(Context* context);
    bool create(Context* context, const Context* shared);

    void makeCurrent() noexcept;
    void doneCurrent() noexcept;

    bool valid() const;
    const std::error_code& lastErrorCode() const;
    const std::string& lastErrorMessage() const;

    Implementation& operator=(const Implementation&) = delete;
    Implementation& operator=(Implementation&& other);


private:
    bool setPixelFormat(Context* context);
    bool createContext(CGLContextObj shared = nullptr);
    void setExternal(CGLContextObj context, CGLPixelFormatObj pixelFormat);

    bool setError(const std::error_code& code, const std::string& message);


private:
    CGLContextObj m_context;
    CGLPixelFormatObj m_pixelFormat;
    bool m_owning;
    std::error_code m_lastErrorCode;
    std::string m_lastErrorMessage;
};

    
}  // namespace glheadless
