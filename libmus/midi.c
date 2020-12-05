#include <stdint.h>
#include <stdio.h>
#include <string.h>

// MIDI is big-endian!

typedef struct midi_t {
    // Header
    uint8_t header_id[4]; // MThd
    uint32_t chunklen; // Length of header, 6 for MUS
    uint16_t format; // 0 for MUS
    uint16_t ntracks; // 1 for MUS
    uint16_t tickdiv; // 70 for MUS

    // Track
    uint8_t track_id[4]; // MTrk
    uint32_t track_len; // Size of track data (Located at 18 for, MUS)
    void *data; // The data

    // User defined
    FILE *fp;
    uint32_t offset; // Where start of data is, 22 for MUS
    uint32_t pos;
} midi_t;

midi_t create_midi(const char* file) {
    midi_t midi;

    // Header
    memcpy(&midi.header_id, "MThd", 4);
    midi.chunklen = 6;
    midi.format = 0;
    midi.ntracks = 1;
    midi.tickdiv = 70;

    // Track
    memcpy(&midi.track_id, "MTrk", 4);

    midi.offset = 22; // Where the data starts
    midi.pos = 0;

    // Read MUS info
    midi.fp = fopen(file, "w+");

    return midi;
}
