// NOLINTEND(*-include-cleaner)

#include "vulkrt/Vulkrt.hpp"
#include <internal_use_only/config.hpp>

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    INIT_LOG()
    LINFO("{} {}v {}", vulkrt::cmake::project_name, vulkrt::cmake::project_version, vulkrt::cmake::git_sha);
    LINFO("{}", glfwGetVersionString());
    lve::App app{};
    try {
        app.run();
    } catch(const std::exception &e) {
        LERROR("{}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// NOLINTEND(*-include-cleaner)
