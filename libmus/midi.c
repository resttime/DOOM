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
    midi->data = malloc(htonl(1024*100)); // Allocate 100 KB to cover enough

    midi->offset = 22; // Where the data starts
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

    // Read midi info
    if (fread(midi,
              4*sizeof(uint8_t) + // header_id
              sizeof(uint32_t) + // chunklen
              3*sizeof(uint16_t) + // format, ntracks, tickdiv
              4*sizeof(uint8_t) + // track_id
              sizeof(uint32_t), // track_len
              1, fp) != 1) {
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

    // Allocate memory for data
    midi->data = malloc(ntohl(midi->track_len));
    if (midi->data == NULL) {
        fprintf(stderr, "Could not allocate memory\n");
        goto error;
    }
    // Read midi data
    if (fread(midi->data, ntohl(midi->track_len), 1, fp) != 1) {
        if (feof(fp)) {
            fprintf(stderr, "Could not read data, reached end of file\n");
        } else if (ferror(fp)) {
            perror("Could not read in data");
        }
        goto error;
    }

    midi->offset = 22; // For midi converted from mus
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
        if (midi->data != NULL) free(midi->data);
        free(midi);
    }
}

void midi_write(midi_t *midi, const char *file) {
    // Open file
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", file);
    }

    // Write midi info
    if (fwrite(midi,
               4*sizeof(uint8_t) + // header_id
               sizeof(uint32_t) + // chunklen
               3*sizeof(uint16_t) + // format, ntracks, tickdiv
               4*sizeof(uint8_t) + // track_id
               sizeof(uint32_t), //track_len
               1, fp) != 1) {
        fprintf(stderr, "Could not write midi info\n");
        fclose(fp);
    }

    // Write midi track data
    if (ntohl(midi->track_len) > 0 &&
        fwrite(midi->data, ntohl(midi->track_len), 1, fp) != 1) {
        fprintf(stderr, "Could not write midi track\n");
        fclose(fp);
    }
    fclose(fp);
}
