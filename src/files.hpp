#include <vector>
#include <string>
#include <filesystem>

struct files {
    std::vector<int> position;
    std::vector<std::filesystem::directory_entry> name;
};
