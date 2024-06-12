#include "ConPty.h"

#include "glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cassert>
#include <codecvt>
#include <iostream>
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

int main()
{
    CPty* pPty = new CConPty();

    pPty->Create("C:\\windows\\system32\\cmd.exe", "", "", "", 80, 25);

    Sleep(1000);

    int64_t cnt = pPty->Write("echo hello\r\n");

    Sleep(1000);

    std::string strBuffer = pPty->ReadAll();
    if (!glfwInit())
    {
        return 1;
    }

    const char* glsl_version = "#version 130";

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3", nullptr, nullptr);
    if (window == nullptr)
    {
        return 1;
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
