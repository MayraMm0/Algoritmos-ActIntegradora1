#ifndef MEMORIA_H
#define MEMORIA_H

#include <cstddef>

extern std::size_t mem_actual;
extern std::size_t mem_pico;

double mb(std::size_t nbytes);

#endif