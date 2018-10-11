#ifndef STM8_H
#define STM8_H
#if (STM8L)
#include "stm8l.h"
#elif (STM8S)
#include "stm8s.h"
#else
#error MCU family not specified!
#endif
#endif /* STM8_H */
