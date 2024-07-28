include(GenerateExportHeader)

find_package(Vulkan REQUIRED)
add_library(tinyobjloader tiny_obj_loader.cpp)



add_library(vulkrt::tinyobjloader ALIAS vulkrt-core)

# Retrieve the target name
get_target_property(target_name tinyobjloader NAME)

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

target_link_libraries(tinyobjloader PRIVATE vulkrt_options vulkrt_warnings)


target_include_directories(tinyobjloader ${WARNING_GUARD} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_compile_features(vulkrt-core PUBLIC cxx_std_20)

set_target_properties(
        tinyobjloader
        PROPERTIES VERSION ${PROJECT_VERSION}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES)

generate_export_header(tinyobjloader EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/vulkrt/sample_library_export.hpp)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(tinyobjloader PUBLIC SAMPLE_LIBRARY_STATIC_DEFINE)
endif()
