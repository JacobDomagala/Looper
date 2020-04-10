
#include "Editor.hpp"

int
main(int /* argc */, char** /* argv */)
{
   nanogui::init();
   Editor editor({1920, 1080});
   editor.MainLoop();

   return EXIT_SUCCESS;
}
