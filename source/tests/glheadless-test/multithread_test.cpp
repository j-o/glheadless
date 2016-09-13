#include <future>

#include <gmock/gmock.h>

#include <glheadless/Context.h>
#include <glheadless/error.h>


using namespace glheadless;


class Multithread_Test : public testing::Test {

};


TEST_F(Multithread_Test, Create) {
    Context mainContext;

    mainContext.create();
    ASSERT_TRUE(mainContext.valid());

    auto ret = std::async(std::launch::async, [] (Context* mainContext) {
        Context workerContext;
        workerContext.create(*mainContext);
        return workerContext.valid();
    }, &mainContext);

    EXPECT_TRUE(ret.get());
}


TEST_F(Multithread_Test, MakeCurrent) {
    Context mainContext;

    mainContext.create();
    ASSERT_TRUE(mainContext.valid());

    const auto success = mainContext.makeCurrent();
    EXPECT_TRUE(success);

    auto ret = std::async(std::launch::async, [] (Context* mainContext) {
        Context workerContext;
        workerContext.create(*mainContext);
        return workerContext.makeCurrent();
    }, &mainContext);

    EXPECT_TRUE(ret.get());
}


TEST_F(Multithread_Test, InvalidAccess) {
    Context mainContext;

    mainContext.create();
    ASSERT_TRUE(mainContext.valid());

    auto ret = std::async(std::launch::async, [] (Context* mainContext) {
        mainContext->destroy();
        return mainContext->lastErrorCode();
    }, &mainContext);

    EXPECT_EQ(Error::INVALID_THREAD_ACCESS, ret.get());
}
