#include <not_implemented.h>

#include "../include/allocator_buddies_system.h"

std::string start = " [START] ";
std::string end = " [END] ";
std::string type = "[Allocator Buddies System]";
std::string occupied = "<occup>";
std::string available = "<avail>";

allocator_buddies_system::~allocator_buddies_system()
{
    std::string function = "Destructor\n";
    debug_with_guard(get_typename() + start + function);

    logger * _logger = get_logger();
    allocator::destruct(&get_mutex());
    deallocate_with_guard(_trusted_memory);

    if (_logger != nullptr) _logger->debug(get_typename() + end + function);
}

allocator_buddies_system::allocator_buddies_system(
    allocator_buddies_system &&other) noexcept
{
    std::string function = "Move constructor\n";
    debug_with_guard(get_typename() + start + function);

    if (_trusted_memory) deallocate_with_guard(_trusted_memory);
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    debug_with_guard(get_typename() + end + function);
}

allocator_buddies_system &allocator_buddies_system::operator=(
    allocator_buddies_system &&other) noexcept
{
    std::string function = "Move operator\n";
    debug_with_guard(get_typename() + start + function);

    if (this == &other)
    {
        debug_with_guard(get_typename() + end + function);
        return *this;
    }

    if (_trusted_memory) deallocate_with_guard(_trusted_memory);
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    debug_with_guard(get_typename() + end + function);
    return *this;
}

allocator_buddies_system::allocator_buddies_system(
    size_t space_size,
    allocator *parent_allocator,
    logger *_logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    // на вход подается степень?

    std::string function = "Constructor\n";
    if (_logger) _logger->debug(get_typename() + start + function);

    auto available_block_meta_size = get_available_block_meta_size();

    if (space_size < get_degree(available_block_meta_size)) // так как мета свободного блока много больше занятого
    {
        std::string error = "Space size is too small\n";
        if (_logger) _logger->error(error);
        throw std::logic_error(error);
    }

    auto allocator_meta_size = get_allocator_meta_size();
    auto memory_size = 1 << space_size;

    try
    {
        if (parent_allocator) _trusted_memory = parent_allocator->allocate(memory_size + allocator_meta_size, 1);
        else _trusted_memory = :: operator new(memory_size + allocator_meta_size);
    }
    catch(const std::bad_alloc& e)
    {
        if (_logger) _logger->error(get_typename() + start + "Bad alloc corrupted while allocating trusted memory\n");
        throw e;
    }
    
    unsigned char * memory = reinterpret_cast<unsigned char *>(_trusted_memory);

    *memory = static_cast<signed char>(space_size); // сколько всего памяти в аллокаторе (степень двойки)
    memory++;

    *reinterpret_cast<size_t *>(memory) = memory_size; // сколько свободной памяти в аллокаторе
    memory += sizeof(size_t);

    *reinterpret_cast<allocator **>(memory) = parent_allocator;
    memory += sizeof(allocator *);

    *reinterpret_cast<logger **>(memory) = _logger;
    memory += sizeof(logger *);

    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(memory) = allocate_fit_mode;
    memory += sizeof(allocator_with_fit_mode::fit_mode);

    allocator::construct(reinterpret_cast<std::mutex *>(memory));
    memory += sizeof(std::mutex);

    // указатель на первый свободный блок
    *reinterpret_cast<void **>(memory) = memory + sizeof(void*);
    memory += sizeof(void*);
    available_meta_creation(memory, static_cast<signed char>(space_size), nullptr, nullptr);

    if (_logger) _logger->debug(get_typename() + end + function);
}

[[nodiscard]] void *allocator_buddies_system::allocate(
    size_t value_size,
    size_t values_count)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    std::string function = "Allocate\n";
    debug_with_guard(get_typename() + start + function);

    auto requested_size = value_size * values_count + get_occupied_block_meta_size();

    unsigned char degree = get_degree(requested_size);
    unsigned char available_meta_degree = get_degree(get_available_block_meta_size());
    unsigned char occupied_meta_degree = get_degree(get_occupied_block_meta_size())
