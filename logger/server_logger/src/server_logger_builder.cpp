#include <not_implemented.h>

#include "../include/server_logger_builder.h"

server_logger_builder::server_logger_builder() = default;

server_logger_builder::server_logger_builder(server_logger_builder const &other) = default;

server_logger_builder & server_logger_builder::operator=(server_logger_builder const &other) = default;

server_logger_builder::server_logger_builder(server_logger_builder &&other) noexcept = default;

server_logger_builder & server_logger_builder::operator=(server_logger_builder &&other) noexcept = default;

server_logger_builder::~server_logger_builder() noexcept = default;

logger_builder * server_logger_builder::add_file_stream(std::string const &stream_file_path, logger::severity severity)
{
    if (_logs.find(stream_file_path) == _logs.end()) _logs[stream_file_path].first = ftok(stream_file_path.c_str(), 'r');
    _logs[stream_file_path].second.insert(severity);
    return this;
}

logger_builder * server_logger_builder::add_console_stream(logger::severity severity)
{
    add_file_stream(CONSOLE, severity);
    return this;
}


/*
формат файла:
{
    "files" :
    [
        [
            "file1",
            [
                "WARNING",
                "DEBUG"
            ]
        ],
        
        [
            "file2",
            [
                "WARNING",
                "ERROR"
            ]
        ]
    ]   
}
*/
logger_builder * server_logger_builder::transform_with_configuration(std::string const &configuration_file_path, std::string const &configuration_path)
{
    nlohmann::json configuration;
    std::ifstream configuration_file(configuration_file_path);
    configuration_file >> configuration;

    key_t key;
    std::string file_name;
    std::string string_severity;
    logger::severity logger_severity;

    for (auto & file : configuration[configuration_path])
    {
        file_name = file[0];
        key = ftok(file_name.c_str(), 'r');
        _logs[file_name].first = key;
        for (auto & severity : file[1])
        {
            string_severity = severity;
            logger_severity = string_to_severity(string_severity);
            _logs[file_name].second.insert(logger_severity);
        }
    }
}

logger_builder * server_logger_builder::clear()
{
    for (auto &[file, pair] : _logs) pair.second.clear();
    _logs.clear();
    return this;
}

logger * server_logger_builder::build() const
{
    return new server_logger(_logs);
}