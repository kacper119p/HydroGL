#pragma once

#if DEBUG
#include <glad/glad.h>

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
        static void APIENTRY GlDebugOutput(GLenum Source, GLenum Type, unsigned int Id, GLenum Severity,
                                           GLsizei Length, const char* Message, const void* UserParam);

    };
}
#endif

