#include "TerminalWindow.h"
#include "TextRenderer.h"
#include "WCUtils.h"

#include "glad/glad.h"

#include "glfw3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <iostream>

CTerminalWindow::CTerminalWindow() : m_pWindow(nullptr), m_dwWidth(0), m_dwHeight(0), m_pSession(nullptr)
{
}

CTerminalWindow::~CTerminalWindow()
{
}

bool CTerminalWindow::Create(uint32_t dwWidth, uint32_t dwHeight)
{
    m_dwWidth = dwWidth;
    m_dwHeight = dwHeight;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_renderThread = std::thread(&CTerminalWindow::RenderThread, this);

    return true;
}

void CTerminalWindow::Destroy()
{
    glfwSetWindowShouldClose(m_pWindow, 1);

    m_renderThread.join();

    glfwTerminate();
}

void CTerminalWindow::RegisterListener(CWindowListener* pListener)
{
    m_pListenerSet.insert(pListener);
}

void CTerminalWindow::UnregisterListener(CWindowListener* pListener)
{
    m_pListenerSet.erase(pListener);
}

void CTerminalWindow::FramebufferSizeCallback(GLFWwindow* pWindow, int nWidth, int nHeight)
{
    CTerminalWindow* pThis = reinterpret_cast<CTerminalWindow*>(glfwGetWindowUserPointer(pWindow));
    pThis->FramebufferSizeCallback(nWidth, nHeight);
}

void CTerminalWindow::WindowCloseCallback(GLFWwindow* pWindow)
{
    CTerminalWindow* pThis = reinterpret_cast<CTerminalWindow*>(glfwGetWindowUserPointer(pWindow));
    pThis->WindowCloseCallback();
}

void CTerminalWindow::OnScreenUpdate()
{
}

void CTerminalWindow::FramebufferSizeCallback(int nWidth, int nHeight)
{
    m_dwWidth = nWidth;
    m_dwHeight = nHeight;

    glViewport(0, 0, nWidth, nHeight);
}

void CTerminalWindow::WindowCloseCallback()
{
    for (std::set<CWindowListener*>::iterator Iter = m_pListenerSet.begin(); Iter != m_pListenerSet.end(); ++Iter)
    {
        CWindowListener* pListener = *Iter;
        pListener->OnWindowDestroy();
    }
}

void CTerminalWindow::RenderThread()
{
    m_pWindow = glfwCreateWindow(m_dwWidth, m_dwHeight, "verm", NULL, NULL);
    if (m_pWindow == NULL)
    {
        std::cout << "Fail to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_pWindow);
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, FramebufferSizeCallback);
    glfwSetWindowCloseCallback(m_pWindow, WindowCloseCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Fail to initialize GLAD" << std::endl;
        return;
    }

    // OpenGL state
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    CTextRenderer TextRenderer;
    TextRenderer.Create();

    CTerminalSession TerminalSession;
    TerminalSession.Create(80, 25);

    TerminalSession.Write("echo hello\r");

    while (!glfwWindowShouldClose(m_pWindow))
    {
        // input

        // render
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        TextRenderer.SetWindowSize(m_dwWidth, m_dwHeight);

        std::string strScreenBuffer = TerminalSession.GetScreenBuffer();

        uint32_t dwX = 0;
        uint32_t dwY = 1;
        std::u32string u32str = CWCUtils::UTF8ToUnicode(strScreenBuffer);
        for (size_t i = 0; i < u32str.size(); ++i)
        {
            char32_t code = u32str[i];
            uint32_t dwChWidth = CWCUtils::GetWideCharWidth(u32str[i]);
            bool bRender = TextRenderer.Render(dwX, dwY, u32str[i], glm::vec3(1, 1, 1));
            if (!bRender)
            {
                continue;
            }

            if (dwX + dwChWidth < 80)
            {
                dwX += dwChWidth;
            }
            else
            {
                dwX = 0;
                ++dwY;
            }
        }

        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }

    TerminalSession.Destroy();
    TextRenderer.Destroy();
}
