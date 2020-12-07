#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mus.h"

mus_t *mus_load(const char* file) {
    // Open file
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", file);
        return NULL;
    }

    // Allocate struct
    mus_t *mus = malloc(sizeof(mus_t));
    if (mus == NULL) {
        fprintf(stderr, "Could not allocate mus\n");
        goto error;
    }

    // Read MUS info
    if (fread(mus, 4*sizeof(uint8_t) + 6*sizeof(uint16_t), 1, fp) != 1) {
        fprintf(stderr, "Could not read MUS header\n");
        goto error;
    }
    if (strncmp(mus->signature, "MUS", 3) != 0 || mus->signature[3] != 0x1a) {
        fprintf(stderr, "Not a valid MUS file!\n");
        goto error;
    }

    // Allocate memory for all data
    mus->instruments = malloc(sizeof(uint16_t)*mus->num_instruments);
    mus->data = malloc(mus->size);
    if (!mus->instruments || !mus->data) {
        fprintf(stderr, "Could not allocate memory\n");
        goto error;
    }

    // Read instruments
    if (fread(mus->instruments, sizeof(uint16_t)*mus->num_instruments, 1, fp) != 1) {
        fprintf(stderr, "Could not read in instruments\n");
        goto error;
    }

    // Read MUS data
    if (fseek(fp, mus->offset, SEEK_SET)) {
        fprintf(stderr, "Seeking to data failed: %d\n", mus->offset);
        goto error;
    }
    if (fread(mus->data, mus->size, 1 ,fp) != 1) {
        fprintf(stderr, "Could not read in data");
        goto error;
    }

    // Set read position to 0
    mus->pos = 0;

    // All done!
    fclose(fp);
    return mus;

error:
    mus_free(mus);
    fclose(fp);
    return NULL;
}

void mus_free(mus_t *mus) {
    if (mus != NULL) {
        if (mus->instruments != NULL) free(mus->instruments);
        if (mus->data != NULL) free(mus->data);
        free(mus);
    }
};

uint8_t mus_getc(mus_t *mus) {
    uint8_t byte = ((uint8_t *) mus->data)[mus->pos];
    mus->pos++;
    return byte;
}

event_t read_event(mus_t *mus) {
    event_t ev;
    ev.pos = mus->pos;

    // Read the event info
    uint8_t event_info = mus_getc(mus);

    // Parse the info
    ev.last = ((0x80 & event_info) >> 7) ? true : false;
    ev.type = (0x70 & event_info) >> 4;
    ev.channel = 0x0F & event_info;

    // Parse event
    switch (ev.type) {
        case RELEASE_NOTE:
            ev.note = (0x7F & mus_getc(mus));
            break;
        case PLAY_NOTE:
            ev.note = mus_getc(mus);
            short vol_flag = (0x80 & ev.note);
            ev.note &= 0x7F;
            // NOTE: If there's no volume flag, use volume of prev
            // note on channel, signified with -1
            if (vol_flag) {
                ev.vol = (0x7F & mus_getc(mus));
            } else {
                ev.vol = -1;
            }
            break;
        case PITCH_BLEND:
            ev.bend = mus_getc(mus);
            break;
        case SYSTEM_EVENT:
            ev.ctrl = (0x7F & mus_getc(mus));
            break;
        case CONTROLLER:
            ev.ctrl = (0x7F & mus_getc(mus));
            ev.val = (0x7F & mus_getc(mus));
            break;
        case END_OF_MEASURE:
        case FINISH:
            break;
        case UNUSED:
            break;
    }

    return ev;
}

uint16_t read_delay(mus_t *mus) {
    uint16_t delay = 0;
    uint8_t byte;

    do {
        byte = mus_getc(mus);
        delay = delay * 128 + byte & 0x7F;
    } while (byte & 0x80);

    return delay;
}
