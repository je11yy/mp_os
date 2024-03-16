#include "../include/server_logger_builder.h"
#include "../include/server_logger.h"

#define MESSAGE_SIZE 100
int main()
{
    std::string file = "/queue";
    mq_unlink(file.c_str());
    std::runtime_error opening_queue_error("Error opening queue");
    mqd_t descriptor;
    struct mq_attr queue_attributes; // атрибуты очереди
    queue_attributes.mq_maxmsg = 10; // максимальное количество сообщений в очереди
    queue_attributes.mq_msgsize = MESSAGE_SIZE; // максимальная размер сообщения

    descriptor = mq_open(file.c_str(), O_CREAT | O_RDONLY, 0644, &queue_attributes);
    if (descriptor < 0) throw opening_queue_error;


    logger_builder * builder = new server_logger_builder;
    logger * lgr = builder -> add_file_stream(file, logger::severity::error) 
    -> add_file_stream(file, logger::severity::trace) -> build();
    delete builder;
    lgr -> log("MESSAGE", logger::severity::trace)
    -> log("3456789fyuvgbdhjuyw78ty2r9uioqwj 839u410rofhi ttjf78953458fhvnksaiwua8tajoviehgtckjoifqvn 2y89 9", logger::severity::error);


    char message[MESSAGE_SIZE];
    unsigned int priority;
    while (mq_receive(descriptor, message, MESSAGE_SIZE, &priority) != 0)
    {
        char * ptr = message;
        bool kind = *reinterpret_cast<bool *>(ptr);
        std::cout << std::endl << "Kind of message: " << kind << std::endl;
        ptr += sizeof(bool);
        size_t packets_count = *reinterpret_cast<size_t *>(ptr);
        std::cout << "Number pf packets: " << packets_count << std::endl;
        ptr += sizeof(size_t);
        size_t request = *reinterpret_cast<size_t *>(ptr);
        std::cout << "Number of request: " << request << std::endl;
        ptr += sizeof(size_t);
        pid_t id = *reinterpret_cast<pid_t *>(ptr);
        std::cout << "Process id: " << id << std::endl;
        ptr += sizeof(pid_t);
        char severity[6];
        strcpy(severity, ptr);
        std::cout << "Severity: " << severity << std::endl;
        std::cout << "Message: " << std::endl;
        for (size_t i = 0; i < packets_count; ++i)
        {
            mq_receive(descriptor, message, MESSAGE_SIZE, nullptr);
            std::cout << "\tPacket " << i + 1 << ": " << std::endl;
            ptr = message;
            kind = *reinterpret_cast<bool *>(ptr);
            std::cout << std::endl << "\t\tKind of message: " << kind << std::endl;
            ptr += sizeof(bool);
            size_t request = *reinterpret_cast<size_t *>(ptr);
            std::cout << "\t\tNumber of request: " << request << std::endl;
            ptr += sizeof(size_t);
            pid_t id = *reinterpret_cast<pid_t *>(ptr);
            std::cout << "\t\tProcess id: " << id << std::endl;
            ptr += sizeof(pid_t);
            std::cout << "\t\tData: " << ptr << std::endl;
        }
        std::cout << std::endl;
    }

    delete lgr;
    mq_close(descriptor);
    mq_unlink(file.c_str());
}