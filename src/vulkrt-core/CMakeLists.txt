include(GenerateExportHeader)

find_package(Vulkan REQUIRED)
add_library(vulkrt-core Vulkrt.cpp
                        FPSCounter.cpp
        Window.cpp
        App.cpp
        Pipeline.cpp
        Device.cpp
        SwapChain.cpp
        Model.cpp
        ../../include/vulkrt/GameObject.hpp
        Renderer.cpp
        ../../include/vulkrt/Renderer.hpp
        SimpleRenderSystem.cpp
        ../../include/vulkrt/SimpleRenderSystem.hpp
        Camera.cpp
        ../../include/vulkrt/Camera.hpp
        KeyboardMovementController.cpp
        ../../include/vulkrt/KeyboardMovementController.hpp
)



add_library(vulkrt::vulkrt-core ALIAS vulkrt-core)

# Retrieve the target name
get_target_property(target_name vulkrt-core NAME)

if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  target_compile_options(${target_name} PRIVATE -march=native)
  message(STATUS "Set SIMD instructions to AVX2 for target '${target_name}' with '${CMAKE_CXX_COMPILER_ID}' compiler.")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${target_name} PRIVATE -march=native)
  message(STATUS "Set SIMD instructions to AVX2 for target '${target_name}' with '${CMAKE_CXX_COMPILER_ID}' compiler.")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND MSVC_VERSION GREATER 1900)
  target_compile_options(${target_name} PRIVATE /arch:AVX2)
  message(STATUS "Set SIMD instructions to AVX2 for target '${target_name}' with '${CMAKE_CXX_COMPILER_ID}' compiler.")
else ()
  message(STATUS "Impossible to set SIMD instructions to AVX2 for target '${target_name}' with '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif ()

target_link_libraries(vulkrt-core PRIVATE vulkrt_options vulkrt_warnings)

target_link_libraries(vulkrt-core PUBLIC
        Vulkan::Vulkan
        fmt::fmt
        spdlog::spdlog
        glm::glm
        glfw
        tinyobjloader
)

target_include_directories(vulkrt-core ${WARNING_GUARD} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                                                  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_compile_features(vulkrt-core PUBLIC cxx_std_20)

set_target_properties(
  vulkrt-core
  PROPERTIES VERSION ${PROJECT_VERSION}
             CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES)

############## Build SHADERS #######################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
############## Build SHADERS #######################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS
        ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
        /usr/bin
        /usr/local/bin
        ${VULKAN_SDK_PATH}/Bin
        ${VULKAN_SDK_PATH}/Bin32
        $ENV{VULKAN_SDK}/Bin/
        $ENV{VULKAN_SDK}/Bin32/
)


find_program(SPIRV_OPT spirv-opt HINTS
        ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
        /usr/bin
        /usr/local/bin
        ${VULKAN_SDK_PATH}/Bin
        ${VULKAN_SDK_PATH}/Bin32
        $ENV{VULKAN_SDK}/Bin/
        $ENV{VULKAN_SDK}/Bin32/
)

find_program(SPIRV_REMAP spirv-remap HINTS
        ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
        /usr/bin
        /usr/local/bin
        ${VULKAN_SDK_PATH}/Bin
        ${VULKAN_SDK_PATH}/Bin32
        $ENV{VULKAN_SDK}/Bin/
        $ENV{VULKAN_SDK}/Bin32/
)


# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/shaders/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME  ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
          OUTPUT ${SPIRV}
          COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
          DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

foreach (BSPIRV ${SPIRV_BINARY_FILES})
  get_filename_component(FILE_NAME  ${BSPIRV} NAME_WLE)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.opt.spv")
  add_custom_command(
          OUTPUT ${SPIRV}
          COMMAND ${SPIRV_OPT} ${BSPIRV} -o ${SPIRV}  -O
          DEPENDS ${BSPIRV})
  list(APPEND SPIRV_BINARY_FILES_OPT ${SPIRV})
endforeach (BSPIRV)

foreach (RSPIRV ${SPIRV_BINARY_FILES_OPT})
  get_filename_component(FILE_NAME  ${RSPIRV} NAME_WLE)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.rmp.spv")
  add_custom_command(
          OUTPUT ${SPIRV}
          COMMAND ${SPIRV_REMAP} -v --do-everything  -i  ${RSPIRV} -o ${SPIRV}
          DEPENDS ${RSPIRV})
  list(APPEND SPIRV_BINARY_FILES_REMAP ${SPIRV})
endforeach (RSPIRV)

list(APPEND SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES_OPT})
list(APPEND SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES_REMAP})

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(vulkrt-core Shaders)

generate_export_header(vulkrt-core EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/vulkrt/sample_library_export.hpp)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(vulkrt-core PUBLIC SAMPLE_LIBRARY_STATIC_DEFINE)
endif()
