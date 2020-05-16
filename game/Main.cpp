#include "Game.hpp"

int
main(int /* argc */, char** /* argv */)
{
   dgame::Game game;
   game.Init("GameInit.txt");
   game.MainLoop();

   return EXIT_SUCCESS;
}
