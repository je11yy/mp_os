#include <gtest/gtest.h>
#include <logger.h>
#include <logger_builder.h>
#include <client_logger_builder.h>
#include <list>
#include <allocator_red_black_tree.h>

logger *create_logger(
    std::vector<std::pair<std::string, logger::severity>> const &output_file_streams_setup,
    bool use_console_stream = true,
    logger::severity console_stream_severity = logger::severity::debug)
{
    logger_builder *logger_builder_instance = new client_logger_builder;
    
    if (use_console_stream)
    {
        logger_builder_instance->add_console_stream(console_stream_severity);
    }
    
    for (auto &output_file_stream_setup: output_file_streams_setup)
    {
        logger_builder_instance->add_file_stream(output_file_stream_setup.first, output_file_stream_setup.second);
    }

    logger_builder_instance->add_console_stream(logger::severity::error);
    logger_builder_instance->add_console_stream(logger::severity::warning);
    logger_builder_instance->add_console_stream(logger::severity::information);
    
    logger *logger_instance = logger_builder_instance->build();
    
    delete logger_builder_instance;
    
    return logger_instance;
}

TEST(positiveTests, test1)
{
    logger *logger_instance = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "allocator_buddies_system_positiveTests_test1.txt",
                logger::severity::information
            }
        });
    allocator *allocator_instance = new allocator_red_black_tree(3000, nullptr, logger_instance, allocator_with_fit_mode::fit_mode::first_fit);

    auto first_block = reinterpret_cast<int *>(allocator_instance->allocate(sizeof(int), 250));
	auto second_block = reinterpret_cast<int *>(allocator_instance->allocate(sizeof(int), 250));
	allocator_instance->deallocate(first_block);

	first_block = reinterpret_cast<int *>(allocator_instance->allocate(sizeof(int), 229));

	auto third_block = reinterpret_cast<int *>(allocator_instance->allocate(sizeof(int), 250));

	allocator_instance->deallocate( second_block);
    
    delete allocator_instance;
    delete logger_instance;
}

TEST(positiveTests, test2)
{
    logger *logger_instance = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "allocator_buddies_system_positiveTests_test1.txt",
                logger::severity::information
            }
        });
    allocator *allocator_instance = new allocator_red_black_tree(2500, nullptr, logger_instance, allocator_with_fit_mode::fit_mode::first_fit);

    void *first_block = allocator_instance->allocate(sizeof(unsigned char), 40);

    void* second = allocator_instance->allocate(1, 226);
	void* third = allocator_instance->allocate(1, 221);
    
    allocator_instance->deallocate(first_block);

    allocator_instance->deallocate(second);

	void* six = allocator_instance->allocate(1, 128);

	allocator_instance->deallocate(six);
    
    delete allocator_instance;
    delete logger_instance;
}

int main(
    int argc,
    char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    
    return RUN_ALL_TESTS();
}