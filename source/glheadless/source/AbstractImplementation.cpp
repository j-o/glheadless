#include "AbstractImplementation.h"

#include "cgl/Implementation.h"


namespace glheadless {


AbstractImplementation* AbstractImplementation::instance() {
    static Implementation implementation;
    return &implementation;
}


AbstractImplementation::AbstractImplementation() {
}


}  // namespace glheadless
