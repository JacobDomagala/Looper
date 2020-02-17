#include "Window.hpp"
#include "Common.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // When a user presses the escape key, we set the WindowShouldClose property to true,
    // closing the application

   printf("KEY - %d", key);

    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
       printf(" pressed\n", key);
       glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

Window::Window(uint32_t width, uint32_t height, const std::string& title)
   : m_width(WIDTH),
     m_height(HEIGHT),
     m_title(title),
     m_isRunning(true),
     m_projectionMatrix(glm::ortho(-WIDTH / 2.0f, WIDTH / 2.0f, HEIGHT / 2.0f, -HEIGHT / 2.0f, -1.0f, 1.0f))
{
	glfwSetErrorCallback(error_callback);

   assert(GLFW_TRUE == glfwInit());
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

   m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(), nullptr, nullptr);
   glfwMakeContextCurrent(m_pWindow);

   int major, minor;
   glGetIntegerv(GL_MAJOR_VERSION, &major);
   glGetIntegerv(GL_MINOR_VERSION, &minor);
   printf("%d.%d \n", major, minor);

   glewExperimental = GL_TRUE;

   if (glewInit() != GLEW_OK)
   {
      printf("GL ERROR!");
   }

   // During init, enable debug output
   glEnable              ( GL_DEBUG_OUTPUT );
   glDebugMessageCallback( MessageCallback, 0 );

   glfwSwapInterval(1);

   int tmpWidth, tmpHeight;
   glfwGetFramebufferSize(m_pWindow, &tmpWidth, &tmpHeight);
   printf("%d.%d \n", tmpWidth, tmpHeight);

   glViewport(0, 0, tmpWidth, tmpHeight);
   Clear(1,1,1,1);
   SwapBuffers();

   glfwSetKeyCallback(m_pWindow, key_callback);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_LINE_SMOOTH);
   glEnable(GL_MULTISAMPLE);
}

Window::~Window()
{
   m_isRunning = false;
   glfwTerminate();
}

void
Window::SetIcon(const std::string& file)
{
   // auto image   = SDL_RWFromFile(file.c_str(), "rb");
   // auto surface = SDL_LoadBMP_RW(image, 1);

   // SDL_SetWindowIcon(m_pWindow, surface);
   // SDL_FreeSurface(surface);
}

void
Window::Resize(uint32_t newWidth, uint32_t newHeight)
{
   // SDL_SetWindowSize(m_pWindow, newWidth, newHeight);

   // m_height = newHeight;
   // m_width  = newWidth;
}

void
Window::Clear(float r, float g, float b, float a)
{
  // printf("CLEAR!");
   glClearColor(r, g, b, a);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
Window::SwapBuffers()
{
//   printf("SWAP!");
   glfwSwapBuffers(m_pWindow);
   //Clear(0.5, 0.5, 0.5, 0.5);
}

void
Window::WrapMouse(bool choice)
{
   // SDL_SetWindowGrab(m_pWindow, static_cast<SDL_bool>(choice));
}

void
Window::ShowCursor(bool choice)
{
   // SDL_ShowCursor(choice);
}

glm::vec2
Window::GetCursorScreenPosition()
{
   double xpos, ypos;
   glfwGetCursorPos(m_pWindow, &xpos, &ypos);

   m_cursorPos = glm::vec2(xpos, ypos);
   m_cursorPos -= glm::vec2((WIDTH / 2.0f), (HEIGHT / 2.0f));
   glm::vec4 tmpCursor = m_projectionMatrix * glm::vec4(m_cursorPos, 0.0f, 1.0f);

   return glm::vec2(tmpCursor.x, tmpCursor.y);
}

glm::vec2
Window::GetCursorNormalized()
{
   double xpos, ypos;
   glfwGetCursorPos(m_pWindow, &xpos, &ypos);

   glm::vec2 center(m_width / 2.0f, m_height / 2.0f);

   xpos -= center.x;
   ypos -= center.y;

   float cursorX = xpos/ center.x;
   float cursorY = ypos / center.y;

   //printf("%f %f\n", cursorX, cursorY);
   return glm::vec2(cursorX, cursorY);
}

glm::vec2
Window::GetCursor()
{
   double xpos, ypos;
   glfwGetCursorPos(m_pWindow, &xpos, &ypos);
  // printf("%f %f\n", xpos, ypos);
   return glm::vec2(xpos, ypos);
}