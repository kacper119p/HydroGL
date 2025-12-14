#include "FileUtility.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace Core
{
    bool FileUtility::GetTextFileContent(const std::string_view Path, std::string& Destination)
    {
        std::string content;
        std::ifstream fileStream;

        fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            fileStream.open(Path);
            std::stringstream stringStream;
            stringStream << fileStream.rdbuf();
            fileStream.close();
            content = stringStream.str();
        } catch (std::ifstream::failure&)
        {
            return false;
        }
        Destination = content;
        return true;
    }

    void FileUtility::FindFilesWithExtension(const std::string_view Directory, const std::string_view& Extension,
                                             std::vector<std::string>& Results)
    {
        for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(Directory))
        {
            if (entry.path().extension() == Extension)
            {
                Results.push_back(entry.path().string());
            }
        }
    }
}
