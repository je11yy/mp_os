#include <not_implemented.h>

#include "../include/client_logger.h"

client_logger::client_logger(std::map<std::string, std::set<logger::severity>> streams, std::string format)
{
    std::runtime_error file_opening("Failed to open stream\n");
    for (auto &[file_name, severities] : streams)
    {
        if (_streams_users.find(file_name) != _streams_users.end() && _streams_users[file_name].second != 0) _streams[file_name] = severities;
        else
        {
            _streams_users[file_name].first.open(file_name);
            if (!(_streams_users[file_name].first.is_open())) throw file_opening;
            _streams[file_name] = severities;
        }
        (_streams_users[file_name].second)++;
    }
    _format = format;
}

client_logger::client_logger(client_logger const &other) = default;

client_logger &client_logger::operator=(client_logger const &other) = default;

client_logger::client_logger(client_logger &&other) noexcept = default;

client_logger &client_logger::operator=(client_logger &&other)  = default;

client_logger::~client_logger() noexcept
{
    for (auto &[file_name, severities] : _streams)
    {
        if (!(--_streams_users[file_name].second)) _streams_users[file_name].first.close();
    }
}

void replace_substring(std::string &str, const std::string &to_replace, const std::string &replace_with)
{
    size_t pos = 0;
    while ((pos = str.find(to_replace, pos)) != std::string::npos)
    {
        str.replace(pos, to_replace.size(), replace_with);
        pos += replace_with.size();
    }
}

void format_to_message(std::string &replaced_format, const std::string &message, const std::string & string_severity)
{
    time_t time_now = time(NULL);
    char tmp_date_time[20];

    std::string msg = "%m";
    std::string sev = "%s";
    std::string dt = "%d";
    std::string tm = "%t";

    replace_substring(replaced_format, msg, message);
    replace_substring(replaced_format, sev, string_severity);
    strftime(tmp_date_time, sizeof(tmp_date_time), "%T", localtime(&time_now));
    replace_substring(replaced_format, tm, tmp_date_time);
    strftime(tmp_date_time, sizeof(tmp_date_time), "%F", localtime(&time_now));
    replace_substring(replaced_format, dt, tmp_date_time);
}

logger const *client_logger::log(const std::string &text, logger::severity severity) const noexcept
{
    std::string string_severity = severity_to_string(severity);
    std::string message = _format;
    format_to_message(message, text, string_severity);

    for (auto & [file_name, severities] : _streams)
    {
        if (severities.find(severity) != severities.end()) _streams_users[file_name].first << message;
    }
    return this;
}