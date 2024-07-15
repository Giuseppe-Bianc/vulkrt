//
// Created by gbian on 15/07/2024.
//

#include "vulkrt/Window.hpp"
#include "vulkrt/timer/Timer.hpp"

namespace lve {
    Window::Window(const int w, const int h, const std::string_view &window_name) : width(w), height(h), windowName(window_name) {
        initWindow();
    }
    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    void Window::initWindow() {
        initializeGLFW();
        setHints();
        createWindow();
        centerWindow();
    }

    void Window::errorCallback(int error, const char *description) { LERROR("GLFW Error ({0}): {1}", error, description); }
    void Window::keyCallback(GLFWwindow *window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) {
        switch(key) {
        case GLFW_KEY_ESCAPE:
            if(action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                LINFO("escape close");
            }
            break;
        // Add more cases for other keys if needed
        [[likely]] default:
            // Handle other keys here
            break;
        }
    }
    void Window::createWindow() {
        vnd::AutoTimer timer("window creation");
        window = glfwCreateWindow(width, height, windowName.data(), nullptr, nullptr);
        if(!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window.");
        }
        glfwSetKeyCallback(window, keyCallback);
        // glfwSetWindowUserPointer(window, this);
        // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    }

    void Window::setHints() {
        vnd::AutoTimer timer("set glfw hints");
        // Set GLFW context version and profile
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
    void Window::initializeGLFW() {
        vnd::AutoTimer timer("glfw setup");
        if(!glfwInit()) {
            LCRITICAL("Failed to initialize GLFW");
            throw std::runtime_error("Failed to initialize GLFW.");
        }
        if(!glfwVulkanSupported()) {
            glfwTerminate();
            LCRITICAL("Failed to initialize GLFW. vulkan not supported");
            throw std::runtime_error("Failed to initialize GLFW. Vulkan not supported");
        }
        glfwSetErrorCallback(errorCallback);
    }

    std::string Window::formatMode(const GLFWvidmode *mode) {
        return FORMAT("({}x{}, Bits rgb{}{}{}, RR:{}Hz)", mode->width, mode->height, mode->redBits, mode->greenBits, mode->blueBits,
                      mode->refreshRate);
    }

    void Window::centerWindow() {
        vnd::Timer monitort("get primary Monitor");
        GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
        if(!primaryMonitor) {
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Failed to get the primary monitor.");
        }
        LINFO("{}", monitort);
        vnd::Timer modet("get mode");
        const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);
        if(!mode) {
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Failed to get the video mode of the primary monitor.");
        }
        LINFO("{}", modet);
        vnd::Timer crepositiont("calculating for reposition");
        const int monitorWidth = mode->width;
        const int monitorHeight = mode->height;
        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        auto centerX = CALC_CENTRO(monitorWidth, windowWidth);
        auto centerY = CALC_CENTRO(monitorHeight, windowHeight);
        LINFO("{}", crepositiont);
        vnd::Timer wrepositiont("window reposition");
        glfwSetWindowPos(window, centerX, centerY);
        int posX = 0, posY = 0;
        glfwGetWindowPos(window, &posX, &posY);
        if(posX != centerX || posY != centerY) {
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Failed to position the window at the center.");
        }
        LINFO("{}", wrepositiont);
        int xPos, yPos;
        float xScale, yScale;
        int monitorPhysicalWidth;
        int monitorPhysicalHeight;
        vnd::Timer tmonitorinfo("get monitor info");
        glfwGetMonitorPos(primaryMonitor, &xPos, &yPos);
        glfwGetMonitorContentScale(primaryMonitor, &xScale, &yScale);
        glfwGetMonitorPhysicalSize(primaryMonitor, &monitorPhysicalWidth, &monitorPhysicalHeight);
        LINFO("{}", tmonitorinfo);
        glfwShowWindow(window);
        LINFO("Monitor:\"{}\", Phys:{}x{}mm, Scale:({}/{}), Pos:({}/{})", glfwGetMonitorName(primaryMonitor), monitorPhysicalWidth,
              monitorPhysicalHeight, xScale, yScale, xPos, yPos);
        LINFO("Monitor Mode:{}", formatMode(mode));
        LINFO("created the window {0}: (w: {1}, h: {2}, pos:(x:{3}, y:{4}))", windowName.data(), width, height, centerX, centerY);
    }

    /*fs::path Window::calculateRelativePathToSrcRes(const fs::path &executablePath, const fs::path &targetFile) {
        // Get the parent directory of the executable path
        fs::path parentDir = executablePath.parent_path();

        // Traverse up the directory tree until we find a directory containing "src"
        while(!fs::exists(parentDir / "src")) {
            parentDir = parentDir.parent_path();
            // Check if we reached the root directory and "src" was not found
            if(parentDir == parentDir.root_path()) {
                std::cerr << "Error: 'src' directory not found in the path." << std::endl;
                return {};  // Return an empty path or handle error as needed
            }
        }

        // Move up one more level to reach the parent directory of "src"
        parentDir = parentDir.parent_path();
        auto resp = fs::path("src") / "engine" / "res";
        // Construct the relative path to the target file
        auto relativePathToTarget = parentDir / resp / targetFile;
        // Construct the path to the target file under "src/engine/res"

        // Calculate the relative path from the executable's directory
        auto relativePath = fs::relative(relativePathToTarget, executablePath);

        return relativePath.lexically_normal();
    }*/

}  // namespace lve