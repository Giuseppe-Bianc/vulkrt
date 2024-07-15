include(cmake/CPM.cmake)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(vulkrt_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if (NOT TARGET glm::glm)
    CPMAddPackage(
            NAME glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG master # Use "master" for the latest version
            OPTIONS # Add options if needed
            "GLM_TEST_ENABLE OFF" # Disable tests if needed
            "GLM_ENABLE_CXX_20 ON"
            "GLM_ENABLE_SIMD_AVX2 ON"
    )
  endif ()

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if (NOT TARGET fmtlib::fmtlib)
    CPMAddPackage("gh:fmtlib/fmt#11.0.1")
  endif ()

  if (NOT TARGET spdlog::spdlog)
    if (WIN32)
      CPMAddPackage(
              NAME
              spdlog
              VERSION
              1.14.1
              GITHUB_REPOSITORY
              "gabime/spdlog"
              OPTIONS
              "SPDLOG_FMT_EXTERNAL ON"
              "SPDLOG_ENABLE_PCH ON"
              "SPDLOG_BUILD_PIC ON"
              "SPDLOG_WCHAR_SUPPORT ON"
              "SPDLOG_WCHAR_FILENAMES ON"
              "SPDLOG_SANITIZE_ADDRESS ON")
    else ()
      CPMAddPackage(
              NAME
              spdlog
              VERSION
              1.14.1
              GITHUB_REPOSITORY
              "gabime/spdlog"
              OPTIONS
              "SPDLOG_FMT_EXTERNAL ON"
              "SPDLOG_ENABLE_PCH ON"
              "SPDLOG_BUILD_PIC ON"
              "SPDLOG_SANITIZE_ADDRESS ON")

    endif ()
  endif ()

  if(NOT TARGET glfw)
    CPMAddPackage(
            NAME glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG master # Use "master" for the latest version
            OPTIONS
            "GLFW_BUILD_EXAMPLES OFF"
            "GLFW_BUILD_TESTS OFF"
            "GLFW_BUILD_DOCS OFF"
    )
  endif()

endfunction()
