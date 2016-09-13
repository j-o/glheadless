#include <gmock/gmock.h>

#include <glheadless/Context.h>


using namespace glheadless;


class SharedContext_Test : public testing::Test {

};


TEST_F(SharedContext_Test, Create) {
    Context context1;

    context1.create();
    ASSERT_TRUE(context1.valid());

    Context context2;
    context2.create(context1);
    EXPECT_TRUE(context2.valid());
    EXPECT_FALSE(context2.lastErrorCode());
}


TEST_F(SharedContext_Test, MakeCurrent) {
    Context context1;

    context1.create();
    ASSERT_TRUE(context1.valid());

    Context context2;
    context2.create(context1);
    ASSERT_TRUE(context2.valid());

    auto success = context1.makeCurrent();
    EXPECT_TRUE(success);

    success = context2.makeCurrent();
    EXPECT_TRUE(success);
}


TEST_F(SharedContext_Test, Capture) {
    Context context1;

    context1.create();
    ASSERT_TRUE(context1.valid());

    auto success = context1.makeCurrent();
    ASSERT_TRUE(success);

    auto context2 = Context::currentContext();
    EXPECT_TRUE(context2.valid());
}


TEST_F(SharedContext_Test, DestroyCaptured) {
    Context context1;

    context1.create();
    ASSERT_TRUE(context1.valid());

    auto success = context1.makeCurrent();
    ASSERT_TRUE(success);

    auto context2 = Context::currentContext();
    ASSERT_TRUE(context2.valid());

    context2.destroy();
    EXPECT_TRUE(context1.valid());

    success = context1.makeCurrent();
    EXPECT_TRUE(success);
}
