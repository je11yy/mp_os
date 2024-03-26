#include "../include/client_logger.h"

int main()
{
    // std::string format = "%s | %d | %t : %m";
    client_logger_builder * builder = new client_logger_builder;
    builder -> add_file_stream("errors.txt", logger::severity::error)
    -> add_file_stream("critical and debug.txt", logger::severity::critical)
    -> add_file_stream("information.txt", logger::severity::information)
    -> add_file_stream("critical and debug.txt", logger::severity::debug)
    -> add_console_stream(logger::severity::warning)
    -> add_console_stream(logger::severity::trace);

    // builder -> set_format(format);
    logger * lgr = builder -> build();

    delete builder;

    lgr -> log("Error corrupted\n", logger::severity::error)
    -> log("Info message\n", logger::severity::information)
    -> log("Critical sutuation\n", logger::severity::critical)
    -> log("Debugging point\n", logger::severity::debug)
    -> log("WARNING!!!\n", logger::severity::warning)
    -> log("Trace\n", logger::severity::trace);

    delete lgr;
}