
#include "Editor.hpp"

#include <GLFW/glfw3.h>

int
main(int /* argc */, char** /* argv */)
{
   nanogui::init();
   
   //const auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

   Editor editor({1920, 1080});
   editor.MainLoop();
   
   return EXIT_SUCCESS;
}
