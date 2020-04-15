#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
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

mus_t* mus_load(char *file) {
    // Allocate struct
    mus_t *mus = malloc(sizeof(mus_t));

    // Read MUS info header
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", file);
        return NULL;
    }
    if (fread(mus, sizeof(mus_t)-sizeof(uint16_t*)-sizeof(void*), 1, fp) != 1) {
        fprintf(stderr, "Could not read MUS header\n");
        return NULL;
    }
    if (strncmp(mus->sig, "MUS", 3) != 0 || mus->sig[3] != 0x1a) {
        fprintf(stderr, "Not a valid MUS file!\n");
        return NULL;
    }

    // Allocate memory for all data
    mus->instr = malloc(sizeof(uint16_t)*mus->num_instr);
    mus->data = malloc(mus->size);
    if (!mus->instr || !mus->data) {
        fprintf(stderr, "Could not allocate memory\n");
        goto error;
    }

    // Read instruments
    if (fread(mus->instr, sizeof(uint16_t)*mus->num_instr, 1, fp) != 1) {
        fprintf(stderr, "Could not read in instruments\n");
        goto error;
    }

    // Read MUS data
    if (fseek(fp, mus->offset, SEEK_SET)) {
        fprintf(stderr, "Seeking to offset failed: %d\n", mus->offset);
        goto error;
    }
    if (fread(mus->data, mus->size, 1 ,fp) != 1) {
        fprintf(stderr, "Could not read in data");
        goto error;
    }

    // All done!
    fclose(fp);
    return mus;

error:
    free(mus->instr);
    free(mus->data);
    free(mus);
    mus = NULL;
    return NULL;
}

void mus_free(mus_t *mus) {
    if (mus != NULL) {
        free(mus->instr);
        free(mus->data);
    }
    free(mus);
    mus = NULL;
};

int main() {
    mus_t *mus = mus_load("D_E1M1.mus");
    mus_free(mus);
    return 0;
}
