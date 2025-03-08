#include <process.hpp>

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <regex>
#include <json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;


const std::regex Processor::include_pattern_system(R"(@include\s*<([^>]+)>)");
const std::regex Processor::include_pattern_project(R"(@include\s*\"([^"]+)\")");
const std::regex Processor::const_pattern(R"(@const\s+(\w+)\s*=\s*([\w\.]+)?;)");

Processor::Processor() {
    systemIncludePath = (fs::current_path() / "bin" / "data" / "gpc" / "libs").string();
}

void Processor::parse_json(const std::string& path) {
    try {
        std::ifstream file(path);
        json obj;
        file >> obj;
        
        if (obj.contains("project")) {
            auto& projectObj = obj["project"];
            pr_name = projectObj.value("name", "");
            pr_ver = projectObj.value("version", "");
            pr_src = projectObj.value("source", "");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
}

std::vector<std::string> Processor::getProjectHeaders(const std::string& path) {
    try {
        std::ifstream file(path);
        json obj;
        file >> obj;
        
        std::vector<std::string> headersList;
        if (obj.contains("headers")) {
            for (const auto& value : obj["headers"]) {
                headersList.push_back(value.get<std::string>());
            }
        }
        return headersList;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse project.json: " + std::string(e.what()));
    }
}

std::vector<std::string> Processor::getStandardLibrary(const std::string& path) {
    try {
        std::ifstream file(path);
        json obj;
        file >> obj;
        
        std::vector<std::string> headersList;
        if (obj.contains("std")) {
            for (const auto& value : obj["std"]) {
                headersList.push_back(value.get<std::string>());
            }
        }
        return headersList;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
}

std::string Processor::resolveIncludePath(const std::string& includePath, const std::string& baseDir, bool isSystemInclude) {
    if (isSystemInclude) {
        fs::path fileName = includePath;
        if (fileName.extension() != ".gpc") {
            fileName += ".gpc";
        }
        return (fs::path(systemIncludePath) / fileName).string();
    }
    
    fs::path path(includePath);
    if (path.is_absolute()) {
        return includePath;
    }
    return (fs::path(baseDir) / path).string();
}

std::string Processor::processMain(const std::string& mainPath, std::vector<std::string>& processedFiles) {
    if (std::find(processedFiles.begin(), processedFiles.end(), mainPath) != processedFiles.end()) {
        throw std::runtime_error("Circular import detected for file: " + mainPath);
    }
    processedFiles.push_back(mainPath);
    
    std::ifstream inputFile(mainPath);
    if (!inputFile) {
        throw std::runtime_error("Unable to open file: " + mainPath);
    }
    
    std::string content((std::istreambuf_iterator<char>(inputFile)), 
                       std::istreambuf_iterator<char>());
    
    return processContent(content, fs::path(mainPath).parent_path().string(), processedFiles);
}

std::string Processor::processContent(const std::string& content, const std::string& baseDir, std::vector<std::string>& processedFiles) {
    // Process constants
    std::sregex_iterator constMatches(content.begin(), content.end(), const_pattern);
    std::sregex_iterator end;
    for (; constMatches != end; ++constMatches) {
        std::smatch match = *constMatches;
        constants[match[1]] = match[2].str();
    }

    std::string processedContent = std::regex_replace(content, const_pattern, "");
    std::istringstream iss(processedContent);
    std::string line;
    std::string result;

    while (std::getline(iss, line)) {
        std::smatch systemMatch;
        if (std::regex_search(line, systemMatch, include_pattern_system)) {
            std::string includeFileName = systemMatch[1].str();
            std::string includeFilePath = resolveIncludePath(includeFileName, baseDir, true);
            
            if (!fs::exists(includeFilePath)) {
                throw std::runtime_error("System include file not found: " + includeFilePath);
            }
            line = processMain(includeFilePath, processedFiles);
        } else {
            std::smatch projectMatch;
            if (std::regex_search(line, projectMatch, include_pattern_project)) {
                std::string includeFileName = projectMatch[1].str();
                std::string includeFilePath = resolveIncludePath(includeFileName, baseDir, false);
                
                if (!fs::exists(includeFilePath)) {
                    throw std::runtime_error("Project include file not found: " + includeFilePath);
                }
                line = processMain(includeFilePath, processedFiles);
            }
        }

        // Replace constants
        for (const auto& [key, value] : constants) {
            std::regex pattern("\\b" + key + "\\b");
            line = std::regex_replace(line, pattern, value);
        }

        result += line + "\n";
    }

    return result;
}