;
    if (degree < (available_meta_degree > occupied_meta_degree ? available_meta_degree : occupied_meta_degree))
    {
        degree = (available_meta_degree > occupied_meta_degree ? available_meta_degree : occupied_meta_degree);
        warning_with_guard("Requested size has been changed\n");
    }

    if (requested_size > get_available_memory())
    {
        error_with_guard("Unable to allocate block cause of lack of memory\n");
        throw std::bad_alloc{};
    }

    allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();

    void * target = nullptr;

    void * current = get_first_available_block();
    void * previous = nullptr;

    if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit)
    {
        while (current)
        {
            if (abs(get_block_degree(current)) >= degree)
            {
                target = current;
                break;
            }
            current = get_next_block(current);
        }
    }
    else if (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit || fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit)
    {
        unsigned char best_degree;
        if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit) best_degree = abs(get_degree(get_available_memory()));
        else best_degree = 0;
        while (current)
        {
            bool compare;
            if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit) compare = abs(get_block_degree(current)) <= best_degree;
            else compare = abs(get_block_degree(current)) >= best_degree;

            if (best_degree >= degree && compare) target = current;
            current = get_next_block(current);
        }
    }

    if (!target)
    {
        error_with_guard("Unable to allocate block\n");
        throw std::bad_alloc();
    }

    target = split(target, degree);

    decrease_available_space(requested_size);

    information_with_guard("Available space size: " + std::to_string(get_available_memory()));

    debug_with_guard(make_blocks_info_string(get_blocks_info()));

    debug_with_guard(get_typename() + end + function);

    return reinterpret_cast<unsigned char *>(target) + get_occupied_block_meta_size();
}

std::string allocator_buddies_system::make_blocks_info_string(std::vector<allocator_test_utils::block_info> info) const noexcept
{
    std::string blocks = "Blocks info:\n";
    int num = 1;
    for (auto &block : info)
    {
        std::string small_info = "\t" + std::to_string(num++) + ". " + block_status(block.is_block_occupied)+ " <" + std::to_string(block.block_size) + ">\n";
        blocks += small_info;
    }
    return blocks;
}

std::string allocator_buddies_system::block_status(bool state) const noexcept
{
    if (state) return occupied;
    return available;
}

void * allocator_buddies_system::split(void * block, unsigned char &degree) const noexcept
{
    void * current = block;

    void * buddy = nullptr;

    while (abs(get_block_degree(current)) - 1 > degree)
    {
        buddy = get_buddy(current, 1 << (abs(get_block_degree(current)) - 1));
        create_available_block(buddy, abs(get_block_degree(current)) - 1, current);
    }

    connect(get_previous_block(current), get_next_block(current));

    occupied_meta_creation(reinterpret_cast<unsigned char *>(current), abs(get_block_degree(current)));

    return block;
}

void allocator_buddies_system::create_available_block(void * block, signed char degree, void * previous) const noexcept
{
    void * next = get_next_block(previous);
    if (next)
    {
        available_meta_creation(reinterpret_cast<unsigned char *>(next), get_block_degree(next), block, get_next_block(next));
    }
    available_meta_creation(reinterpret_cast<unsigned char *>(block), degree, previous, next);
    available_meta_creation(reinterpret_cast<unsigned char *>(previous), degree, get_previous_block(previous), block);
}


void * allocator_buddies_system::get_buddy(void * block, size_t size) const noexcept
{
    unsigned char * start = reinterpret_cast<unsigned char *>(reinterpret_cast<unsigned char*>(_trusted_memory) + get_allocator_meta_size());
    return ((reinterpret_cast<unsigned char *>(block) - start) ^ (size)) + start;
}

void allocator_buddies_system::connect(void * prev, void * next) const noexcept
{
    if (prev) available_meta_creation(reinterpret_cast<unsigned char*>(prev), get_block_degree(prev), get_previous_block(prev), next);
    else set_first_available_block(next);

    if (next) available_meta_creation(reinterpret_cast<unsigned char*>(next), get_block_degree(next), prev, get_next_block(next));
}

