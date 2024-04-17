#include <not_implemented.h>

#include "../include/allocator_boundary_tags.h"

std::string start = " [START] ";
std::string end = " [END] ";
std::string type = "[Allocator Boundary Tags]";
std::string occupied = "occup";
std::string available = "avail";

allocator_boundary_tags::~allocator_boundary_tags()
{
    std::string function = "Destructor\n";
    debug_with_guard(get_typename() + start + function);
    logger * _logger = get_logger();
    allocator::destruct(&get_mutex());
    deallocate_with_guard(_trusted_memory);
    if (_logger != nullptr) _logger->debug(get_typename() + end + function);
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
{
    std::string function = "Move constructor\n";
    logger * log = other.get_logger();
    if (log != nullptr) log->debug(get_typename() + start + function);

    if (_trusted_memory != nullptr) deallocate_with_guard(_trusted_memory);
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    if (log != nullptr) log->debug(get_typename() + end + function);
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    std::string function = "Move operator\n";
    logger * log = other.get_logger();
    if (log != nullptr) log->debug(get_typename() + start + function);

    if (this == &other)
    {
        debug_with_guard(get_typename() + end + function);
        return *this;
    }

    if (_trusted_memory != nullptr) deallocate_with_guard(_trusted_memory);
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    if (log != nullptr) log->debug(get_typename() + end + function);
    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    allocator *parent_allocator,
    logger *_logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    std::string function = "Constructor\n";

    if (_logger != nullptr) _logger->debug(get_typename() + start + function);

    // meta for trusted memory: parent allocator, logger, fit mode, ptr to first occupied block, mutex, size of trusted memory
    size_t meta_size = sizeof(size_t) + sizeof(allocator *) + sizeof(logger *) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(void*) + sizeof(std::mutex);
    // meta for block: allocator, ptr to previous block, ptr to next block, size of block
    size_t block_meta_size = sizeof(allocator *) + 2 * sizeof(void*) + sizeof(size_t);

    // if space can't allocate at least one occupied block with 0 space (meta only), allocator can't be created
    if (space_size < block_meta_size + sizeof(void*))
    {
        std::string error = get_typename() + start + "Not enough space for allocator\n";
        if (_logger != nullptr) _logger->error(error);
        throw std::logic_error(error);
    }

    // allocating memory for trusted memory
    try
    {
        if (parent_allocator != nullptr) _trusted_memory = parent_allocator->allocate(space_size + meta_size, 1);
        else _trusted_memory = :: operator new(meta_size + space_size);
    }
    catch(const std::exception& e)
    {
        std::string error = get_typename() + start + "Bad alloc corrupted while allocating space\n";
        if (_logger != nullptr) _logger->error(error);
        throw std::logic_error(error);
    }

    // bringing memory to bytes for placing meta
    unsigned char * memory_ptr = reinterpret_cast<unsigned char *>(_trusted_memory);

    // placing meta
    *reinterpret_cast<size_t*>(memory_ptr) = space_size;
    memory_ptr += sizeof(size_t);
    *reinterpret_cast<allocator**>(memory_ptr) = parent_allocator;
    memory_ptr += sizeof(allocator*);
    *reinterpret_cast<logger**>(memory_ptr) = _logger;
    memory_ptr += sizeof(logger*);
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(memory_ptr) = allocate_fit_mode;
    memory_ptr += sizeof(allocator_with_fit_mode::fit_mode);
    allocator::construct(reinterpret_cast<std::mutex *>(memory_ptr));
    memory_ptr += sizeof(std::mutex);
    *reinterpret_cast<void**>(memory_ptr) = nullptr; // ptr to first occupied block, but all blocks are free
    memory_ptr += sizeof(void*);

    if (_logger != nullptr) _logger->debug(get_typename() + end + function);
}

[[nodiscard]] void *allocator_boundary_tags::allocate(
    size_t value_size,
    size_t values_count)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());
    std::string function = " Allocate\n";
    debug_with_guard(get_typename() + start + function);

    auto requested_size = value_size * values_count;

    if (requested_size < sizeof(void*))
    {
        requested_size = sizeof(void*);
        warning_with_guard(get_typename() + " Requested size has been changed\n");
    }

    allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();
    auto block_meta_size = sizeof(allocator*) + 2 * sizeof(void*) + sizeof(size_t);

    void * prev_occupied_block = nullptr;
    void * current_occupied_block = get_first_occupied_block();
    void * target_block = nullptr;
    size_t prev_size = 0;

    void * target_prev = nullptr;
    void * target_next = nullptr;

    size_t available_block_size = get_memory_size();
    void * current_available_block;

    // all memory is free
    if (current_occupied_block == nullptr && available_block_size >= requested_size + block_meta_size)
    {
        target_block = get_first_block();
    }

    while (current_occupied_block != nullptr)
    {
        if (prev_occupied_block == nullptr) current_available_block = get_first_block();
        else current_available_block = reinterpret_cast<unsigned char *>(prev_occupied_block) + block_meta_size + get_block_size(prev_occupied_block);

        if (current_available_block != current_occupied_block)
        {
            available_block_size = reinterpret_cast<unsigned char*>(current_occupied_block) - reinterpret_cast<unsigned char *>(current_available_block);
            if (available_block_size >= requested_size + block_meta_size)
            {
                if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit)
                {
                    target_block = current_available_block;
                    prev_size = available_block_size;
                    target_prev = prev_occupied_block;
                    target_next = current_occupied_block;
                    break;
                }
                else if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit)
                {
                    if (available_block_size < prev_size || prev_size == 0)
                    {
                        target_block = current_available_block;
                        prev_size = available_block_size;
                        target_prev = prev_occupied_block;
                        target_next = current_occupied_block;
                    }
                }
                else if (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit)
                {
                    if (available_block_size > prev_size)
                    {
                        target_block = current_available_block;
                        prev_size = available_block_size;
                        target_prev = prev_occupied_block;
                        target_next = current_occupied_block;
                    }
                }
            }
        }
        prev_occupied_block = current_occupied_block;
        current_occupied_block = get_next_block(current_occupied_block);
    }
    if (current_occupied_block == nullptr && prev_occupied_block != nullptr && prev_occupied_block != get_end())
    {
        current_available_block = reinterpret_cast<unsigned char *>(prev_occupied_block) + block_meta_size + get_block_size(prev_occupied_block);
        available_block_size = reinterpret_cast<unsigned char*>(get_end()) - reinterpret_cast<unsigned char *>(current_available_block);
        if (available_block_size >= requested_size + block_meta_size)
        {
            if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit && target_block == nullptr)
            {
                target_block = current_available_block;
                prev_size = available_block_size;
                target_prev = prev_occupied_block;
                target_next = current_occupied_block;
            }
            else if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit)
            {
                if (available_block_size < prev_size || prev_size == 0)
                {
                    target_block = current_available_block;
                    prev_size = available_block_size;
                    target_prev = prev_occupied_block;
                    target_next = current_occupied_block;
                }
            }
            else if (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit)
            {
                if (available_block_size > prev_size)
                {
                    target_block = current_available_block;
                    prev_size = available_block_size;
                    target_prev = prev_occupied_block;
                    target_next = current_occupied_block;
                }
            }
        }
    }

    if (target_block == nullptr) // no space to allocate block with such size
    {   
        error_with_guard(get_typename() + " Can't allocate block\n");
        throw std::bad_alloc();
    }

    if (current_occupied_block == nullptr && prev_occupied_block == nullptr) prev_size = get_memory_size();

    size_t blocks_sizes_difference = prev_size - (requested_size + block_meta_size);
    if (blocks_sizes_difference > 0 && blocks_sizes_difference < block_meta_size)
    {
        requested_size += blocks_sizes_difference;
        warning_with_guard(get_typename() + " Requested space size has been changed\n");
    }
    
    connect_blocks(target_prev, target_block);
    connect_blocks(target_block, target_next);
    
    *reinterpret_cast<size_t*>(target_block) = requested_size;
    *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char *>(target_block) + sizeof(size_t)) = this;

    void * result_block = reinterpret_cast<unsigned char *>(target_block) + block_meta_size;

    information_with_guard(get_typename() + " Available memory: " + std::to_string(get_available_memory()));

    debug_with_guard(make_blocks_info_string(get_blocks_info()));

    debug_with_guard(get_typename() + end + function);
    return result_block;
}

