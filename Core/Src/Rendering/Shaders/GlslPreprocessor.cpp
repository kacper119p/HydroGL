#include "GlslPreprocessor.h"

#include <sstream>
#include <Utility/FileUtility.h>
#include <spdlog/spdlog.h>

#include "Debug.h"

namespace Core
{
    std::filesystem::path GlslPreprocessor::_includeDirectory("Assets/Shaders/Include/");
    std::filesystem::path GlslPreprocessor::_shadersDirectory("Assets/Shaders");

    std::string GlslPreprocessor::PreprocessFile(const std::string_view& Path)
    {
        std::unordered_set<std::string> includedFiles;
        return PreprocessInternal(Path, includedFiles);
    }

    std::string GlslPreprocessor::PreprocessInternal(const std::string_view& Path,
                                                     std::unordered_set<std::string>& IncludedFiles)
    {
        std::string source;
        if (!FileUtility::GetTextFileContent(Path, source))
        {
            std::filesystem::path cwd = std::filesystem::current_path();
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
                    CHECK_MESSAGE(!filePath.empty(), "Couldn't find file to include");
                    if (filePath.empty())
                    {
                        return std::string();
                    }
                    output << PreprocessInternal(filePath, IncludedFiles);
                    IncludedFiles.insert(filename);
                }
            } else
            {
                output << line << "\n";
            }
        }
        return output.str();
    }

    std::string GlslPreprocessor::FindFile(const std::string_view& Path, const std::string_view& CallerPath)
    {
        // Search in Caller's directory.
        const std::filesystem::path callerPath(CallerPath);
        const std::filesystem::path callerDirectory = callerPath.parent_path();
        std::filesystem::path path = callerDirectory / Path;

        if (exists(path))
        {
            const std::filesystem::perms permissions = status(path).permissions();
            if ((permissions & std::filesystem::perms::owner_read) == std::filesystem::perms::owner_read)
            {
                return path.string();
            }
        }

        // Search in Include directory.
        path = _includeDirectory / Path;

        if (exists(path))
        {
            std::filesystem::perms permissions = status(path).permissions();
            if ((permissions & std::filesystem::perms::owner_read) == std::filesystem::perms::owner_read)
            {
                return path.string();
            }
        }

        //Search in shaders directory
        path = _shadersDirectory / Path;

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
