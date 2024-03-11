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

logger_builder * server_logger_builder::transform_with_configuration(std::string const &configuration_file_path, std::string const &configuration_path)
{
    throw not_implemented("logger_builder* server_logger_builder::transform_with_configuration(std::string const &configuration_file_path, std::string const &configuration_path)", "your code should be here...");
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