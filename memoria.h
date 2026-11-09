//
// Created by mariajosn on 08/11/26.
//

#ifndef ALGORITMOS_ACTINTEGRADORA1_MEMORIA_H
#define ALGORITMOS_ACTINTEGRADORA1_MEMORIA_H

#pragma once
// Contador de memoria (equivalente a tracemalloc)
#include <new>
#include <cstdlib>
#include <cstddef>

inline std::size_t mem_actual = 0;   // bytes ocupados ahora
inline std::size_t mem_pico   = 0;   // máximo alcanzado
inline constexpr std::size_t ENCABEZADO = alignof(std::max_align_t);

inline double mb(std::size_t nbytes) { return nbytes / (1024.0 * 1024.0); }

// Cada vez que algo (un vector, por ejemplo) pide memoria, pasa por aquí
void* operator new(std::size_t size) {
    void* p = std::malloc(size + ENCABEZADO);
    if (!p) throw std::bad_alloc();
    *static_cast<std::size_t*>(p) = size;   // guardamos el tamaño para saber cuánto restar al liberar
    mem_actual += size;
    if (mem_actual > mem_pico) mem_pico = mem_actual;
    return static_cast<char*>(p) + ENCABEZADO;
}

// Cada vez que se libera memoria, pasa por aquí
void operator delete(void* p) noexcept {
    if (!p) return;
    char* base = static_cast<char*>(p) - ENCABEZADO;
    mem_actual -= *reinterpret_cast<std::size_t*>(base);
    std::free(base);
}
void operator delete(void* p, std::size_t) noexcept { operator delete(p); }


#endif //ALGORITMOS_ACTINTEGRADORA1_MEMORIA_H
