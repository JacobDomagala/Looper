
#include "editor.hpp"

int
main(int /* argc */, char** /* argv */)
{
   looper::Editor editor(looper::USE_DEFAULT_SIZE);
   editor.MainLoop();

   return EXIT_SUCCESS;
}
