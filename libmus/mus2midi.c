#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
    char sig[4];
    uint16_t len;
    uint16_t offset;
    uint16_t pchannels;
    uint16_t schannels;
    uint16_t numinstruments;
    uint16_t reserved;
} musheader_t;

int main() {
    FILE *fp = fopen("D_E1M1.MUS", "r");
    musheader_t musheader;
    if (fread(&musheader, sizeof(musheader_t)-sizeof(uint16_t*)-sizeof(void*), 1, fp) == 1) {
        printf("len: %d\n", musheader.len);
        printf("offset: %d\n", musheader.offset);
        printf("instruments: %d\n", musheader.numinstruments);
    }

    uint16_t instruments[musheader.numinstruments];
    if (fread(instruments, sizeof(uint16_t)*musheader.numinstruments, 1, fp) == 1) {
        printf("Read instruments patches\n");
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp)-musheader.offset;
    void *data = (void*)malloc(size);
    if (data != NULL) {
        printf("Allocating size of: %ld\n", size);
    }

    if (fseek(fp, musheader.offset, SEEK_SET)) {
        printf("Seeking to offset failed: %d\n", musheader.offset);
    } else {
        printf("Seeking to offset: %d\n", musheader.offset);
    }

    if (fread(data, size, 1 ,fp) == 1) {
        printf("Read data\n");
    }
    printf("Last byte in data: %c\n", ((char*)data)[size-1]);
    fclose(fp);
    printf("Done\n");
    return 0;
}
