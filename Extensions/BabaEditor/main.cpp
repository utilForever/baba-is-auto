// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include "LevelEditor.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>

namespace
{
//! Applies a custom theme to ImGui.
void ApplyTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 2.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.11f, 0.16f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.14f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.20f, 0.36f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.24f, 0.52f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.35f, 0.72f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.82f, 0.19f, 0.41f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.16f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.23f, 0.38f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.24f, 0.52f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.35f, 0.72f, 1.00f);
}

//! Centers the given window on the primary monitor.
void CenterWindow(GLFWwindow* window)
{
    // Use the monitor work area so the window does not overlap system bars.
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor == nullptr)
    {
        return;
    }

    int monitorX = 0;
    int monitorY = 0;
    int monitorWidth = 0;
    int monitorHeight = 0;
    glfwGetMonitorWorkarea(monitor, &monitorX, &monitorY, &monitorWidth,
                           &monitorHeight);

    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    windowWidth = std::min(windowWidth, monitorWidth);
    windowHeight = std::min(windowHeight, monitorHeight);
    glfwSetWindowSize(window, windowWidth, windowHeight);

    const int x = std::max(monitorX, monitorX + (monitorWidth - windowWidth) / 2);
    const int y =
        std::max(monitorY, monitorY + (monitorHeight - windowHeight) / 2);
    glfwSetWindowPos(window, x, y);
}
}  // namespace

int main()
{
    if (!glfwInit())
    {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    // macOS only exposes OpenGL 3.x through a forward-compatible core profile.
#ifdef __APPLE__
    const char* glslVersion = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
    const char* glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    GLFWwindow* window = glfwCreateWindow(
        1280, 720, "baba-is-auto Level Editor", nullptr, nullptr);
    if (window == nullptr)
    {
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }

    CenterWindow(window);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ApplyTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    {
        // The editor owns GL textures, so destroy it before the GL context.
        baba_is_auto::editor::LevelEditor editor;

        while (!editor.ShouldClose())
        {
            glfwPollEvents();

            if (glfwWindowShouldClose(window))
            {
                glfwSetWindowShouldClose(window, GLFW_FALSE);
                editor.RequestClose();
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            editor.Draw();

            ImGui::Render();

            // Framebuffer size accounts for HiDPI scaling unlike window size.
            int displayWidth = 0;
            int displayHeight = 0;
            glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
            glViewport(0, 0, displayWidth, displayHeight);
            glClearColor(0.09f, 0.11f, 0.16f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