void allocator_buddies_system::deallocate(
    void *at)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    std::string function = "Deallocate\n";
    debug_with_guard(get_typename() + start + function);

    void * block = reinterpret_cast<unsigned char*>(at) - get_occupied_block_meta_size();
    // block info

    if (get_occupied_block_allocator_beginning(block) != get_beginning_of_memory())
    {
        std::string error = "Wrong allocator\n";
        error_with_guard(error);
        throw std::logic_error(error);
    }

    merge(block, reinterpret_cast<unsigned char *>(_trusted_memory) + get_allocator_meta_size() + (1 << get_degree(get_available_memory())));

    increase_available_space(1 << abs(get_block_degree(block)));

    information_with_guard("Available memory size: " + std::to_string(get_available_memory()));

    debug_with_guard(make_blocks_info_string(get_blocks_info()));

    debug_with_guard(get_typename() + end + function);
}

void allocator_buddies_system::merge(void * block, void * end) const noexcept
{
    unsigned char degree = abs(get_block_degree(block));

    unsigned char * buddy = reinterpret_cast<unsigned char *>(get_buddy(block, 1 << degree));

    if (buddy == reinterpret_cast<unsigned char *>(end)) return;

    signed char buddy_degree = get_block_degree(buddy);

    if (buddy_degree > 0)
    {
        unsigned char * prev_available = reinterpret_cast<unsigned char *>(find_block(block, end));
        void * next = nullptr;
        if (!prev_available)
        {
            next = get_first_available_block();
            set_first_available_block(block);
        }
        else
        {
            next = get_next_block(prev_available);
            available_meta_creation(reinterpret_cast<unsigned char *>(prev_available), get_block_degree(prev_available), get_previous_block(prev_available), block);
        }

        available_meta_creation(reinterpret_cast<unsigned char *>(next), get_block_degree(next), block, get_next_block(next));
        available_meta_creation(reinterpret_cast<unsigned char *>(block), get_block_degree(block), prev_available, next);

        return;
    }

    if (buddy < reinterpret_cast<unsigned char*>(block))
    {
        available_meta_creation(buddy, abs(get_block_degree(buddy)) + 1, get_previous_block(buddy), get_next_block(buddy));
        merge(buddy, end);
    }
    else
    {
        available_meta_creation(reinterpret_cast<unsigned char*>(block), get_block_degree(block) + 1, get_previous_block(buddy), get_next_block(buddy));
        merge(block, end);
    }
}

void * allocator_buddies_system::find_block(void * block, void *end) const noexcept
{
    unsigned char * end_t = reinterpret_cast<unsigned char *>(end);
    unsigned char * current = reinterpret_cast<unsigned char *>(_trusted_memory) + get_allocator_meta_size();
    unsigned char * prev = nullptr;

    while (current != end)
    {
        if (current == block) return prev;
        if (get_block_degree(current) > 0) prev = current;
        current = current + (1 << abs(get_block_degree(current)));
    }

    return nullptr;
}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<unsigned char *>(_trusted_memory) + 1 + 
        sizeof(size_t) + sizeof(allocator*) + sizeof(logger*)) = mode;
}

void * allocator_buddies_system::get_first_available_block() const noexcept
{
    return *reinterpret_cast<void **>(reinterpret_cast<unsigned char *>(_trusted_memory) + 1 + 
        sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex));
}

void allocator_buddies_system::set_first_available_block(void * block) const noexcept
{
    *reinterpret_cast<void **>(reinterpret_cast<unsigned char *>(_trusted_memory) + get_allocator_meta_size() - sizeof(void*)) = block;
}

std::mutex& allocator_buddies_system::get_mutex() const noexcept
{
    return *reinterpret_cast<std::mutex *>(reinterpret_cast<unsigned char *>(_trusted_memory) + 1 + 
        sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode));
}

allocator_with_fit_mode::fit_mode allocator_buddies_system::get_fit_mode() const noexcept
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<unsigned char *>(_trusted_memory) + 
        1 + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*));
}

