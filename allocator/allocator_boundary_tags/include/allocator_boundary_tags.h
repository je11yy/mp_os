#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H

#include <allocator_guardant.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>

#include <mutex>
#include <cstring>

class allocator_boundary_tags final:
    private allocator_guardant,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:
    
    void *_trusted_memory = nullptr;

public:
    
    ~allocator_boundary_tags() override;
    
    allocator_boundary_tags(
        allocator_boundary_tags const &other) = delete;
    
    allocator_boundary_tags &operator=(
        allocator_boundary_tags const &other) = delete;
    
    allocator_boundary_tags(
        allocator_boundary_tags &&other) noexcept;
    
    allocator_boundary_tags &operator=(
        allocator_boundary_tags &&other) noexcept;

public:
    
    explicit allocator_boundary_tags(
        size_t space_size,
        allocator *parent_allocator = nullptr,
        logger *_logger = nullptr,
        allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:
    
    [[nodiscard]] void *allocate(
        size_t value_size,
        size_t values_count) override;
    
    void deallocate(
        void *at) override;

public:
    
    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;

private:
    
    inline allocator *get_allocator() const override;

public:
    
    std::vector<allocator_test_utils::block_info> get_blocks_info() const noexcept override;

private:
    
    inline logger *get_logger() const override;

private:
    
    inline std::string get_typename() const noexcept override;

private:

    inline allocator_with_fit_mode::fit_mode get_fit_mode() override;

private: 

    void * get_first_occupied_block() const noexcept;

    void * get_first_block() const noexcept;

    size_t get_block_size(void * block) const noexcept;

    allocator * get_block_allocator(void * block) const noexcept;

    void * get_prev_block(void * block) const noexcept;

    void * get_next_block(void * block) const noexcept;

    void connect_blocks(void * prev, void * next) noexcept;

    std::mutex & get_mutex() const noexcept;

    size_t get_memory_size() const noexcept;

    void set_first_occupied_block(void * block) const noexcept;

    void clear_block(void * block) const noexcept;

    void * get_end() const noexcept;

    size_t get_available_memory() const noexcept;

    std::string make_blocks_info_string(std::vector<allocator_test_utils::block_info> info) const noexcept;

    std::string block_status(bool state) const noexcept;

    std::string get_block_info(void * block) const noexcept;
    
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H