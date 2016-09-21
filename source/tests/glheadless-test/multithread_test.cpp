#include <future>

#include <gmock/gmock.h>

#include <glheadless/Context.h>
#include <glheadless/ContextFactory.h>

#include "AbstractImplementation.h"


using namespace glheadless;


class Multithread_Test : public testing::Test {
};
using Multithread_DeathTest = Multithread_Test;


TEST_F(Multithread_Test, Create) {
    auto mainContext = ContextFactory::create();
    ASSERT_TRUE(mainContext->valid());

    auto ret = std::async(std::launch::async, [] (Context* mainContext) {
        auto workerContext = ContextFactory::create(mainContext);
        return workerContext->valid();
    }, mainContext.get());

    EXPECT_TRUE(ret.get());
}


TEST_F(Multithread_Test, MakeCurrent) {
    auto mainContext = ContextFactory::create();
    ASSERT_TRUE(mainContext->valid());

    const auto success = mainContext->makeCurrent();
    EXPECT_TRUE(success);

    auto ret = std::async(std::launch::async, [] (Context* mainContext) {
        auto workerContext = ContextFactory::create(mainContext);
        return workerContext->makeCurrent();
    }, mainContext.get());

    EXPECT_TRUE(ret.get());
}


TEST_F(Multithread_DeathTest, InvalidThreadAccess) {
#if defined(_WIN32)
    auto message = "Assertion failed: m_owningThread == std::this_thread::get_id\\(\\) && \"a context must be destroyed on the same thread that created it\".*";
#elif defined(__APPLE__)
#elif defined(__linux__)
    auto message = "glheadless::Context::~Context\\(\\): Assertion `m_owningThread == std::this_thread::get_id\\(\\) && \"a context must be destroyed on the same thread that created it\"' failed.";
#endif
    EXPECT_DEBUG_DEATH({
        auto ret = std::async(std::launch::async, [] { return ContextFactory::create(); });
        auto context = ret.get();
        context = nullptr;
    }, message);
}
