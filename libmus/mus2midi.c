#include <stdio.h>

#include "mus.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
    }
    mus_t *mus = mus_load("D_E1M1.mus");
    mus->data;
    mus_free(mus);
    return 0;
}
