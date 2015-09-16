#pragma once


namespace glheadless {


enum class ExceptionTrigger : unsigned int {
    NONE           = 0x0,
    CREATE         = 0x1,
    CHANGE_CURRENT = 0x2
};


constexpr ExceptionTrigger operator&(ExceptionTrigger x, ExceptionTrigger y);
constexpr ExceptionTrigger operator|(ExceptionTrigger x, ExceptionTrigger y);
constexpr ExceptionTrigger operator^(ExceptionTrigger x, ExceptionTrigger y);
constexpr ExceptionTrigger operator~(ExceptionTrigger x);
constexpr ExceptionTrigger& operator&=(ExceptionTrigger& x, ExceptionTrigger y);
constexpr ExceptionTrigger& operator|=(ExceptionTrigger& x, ExceptionTrigger y);
constexpr ExceptionTrigger& operator^=(ExceptionTrigger& x, ExceptionTrigger y);


}  // namespace glheadless


#include <glheadless/ExceptionTrigger.hpp>
