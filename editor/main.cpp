
#include "editor.hpp"

int
main(int /* argc */, char** /* argv */)
{
   // looper::Editor editor(looper::USE_DEFAULT_SIZE);
   looper::Editor editor({1920, 1080});
   editor.MainLoop();

   return EXIT_SUCCESS;
}
