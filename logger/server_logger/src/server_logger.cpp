#include <not_implemented.h>

#include "../include/server_logger.h"

server_logger::server_logger(std::map<std::string, std::pair<key_t, std::set<logger::severity>>> const logs)
{
    std::runtime_error opening_queue_error("Error opening queue");
    int new_queue;

    for (auto &[file, pair] : logs)
    {
        if (_queues_users.find(file) == _queues_users.end()) // если очередь не была создана
        {
            if ((new_queue = msgget(pair.first, 0666 | IPC_CREAT)) < 0) throw opening_queue_error;
            _queues_users[file].first = new_queue; // установка id очереди
            _queues_users[file].second = 1; // установка количества юзеров
            _queues[file].first = new_queue;
        }
        else 
        {
            (_queues_users[file].second)++;
            _queues[file].first = new_queue;
        }
    }
}

server_logger::server_logger(server_logger const &other) = default;

server_logger &server_logger::operator=(server_logger const &other) = default;

server_logger::server_logger(server_logger &&other) noexcept = default;

server_logger &server_logger::operator=(server_logger &&other) noexcept = default;

server_logger::~server_logger() noexcept
{
    for (auto &[file, pair] : _queues)
    {
        // закрытие очереди
        if (_queues_users[file].second == 0) continue;
        if (!--_queues_users[file].second) msgctl(pair.first, IPC_RMID, 0);
    }
}

logger const *server_logger::log(const std::string &text, logger::severity severity) const noexcept
{
    int packets_count = sizeof(text)/1024 + 1;

    // нужно передать: количество пакетов, severity и само сообщение пакетами
    // в первом сообщении будет количество пакетов и тип логирования (info_message), а последующие сообщения - части сообщения (message)

    info_message info_msg;
    info_msg.type = 1;
    std::pair<int, logger::severity> pair_for_msg;
    pair_for_msg.first = packets_count;
    pair_for_msg.second = severity;
    info_msg.info = pair_for_msg;


    message * messages = new message[packets_count];
    // проверка?
    for (int i = 0; i < packets_count; ++i)
    {
        strcpy(messages[i].text, text.substr(i * 1024, 1024).c_str());
        messages[i].type = 1;
    }

    for (auto &[file, pair] : _queues)
    {
        if (pair.second.find(severity) != pair.second.end())
        {
            msgsnd(pair.first, &info_msg, sizeof(info_msg), 0);
            for (int i = 0; i < packets_count; ++i) msgsnd(pair.first, &messages[i], sizeof(messages[i]), 0);
        }
    }
    return this;
}