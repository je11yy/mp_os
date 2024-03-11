#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H

#include <logger.h>
#include "server_logger_builder.h"

#include <cstring>

class server_logger final:
    public logger
{
    
    friend class server_logger_builder;

private:

    std::map<std::string, std::pair<int, std::set<logger::severity>>> _queues; // name, (queue id, severities)

    static std::map<std::string, std::pair<int, int>> _queues_users; // name, (queue id, number of users)

    server_logger(std::map<std::string, std::pair<key_t, std::set<logger::severity>>> const logs);

    struct info_message
    {
        long type; 
        std::pair<int, logger::severity> info; // количество пакетов и тип логирования
    };

    struct message
    {
        long type;
        char text[1024];
    };

public:

    server_logger(server_logger const &other);

    server_logger &operator=(server_logger const &other);

    server_logger(server_logger &&other) noexcept;

    server_logger &operator=(server_logger &&other) noexcept;

    ~server_logger() noexcept final;

public:

    [[nodiscard]] logger const *log(const std::string &message, logger::severity severity) const noexcept override;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H