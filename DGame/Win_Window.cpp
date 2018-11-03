#include <Win_Window.hpp>
#include <Common.hpp>
#include <gtc/matrix_transform.hpp>
#include <glew.h>

std::unordered_map< WPARAM, bool > Win_Window::keyMap;

Win_Window::Win_Window( HINSTANCE hInstance )
    : m_hInstance( hInstance )
    , m_projectionMatrix( glm::ortho( static_cast< float >( -WIDTH / 2.0f ),
                                      static_cast< float >( WIDTH / 2.0f ),
                                      static_cast< float >( HEIGHT / 2.0f ),
                                      static_cast< float >( -HEIGHT / 2.0f ),
                                      -1.0f, 1.0f ) )
    , m_isRunning( true )
{
}

Win_Window& Win_Window::GetInstance( )
{
    static Win_Window* window;
    if( window == nullptr )
    {
        //GetModuleHandle(0) for geting hInstance
        window = new Win_Window( GetModuleHandle( 0 ) );
    }

    return *window;
}

LRESULT CALLBACK Win_Window::MainWinProc( HWND hWind, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT returnVal = 0;
    switch( uMsg )
    {
    case WM_KEYDOWN:
    {
        keyMap[ wParam ] = true;
    }
    break;
    case WM_KEYUP:
    {
        keyMap[ wParam ] = false;
    }
    break;
    case WM_LBUTTONDOWN:
    {
        keyMap[ wParam ] = true;
    }
    break;
    case WM_LBUTTONUP:
    {
        keyMap[ wParam ] = false;
    }
    break;
    case WM_RBUTTONDOWN:
    {
        keyMap[ wParam ] = true;
    }
    break;
    case WM_RBUTTONUP:
    {
        keyMap[ wParam ] = true;
    }
    break;
    case WM_SIZE:
    {
    }
    break;
    case WM_DESTROY:
    {
    }
    break;
    default:
        returnVal = DefWindowProc( hWind, uMsg, wParam, lParam );
    }

    return returnVal;
}

void Win_Window::Createwindow( )
{
    m_windowClass = { 0 };
    //windowClass.style = CS_HREDRAW;
    m_windowClass.hInstance     = m_hInstance;
    m_windowClass.lpfnWndProc   = MainWinProc;
    m_windowClass.hCursor       = LoadCursorA( NULL, IDC_CROSS );
    m_windowClass.lpszClassName = L"className";

    RegisterClassW( &m_windowClass );

    DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
    RECT  rect        = { 0 };
    rect.right        = WIDTH;
    rect.bottom       = HEIGHT;
    AdjustWindowRectEx( &rect, windowStyle, NULL, NULL );

    m_windowHandle = CreateWindowExA( 0, "className", "windowName", windowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
                                      rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, m_hInstance, 0 );
    GetClientRect( m_windowHandle, &rect );
}

void Win_Window::SetUpOpenGL( )
{
    m_hdc = GetDC( m_windowHandle );

    PIXELFORMATDESCRIPTOR pixelDescriptor;
    pixelDescriptor.nSize      = sizeof( PIXELFORMATDESCRIPTOR );
    pixelDescriptor.nVersion   = 1;
    pixelDescriptor.iPixelType = PFD_TYPE_RGBA;
    pixelDescriptor.iLayerType = PFD_MAIN_PLANE;
    pixelDescriptor.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pixelDescriptor.cDepthBits = 0;
    pixelDescriptor.cColorBits = 32;
    pixelDescriptor.cAlphaBits = 8;

    int32_t               pixelformat = ChoosePixelFormat( m_hdc, &pixelDescriptor );
    PIXELFORMATDESCRIPTOR tmp;
    DescribePixelFormat( m_hdc, pixelformat, sizeof( tmp ), &tmp );
    SetPixelFormat( m_hdc, pixelformat, &tmp );
    m_hrc = wglCreateContext( m_hdc );
    wglMakeCurrent( m_hdc, m_hrc );

    if( glewInit( ) != GLEW_OK )
    {
        ShowError( "Can't initialize OpenGL!", "OpenGL error" );
    }

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_MULTISAMPLE );
    glViewport( 0, 0, WIDTH, HEIGHT );

    //glDisable(GL_DEPTH_TEST);
}

glm::vec2 Win_Window::GetCursor( )
{
    POINT cursor;

    GetCursorPos( &cursor );
    ScreenToClient( m_windowHandle, &cursor );

    m_cursorPos.x = static_cast< float >( cursor.x );
    m_cursorPos.y = static_cast< float >( cursor.y );

    return m_cursorPos;
}

glm::vec2 Win_Window::GetCursorScreenPosition( )
{
    POINT cursor;

    GetCursorPos( &cursor );
    ScreenToClient( m_windowHandle, &cursor );

    m_cursorPos = glm::vec2( cursor.x, cursor.y );
    m_cursorPos -= glm::vec2( static_cast< float >( WIDTH / 2.0f ), static_cast< float >( HEIGHT / 2.0f ) );

    glm::vec4 tmpCursor = m_projectionMatrix * glm::vec4( m_cursorPos, 0.0f, 1.0f );

    return glm::vec2( tmpCursor.x, tmpCursor.y );
}

glm::vec2 Win_Window::GetCursorNormalized( )
{
    POINT tmpCursor;
    GetCursorPos( &tmpCursor );
    ScreenToClient( m_windowHandle, &tmpCursor );

    glm::vec2 center( WIDTH / 2.0f, HEIGHT / 2.0f );

    tmpCursor.x -= static_cast< LONG >( center.x );
    tmpCursor.y -= static_cast< LONG >( center.y );

    float cursorX = tmpCursor.x / center.x;
    float cursorY = tmpCursor.y / center.y;

    return glm::vec2( cursorX, cursorY );
}

void Win_Window::ShutDown( )
{
    m_isRunning = false;
    DestroyWindow( m_windowHandle );
    exit( EXIT_SUCCESS );
}

void Win_Window::Swapwindow( )
{
    SwapBuffers( m_hdc );
}

void Win_Window::ShowError( const std::string& errorMessage, const std::string& errorTitle )
{
    MessageBoxExA( m_windowHandle, errorMessage.c_str( ), errorTitle.c_str( ), MB_OK, 0 );
    ShutDown( );
}
