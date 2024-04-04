#include <not_implemented.h>

#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list()
{
    std::string type = get_typename();
    std::string function = " Destructor\n";
    trace_with_guard(type + function);
    deallocate_with_guard(_trusted_memory);
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list const &other) : _trusted_memory(other._trusted_memory)
{
    std::string type = get_typename();
    std::string function = " Copy constructor\n";
    trace_with_guard("[Begin] " + type + function);
    trace_with_guard("[End] " + type + function);
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list const &other)
{
    std::string type = get_typename();
    std::string function = " Copy operator\n";
    trace_with_guard("[Begin] " + type + function);
    if (this == &other) return *this;
    _trusted_memory = other._trusted_memory;
    trace_with_guard("[End] " + type + function);
    return *this;
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept : _trusted_memory(std::move(other._trusted_memory))
{
    std::string type = get_typename();
    std::string function = " Move constructor\n";
    trace_with_guard("[Begin] " + type + function);
    trace_with_guard("[End] " + type + function);
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    std::string type = get_typename();
    std::string function = " Move operator\n";
    trace_with_guard("[Begin] " + type + function);
    if (this == &other) return *this;
    _trusted_memory = std::move(other._trusted_memory);
    trace_with_guard("[End] " + type + function);
    return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    std::string function = " Constructor\n";
    std::string space_error = "Not enough space for allocator\n";
    std::string type = get_typename();

    if (logger != nullptr) logger->trace("[Begin] " + type + function);

    auto meta_size = sizeof(size_t) + sizeof(allocator *) + sizeof(class logger *) + sizeof(allocator_with_fit_mode::fit_mode);
    auto block_meta_size = sizeof(size_t) + sizeof(void*);

    if (space_size < block_meta_size + sizeof(void*))
    {
        if (logger != nullptr) logger->error(type + " " + space_error);
        throw std::logic_error(space_error);
    }
    auto result_size = space_size + meta_size;
    
    try
    {
        if (parent_allocator != nullptr) _trusted_memory = parent_allocator -> allocate(result_size, 1);
        else _trusted_memory = :: operator new(result_size);
    }
    catch(std::bad_alloc const &ex)
    {
        std::string error = "Bad alloc while allocating trusted memory\n";
        if (logger != nullptr) logger->error(type + " " + error);
        throw ex;
    }

    allocator **parent_allocator_space_adress = reinterpret_cast<allocator**>(_trusted_memory);
    *parent_allocator_space_adress = parent_allocator;

    class logger **logger_space_adress = reinterpret_cast<class logger **>(parent_allocator_space_adress + 1);
    *logger_space_adress = logger;
    
    size_t *size_space_adress = reinterpret_cast<size_t *>(logger_space_adress + 1);
    *size_space_adress = space_size;

    allocator_with_fit_mode::fit_mode * fit_mode_space_adress = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(size_space_adress + 1);
    *fit_mode_space_adress = allocate_fit_mode;

    void **first_block_space_adress = reinterpret_cast<void **>(fit_mode_space_adress + 1);
    *first_block_space_adress = reinterpret_cast<void **>(first_block_space_adress + 1); // указатель на первый свободный блок

    *reinterpret_cast<void**>(*first_block_space_adress) = nullptr; // указатель на следующий свободный блок

    *reinterpret_cast<size_t*>(reinterpret_cast<void**>(*first_block_space_adress) + 1) = space_size;
    trace_with_guard("[End] " + type + function);
}


[[nodiscard]] void *allocator_sorted_list::allocate(size_t value_size, size_t values_count)
{
    std::string function = " Allocate\n";
    std::string type = get_typename();
    trace_with_guard("[Begin] " + type + function);


    auto requested_size = value_size * values_count;
    if (requested_size < sizeof(void*))
    {
        requested_size = sizeof(void*);
        warning_with_guard(type + " Requested size has been changed\n");
    }

    allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();

    auto meta_size = sizeof(size_t) + sizeof(allocator*);
    auto result_size = meta_size + requested_size;

    void * block = nullptr;
    void * prev_target = nullptr;
    void * next_target = nullptr;
    size_t prev_size = 0;

    void * current = get_first_available_block();
    void * previous = nullptr;

    while (current != nullptr)
    {
        size_t current_block_size = get_available_block_size(current);
        if (current_block_size == 0) break;
        if (current_block_size >= requested_size)
        {
            if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit)
            {
                block = current;
                prev_target = previous;
                next_target = get_available_block_next_block_address(current);
                prev_size = current_block_size;
                break;
            }
            else if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit)
            {
                if (current_block_size < prev_size || prev_size == 0)
                {
                    block = current;
                    prev_target = previous;
                    next_target = get_available_block_next_block_address(current);
                    prev_size = current_block_size;
                }
            }
            else if (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit)
            {
                if (current_block_size > prev_size)
                {
                    block = current;
                    prev_target = previous;
                    next_target = get_available_block_next_block_address(current);
                    prev_size = current_block_size;
                }
            }
        }
        previous = current;
        current = get_available_block_next_block_address(current);
    }
    if (block == nullptr)
    {
        error_with_guard(type + " Cannot allocate block\n");
        throw std::bad_alloc();
    }
    auto blocks_sizes_difference = get_available_block_size(block) - result_size;
    if (blocks_sizes_difference > 0 && blocks_sizes_difference < meta_size)
    {
        warning_with_guard("requested space size was changed\n");
        result_size = get_available_block_size(block);
    }

    void ** new_next = reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + result_size);
    *reinterpret_cast<size_t*>(new_next + 1) = blocks_sizes_difference - sizeof(void*) - sizeof(size_t);
    if (new_next + blocks_sizes_difference != next_target) *new_next = next_target; // если справа занятый
    else merge_blocks(new_next, 0, next_target); // если справа свободный
    
    if (prev_target != nullptr)
    {
        void **next_block_adress = reinterpret_cast<void **>(prev_target);
        *next_block_adress = next_target;
    }
    else set_first_available_block(new_next);

    void ** prev_adress = reinterpret_cast<void**>(block);
    *prev_adress = nullptr;

    size_t * size = reinterpret_cast<size_t*>(block);
    *size = requested_size;

    allocator ** alc = reinterpret_cast<allocator**>(size + 1);
    *alc = get_allocator();

    void * result_block = reinterpret_cast<unsigned char *>(block) + meta_size;
    trace_with_guard("[End] " + type + function);
    return result_block;
}

