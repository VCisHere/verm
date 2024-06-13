#include "TerminalWindow.h"
#include "FontLoader.h"
#include "Shader.h"

#include "glad/glad.h"

#include "glfw3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <iostream>

struct Character
{
    GLuint TextureID;   // 字形纹理的ID
    glm::ivec2 Size;    // 字形大小
    glm::ivec2 Bearing; // 从基准线到字形左部/顶部的偏移值
    GLuint Advance;     // 原点距下一个字形原点的距离
};

GLuint VAO, VBO;
CFontLoader FontLoader;

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

    FontLoader.Create("./res/font/simsun.ttc"); // todo

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
    glViewport(0, 0, nWidth, nHeight);
}

void CTerminalWindow::WindowCloseCallback(GLFWwindow* pWindow)
{
    CTerminalWindow* pThis = reinterpret_cast<CTerminalWindow*>(glfwGetWindowUserPointer(pWindow));
    pThis->WindowCloseCallback();
}

void CTerminalWindow::WindowCloseCallback()
{
    for (std::set<CWindowListener*>::iterator Iter = m_pListenerSet.begin(); Iter != m_pListenerSet.end(); ++Iter)
    {
        CWindowListener* pListener = *Iter;
        pListener->OnDestroy();
    }
}

void RenderText(CShader& Shader, std::wstring text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    Shader.Use();
    glUniform3f(glGetUniformLocation(Shader.GetID(), "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    std::wstring::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        CFontLoader::GLYPH Glyph;
        FontLoader.GetData(*c, Glyph);

        uint32_t dwTextureID;
        glGenTextures(1, &dwTextureID);
        glBindTexture(GL_TEXTURE_2D, dwTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Glyph.dwWidth, Glyph.dwHeight, 0, GL_RED, GL_UNSIGNED_BYTE, Glyph.pData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character ch = {dwTextureID, glm::ivec2(Glyph.dwWidth, Glyph.dwHeight), glm::ivec2(Glyph.dwLeft, Glyph.dwTop),
                        static_cast<unsigned int>(Glyph.dwAdvanceX)};

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        GLfloat vertices[6][4] = {{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},
                                  {xpos + w, ypos, 1.0, 1.0}, {xpos, ypos + h, 0.0, 0.0},
                                  {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale; // 2 << 6 = 64
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
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

    CShader Shader("./res/shader/text.vs", "./res/shader/text.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(m_dwWidth), 0.0f, static_cast<float>(m_dwHeight));
    Shader.Use();
    glUniformMatrix4fv(glGetUniformLocation(Shader.GetID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // configure VAO/VBO for texture quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(m_pWindow))
    {
        // input

        // render
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        RenderText(Shader, L"This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        RenderText(Shader, L"测试文本", 0.0f, 0.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}
