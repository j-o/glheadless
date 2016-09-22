#pragma  once

#include <functional>
#include <unordered_map>
#include <memory>


#define GLHEADLESS_REGISTER_IMPLEMENTATION(api, clazz) \
namespace { static const auto _registered = AbstractImplementation::registerImplementation(#api, [] { return new clazz(); }); }


namespace glheadless {


class Context;
struct ContextFormat;


class AbstractImplementation {
public:
    static AbstractImplementation* create();

    using ImplementationFactory = std::function<AbstractImplementation*()>;
    static bool registerImplementation(const std::string& name, const ImplementationFactory& factory);


public:
    AbstractImplementation();
    virtual ~AbstractImplementation();

    virtual std::unique_ptr<Context> getCurrent() = 0;

    virtual std::unique_ptr<Context> create(const ContextFormat& format) = 0;
    virtual std::unique_ptr<Context> create(const Context* shared, const ContextFormat& format) = 0;

    virtual bool destroy() = 0;

    virtual bool valid() = 0;

    virtual bool makeCurrent() = 0;
    virtual bool doneCurrent() = 0;


protected:
    Context* m_context;


private:
    static std::unordered_map<std::string, ImplementationFactory> s_implementationFactories;
};


}  // namespace glheadless
