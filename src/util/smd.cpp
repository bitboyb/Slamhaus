#include "smd.hpp"
#include <set>

namespace SMD
{
    bool IsContentFile(const std::filesystem::path& path)
    {
        static const std::set<std::string> validExtensions = { ".md", ".smd", ".slam" };
        return path.has_extension() && validExtensions.count(path.extension().string()) > 0;
    }
}
