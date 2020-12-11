#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "mus.h"
#include "midi.h"
#include "test.h"

midi_t* mus_to_midi(const char *file) {
    // TODO: Change to cmdline arg
    mus_t *mus = mus_load(file);
    midi_t *midi = midi_create();
    midi_data_putc(midi, 0x00); // First delay in midi is 0x00

    printf("Size of MUS data is: %d\n\n", mus->size);

    uint8_t channel_vol[16] = {0};
    int num = 0;
    event_t ev;
    while (1) {
        printf("Event: %d\n", num, ev.pos);
        ev = read_event(mus);
        printf("Last: %d, Type: %d, Channel: %d\n", ev.last, ev.type, ev.channel);
        ev.channel  = (ev.channel == 15) ? 9 : ev.channel; // Channel 15 MUS is 9 MIDI
        switch (ev.type) {
            case RELEASE_NOTE:
                printf("RELEASE: %d\n", ev.note);
                midi_data_putc(midi, 0x80 | ev.channel);
                midi_data_putc(midi, ev.note);
                midi_data_putc(midi, 0x00);
                break;
            case PLAY_NOTE:
                printf("PLAY: %d VOL: %d\n", ev.note, (ev.note&0x80) ? ev.vol : -1);
                midi_data_putc(midi, 0x90 | ev.channel);
                midi_data_putc(midi, ev.note & 0x7F);
                if (ev.note & 0x80) {
                    midi_data_putc(midi, ev.vol & 0x7F);
                    channel_vol[ev.channel] = ev.vol;
                } else {
                    midi_data_putc(midi, channel_vol[ev.channel]);
                }
                break;
            case PITCH_BEND:
                printf("PITCH: %d\n", ev.bend);
                uint16_t midi_bend = ev.bend / 256.0 * 16384;
                midi_bend = (((midi_bend >> 7) & 0x7F) << 8)+(midi_bend & 0x7F);
                midi_data_putc(midi, 0xE0 | ev.channel);
                midi_data_putc(midi, midi_bend & 0xFF);
                midi_data_putc(midi, (midi_bend >> 8) & 0xFF);

                break;
            case SYSTEM_EVENT:
                printf("SYS: %d\n", ev.ctrl);
                char midi_ctrl;
                switch (ev.ctrl) {
                    case 10:
                        midi_ctrl = 120;
                        break;
                    case 11:
                        midi_ctrl = 123;
                        break;
                    case 12:
                        midi_ctrl = 126;
                        break;
                    case 13:
                        midi_ctrl = 127;
                        break;
                    case 14:
                        midi_ctrl = 121;
                        break;
                    case 15:
                        printf("Event never implemented\n");
                        goto error;
                    default:
                        printf("Couldn't parse SYSTEM_EVENT\n");
                        goto error;
                }
                midi_data_putc(midi, 0xB0 | ev.channel);
                midi_data_putc(midi, midi_ctrl & 0x7F);
                midi_data_putc(midi, 0);
                break;
            case CONTROLLER:
                printf("CTRL: %d Val: %d\n", ev.ctrl, ev.val);
                if (ev.ctrl == 0) {
                    midi_data_putc(midi, 0xC0 | ev.channel);
                    midi_data_putc(midi, ev.val & 0x7F);
                } else {
                    char midi_ctrl[] = {-1, 0, 1, 7, 10, 11, 91, 93, 64, 67};
                    midi_data_putc(midi, 0xB0 | ev.channel);
                    midi_data_putc(midi, midi_ctrl[ev.ctrl] & 0x7F);
                    midi_data_putc(midi, ev.val & 0x7F);
                }
                break;
            case END_OF_MEASURE:
                printf("END MEASURE\n");
                break;
            case FINISH:
                printf("FINISH\n");
                midi_data_putc(midi, 0xFF);
                midi_data_putc(midi, 0x2F);
                midi_data_putc(midi, 0x00);
                goto done;
            default:
                printf("Unknown Event Type\n");
        }

        printf("\n");
        num++;

        if (ev.last == true) {
            uint16_t delay = read_delay(mus);
            printf("Delay: %d\n\n", delay);
            midi_data_putdelay(midi, delay);
        } else { midi_data_putdelay(midi, 0x00); }
    }

done:
    mus_free(mus);
    return midi;
error:
    mus_free(mus);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
    }

    midi_t *midi = mus_to_midi("D_E1M1.mus");
    test_play_midi(midi);
    midi_free(midi);
    return 0;
}
