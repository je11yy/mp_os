#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <set>

int main()
{
    // std::set<std::ofstream*> streams;
    // std::ofstream * file = new std::ofstream;
    // file -> open("file1.txt");
    // streams.insert(file);
    // file = new std::ofstream;
    // file -> open("file2.txt");
    // streams.insert(file);
    // for (auto & stream : streams)
    // {
    //     (*stream) << "HERE";
    //     stream -> close();
    //     delete stream;
    // }

    std::map<std::string, std::ofstream> streams;
    streams["file1"].open("file1.txt");
    streams["file2"].open("file2.txt");
    for (auto &[name, stream] : streams)
    {
        std::cout << "HERE" << std::endl;
        stream << "HERE";
        stream.close();
    }
}