#include <future>

#include <gmock/gmock.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glheadless/Context.h>
#include <glheadless/ContextFactory.h>

#include "AbstractImplementation.h"


using namespace glheadless;


class Glfw_Test : public testing::Test {
protected:
    virtual void SetUp() override {
        glfwDefaultWindowHints();
        m_window = glfwCreateWindow(320, 240, "", nullptr, nullptr);

        testing::Test::SetUp();
    }


    virtual void TearDown() override {
        glfwDestroyWindow(m_window);

        testing::Test::TearDown();
    }


    std::unique_ptr<Context> getCurrent() {
        glfwMakeContextCurrent(m_window);
        auto context = ContextFactory::getCurrent();
        glfwMakeContextCurrent(nullptr);
        return context;
    }


protected:
    GLFWwindow* m_window;
};


TEST_F(Glfw_Test, GetCurrent) {
    glfwMakeContextCurrent(m_window);

    auto context = ContextFactory::getCurrent();
    ASSERT_TRUE(context->valid());

    glfwMakeContextCurrent(nullptr);
}


TEST_F(Glfw_Test, Create) {
    auto mainContext = getCurrent();
    ASSERT_TRUE(mainContext->valid());

    auto ret = std::async(std::launch::async, [] (Context* mainContext) {
        auto workerContext = ContextFactory::create(mainContext);
        return workerContext->valid();
    }, mainContext.get());

    EXPECT_TRUE(ret.get());
}


TEST_F(Glfw_Test, MakeCurrent) {
    auto mainContext = getCurrent();
    ASSERT_TRUE(mainContext->valid());

    const auto success = mainContext->makeCurrent();
    EXPECT_TRUE(success);

    auto ret = std::async(std::launch::async, [] (Context* mainContext) {
        auto workerContext = ContextFactory::create(mainContext);
        return workerContext->makeCurrent();
    }, mainContext.get());

    EXPECT_TRUE(ret.get());
}
