#pragma once

/*!
 * \file ExceptionTrigger.h
 * \brief Declares enum ExceptionTrigger.
 */


namespace glheadless {


/*!
 * \brief Bitmask-compatible enum describing various error situations.
 *
 * An ExceptionTrigger describes a situation in which an exception can be thrown. If a certain trigger is enabled,
 * the corresponding error situation will provoke an exception, otherwise the success or failure of an operation
 * will be signaled using a bool return value and the lastErrorCode() and lastErrorMessage() methods.
 *
 * \see Context::setExceptionTriggers().
 */
enum class ExceptionTrigger : unsigned int {
    NONE           = 0x0, //!< No exceptions will be thrown.
    CREATE         = 0x1, //!< Errors during context creation will raise an exception
    CHANGE_CURRENT = 0x2  //!< Errors during a context switch (makeCurrent/doneCurrent) will raise an exception
};


constexpr ExceptionTrigger operator&(ExceptionTrigger x, ExceptionTrigger y);
constexpr ExceptionTrigger operator|(ExceptionTrigger x, ExceptionTrigger y);
constexpr ExceptionTrigger operator^(ExceptionTrigger x, ExceptionTrigger y);
constexpr ExceptionTrigger operator~(ExceptionTrigger x);
inline ExceptionTrigger& operator&=(ExceptionTrigger& x, ExceptionTrigger y);
inline ExceptionTrigger& operator|=(ExceptionTrigger& x, ExceptionTrigger y);
inline ExceptionTrigger& operator^=(ExceptionTrigger& x, ExceptionTrigger y);


}  // namespace glheadless


#include <glheadless/ExceptionTrigger.hpp>
