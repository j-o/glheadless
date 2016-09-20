#if defined(WIN32)
#include <Windows.h>
#include <gl/GL.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#endif

#include <gmock/gmock.h>

#include <glheadless/Context.h>


using namespace glheadless;


class BasicContext_Test : public testing::Test {
    
};


TEST_F(BasicContext_Test, Initialization) {
    Context context;

    EXPECT_FALSE(context.valid());
    EXPECT_FALSE(context.lastErrorCode());
}


TEST_F(BasicContext_Test, Create32) {
    Context context;

    context.setVersion(3, 2);
    context.create();
    EXPECT_TRUE(context.valid());
    EXPECT_FALSE(context.lastErrorCode());
}


TEST_F(BasicContext_Test, CreateDefault) {
    Context context;

    context.create();
    EXPECT_TRUE(context.valid());
    EXPECT_FALSE(context.lastErrorCode());
}


TEST_F(BasicContext_Test, Destroy) {
    Context context;

    context.create();
    ASSERT_TRUE(context.valid());
    ASSERT_FALSE(context.lastErrorCode());

    context.destroy();
    EXPECT_FALSE(context.valid());
    EXPECT_FALSE(context.lastErrorCode());
}


TEST_F(BasicContext_Test, MakeCurrent) {
    Context context;

    context.create();
    ASSERT_TRUE(context.valid());

    const auto success = context.makeCurrent();
    EXPECT_TRUE(success);
    EXPECT_FALSE(context.lastErrorCode());
}


TEST_F(BasicContext_Test, DoneCurrent) {
    Context context;

    context.create();
    ASSERT_TRUE(context.valid());

    auto success = context.makeCurrent();
    ASSERT_TRUE(success);

    success = context.doneCurrent();
    EXPECT_TRUE(success);
    EXPECT_FALSE(context.lastErrorCode());
}


TEST_F(BasicContext_Test, GetVersion) {
    Context context;

    context.create();
    ASSERT_TRUE(context.valid());

    auto success = context.makeCurrent();
    ASSERT_TRUE(success);

    const auto versionStringRaw = glGetString(GL_VERSION);
    ASSERT_NE(nullptr, versionStringRaw);

    const auto versionString = std::string(reinterpret_cast<const char*>(versionStringRaw));
    EXPECT_NE(0, versionString.size());
}
