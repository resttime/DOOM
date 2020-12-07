#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midi.h"

void write_track_len(midi_t* midi, uint16_t len) {
    // store current pos
    long int curr_pos = ftell(midi->fp);

    // convert length to big endian because MIDI
    len = htons(len);

    // fseek to 18 and write length
    fseek(midi->fp, 18L, SEEK_SET);
    fwrite(&len, sizeof(uint16_t), 1, midi->fp);

    // restore pos
    fseek(midi->fp, curr_pos, SEEK_SET);
}

midi_t *midi_create(const char* file) {
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

    midi->offset = 22; // Where the data starts
    midi->pos = 0;

    // Read MUS info
    FILE *fp = fopen(file, "w+");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", file);
        return NULL;
    }

    // Write the header
    if (fwrite(midi,
               4*sizeof(uint8_t) + // header_id
               sizeof(uint32_t) + // chunklen
               3*sizeof(uint16_t) + // format, ntracks, tickdiv
               4*sizeof(uint8_t) + // track_id
               sizeof(uint32_t), //track_len
               1, fp) != 1) {
        fprintf(stderr, "Could not write midi header: %s\n", file);
        fclose(fp);
        return NULL;
    }

    midi->fp = fp;
    return midi;
}

void midi_free(midi_t *midi) {
    if (midi != NULL) {
        if (midi->data != NULL) free(midi->data);
        if (midi->fp != NULL) fclose(midi->fp);
        free(midi);
    }
}
