#include "game.hpp"

int
main(int /* argc */, char** /* argv */)
{
   looper::Game game;
   game.Init("GameInit.txt");
   game.MainLoop();

   return EXIT_SUCCESS;
}
