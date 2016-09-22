#include <glheadless/ContextFactory.h>

#include "AbstractImplementation.h"

#include <glheadless/Context.h>


namespace glheadless {


std::unique_ptr<Context> ContextFactory::getCurrent() {
    auto implementation = AbstractImplementation::create();
    return implementation->getCurrent();
}

std::unique_ptr<Context> ContextFactory::create(const ContextFormat& format) {
    auto implementation = AbstractImplementation::create();
    return implementation->create(format);
}

std::unique_ptr<Context> ContextFactory::create(const Context* shared, const ContextFormat& format) {
    auto implementation = AbstractImplementation::create();
    return implementation->create(shared, format);
}


} // namespace glheadless
