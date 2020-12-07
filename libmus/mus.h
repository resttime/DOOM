#ifndef MUS_H
#define MUS_H
#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__)) mus_t {
    uint8_t signature[4];
    uint16_t size;
    uint16_t offset; // Where the data starts
    uint16_t primary_channels;
    uint16_t secondary_channels;
    uint16_t num_instruments;
    uint16_t reserved;
    uint16_t *instruments;
    void *data;

    // User Defined
    uint16_t pos; // Position with respect to the offset
} mus_t;


typedef enum EVENT_TYPE {
    RELEASE_NOTE,
    PLAY_NOTE,
    PITCH_BLEND,
    SYSTEM_EVENT,
    CONTROLLER,
    END_OF_MEASURE,
    FINISH,
    UNUSED
} EVENT_TYPE;

typedef struct event_t {
    uint16_t pos;

    bool last;
    EVENT_TYPE type;
    uint8_t channel;
    uint8_t note;
    uint8_t vol;
    uint8_t bend;
    uint8_t ctrl;
    uint8_t val;
} event_t;

mus_t* mus_load(const char* file);
void mus_free(mus_t *mus);
uint8_t mus_getc(mus_t *mus);
event_t read_event(mus_t *mus);
uint16_t read_delay(mus_t *mus);

#endif
