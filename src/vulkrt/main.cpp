// NOLINTEND(*-include-cleaner)

#include "vulkrt/Vulkrt.hpp"
#include <internal_use_only/config.hpp>

[[nodiscard]] constexpr std::size_t factorial_constexpr(std::size_t input) noexcept {
    if(input == 0) { return 1; }

    return input * factorial_constexpr(input - 1);
}

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    INIT_LOG()
    LINFO("{} {}v {}", vulkrt::cmake::project_name, vulkrt::cmake::project_version, vulkrt::cmake::git_sha);
    LINFO("{}", glfwGetVersionString());
    vnd::Timer timer{"fib"};
    for(std::size_t i = 0; i <= 100; ++i) { std::println("factorial({}) = {}", i, factorial_constexpr(i)); }
    LINFO("{}", timer);
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
