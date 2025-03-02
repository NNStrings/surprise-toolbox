#include <vector>
#include <string>
#include <iostream>
#include <ranges>
#include <algorithm>

#include "inc_scanner.hpp"

inline void print_res(std::vector<std::string> const &loc, 
        std::vector<std::string> const &deps) {
    std::cout << "Local files:\n";
    for (auto &file : loc) {
        std::cout << file << "\n";
    }
    std::cout << "Dependencies files and system files:\n";
    for (auto &file : deps) {
        std::cout << file << "\n";
    }
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cout << "The parameter should not be less than 3\n";
        exit(1);
    }
    std::string query_file = argv[1];
    std::vector<std::string> dir_list;
    for (int i = 2; i < argc; i++) {
        dir_list.emplace_back(argv[i]);
    }
    inc_scanner::file_statistics fst(dir_list);
    std::vector<std::string> loc_file, deps_file;
    inc_scanner::file_query fqy(query_file, loc_file, deps_file, fst);
    fqy.execute_query();
    std::ranges::sort(loc_file);
    std::ranges::sort(deps_file);
    print_res(loc_file, deps_file);
    return 0;
}