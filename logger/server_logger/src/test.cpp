#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

int main()
{
    nlohmann::json j;
    std::ifstream i("configuration.json");
    i >> j;
    std::string a;
    for (auto & file : j["files"])
    {
        a = file[0];
        std::cout << a << std::endl;
        for (auto & severity : file[1])
        {
            a = severity;
            std:: cout << a << std::endl;
        }
    }
}