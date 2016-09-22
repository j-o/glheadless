#include "AbstractImplementation.h"

#include <cassert>
#include <string>


namespace glheadless {


std::unordered_map<std::string, AbstractImplementation::ImplementationFactory> AbstractImplementation::s_implementationFactories;


AbstractImplementation* AbstractImplementation::create() {
    auto itr = s_implementationFactories.begin();
    assert(itr != s_implementationFactories.end() && "No implementation found");
    return itr->second();
}


bool AbstractImplementation::registerImplementation(const std::string& name, const ImplementationFactory& factory) {
    s_implementationFactories.emplace(name, factory);
    return true;
}


AbstractImplementation::AbstractImplementation()
: m_context(nullptr) {
}


AbstractImplementation::~AbstractImplementation() {
}


}  // namespace glheadless
