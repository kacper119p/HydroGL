#pragma once
#include <string>
#include <vector>

namespace Core
{
    class FileUtility
    {
    private:
        FileUtility() = default;

    public:
        /**
         * @brief Loads the entire contents of a text file into a string.
         * @param Path Path to the file to read.
         * @param Destination Reference to a string where the file contents will be stored.
         * @return true if the file was successfully read; false otherwise.
         */
        static bool GetTextFileContent(const std::string_view& Path, std::string& Destination);

        /**
         * @brief Recursively finds all files with a specific extension in the given directory.
         * @param Directory The root directory to search in.
         * @param Extension File extension to filter by (must include the dot, e.g., ".txt").
         * @param Results Vector that will be populated with the full paths of the matching files.
         */
        static void FindFilesWithExtension(const std::string_view& Directory, const std::string_view& Extension,
                                           std::vector<std::string>& Results);
    };
} // namespace Core
