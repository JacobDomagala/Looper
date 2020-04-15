
#include "Editor.hpp"

#include <GLFW/glfw3.h>

int
main(int /* argc */, char** /* argv */)
{
   nanogui::init();
   
   const auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

   Editor editor({mode->width, mode->height});
   editor.MainLoop();
   
   return EXIT_SUCCESS;
}
