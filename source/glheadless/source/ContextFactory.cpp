#include "glheadless/ContextFactory.h"

#include "AbstractImplementation.h"


namespace glheadless {


std::unique_ptr<Context> ContextFactory::getCurrent() {
    return AbstractImplementation::instance()->getCurrent();
}

std::unique_ptr<Context> ContextFactory::create(const ContextFormat& format) {
    return AbstractImplementation::instance()->create(format);
}

std::unique_ptr<Context> ContextFactory::create(const Context* shared, const ContextFormat& format) {
    return AbstractImplementation::instance()->create(shared, format);
}


}