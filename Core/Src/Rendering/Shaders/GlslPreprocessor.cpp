#include "GlslPreprocessor.h"

#include <spdlog/spdlog.h>
#include <sstream>
#include <Utility/FileUtility.h>

namespace Core
{
    std::filesystem::path GLSLPreprocessor::IncludeDirectory("Assets/Shaders/Include/");

    std::string GLSLPreprocessor::PreprocessFile(const std::string& Path)
    {
        std::unordered_set<std::string> includedFiles;
        return PreprocessInternal(Path, includedFiles);
    }

    std::string GLSLPreprocessor::PreprocessInternal(const std::string& Path,
                                                     std::unordered_set<std::string>& IncludedFiles)
    {
        std::string source;
        if (!FileUtility::GetTextFileContent(Path.c_str(), source))
        {
            spdlog::error("Failed to read file ", Path);
            return std::string();
        }
        std::istringstream input(source);
        std::ostringstream output;
        std::string line;

        while (std::getline(input, line))
        {
            std::istringstream lineStream(line);
            std::string token;
            lineStream >> token;

            if (token == "#include")
            {
                std::string filename;
                lineStream >> filename;
                filename = filename.substr(1, filename.size() - 2); // Remove quote marks
                if (!IncludedFiles.contains(filename))
                {
                    std::string filePath = FindFile(filename, Path);
                    if (filePath.empty())
                    {
                        return std::string();
                    }
                    output << PreprocessInternal(filePath, IncludedFiles);
                    IncludedFiles.insert(filename);
                }
            }
            else
            {
                output << line << "\n";
            }
        }
        return output.str();
    }

    std::string GLSLPreprocessor::FindFile(const std::string& Path, const std::string& CallerPath)
    {
        // Search in Caller's directory.
        const std::filesystem::path callerPath(CallerPath);
        const std::filesystem::path callerDirectory = callerPath.parent_path();
        std::filesystem::path path = callerDirectory / Path;

        if (exists(path))
        {
            std::filesystem::perms permissions = status(path).permissions();
            if ((permissions & std::filesystem::perms::owner_read) == std::filesystem::perms::owner_read)
            {
                return path.string();
            }
        }

        // Search in Include directory.
        path = IncludeDirectory / Path;

        if (exists(path))
        {
            std::filesystem::perms permissions = status(path).permissions();
            if ((permissions & std::filesystem::perms::owner_read) == std::filesystem::perms::owner_read)
            {
                return path.string();
            }
        }

        return std::string();
    }
}
