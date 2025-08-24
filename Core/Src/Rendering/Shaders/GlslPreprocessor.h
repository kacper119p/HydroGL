#pragma once
#include <filesystem>
#include <string>
#include <unordered_set>

namespace Core
{
    /**
     * @brief Simple preprocessor static-only class for resolving GLSL #includes.<br>
     * Rest of preprocessing directives is handled by driver.
     */
    class GlslPreprocessor final
    {
    private:
        static std::filesystem::path _includeDirectory;

    private:
        GlslPreprocessor() = default;

    public:
        /**
         * @brief Replaces #include directives recursively with included file's content.
         * @param Path Path to a file.
         * @return Preprocessed file's content.
         */
        static std::string PreprocessFile(const std::string_view& Path);

    private:
        static std::string PreprocessInternal(const std::string_view& Path,
                                              std::unordered_set<std::string>& IncludedFiles);

        static std::string FindFile(const std::string_view& Path, const std::string_view& CallerPath);
    };
}
