#include <stdio.h>

#include "mus.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
    }

    // TODO: Change to cmdline arg
    mus_t *mus = mus_load("D_E1M1.mus");

    printf("Size of MUS data is: %d\n\n", mus->size);

    for (int num = 0;;) {
        for (event_t ev = {}; ev.last != true;) {
            printf("Event: %d\n", num, ev.pos);
            ev = read_event(mus);
            printf("Last: %d, Type: %d, Channel: %d\n", ev.last, ev.type, ev.channel);
            switch (ev.type) {
                case RELEASE_NOTE:
                    printf("RELEASE: %d\n", ev.note);
                    break;
                case PLAY_NOTE:
                    printf("PLAY: %d VOL: %d\n", ev.note, ev.vol);
                    break;
                case PITCH_BLEND:
                    printf("PITCH: %d\n", ev.bend);
                    break;
                case SYSTEM_EVENT:
                    printf("SYS: %d\n", ev.ctrl);
                    break;
                case CONTROLLER:
                    printf("CTRL: %d Val: %d\n", ev.ctrl, ev.val);
                    break;
                case END_OF_MEASURE:
                    printf("END MEASURE\n");
                    break;
                case FINISH:
                    printf("FINISH\n");
                    goto done;
                default:
                    printf("Unknown Event Type\n");
            }

            printf("\n");
            num++;
        }
        uint16_t delay = read_delay(mus);
        printf("Delay: %d\n\n", delay);
    }

done:
    mus_free(mus);
    return 0;
}
