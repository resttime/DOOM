#ifndef MIDI_H
#define MIDI_H

#define MIDI_HEADER_SIZE                        \
    4*sizeof(uint8_t) +                         \
    sizeof(uint32_t) +                          \
    3*sizeof(uint16_t) +                        \
    4*sizeof(uint8_t) +                         \
    sizeof(uint32_t)

#define MIDI_DATA_BUFFER_SIZE 96*1024

// MIDI is big-endian!

typedef struct __attribute__((__packed__)) midi_t {
    // Header
    uint8_t header_id[4]; // MThd
    uint32_t chunklen; // Length of header, 6 for MUS
    uint16_t format; // 0 for MUS
    uint16_t ntracks; // 1 for MUS
    uint16_t tickdiv; // 70 for MUS

    // Track
    uint8_t track_id[4]; // MTrk
    uint32_t track_len; // Size of track data (Located at 18 for, MUS)
    uint8_t data[MIDI_DATA_BUFFER_SIZE]; // The data

    // User defined
    uint32_t pos; // Position with respect to the offset 22
} midi_t;

midi_t *midi_create();
midi_t *midi_load(const char *file);
void midi_free(midi_t *midi);
void midi_write(midi_t* midi, const char *file);
void midi_data_putc(midi_t *midi, uint8_t c);
void midi_data_putdelay(midi_t *midi, uint16_t delay);
#endif
