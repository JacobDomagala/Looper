#include "Game.hpp"
#include "Timer.hpp"
#include "Window.hpp"

int
main(int /* argc */, char** /* argv */)
{
   Timer globalTimer;

   Game game;
   game.Init("GameInit.txt");

   auto oldTime = globalTimer.GetGlobalTime();

   MSG msg;
   int32_t frames = 0;
   float frameTimer = 0.0f;
   int32_t framesLastSecond = 0;

   while (game.IsRunning())
   {
      game.PollEvents();

      globalTimer.ToggleTimer();
      auto timeStamp = globalTimer.GetGlobalTime();

      if ((timeStamp - oldTime) > TARGET_TIME)
      {
         game.SwapBuffers();

         float dt = (timeStamp - oldTime) * Timer::AreTimersRunning();
         game.ProcessInput(dt);

         oldTime = timeStamp;
         game.Render();
         if (frameTimer > 1.0f)
         {
            framesLastSecond = frames;
            frameTimer = 0.0f;
            frames = 0;
         }
         game.RenderText(std::to_string(framesLastSecond) + " FPS", glm::vec2(-WIDTH / 2.0f, -HEIGHT / 2.0f), 0.4f,
                         glm::vec3(1.0f, 0.0f, 1.0f));

         ++frames;
      }
      frameTimer += globalTimer.GetDeltaTime();
   }

   return EXIT_SUCCESS;
}
