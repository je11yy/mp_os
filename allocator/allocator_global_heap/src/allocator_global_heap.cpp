#include <not_implemented.h>

#include <allocator_global_heap.h>

#include <utility>

allocator_global_heap::allocator_global_heap(
    logger *logger)
{
    trace_with_guard("[Begin] [Allocator Global Heap] Constructor\n");
    _logger = logger;
    trace_with_guard("[End] [Allocator Global Heap] Constructor\n");
}

allocator_global_heap::~allocator_global_heap()
{
    trace_with_guard("[Begin] [Allocator Global Heap] Destructor\n");
    trace_with_guard("[End] [Allocator Global Heap] Destructor\n");
}

allocator_global_heap::allocator_global_heap(
    allocator_global_heap &&other) noexcept : _logger(std::move(other._logger))
{
    trace_with_guard("[Begin] [Allocator Global Heap] Move constructor\n");
    trace_with_guard("[End] [Allocator Global Heap] Move constructor\n");
}

allocator_global_heap &allocator_global_heap::operator=(
    allocator_global_heap &&other) noexcept
{
    trace_with_guard("[Begin] [Allocator Global Heap] Move operator\n");
    if (this == &other) return *this;
    _logger =  std::move(other._logger);
    trace_with_guard("[End] [Allocator Global Heap] Move operator\n");
    return *this;
}

[[nodiscard]] void *allocator_global_heap::allocate(size_t value_size, size_t values_count)
{
    debug_with_guard("[Begin] [Allocator Global Heap] Allocate function\n");

    block_size_t requested_size = value_size * values_count;
    if (requested_size < sizeof(void*))
    {
        requested_size = sizeof(void*);
        warning_with_guard("[Allocator Global Heap] Requested size has been changed\n");
    }
    block_size_t meta_size = sizeof(allocator*) + sizeof(size_t);

    block_size_t size = requested_size + meta_size;

    block_pointer_t new_block;
    try
    {
        new_block = ::operator new(size);
    }
    catch(std::bad_alloc const &ex)
    {
        error_with_guard("[Allocate function] Can't allocate\n");
    }

    block_pointer_t ptr = new_block;

    *(reinterpret_cast<allocator **>(ptr)) = this;
    // TODO: this is bad :(
    ptr += sizeof(allocator*);
    *(reinterpret_cast<size_t *>(ptr)) = requested_size;

    block_pointer_t result_block = reinterpret_cast<uint8_t*>(new_block) + meta_size;

    debug_with_guard("[End] [Allocator Global Heap] Allocate function\n");

    return result_block;
}

void allocator_global_heap::deallocate(void *at)
{
    debug_with_guard("[Begin] [Allocator Global Heap] Deallocate function\n");
    if (at == nullptr) return; // уже очищено?

    block_size_t meta_size = sizeof(allocator*) + sizeof(size_t);
    block_pointer_t block = reinterpret_cast<uint8_t*>(at) - meta_size;
    
    if ((*(reinterpret_cast<allocator **>(block))) != this)
    {
        if (get_logger()) _logger->error("[Deallocate function] Wrong allocator\n");
        throw std::logic_error("[Deallocate function] Wrong allocator\n");
    }

    block_size_t size = *reinterpret_cast<size_t*>(reinterpret_cast<uint8_t*>(block) + sizeof(size_t));

    // состояние блока
    unsigned char * bytes = reinterpret_cast<unsigned char *>(at);
    std::string bytes_array;
    for (block_size_t i = 0; i < size; ++i)
    {
        bytes_array += std::to_string(*bytes);
        bytes += sizeof(unsigned char);
        bytes_array += ' ';
    }

    if (get_logger()) 
    {
        if (size != 0) _logger->debug("Block info: " + bytes_array + "\n");
        else _logger->debug("Block info: block is empty\n");
    }

    ::operator delete(block);

    debug_with_guard("[End] [Allocator Global Heap] Deallocate function\n");
}

inline logger *allocator_global_heap::get_logger() const
{
    return _logger;
}

inline std::string allocator_global_heap::get_typename() const noexcept
{
    trace_with_guard("[Begin] [Allocator Gloval Heap] Get_typename function\n");
    trace_with_guard("[End] [Allocator Gloval Heap] Get_typename function\n");
    return "allocator_global_heap";
}