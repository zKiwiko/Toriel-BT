#include <vector>
#include <string>
#include <map>
#include <regex>

class Processor {
public:
    Processor();
    
    void parse_json(const std::string& path);
    std::vector<std::string> getProjectHeaders(const std::string& path);
    std::vector<std::string> getStandardLibrary(const std::string& path);
    std::string processMain(const std::string& mainPath, std::vector<std::string>& processedFiles);
    std::string pr_name;
    std::string pr_ver;
    std::string pr_src;

private:
    std::string systemIncludePath;
    std::map<std::string, std::string> constants;

    static const std::regex include_pattern_system;
    static const std::regex include_pattern_project;
    static const std::regex const_pattern;

    std::string resolveIncludePath(const std::string& includePath, const std::string& baseDir, bool isSystemInclude);
    std::string processContent(const std::string& content, const std::string& baseDir, std::vector<std::string>& processedFiles);
};