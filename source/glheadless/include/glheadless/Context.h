#pragma once

#include <glheadless/glheadless_api.h>

#include <memory>
#include <system_error>

#include <glheadless/PixelFormat.h>
#include <glheadless/error.h>


namespace glheadless {


class Implementation;


enum class ContextProfile : unsigned int {
    CORE,
    COMPATIBILITY
};


enum class ExceptionMask : unsigned int {
    NONE           = 0x0,
    CREATE         = 0x1,
    CHANGE_CURRENT = 0x2
};

constexpr ExceptionMask operator&(ExceptionMask x, ExceptionMask y) {
    return static_cast<ExceptionMask>(
            static_cast<std::underlying_type<ExceptionMask>::type>(x) &
            static_cast<std::underlying_type<ExceptionMask>::type>(y));
}

constexpr ExceptionMask operator|(ExceptionMask x, ExceptionMask y) {
    return static_cast<ExceptionMask>(
            static_cast<std::underlying_type<ExceptionMask>::type>(x) |
            static_cast<std::underlying_type<ExceptionMask>::type>(y));
}

constexpr ExceptionMask operator^(ExceptionMask x, ExceptionMask y) {
    return static_cast<ExceptionMask>(
            static_cast<std::underlying_type<ExceptionMask>::type>(x) ^
            static_cast<std::underlying_type<ExceptionMask>::type>(y));
}

constexpr ExceptionMask& operator&=(ExceptionMask& x, ExceptionMask y) {
    x = x & y;
    return x;
}

constexpr ExceptionMask& operator|=(ExceptionMask& x, ExceptionMask y) {
    x = x | y;
    return x;
}

constexpr ExceptionMask& operator^=(ExceptionMask& x, ExceptionMask y) {
    x = x ^ y;
    return x;
}


class GLHEADLESS_API Context {
public:
    using Version = std::pair<int, int>;


public:
    static Context currentContext();


public:
    Context();
    Context(const Context&) = delete;
    Context(Context&& other);
    ~Context();

    const PixelFormat& pixelFormat() const;
    void setPixelFormat(const PixelFormat& pixelFormat);

    const Version& version() const;
    void setVersion(int major, int minor);

    ContextProfile profile() const;
    void setProfile(const ContextProfile profile);

    bool debugContext() const;
    void setDebugContext(const bool debugContext);

    const std::map<int, int>& attributes() const;
    void setAttribute(int name, int value);

    bool create();
    bool create(const Context& shared);

    void makeCurrent() noexcept;
    void doneCurrent() noexcept;

    bool valid() const;
    const std::error_code& lastErrorCode() const;
    const std::string& lastErrorMessage() const;

    void setExceptions(ExceptionMask exceptions);
    ExceptionMask exceptions() const;

    Implementation* implementation();
    const Implementation* implementation() const;

    Context& operator=(const Context&) = delete;
    Context& operator=(Context&& other);


private:
    PixelFormat m_pixelFormat;
    Version m_version;
    ContextProfile m_profile;
    bool m_debugContext;
    std::map<int, int> m_attributes;
    ExceptionMask m_exceptions;

    std::unique_ptr<Implementation> m_implementation;
};

    
}  // namespace glheadless
