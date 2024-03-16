#include "../include/server_logger.h"

#define MESSAGE_SIZE 100

std::map<std::string, std::pair<mqd_t, int>> server_logger::_queues_users = std::map<std::string, std::pair<mqd_t, int>>();

server_logger::server_logger(std::map<std::string, std::set<logger::severity>> const logs)
{
    std::runtime_error opening_queue_error("Error opening queue");
    mqd_t descriptor;
    struct mq_attr queue_attributes; // атрибуты очереди
    queue_attributes.mq_maxmsg = 30; // максимальное количество сообщений в очереди
    queue_attributes.mq_msgsize = MESSAGE_SIZE; // максимальная размер сообщения

    for (auto &[file, severities] : logs)
    {
        if (_queues_users.find(file) == _queues_users.end())
        {
            descriptor = mq_open(file.c_str(), O_WRONLY, 0644, &queue_attributes);
            if (descriptor < 0) throw opening_queue_error;
            _queues_users[file].first = descriptor;
        }
        _queues_users[file].second++;
        _queues[file].first = _queues_users[file].first;
        _queues[file].second = severities;
    }
    _request = 0;
}

server_logger::server_logger(server_logger const &other) = default;

server_logger &server_logger::operator=(server_logger const &other) = default;

server_logger::server_logger(server_logger &&other) noexcept = default;

server_logger &server_logger::operator=(server_logger &&other) noexcept = default;

server_logger::~server_logger() noexcept
{
    for (auto & [file, pair] : _queues)
    {
        if (--_queues_users[file].second != 0) continue;
        mq_close(_queues_users[file].first);
    }
}

logger const *server_logger::log(const std::string &text, logger::severity severity) const noexcept
{
    size_t meta_size = sizeof(size_t) + sizeof(size_t) + sizeof(pid_t) + sizeof(const char *) + sizeof(bool);
    size_t message_size = MESSAGE_SIZE - meta_size;
    size_t packets_count = text.size() / message_size + 1;
    std::cout << text.size() << " " << message_size << " " << packets_count << std::endl << std::endl;

    char info_message[meta_size];
    char *ptr;

    ptr = info_message;
    *reinterpret_cast<bool*>(ptr) = false; // если ложь, то идет инфо сообщение
    ptr += sizeof(bool);
    *reinterpret_cast<size_t*>(ptr) = packets_count;
    ptr += sizeof(size_t);
    *reinterpret_cast<size_t*>(ptr) = _request;
    ptr += sizeof(size_t);
    *reinterpret_cast<pid_t*>(ptr) = _process_id;
    ptr += sizeof(pid_t);
    char const * severity_string = severity_to_string(severity).c_str();
    strcpy(ptr, severity_string);

    char message[MESSAGE_SIZE];

    for (auto & [file, pair] : _queues)
    {
        if (pair.second.find(severity) == pair.second.end()) continue;

        mq_send(pair.first, info_message, MESSAGE_SIZE, 0); // отправка инфо сообщения

        ptr = message;
        *reinterpret_cast<bool*>(ptr) = true;
        ptr += sizeof(bool);
        *reinterpret_cast<size_t*>(ptr) = _request;
        ptr += sizeof(size_t);
        *reinterpret_cast<pid_t*>(ptr) = _process_id;
        ptr += sizeof(pid_t);
        for (size_t i = 0; i < packets_count; ++i)
        {
            size_t pos = i * message_size;
            size_t rest = text.size() - pos;
            size_t substr_size = (rest < message_size) ? rest : message_size;
            memcpy(ptr, text.substr(pos, substr_size).c_str(), substr_size);
            *(ptr + substr_size) = 0;
            mq_send(pair.first, message, message_size, 0);
        }
    }
    _request++;
    return this;
}