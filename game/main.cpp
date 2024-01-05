#include "game.hpp"

int
main(int /* argc */, char** /* argv */)
{
   looper::Game game;
   game.Init("GameInit.json");
   game.MainLoop();

   return EXIT_SUCCESS;
}
