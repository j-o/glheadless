#pragma once

/*!
 * \file error.h
 * \brief Declares custom error codes for use with std::error_code.
 */

#include <glheadless/glheadless_api.h>

#include <system_error>
#include <string>


namespace glheadless {


/*!
 * \brief Error codes that may originate from the glheadless library itself.
 */
enum class Error : int {
    INVALID_THREAD_ACCESS = 1,  //!< A function was called on an object from a thread that does not own the object
    INVALID_CONTEXT,            //!< A context handle is invalid
    INVALID_CONFIGURATION       //!< The selected configuration is invalid or unsupported
};


/*!
 * \brief glheadless error category for use with std::error_code and std::error_condition.
 */
class GLHEADLESS_API ErrorCategory : public std::error_category {
public:
    ErrorCategory();

    virtual const char* name() const noexcept override;

    virtual std::string message(int condition) const override;
};


/*!
 * \brief Returns the glheadless error category instance.
 */
GLHEADLESS_API const std::error_category& glheadless_category();

/*!
 * \return an std::error_condition created form the error code and the glheadless error category.
 */
GLHEADLESS_API std::error_condition make_error_condition(Error error);

/*!
 * \return an std::error_code created form the error code and the glheadless error category.
 */
GLHEADLESS_API std::error_code make_error_code(Error error);


}  // namespace glheadless


namespace std {


template <>
struct is_error_condition_enum<::glheadless::Error> : public std::true_type {
};


}  // namespace std
