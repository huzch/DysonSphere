#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "ComputeParticles.h"
#include <iostream>
#include <chrono>

void errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ComputeParticles* app = static_cast<ComputeParticles*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->handleKey(key, action);
    }
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    ComputeParticles* app = static_cast<ComputeParticles*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->reshape(width, height);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ComputeParticles* app = static_cast<ComputeParticles*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->handleMouseButton(button, action, mods);
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    ComputeParticles* app = static_cast<ComputeParticles*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->handleMouseMove(xpos, ypos);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ComputeParticles* app = static_cast<ComputeParticles*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->handleScroll(xoffset, yoffset);
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwSetErrorCallback(errorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "ParticleSystem", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if (gl3wInit() != 0) {
        std::cerr << "Failed to initialize OpenGL loader" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
  
    // Create application
    ComputeParticles app;
    if (!app.init(window)) {
        std::cerr << "Failed to initialize application" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    app.reshape(width, height);
    
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    app.handleMouseMove(mouseX, mouseY);
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
        std::cout << "\n控制说明:" << std::endl;
        std::cout << "  SPACE - 切换动画开关" << std::endl;
        std::cout << "  A - 切换吸引子开关" << std::endl;
        std::cout << "  R - 重置粒子" << std::endl;
        std::cout << "  左键拖动 - 旋转相机" << std::endl;
        std::cout << "  右键拖动 - 平移相机" << std::endl;
        std::cout << "  鼠标滚轮 - 缩放" << std::endl;
        std::cout << "  ESC - 退出程序" << std::endl;
    
    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        app.draw(deltaTime);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}

