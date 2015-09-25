#pragma once

/*!
 * \file error.h
 * \brief Declares custom error codes for use with std::error_code.
 */


#include <system_error>
#include <string>


namespace glheadless {


/*!
 * \brief Error codes that may originate from the glheadless library itself.
 */
enum class Error : int {
    SUCCESS,                    //!< No error occured.
    CONTEXT_NOT_CURRENT,        //!< Context::currentContext() was called without a current context.
    PIXEL_FORMAT_UNAVAILABLE,   //!< No pixel format is available that satisfies the requested configuration.
    INVALID_THREAD_ACCESS       //!< A function was called on an object from a thread that does not own the object
};


/*!
 * \brief glheadless error category for use with std::error_code and std::error_condition.
 */
class ErrorCategory : public std::error_category {
public:
    ErrorCategory();

    virtual const char* name() const noexcept override;

    virtual std::string message(int condition) const override;
};


/*!
 * \brief Returns the glheadless error category instance.
 */
const std::error_category& glheadless_category();

/*!
 * \return an std::error_condition created form the error code and the glheadless error category.
 */
std::error_condition make_error_condition(Error error);

/*!
 * \return an std::error_code created form the error code and the glheadless error category.
 */
std::error_code make_error_code(Error error);


}  // namespace glheadless


namespace std {


template <>
struct is_error_condition_enum<::glheadless::Error> : public std::true_type {
};


template <>
struct is_error_code_enum<::glheadless::Error> : public std::true_type{
};


}  // namespace std