inline allocator *allocator_buddies_system::get_allocator() const
{
    return *reinterpret_cast<allocator **>(reinterpret_cast<unsigned char *>(_trusted_memory) + 1 + sizeof(size_t));
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> result;

    unsigned char *current = reinterpret_cast<unsigned char*>(_trusted_memory) + get_allocator_meta_size();

    unsigned char *allocatable_memory_end = current + (1 << get_block_degree(_trusted_memory));

    block_info info;
    
    while (current != allocatable_memory_end)
    {
        info.is_block_occupied = get_block_degree(current) > 0 ? true : false;
        info.block_size = 1 << abs(get_block_degree(current));
        result.push_back(info);

        current += (1 << abs(get_block_degree(current)));
    }

    return result;
}

inline logger *allocator_buddies_system::get_logger() const
{
    return *reinterpret_cast<logger **>(reinterpret_cast<unsigned char *>(_trusted_memory) + 1 + sizeof(size_t) + sizeof(allocator*));
}

inline std::string allocator_buddies_system::get_typename() const noexcept
{
    return type;
}

size_t allocator_buddies_system::get_allocator_meta_size() const noexcept
{
    // аллокатор, логгер, фит мод, объем текущей доступной памяти, мьютекс
    return sizeof(unsigned char) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(void*);
}

unsigned char allocator_buddies_system::get_degree(size_t number) const noexcept
{
    unsigned char degree = 0;
    while (number >>= 1) ++degree;
    return degree;
}

size_t allocator_buddies_system::get_available_memory() const noexcept
{
    return *reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(_trusted_memory) + 1);
}

void allocator_buddies_system::decrease_available_space(size_t &to_decrease) const noexcept
{
    *reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(_trusted_memory) + 1) = get_available_memory() - to_decrease;
}

void allocator_buddies_system::increase_available_space(size_t to_increase) const noexcept
{
    *reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(_trusted_memory) + 1) = get_available_memory() + to_increase;
}

void allocator_buddies_system::available_meta_creation(unsigned char * block, signed char degree, void * previous, void * next) const noexcept
{
    *reinterpret_cast<signed char *>(block) = -abs(degree); // отрицательное, так как свободен
    *reinterpret_cast<void**>(block + sizeof(signed char)) = previous;
    *reinterpret_cast<void**>(block + sizeof(signed char) + sizeof(void*)) = next;
}

void allocator_buddies_system::occupied_meta_creation(unsigned char * block, signed char degree) const noexcept
{
    *reinterpret_cast<int8_t *>(block) = abs(degree); // положительное, так как занят
    // указатель на начало памяти аллокатора
    *reinterpret_cast<unsigned char **>(block + sizeof(signed char)) = get_beginning_of_memory();
}

size_t allocator_buddies_system::get_available_block_meta_size() const noexcept
{
    return sizeof(signed char) + 2 * sizeof(void*);
}

size_t allocator_buddies_system::get_occupied_block_meta_size() const noexcept
{
    return sizeof(signed char) + sizeof(unsigned char*);
}

unsigned char * allocator_buddies_system::get_beginning_of_memory() const noexcept
{
    return reinterpret_cast<unsigned char*>(_trusted_memory);
}

size_t allocator_buddies_system::get_memory_size() const noexcept
{
    return static_cast<size_t>(*reinterpret_cast<unsigned char *>(_trusted_memory));
}

size_t allocator_buddies_system::get_block_size(void * block) const noexcept
{
    return 1 << abs(get_block_degree(block));
}

signed char allocator_buddies_system::get_block_degree(void * block) const noexcept
{
    return static_cast<signed char>(*reinterpret_cast<unsigned char*>(block));
}

void * allocator_buddies_system::get_next_block(void * block) const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + sizeof(signed char) + sizeof(void*));
}

void * allocator_buddies_system::get_previous_block(void * block) const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + sizeof(signed char));
}

unsigned char * allocator_buddies_system::get_occupied_block_allocator_beginning(void * block) const noexcept
{
    return *reinterpret_cast<unsigned char **>(reinterpret_cast<unsigned char*>(block) + sizeof(signed char));
}