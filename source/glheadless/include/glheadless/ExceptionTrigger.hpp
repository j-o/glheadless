#pragma once

#include <glheadless/ExceptionTrigger.h>


namespace glheadless {


constexpr ExceptionTrigger operator&(ExceptionTrigger x, ExceptionTrigger y) {
    return static_cast<ExceptionTrigger>(
            static_cast<std::underlying_type<ExceptionTrigger>::type>(x) &
            static_cast<std::underlying_type<ExceptionTrigger>::type>(y));
}


constexpr ExceptionTrigger operator|(ExceptionTrigger x, ExceptionTrigger y) {
    return static_cast<ExceptionTrigger>(
            static_cast<std::underlying_type<ExceptionTrigger>::type>(x) |
            static_cast<std::underlying_type<ExceptionTrigger>::type>(y));
}


constexpr ExceptionTrigger operator^(ExceptionTrigger x, ExceptionTrigger y) {
    return static_cast<ExceptionTrigger>(
            static_cast<std::underlying_type<ExceptionTrigger>::type>(x) ^
            static_cast<std::underlying_type<ExceptionTrigger>::type>(y));
}


constexpr ExceptionTrigger operator~(ExceptionTrigger x) {
    return static_cast<ExceptionTrigger>(~static_cast<std::underlying_type<ExceptionTrigger>::type>(x));
}


inline ExceptionTrigger& operator&=(ExceptionTrigger& x, ExceptionTrigger y) {
    x = x & y;
    return x;
}


inline ExceptionTrigger& operator|=(ExceptionTrigger& x, ExceptionTrigger y) {
    x = x | y;
    return x;
}


inline ExceptionTrigger& operator^=(ExceptionTrigger& x, ExceptionTrigger y) {
    x = x ^ y;
    return x;
}


}  // namespace glheadless
