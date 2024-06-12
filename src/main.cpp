#include "ConPty.h"
#include "FontLoader.h"
#include "Shader.h"

#include "ft2build.h"
#include FT_FREETYPE_H
// #include FT_GLYPH_H

#include "glad/glad.h"

#include "glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <cassert>
#include <codecvt>
#include <iostream>
#include <map>
#include <thread>

std::string UTF8ToGBK(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::wstring tmp_wstr = conv.from_bytes(str);
    // GBK locale name in windows
    const char* GBK_LOCALE_NAME = ".936";
    std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> convert(
        new std::codecvt_byname<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
    return convert.to_bytes(tmp_wstr);
}

void FramebufferSizeCallback(GLFWwindow* pWindow, int width, int height)
{
    glViewport(0, 0, width, height);
}

struct Character
{
    GLuint TextureID;   // 字形纹理的ID
    glm::ivec2 Size;    // 字形大小
    glm::ivec2 Bearing; // 从基准线到字形左部/顶部的偏移值
    GLuint Advance;     // 原点距下一个字形原点的距离
};

std::map<GLchar, Character> Characters;

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;
GLuint VAO, VBO;

CFontLoader FontLoader;

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

        // Character ch = Characters[*c];

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

void TestRenderText()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Fail to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Fail to initialize GLAD" << std::endl;
        return;
    }

    FontLoader.Create("simsun.ttc");

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // OpenGL state
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // compile and setup the shader
    CShader Shader("text.vs", "text.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
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

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        RenderText(Shader, L"This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        RenderText(Shader, L"测试文本", 0.0f, 0.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return;
}

void TestImGui()
{

    CPty* pPty = new CConPty();

    pPty->Create("C:\\windows\\system32\\cmd.exe", "", "", "", 80, 25);

    Sleep(1000);

    int64_t cnt = pPty->Write("echo hello\r\n");

    Sleep(1000);

    std::string strBuffer = pPty->ReadAll();
    if (!glfwInit())
    {
        return;
    }

    const char* glsl_version = "#version 130";

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3", nullptr, nullptr);
    if (window == nullptr)
    {
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    ImGui::CreateContext();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ************ GUI **************
        bool bOpen = true;
        ImGui::Begin("hello world", &bOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text(strBuffer.c_str());
        ImGui::End();
        // ************ GUI **************

        ImGui::Render();
        int nDisplayWidth = 0;
        int nDisplayHeight = 0;
        glfwGetFramebufferSize(window, &nDisplayWidth, &nDisplayHeight);
        glViewport(0, 0, nDisplayWidth, nDisplayHeight);

        const ImVec4 CLEAR_COLOR = ImVec4(0, 0, 0, 1);
        glClearColor(CLEAR_COLOR.x * CLEAR_COLOR.w, CLEAR_COLOR.y * CLEAR_COLOR.w, CLEAR_COLOR.z * CLEAR_COLOR.w,
                     CLEAR_COLOR.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    pPty->Destroy();
}

int main()
{
    TestRenderText();
}
