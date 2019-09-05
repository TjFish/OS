#ifndef _RAND_H_
#define _RNAD_H_

#include "type.h"

#define RAND_MAX 32768

static unsigned long next = 1;

PUBLIC int rand2048(void) {
    next = next * 1103515245 + 12345;
    return (unsigned int) (next / 65536) % RAND_MAX;
}

PUBLIC void srand2048(unsigned int seed) {
    next = seed;
}

#endif // _RAND_H_
