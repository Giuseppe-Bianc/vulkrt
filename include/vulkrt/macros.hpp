//
// Created by gbian on 29/06/2024.
//

#pragma once

#include "Log.hpp"

/**
 * @def SYSPAUSE
 * @brief A macro to pause the system and wait for user input.
 * This macro outputs a message to the user and waits for them to press
 * the enter key before continuing. It is useful for pausing the execution
 * of a console application to allow the user to read any final output
 * before the program exits.
 * Usage example:
 * @code
 * SYSPAUSE();
 * @endcode
 */
#define SYSPAUSE()                                                                                                                         \
do {                                                                                                                                   \
LINFO("Press enter to exit...");                                                                                                   \
std::cin.ignore();                                                                                                                 \
} while(0);


[[nodiscard]] static constexpr auto calcolaCentro(const int &width, const int &w) noexcept { return (width - w) / 2; }
#define CALC_CENTRO(width, w) calcolaCentro(width, w)
/**
 * @brief Converts the given parameter into a string literal.
 * This macro converts the provided parameter into a string literal.
 * For example, if `x` is provided as `param`, the macro expands to `"param"`.
 * @param x The parameter to convert into a string.
 * @return The string representation of the parameter.
 */
#define STRINGIFY(x) #x

/**
 * @brief Converts the given parameter into a string literal.
 * This macro is an alias for STRINGIFY(x). It converts the provided parameter into a string literal.
 * For example, if `x` is provided as `param`, the macro expands to `"param"`.
 * @param x The parameter to convert into a string.
 * @return The string representation of the parameter.
 */
#define TOSTRING(x) STRINGIFY(x)

/**
 * @brief Creates a unique pointer to an object of the specified type.
 * This macro creates a unique pointer to an object of the specified type, optionally forwarding arguments to its constructor.
 * @param type The type of the object to create.
 * @param ... The arguments to pass to the constructor of the object.
 * @return A unique pointer to the created object.
 */
#define MAKE_UNIQUE(type, ...) std::make_unique<type>(__VA_ARGS__)

/**
 * @brief Creates a shared pointer to an object of the specified type.
 * This macro creates a shared pointer to an object of the specified type, optionally forwarding arguments to its constructor.
 * @param type The type of the object to create.
 * @param ... The arguments to pass to the constructor of the object.
 * @return A shared pointer to the created object.
 */
#define MAKE_SHARED(type, ...) std::make_shared<type>(__VA_ARGS__)

/**
 * @brief Gets the index of the active alternative in a variant.
 * This macro retrieves the index of the active alternative in a variant.
 * @param var The variant to inspect.
 * @return The index of the active alternative in the variant.
 */
#define GET_VARIANT_INDEX(var) var.index()

/**
 * @brief Gets the value of the specified type from a variant.
 * This macro retrieves the value of the specified type from a variant.
 * @param var The variant to extract the value from.
 * @param type The type of value to extract.
 * @return The value of the specified type stored in the variant.
 */
#define GET_VARIANT_TYPE(var, type) std::get<type>(var)
/** \cond */

#if defined(_MSC_VER)
// Compilatore MSVC (Microsoft Visual C++)
#define DEBUG_BREAK __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
// Compilatori GCC o Clang
#ifdef __cplusplus
#if __has_builtin(__builtin_trap)
#define DEBUG_BREAK __builtin_trap()
#else
#error "Compiler does not support __builtin_trap()"
#endif
#else
#ifdef __clang__
#define DEBUG_BREAK __debugbreak()
#else
#define DEBUG_BREAK __builtin_trap()
#endif
#endif
#else
#define DEBUG_BREAK
#endif