void allocator_sorted_list::deallocate(void *at)
{
    std::string function = " Deallocate\n";
    std::string type = get_typename();

    trace_with_guard("[Begin] " + type + function);

    size_t meta_size = sizeof(allocator*) + sizeof(size_t);

    void * block = reinterpret_cast<unsigned char *>(at) - meta_size;
    size_t size = get_occupied_block_size(block) + sizeof(size_t) + sizeof(allocator*);
    if (get_occupied_block_allocator(block) != get_allocator())
    {
        std::string error = "Block doesn't belong to this allocator";
        error_with_guard(error);
        throw std::logic_error(error);
    }
    void * current_available = get_first_available_block();
    void * previous_available = get_first_block();
    void * next_available = nullptr;

    while (current_available != nullptr) // идем по списку свободных
    {
        size_t size_difference = sizeof(reinterpret_cast<unsigned char *>(current_available) - (reinterpret_cast<unsigned char *>(previous_available) + get_available_block_size(previous_available) + sizeof(void*) + sizeof(size_t)));
        if (size_difference > size)
        {
            void * current_occupied = previous_available;
            while (current_occupied != current_available)
            {
                if (current_occupied == block) break;
                current_occupied = *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(current_occupied) + sizeof(size_t) + sizeof(allocator*) + get_occupied_block_size(current_occupied));
            }
            if (current_occupied == block) break; // нашли блок
        }
        else if (size_difference == size && previous_available == block) break; // нашли блок
        else if (size_difference <= 0 && previous_available == block) break; // нашли блок

        previous_available = current_available;
        current_available = get_available_block_next_block_address(current_available);
    }

    if (previous_available == get_first_block()) // если блок в начале
    {
        if (reinterpret_cast<unsigned char *>(block) + 
            sizeof(size_t) + sizeof(allocator*) + size == current_available) // справа свободный
        {
            merge_blocks(block, 1, current_available);
        }
        else // справа занятый
        {
            size_t * old_size_ptr = reinterpret_cast<size_t*>(block);
            old_size_ptr = nullptr;
            void ** new_adress_ptr = reinterpret_cast<void**>(block);
            *new_adress_ptr = current_available;
            *reinterpret_cast<size_t*>(new_adress_ptr + 1) = size;
        }
        set_first_available_block(block);
        trace_with_guard("[End] " + type + function);
        return;
    }

    if (current_available == *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + sizeof(size_t) + 
    get_occupied_block_size(block)) && current_available != nullptr) // если справа свободный
    {
        // сначала мерж с правым
        merge_blocks(block, 1, current_available);

        //меняем указатель у левого
        *reinterpret_cast<void**>(previous_available) = block;

        if (*reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(previous_available) + sizeof(size_t) + 
        sizeof(void*) + get_available_block_size(previous_available)) == block) // если слева свободный
        {
            // мержим с левым
            merge_blocks(previous_available, 0, block);
        }
        // если слева занятый, то ничего не надо
    }
    else if (current_available == *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + sizeof(size_t) + 
    get_occupied_block_size(block)) && current_available == nullptr) // если справа конец
    {
        size_t * old_size = reinterpret_cast<size_t*>(block);
        old_size = nullptr;
        void ** new_adress = reinterpret_cast<void**>(block);
        *new_adress = nullptr;
        *reinterpret_cast<size_t*>(new_adress + 1) = size;

        //меняем указатель у левого
        *reinterpret_cast<void**>(previous_available) = block;

        if (*reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(previous_available) + sizeof(size_t) + 
        sizeof(void*) + get_available_block_size(previous_available)) == block) // если слева свободный
        {
            merge_blocks(previous_available, 0, block);
        }
    }
    else // если справа занятый
    {
        size_t * old_size = reinterpret_cast<size_t*>(block);
        old_size = nullptr;
        void ** new_adress = reinterpret_cast<void**>(block);
        *new_adress = get_available_block_next_block_address(current_available);
        *reinterpret_cast<size_t*>(new_adress + 1) = size;

        //меняем указатель у левого
        *reinterpret_cast<void**>(previous_available) = block;

        if (*reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(previous_available) + sizeof(size_t) + 
        sizeof(void*) + get_available_block_size(previous_available)) == block) // если слева свободный
        {
            merge_blocks(previous_available, 0, block);
        }
    }
    trace_with_guard("[End] " + type + function);
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<logger **>(reinterpret_cast<allocator**>(_trusted_memory) + 1) + 1) = mode;
}

