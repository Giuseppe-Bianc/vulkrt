//
// Created by gbian on 15/07/2024.
//

#include "vulkrt/Window.hpp"
#include "vulkrt/timer/Timer.hpp"

namespace lve {
    DISABLE_WARNINGS_PUSH(26432 26447)
    Window::Window(const int w, const int h, const std::string_view &window_name) noexcept : width(w), height(h), windowName(window_name) {
        initWindow();
    }
    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    DISABLE_WARNINGS_POP()
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
    void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) noexcept {
        auto lveWindow = static_cast<Window *>(glfwGetWindowUserPointer(window));
        lveWindow->framebufferResized = true;
        lveWindow->width = width;
        lveWindow->height = height;
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

    void Window::setHints() const {
        vnd::AutoTimer timer("set glfw hints");
        // Set GLFW context version and profile
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
    void Window::initializeGLFW() const {
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

    std::string Window::formatMode(const GLFWvidmode *mode) const {
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
        int windowWidth;
        int windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        auto centerX = CALC_CENTRO(monitorWidth, windowWidth);
        auto centerY = CALC_CENTRO(monitorHeight, windowHeight);
        LINFO("{}", crepositiont);
        vnd::Timer wrepositiont("window reposition");
        glfwSetWindowPos(window, centerX, centerY);
        int posX = 0;
        int posY = 0;
        glfwGetWindowPos(window, &posX, &posY);
        if(posX != centerX || posY != centerY) {
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Failed to position the window at the center.");
        }
        LINFO("{}", wrepositiont);
        int xPos;
        int yPos;
        float xScale;
        float yScale;
        int monitorPhysicalWidth;
        int monitorPhysicalHeight;
        vnd::Timer tmonitorinfo("get monitor info");
        glfwGetMonitorPos(primaryMonitor, &xPos, &yPos);
        glfwGetMonitorContentScale(primaryMonitor, &xScale, &yScale);
        glfwGetMonitorPhysicalSize(primaryMonitor, &monitorPhysicalWidth, &monitorPhysicalHeight);
        LINFO("{}", tmonitorinfo);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwShowWindow(window);
        LINFO("Monitor:\"{}\", Phys:{}x{}mm, Scale:({}/{}), Pos:({}/{})", glfwGetMonitorName(primaryMonitor), monitorPhysicalWidth,
              monitorPhysicalHeight, xScale, yScale, xPos, yPos);
        LINFO("Monitor Mode:{}", formatMode(mode));
        LINFO("created the window {0}: (w: {1}, h: {2}, pos:({3}/{4}))", windowName.data(), width, height, centerX, centerY);
    }

    fs::path Window::calculateRelativePathToSrcRes(const fs::path &executablePath, const fs::path &targetFile) {
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
        const auto resp = fs::path("shaders");
        // Construct the relative path to the target file
        const auto relativePathToTarget = parentDir / resp / targetFile;
        // Construct the path to the target file under "src/engine/res"

        // Calculate the relative path from the executable's directory
        const auto relativePath = fs::relative(relativePathToTarget, executablePath);

        return relativePath.lexically_normal();
        // return parentDir;
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if(glfwCreateWindowSurface(instance, window, nullptr, surface)) { throw std::runtime_error("failed to  create window surface"); }
    }

}  // namespace lve
