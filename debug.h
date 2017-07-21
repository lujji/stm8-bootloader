#ifndef DEBUG_H
#define DEBUG_H

#include "stm8s.h"

#define DBG_PIN  4
#define DBG_INIT() { PD_DDR |= 1 << DBG_PIN; PD_CR1 |= 1 << DBG_PIN; }
#define SET() do { PD_ODR |= (1 << DBG_PIN);  } while (0);
#define CLR() do { PD_ODR &= ~(1 << DBG_PIN); } while (0);
#define TOG() do { PD_ODR ^= (1 << DBG_PIN);  } while (0);

#endif /* DEBUG_H */
