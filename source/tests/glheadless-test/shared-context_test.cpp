#include <gmock/gmock.h>

#include <glheadless/Context.h>
#include <glheadless/ContextFactory.h>


using namespace glheadless;


class SharedContext_Test : public testing::Test {

};


TEST_F(SharedContext_Test, Create) {
    auto context1 = ContextFactory::create();
    ASSERT_TRUE(context1->valid());

    auto context2 = ContextFactory::create(context1.get());
    EXPECT_TRUE(context2->valid());
    EXPECT_FALSE(context2->lastErrorCode());
}


TEST_F(SharedContext_Test, MakeCurrent) {
    auto context1 = ContextFactory::create();
    ASSERT_TRUE(context1->valid());

    auto context2 = ContextFactory::create(context1.get());
    ASSERT_TRUE(context2->valid());

    auto success = context1->makeCurrent();
    EXPECT_TRUE(success);

    success = context2->makeCurrent();
    EXPECT_TRUE(success);
}


TEST_F(SharedContext_Test, Capture) {
    auto context1 = ContextFactory::create();
    ASSERT_TRUE(context1->valid());

    auto success = context1->makeCurrent();
    ASSERT_TRUE(success);

    auto context2 = ContextFactory::getCurrent();
    EXPECT_TRUE(context2->valid());
}


TEST_F(SharedContext_Test, DestroyCaptured) {
    auto context1 = ContextFactory::create();
    ASSERT_TRUE(context1->valid());

    auto success = context1->makeCurrent();
    ASSERT_TRUE(success);

    auto context2 = ContextFactory::getCurrent();
    ASSERT_TRUE(context2->valid());

    context2 = nullptr;

    success = context1->makeCurrent();
    EXPECT_TRUE(success);
}