inline allocator_with_fit_mode::fit_mode allocator_sorted_list::get_fit_mode()
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<logger **>(reinterpret_cast<allocator**>(_trusted_memory) + 1) + 1 + sizeof(size_t));
}

inline allocator *allocator_sorted_list::get_allocator() const
{
    return *reinterpret_cast<allocator**>(_trusted_memory);
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
 // TODO
}

inline logger *allocator_sorted_list::get_logger() const
{
    return *reinterpret_cast<logger**>(reinterpret_cast<allocator **>(_trusted_memory) + 1);
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
    return "[Allocator Sorted List]";
}

void *allocator_sorted_list::get_first_available_block() const noexcept
{
    return *reinterpret_cast<void **>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator *) + sizeof(logger *) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode));
}

allocator::block_size_t allocator_sorted_list::get_available_block_size(void *block_address) noexcept
{
    return *reinterpret_cast<allocator::block_size_t *>(reinterpret_cast<void **>(block_address) + 1);
}

void *allocator_sorted_list::get_available_block_next_block_address(void *block_address) noexcept
{
    return *reinterpret_cast<void **>(block_address);
}

allocator::block_size_t allocator_sorted_list::get_occupied_block_size(void *block_address) noexcept
{
    return *reinterpret_cast<allocator::block_size_t *>(block_address);
}

void allocator_sorted_list::set_first_available_block(void * first_available_block) const noexcept
{
    *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t)) = first_available_block;
}

void * allocator_sorted_list::get_first_block() const noexcept
{
    return *reinterpret_cast<void **>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator *) + sizeof(logger *) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(void*));
}

void allocator_sorted_list::clear_available_block(void * block) const noexcept
{
    void ** adress = reinterpret_cast<void**>(block);
    size_t * size = reinterpret_cast<size_t*>(adress + 1);
    adress = nullptr;
    size = nullptr;
}

void allocator_sorted_list::merge_blocks(void * first, int type, void * second) noexcept
{
    void * next_adress = get_available_block_next_block_address(second);
    size_t second_size = get_available_block_size(second);
    size_t first_size;
    if (type == 0) // свободный
    {
        first_size = get_available_block_size(first);
    }
    else // занятый
    {
        first_size = get_occupied_block_size(first);
        size_t * old_size = reinterpret_cast<size_t*>(first);
        old_size = nullptr;
        allocator ** alc = reinterpret_cast<allocator**>(old_size + 1);
        alc = nullptr;
    }
    void ** new_adress_ptr = reinterpret_cast<void**>(first);
    *new_adress_ptr = next_adress;
    *reinterpret_cast<size_t*>(new_adress_ptr + 1) = first_size + second_size;
    clear_available_block(second);
}

allocator * allocator_sorted_list::get_occupied_block_allocator(void * block) const noexcept
{
    return *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char *>(block) + sizeof(size_t));
}