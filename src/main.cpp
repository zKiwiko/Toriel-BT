#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include "process.hpp"
#include <json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

Processor processor;

void build(const std::string& content, const std::string& output_dir) {
    try {
        fs::create_directories(output_dir);

        std::string filename = processor.pr_name + "-" + processor.pr_ver + ".gpc";
        fs::path output_path = fs::path(output_dir) / filename;
        std::ofstream out_file(output_path);
        if (!out_file) {
            throw std::runtime_error("Failed to create output file: " + output_path.string());
        }
        
        out_file << content;
        out_file.close();
        
        std::cout << "Successfully built: " << output_path.string() << "\n";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    std::string source_dir;
    std::string output_dir;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--source" && i + 1 < argc) {
            source_dir = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            output_dir = argv[++i];
        }
    }

    if (source_dir.empty() || output_dir.empty()) {
        std::cerr << "Usage: " << argv[0] 
                  << " --source <source_dir> --output <output_dir>\n";
        return 1;
    }

    try {
        fs::path project_file = fs::path(source_dir) / "project.json";
        processor.parse_json(project_file.string());

        std::vector<std::string> processed_files;
        fs::path main_path = fs::path(source_dir) / processor.pr_src;
        
        std::string processed_content = processor.processMain(
            main_path.string(), 
            processed_files
        );

        build(processed_content, output_dir);
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}