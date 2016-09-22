
#include <gmock/gmock.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int main(int argc, char* argv[])
{
    glfwInit();
    ::testing::InitGoogleMock(&argc, argv);
    auto ret = RUN_ALL_TESTS();
    glfwTerminate();
    return ret;
}