std::string allocator_boundary_tags::get_block_info(void * block) const noexcept
{
    // состояние блока
    unsigned char * bytes = reinterpret_cast<unsigned char *>(block);
    size_t size = get_block_size(bytes - sizeof(size_t) - sizeof(allocator*) - 2 * sizeof(void*));
    std::string bytes_array = "";
    for (block_size_t i = 0; i < size; ++i)
    {
        bytes_array += std::to_string(*bytes);
        bytes += sizeof(unsigned char);
        bytes_array += ' ';
    }
    return bytes_array;
}

void allocator_boundary_tags::deallocate(void *at)
{
    std::string function = " Deallocate\n";
    debug_with_guard(get_typename() + start + function);

    std::string block_info_array = get_block_info(at);
    debug_with_guard(get_typename() + " " + block_info_array);

    auto meta = 2 * sizeof(void*) + sizeof(size_t) + sizeof(allocator*);

    unsigned char * block = reinterpret_cast<unsigned char *>(at) - meta;
    if (get_block_allocator(block) != this)
    {
        std::string error = " Block doesn't belong to this allocator\n";
        error_with_guard(get_typename() + error);
        throw std::logic_error(error);
    }

    void * prev_block = get_prev_block(block);
    void * next_block =  get_next_block(block);

    connect_blocks(prev_block, next_block);

    clear_block(block);

    information_with_guard(get_typename() + " Available memory: " + std::to_string(get_available_memory()));

    debug_with_guard(make_blocks_info_string(get_blocks_info()));

    debug_with_guard(get_typename() + end + function);
}

