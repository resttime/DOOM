#ifndef MUS_H
#define MUS_H
#include <stdint.h>
typedef struct mus_t {
    char sig[4];
    uint16_t size;
    uint16_t offset;
    uint16_t p_chnls;
    uint16_t s_chnls;
    uint16_t num_instr;
    uint16_t resrvd;
    uint16_t *instr;
    void *data;
} mus_t;

mus_t* mus_load(char *file); 
void mus_free(mus_t *mus);

#endif
