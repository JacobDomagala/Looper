#include "Game.hpp"
#include "Timer.hpp"
#include "Window.hpp"
#include <GLFW/glfw3.h>

int main(int /* argc */, char ** /* argv */)
{
   Timer globalTimer;

   Window window(1024, 765, "WindowTitle");

   auto& game = Game::GetInstance();
   game.Init("GameInit.txt", &window);

   auto oldTime = globalTimer.GetGlobalTime();

   MSG msg;
   int32_t frames = 0;
   float frameTimer = 0.0f;
   int32_t framesLastSecond = 0;

   while (window.IsRunning())
   {

      glfwPollEvents();

      globalTimer.ToggleTimer();
      auto timeStamp = globalTimer.GetGlobalTime();

      if ((timeStamp - oldTime) > TARGET_TIME)
      {
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

         window.SwapBuffers();
         ++frames;
      }
      frameTimer += globalTimer.GetDeltaTime();
   }

   return EXIT_SUCCESS;
}