size_t allocator_boundary_tags::get_available_memory() const noexcept
{
    void * current = get_first_occupied_block();
    if (current == nullptr) return get_memory_size();

    void * next = nullptr;

    size_t memory_occupied = 0;
    auto meta_size = sizeof(size_t) + sizeof(allocator*) + 2 * sizeof(void*);

    while (current != nullptr)
    {
        memory_occupied += get_block_size(current) + meta_size;
        current = get_next_block(current);
    }
    return get_memory_size() - memory_occupied;
}

void allocator_boundary_tags::clear_block(void * block) const noexcept
{
    unsigned char * block_ptr = reinterpret_cast<unsigned char *>(block);
    size_t * block_size = reinterpret_cast<size_t*>(block);
    block_size = nullptr;

    block_ptr += sizeof(size_t);
    allocator ** alc = reinterpret_cast<allocator**>(block_ptr);
    alc = nullptr;

    block_ptr += sizeof(allocator*);
    void ** prev = reinterpret_cast<void**>(block_ptr);
    prev = nullptr;

    block_ptr += sizeof(void*);
    void ** next = reinterpret_cast<void**>(block_ptr);
    next = nullptr;
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t)) = mode;
}

inline allocator *allocator_boundary_tags::get_allocator() const
{
    return *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t));
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> info;
    void * current = get_first_occupied_block();
    if (current == nullptr)
    {
        allocator_test_utils::block_info block;
        block.block_size = get_memory_size();
        block.is_block_occupied = false;
        info.push_back(block);
        return info;
    }

    void * prev = nullptr;

    auto meta_size = sizeof(size_t) + sizeof(allocator*) + 2 * sizeof(void*);

    while (current != nullptr)
    {
        if ((prev == nullptr && current != get_first_block()) || (prev != nullptr && (reinterpret_cast<unsigned char *>(prev) + meta_size + get_block_size(prev) != current)))
        {
            size_t size;
            if (prev == nullptr) size = reinterpret_cast<unsigned char *>(current) - (reinterpret_cast<unsigned char*>(get_first_block()));
            else size = reinterpret_cast<unsigned char *>(current) - (reinterpret_cast<unsigned char*>(prev) + meta_size + get_block_size(prev));

            allocator_test_utils::block_info avail_block;
            avail_block.block_size = size;
            avail_block.is_block_occupied = false;
            info.push_back(avail_block);
        }
        allocator_test_utils::block_info occup_block;
        occup_block.block_size = get_block_size(current);
        occup_block.is_block_occupied = true;
        info.push_back(occup_block);
        prev = current;
        current = get_next_block(current);
    }
    if (prev != get_end() && prev != nullptr)
    {
        size_t size = reinterpret_cast<unsigned char *>(get_end()) - (reinterpret_cast<unsigned char*>(prev) + meta_size + get_block_size(prev));
        allocator_test_utils::block_info avail_block;
        avail_block.block_size = size;
        avail_block.is_block_occupied = false;
        if (size != 0) info.push_back(avail_block);
    }
    return info;
}

