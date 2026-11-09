#include "memoria.h"
#include <cstdlib>
#include <new>

std::size_t mem_actual = 0;
std::size_t mem_pico = 0;

static constexpr std::size_t ENCABEZADO =
    alignof(std::max_align_t);

double mb(std::size_t nbytes) {
    return nbytes / (1024.0 * 1024.0);
}

void* operator new(std::size_t size) {

    if (size == 0) {
        size = 1;
    }

    void* base = std::malloc(size + ENCABEZADO);

    if (base == nullptr) {
        throw std::bad_alloc();
    }

    *static_cast<std::size_t*>(base) = size;

    mem_actual += size;

    if (mem_actual > mem_pico) {
        mem_pico = mem_actual;
    }

    return static_cast<char*>(base) + ENCABEZADO;
}

void operator delete(void* p) noexcept {

    if (p == nullptr) {
        return;
    }

    char* base = static_cast<char*>(p) - ENCABEZADO;

    std::size_t size =
        *reinterpret_cast<std::size_t*>(base);

    mem_actual -= size;

    std::free(base);
}

void operator delete(void* p, std::size_t) noexcept {
    operator delete(p);
}