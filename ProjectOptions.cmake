include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(vulkrt_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(vulkrt_setup_options)
  option(vulkrt_ENABLE_HARDENING "Enable hardening" ON)
  option(vulkrt_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    vulkrt_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    vulkrt_ENABLE_HARDENING
    OFF)

  vulkrt_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR vulkrt_PACKAGING_MAINTAINER_MODE)
    option(vulkrt_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(vulkrt_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(vulkrt_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(vulkrt_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(vulkrt_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(vulkrt_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(vulkrt_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(vulkrt_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(vulkrt_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(vulkrt_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(vulkrt_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(vulkrt_ENABLE_PCH "Enable precompiled headers" OFF)
    option(vulkrt_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(vulkrt_ENABLE_IPO "Enable IPO/LTO" ON)
    option(vulkrt_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(vulkrt_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(vulkrt_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(vulkrt_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(vulkrt_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(vulkrt_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(vulkrt_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(vulkrt_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(vulkrt_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(vulkrt_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(vulkrt_ENABLE_PCH "Enable precompiled headers" OFF)
    option(vulkrt_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      vulkrt_ENABLE_IPO
      vulkrt_WARNINGS_AS_ERRORS
      vulkrt_ENABLE_USER_LINKER
      vulkrt_ENABLE_SANITIZER_ADDRESS
      vulkrt_ENABLE_SANITIZER_LEAK
      vulkrt_ENABLE_SANITIZER_UNDEFINED
      vulkrt_ENABLE_SANITIZER_THREAD
      vulkrt_ENABLE_SANITIZER_MEMORY
      vulkrt_ENABLE_UNITY_BUILD
      vulkrt_ENABLE_CLANG_TIDY
      vulkrt_ENABLE_CPPCHECK
      vulkrt_ENABLE_COVERAGE
      vulkrt_ENABLE_PCH
      vulkrt_ENABLE_CACHE)
  endif()

  vulkrt_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (vulkrt_ENABLE_SANITIZER_ADDRESS OR vulkrt_ENABLE_SANITIZER_THREAD OR vulkrt_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(vulkrt_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(vulkrt_global_options)
  if(vulkrt_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    vulkrt_enable_ipo()
  endif()

  vulkrt_supports_sanitizers()

  if(vulkrt_ENABLE_HARDENING AND vulkrt_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR vulkrt_ENABLE_SANITIZER_UNDEFINED
       OR vulkrt_ENABLE_SANITIZER_ADDRESS
       OR vulkrt_ENABLE_SANITIZER_THREAD
       OR vulkrt_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${vulkrt_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${vulkrt_ENABLE_SANITIZER_UNDEFINED}")
    vulkrt_enable_hardening(vulkrt_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(vulkrt_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(vulkrt_warnings INTERFACE)
  add_library(vulkrt_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  vulkrt_set_project_warnings(
    vulkrt_warnings
    ${vulkrt_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(vulkrt_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    vulkrt_configure_linker(vulkrt_options)
  endif()

  include(cmake/Sanitizers.cmake)
  vulkrt_enable_sanitizers(
    vulkrt_options
    ${vulkrt_ENABLE_SANITIZER_ADDRESS}
    ${vulkrt_ENABLE_SANITIZER_LEAK}
    ${vulkrt_ENABLE_SANITIZER_UNDEFINED}
    ${vulkrt_ENABLE_SANITIZER_THREAD}
    ${vulkrt_ENABLE_SANITIZER_MEMORY})

  set_target_properties(vulkrt_options PROPERTIES UNITY_BUILD ${vulkrt_ENABLE_UNITY_BUILD})

  if(vulkrt_ENABLE_PCH)
    target_precompile_headers(
      vulkrt_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(vulkrt_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    vulkrt_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(vulkrt_ENABLE_CLANG_TIDY)
    vulkrt_enable_clang_tidy(vulkrt_options ${vulkrt_WARNINGS_AS_ERRORS})
  endif()

  if(vulkrt_ENABLE_CPPCHECK)
    vulkrt_enable_cppcheck(${vulkrt_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(vulkrt_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    vulkrt_enable_coverage(vulkrt_options)
  endif()

  if(vulkrt_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(vulkrt_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(vulkrt_ENABLE_HARDENING AND NOT vulkrt_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR vulkrt_ENABLE_SANITIZER_UNDEFINED
       OR vulkrt_ENABLE_SANITIZER_ADDRESS
       OR vulkrt_ENABLE_SANITIZER_THREAD
       OR vulkrt_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    vulkrt_enable_hardening(vulkrt_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