std::string allocator_boundary_tags::make_blocks_info_string(std::vector<allocator_test_utils::block_info> info) const noexcept
{
    std::string blocks = "Blocks info:\n";
    int num = 1;
    for (auto &block : info)
    {
        std::string small_info = "\t" + std::to_string(num++) + ". <" + block_status(block.is_block_occupied) + "> <" + std::to_string(block.block_size) + ">\n";
        blocks += small_info;
    }
    return blocks;
}

std::string allocator_boundary_tags::block_status(bool state) const noexcept
{
    if (state) return occupied;
    return available;
}

inline logger *allocator_boundary_tags::get_logger() const
{
    return *reinterpret_cast<logger**>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*));
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return type;
}

inline allocator_with_fit_mode::fit_mode allocator_boundary_tags::get_fit_mode()
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char *>(_trusted_memory)  + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*));
}

void * allocator_boundary_tags::get_first_occupied_block() const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + 
        sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex));
}

void * allocator_boundary_tags::get_first_block() const noexcept
{
    return reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + 
        sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(void*);
}

size_t allocator_boundary_tags::get_block_size(void * block) const noexcept
{
    return *reinterpret_cast<size_t*>(block);
}

 void * allocator_boundary_tags::get_end() const noexcept
 {
    return reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) + 
        sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(void*) + get_memory_size();
 }

allocator * allocator_boundary_tags::get_block_allocator(void * block) const noexcept
{
    return *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char *>(block) + sizeof(size_t));
}

void * allocator_boundary_tags::get_prev_block(void * block) const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + sizeof(allocator*) + sizeof(size_t));
}

void * allocator_boundary_tags::get_next_block(void * block) const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + sizeof(allocator*) + sizeof(size_t) + sizeof(void*));
}

void allocator_boundary_tags::connect_blocks(void * prev, void * next) noexcept
{
    if (prev != nullptr)
    {
        void ** new_next = reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(prev) + sizeof(size_t) + sizeof(allocator*) + sizeof(void*));
        *new_next = next;
    }
    else set_first_occupied_block(next);
    if (next != nullptr)
    {
        void ** new_prev = reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(next) + sizeof(size_t) + sizeof(allocator*));
        *new_prev = prev;
    }
}

void allocator_boundary_tags::set_first_occupied_block(void * block) const noexcept
{
    *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) +
         sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex)) = block;
}

std::mutex & allocator_boundary_tags::get_mutex() const noexcept
{
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + 
        sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode));
}

size_t allocator_boundary_tags::get_memory_size() const noexcept
{
    return *reinterpret_cast<size_t*>(_trusted_memory);
}