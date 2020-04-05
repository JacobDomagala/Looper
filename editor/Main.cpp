
#include "Editor.hpp"
#include "FileManager.hpp"
#include "Game.hpp"

int
main()
{
   try
   {
      nanogui::init();

      /* scoped variables */
      {
         nanogui::ref< Editor > app = new Editor();

         app->drawAll();
         app->setVisible(true);

         nanogui::mainloop();
      }

      nanogui::shutdown();
   }
   catch (const std::runtime_error& e)
   {
      std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
      return -1;
   }

   return 0;
}
