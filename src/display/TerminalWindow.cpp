#include "TerminalWindow.h"
#include "TextRenderer.h"

#include "glad/glad.h"

#include "glfw3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <iostream>

CTerminalWindow::CTerminalWindow() : m_pWindow(nullptr), m_dwWidth(0), m_dwHeight(0)
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
        pListener->OnDestroy();
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

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // OpenGL state
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    CTextRenderer TextRenderer;
    TextRenderer.Create();

    while (!glfwWindowShouldClose(m_pWindow))
    {
        // input

        // render
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        TextRenderer.SetWindowSize(m_dwWidth, m_dwHeight);

        TextRenderer.RenderText(L"Test text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        TextRenderer.RenderText(L"²âÊÔÎÄ±¾", 0.0f, 0.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}
