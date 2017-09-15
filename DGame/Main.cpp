#include "Win_Window.h"
#include "Timer.h"
#include "Game.h"

Timer* globalTimer;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Win_Window::GetInstance()->Createwindow();
	Win_Window::GetInstance()->SetUpOpenGL();
	
	globalTimer = new Timer();
	Game::GetInstance().Init();

	float oldTime = globalTimer->GetGlobalTime();
	
	MSG msg;
	float frames = 0.0f;
	float frameTimer = 0.0f;
	int framesLastSecond = 0;

	while (Win_Window::GetInstance()->IsRunning())
	{
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		globalTimer->ToggleTimer();
		float newTime = globalTimer->GetGlobalTime();
		
		if (newTime - oldTime > TARGET_TIME)
		{
			float dt = newTime - oldTime;
			Game::GetInstance().ProcessInput(dt);

			oldTime = newTime;
			Game::GetInstance().Render();
			if (frameTimer > 1.0f)
			{
				framesLastSecond = static_cast<int>(frames);
				frameTimer = 0.0f;
				frames = 0.0f;
			}
			Game::GetInstance().RenderText(std::to_string(framesLastSecond) + " FPS", glm::vec2(static_cast<float>(-WIDTH/2), static_cast<float>(-HEIGHT/2)), 0.4f, glm::vec3(1.0f, 0.0f, 1.0f));
			
			Win_Window::GetInstance()->Swapwindow();
			frames++;
		}
		frameTimer += globalTimer->GetDeltaTime();
	}

	delete(Win_Window::GetInstance());
	delete(globalTimer);

	return EXIT_SUCCESS;
}