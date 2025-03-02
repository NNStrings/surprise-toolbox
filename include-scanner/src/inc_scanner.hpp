#pragma once

#include <set>
#include <string>
#include <queue>
#include <vector>
#include <filesystem>
#include <regex>
#include <fstream>
#include <iostream>

#include "config.h"

namespace inc_scanner {

namespace fs = std::filesystem;

class file_statistics {
public:
    file_statistics(std::vector<std::string> const &dlist) {
        for (auto &dir : dlist) {
            list_files_recursively(dir);
        }
    }
private:
    void list_files_recursively(fs::path const &dir) {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            throw "wrong or non-existent directory";
        }
        for (auto const &file : fs::recursive_directory_iterator(dir)) {
            if (fs::is_regular_file(file)) {
                total_file.emplace(file.path().lexically_relative("."));
            }
        }
    }
public:
    std::set<std::string> total_file;
};

class file_query {
public:
    file_query(std::string rx, std::string const &query, std::vector<std::string> &loc,
            std::vector<std::string> &deps, file_statistics const &fst):
            include_rx(rx), loc_file(loc),deps_file(deps), fst(fst) {
        queued_file.insert(query);
        include_query.push(query);
    }
    file_query(std::string const &query, std::vector<std::string> &loc,
            std::vector<std::string> &deps, file_statistics const &fst):
            include_rx(R"(#include\s*\"([^\"]+)\"\s*(//.*)?)"), loc_file(loc),
            deps_file(deps), fst(fst) {
        queued_file.insert(query);
        include_query.push(query);
    }
    void execute_query() {
        while (!include_query.empty()) {
            std::string filename = std::move(include_query.front());
            include_query.pop();
            if (fst.total_file.count(filename)) {
                loc_file.emplace_back(filename);
                extract_include_files(filename);
            }
            else {
                deps_file.emplace_back(filename);
            }
        }
    }
private:
    void push_file(std::string const &hfile) {
        if (!queued_file.count(hfile)) {
            queued_file.insert(hfile);
            include_query.push(hfile);
            auto point_pos = hfile.find_last_of(".");
            for (auto const &suffix : cxx_suffix) {
                std::string cfile = hfile.substr(0, point_pos) + suffix;
                if (fst.total_file.count(cfile)) {
                    queued_file.insert(cfile);
                    include_query.push(cfile);
                }
            }
        }
    }
    void extract_include_files(std::string const &filename) {
        std::ifstream ifs(filename);
        if (!ifs) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            throw;
        }
        std::string line;
        while (std::getline(ifs, line)) {
            std::smatch match;
            if (std::regex_match(line, match, include_rx)) {
                push_file(match[1]);
            }
        }
    }
    std::set<std::string> queued_file;
    std::queue<std::string> include_query;
    std::vector<std::string> &loc_file, &deps_file;
    std::regex include_rx;
    file_statistics const &fst;
};

}
