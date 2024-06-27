
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "renderer/renderer.hpp"

int main()
{
        renderer_t renderer = {};
        renderer_init(&renderer);

        

        renderer_draw(&renderer);
}