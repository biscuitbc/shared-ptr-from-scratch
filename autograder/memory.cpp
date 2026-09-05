#include "memory.hpp"

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>

namespace {
long blocks = 0;
long fail_after = -1;

void* allocate(std::size_t size, std::size_t alignment = 0) {
    if (fail_after >= 0) {
        if (fail_after-- == 0) {
            throw std::bad_alloc();
        }
    }
    if (size == 0) size = 1;
    void* ptr = nullptr;
    if (alignment == 0) {
        ptr = std::malloc(size);
    } else {
        if (size > std::numeric_limits<std::size_t>::max() - (alignment - 1)) {
            throw std::bad_alloc();
        }
        const auto rounded = ((size + alignment - 1) / alignment) * alignment;
        ptr = std::aligned_alloc(alignment, rounded);
    }
    if (!ptr) throw std::bad_alloc();
    ++blocks;
    return ptr;
}

void deallocate(void* ptr) noexcept {
    if (ptr) {
        --blocks;
        std::free(ptr);
    }
}
} // namespace / 匿名命名空间

long lab_memory::outstanding() noexcept { return blocks; }
long lab_memory::exchange_failure(long after) noexcept {
    const long previous = fail_after;
    fail_after = after;
    return previous;
}

void* operator new(std::size_t n) { return allocate(n); }
void* operator new[](std::size_t n) { return allocate(n); }
void operator delete(void* p) noexcept { deallocate(p); }
void operator delete[](void* p) noexcept { deallocate(p); }
void operator delete(void* p, std::size_t) noexcept { deallocate(p); }
void operator delete[](void* p, std::size_t) noexcept { deallocate(p); }
void* operator new(std::size_t n, std::align_val_t a) {
    return allocate(n, static_cast<std::size_t>(a));
}
void* operator new[](std::size_t n, std::align_val_t a) {
    return allocate(n, static_cast<std::size_t>(a));
}
void operator delete(void* p, std::align_val_t) noexcept { deallocate(p); }
void operator delete[](void* p, std::align_val_t) noexcept { deallocate(p); }
void operator delete(void* p, std::size_t, std::align_val_t) noexcept { deallocate(p); }
void operator delete[](void* p, std::size_t, std::align_val_t) noexcept { deallocate(p); }
void* operator new(std::size_t n, const std::nothrow_t&) noexcept {
    try { return allocate(n); } catch (...) { return nullptr; }
}
void* operator new[](std::size_t n, const std::nothrow_t&) noexcept {
    try { return allocate(n); } catch (...) { return nullptr; }
}
void operator delete(void* p, const std::nothrow_t&) noexcept { deallocate(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept { deallocate(p); }
void* operator new(std::size_t n, std::align_val_t a, const std::nothrow_t&) noexcept {
    try { return allocate(n, static_cast<std::size_t>(a)); } catch (...) { return nullptr; }
}
void* operator new[](std::size_t n, std::align_val_t a, const std::nothrow_t&) noexcept {
    try { return allocate(n, static_cast<std::size_t>(a)); } catch (...) { return nullptr; }
}
void operator delete(void* p, std::align_val_t, const std::nothrow_t&) noexcept { deallocate(p); }
void operator delete[](void* p, std::align_val_t, const std::nothrow_t&) noexcept { deallocate(p); }
