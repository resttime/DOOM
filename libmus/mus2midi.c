#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "mus.h"
#include "midi.h"


void test_putdelay() {
    midi_t *midi = midi_create();
    midi_data_putdelay(midi, 0x00);
    midi_data_putdelay(midi, 0x40);
    midi_data_putdelay(midi, 0x7F);
    midi_data_putdelay(midi, 0x80);
    midi_data_putdelay(midi, 0x2000);
    midi_data_putdelay(midi, 0x3FFF);
    midi_data_putdelay(midi, 0x4000);
    midi_write(midi, "test.mid");

    midi_t *test = midi_load("test.mid");
    uint8_t results[] = {
        0x00,
        0x40,
        0x7F,
        0x81, 0x00,
        0xC0, 0x00,
        0xFF, 0x7F,
        0x81, 0x80, 0x00
    };
    if (memcmp(midi->data, results, ntohl(midi->track_len)) == 0) {
        printf("Delay working!\n");
    }

    midi_free(midi);
    midi_free(test);
}

void test_load() {
    midi_t *midi = midi_load("D_E1M1.mid");
    midi_write(midi, "test.mid");
    midi_t *test = midi_load("test.mid");

    if (memcmp(midi, test,
               4*sizeof(uint8_t) + // header_id
               sizeof(uint32_t) + // chunklen
               3*sizeof(uint16_t) + // format, ntracks, tickdiv
               4*sizeof(uint8_t) + // track_id
               sizeof(uint32_t) //track_len
            ) == 0) {
        printf("Headers same!\n");
        if (memcmp(midi->data, test->data, ntohl(midi->track_len)) == 0) {
            printf("Data same!\n");
        }
    }
    midi_free(midi);
    midi_free(test);
}

void test_pitch_conversion() {
    // 124 -> 0x00, 0x3E
    // 128 -> 0x00, 0x40
    // 130 -> 0x00, 0x41
    uint16_t midi_bend = 130 / 256.0 * 16384;
    printf("%X\n", midi_bend);
    midi_bend = htons((((midi_bend >> 7) & 0x7F) << 8)+(midi_bend & 0x7F));
    printf("%X\n", midi_bend);
    printf("%X\n", midi_bend & 0xFF);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
    }
    // test_putdelay();
    // test_load();
    // test_pitch();

    // TODO: Change to cmdline arg
    mus_t *mus = mus_load("D_E1M1.mus");
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
                        return 1;
                    default:
                        printf("Couldn't parse SYSTEM_EVENT\n");
                        return 1;
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
    midi_write(midi, "test.mid");
    midi_free(midi);
    mus_free(mus);
    return 0;
}
