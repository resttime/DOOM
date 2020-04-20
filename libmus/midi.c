#include <stdint.h>
#include <string.h>

typedef struct {
    char id[4];
    uint32_t chunklen;
    uint16_t format;
    uint16_t ntracks;
    uint16_t tickdiv;
} midiheader_t;

typedef struct {
} trackchunk_t;

typedef struct {
} deltatime_t;

typedef struct {
} event_t;

void create_header(uint16_t chunklen) {
    midiheader_t *header;
    strncpy(header->id, "MThd", 4);
    header->chunklen = chunklen;
    header->format = 0;
    header->ntracks = 1;
    header->tickdiv = 70;
}
