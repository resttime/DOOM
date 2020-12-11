#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midi.h"

midi_t *midi_create() {
    // Allocate struct
    midi_t *midi = malloc(sizeof(midi_t));

    // Header (htons and htonl because midi wants big endian)
    memcpy(&midi->header_id, "MThd", 4);
    midi->chunklen = htonl(6);
    midi->format = htons(0);
    midi->ntracks = htons(1);
    midi->tickdiv = htons(70);

    // Track
    memcpy(&midi->track_id, "MTrk", 4);
    midi->track_len = htons(0);

    midi->pos = 0;

    return midi;
}

midi_t *midi_load(const char* file) {
    // Open file
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", file);
        return NULL;
    }

    // Allocate struct
    midi_t *midi = malloc(sizeof(midi_t));
    if (midi == NULL) {
        fprintf(stderr, "Could not allocate midi\n");
        goto error;
    }

    // Read midi header
    if (fread(midi, MIDI_HEADER_SIZE, 1, fp) != 1) {
        fprintf(stderr, "Could not read midi info\n");
        goto error;
    }
    if (strncmp(midi->header_id, "MThd", 4) != 0) {
        fprintf(stderr, "Not a valid midi file!\n");
        goto error;
    }
    if (strncmp(midi->track_id, "MTrk", 4) != 0) {
        fprintf(stderr, "Not a valid midi file!\n");
        goto error;
    }

    // Read midi data
    if (MIDI_DATA_BUFFER_SIZE < ntohl(midi->track_len)) {
        fprintf(stderr, "Midi data too large! Increase size of buffer\n");
        goto error;
    }
    if (fread(midi->data, ntohl(midi->track_len), 1, fp) != 1) {
        if (feof(fp)) {
            fprintf(stderr, "Could not read data, reached end of file\n");
        } else if (ferror(fp)) {
            perror("Could not read in data");
        }
        goto error;
    }

    midi->pos = 0;

    fclose(fp);
    return midi;
error:
    midi_free(midi);
    fclose(fp);
    return NULL;

}

void midi_free(midi_t *midi) {
    if (midi != NULL) {
        free(midi);
    }
}

void midi_write(midi_t *midi, const char *file) {
    // Open file
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", file);
    }

    // Write midi
    if (fwrite(midi, MIDI_HEADER_SIZE+ntohl(midi->track_len), 1, fp) != 1) {
        fprintf(stderr, "Could not write midi\n");
    }
    fclose(fp);
}

void midi_data_putc(midi_t *midi, uint8_t c) {
    if (ntohl(midi->track_len)+1 >= MIDI_DATA_BUFFER_SIZE) {
        fprintf(stderr, "Exceeded maximum size of the midi data buffer\n");
        return;
    }
    midi->data[midi->pos++] = c;
    midi->track_len = htonl(ntohl(midi->track_len)+1);

}

void midi_data_putdelay(midi_t *midi, uint16_t delay) {
    uint32_t stack = 0;
    stack += delay & 0x7F;
    while (delay >>= 7) {
        stack <<= 8;
        stack += (delay & 0x7F) | 0x80;
    }

    while (1) {
        midi_data_putc(midi, stack & 0xFF);
        if (stack & 0x80) stack >>= 8; else break;
    }
}
