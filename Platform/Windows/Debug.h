#pragma once

#ifdef DEBUG
#include <cstdio>
#include <cstdlib>
#include <intrin.h>
#include <glad/glad.h>

#define CHECK_EXPAND(x) x

#define CHECK_INTERNAL_NO_MESSAGE(EXPRESSION) \
    do\
    { \
        if (!(!!(EXPRESSION)))\
        { \
            fprintf(stderr, "%s %d: Assertion failed: %s\n", \
            __FILE__, __LINE__, #EXPRESSION); \
            __debugbreak(); \
        } \
    } while (0)

#define CHECK_INTERNAL_MESSAGE(EXPRESSION, FORMAT, ...) \
    do\
    { \
        if (!(!!(EXPRESSION))) { \
        fprintf(stderr, "%s %d: Assertion failed: %s\nMessage: " FORMAT "\n", \
        __FILE__, __LINE__, #EXPRESSION, ##__VA_ARGS__); \
        __debugbreak(); \
        } \
    } while (0)

#define GET_CHECK_MACRO(_1, _2, NAME, ...) NAME

/**
 * @brief Halts application's execution if expression is false. Expression is not evaluated in release builds.\n
 * Usage:\n
 * CHECK(x > 0); - Checks expression\n
 * CHECK(x > 0, "X is too low"); - Checks expression with message\n
 * CHECK(x > 0, "Value: %d", x); - Checks expression with formatted message\n
 */
#define CHECK(...) CHECK_EXPAND(GET_CHECK_MACRO(__VA_ARGS__, CHECK_INTERNAL_MESSAGE, CHECK_INTERNAL_NO_MESSAGE)(__VA_ARGS__))

#else
/**
 * @brief Check is disabled in release builds.
 */
#define CHECK(...) ((void)0)

#endif

#if DEBUG
namespace Platform
{
    class Debug
    {
    private:
        Debug() = default;

    public:
        Debug(const Debug&) = delete;

        Debug& operator=(const Debug&) = delete;

    public:
        static void EnableOpenGlDebugging();

    private:
        static void APIENTRY GlDebugOutput(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length,
                                           const char* Message, const void* UserParam);
    };
}
#endif
