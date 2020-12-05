#include <stdio.h>

#include "mus.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
    }

    // TODO: Change to cmdline arg
    mus_t *mus = mus_load("D_E1M1.mus");

    for (int num = 0;;) {
        for (event_t ev = {}; ev.last != true;) {
            ev = read_event(mus);
            switch (ev.type) {
                case RELEASE_NOTE:
                    printf("Event %d RELEASE: %d\n", num, ev.note);
                    break;
                case PLAY_NOTE:
                    printf("Event %d PLAY: %d VOL: %d\n", num, ev.note, ev.vol);
                    break;
                case PITCH_BLEND:
                    printf("Event %d PITCH: %d\n", num, ev.bend);
                    break;
                case SYSTEM_EVENT:
                    printf("Event %d SYS: %d\n", num, ev.ctrl);
                    break;
                case CONTROLLER:
                    printf("Event %d CTRL: %d Val: %d\n", num, ev.ctrl, ev.val);
                    break;
                case END_OF_MEASURE:
                    printf("Event %d END MEASURE\n", num);
                    break;
                case FINISH:
                    printf("Event %d FINISH: %d\n", num, ev.type);
                    goto done;
                default:
                    printf("Unknown Event %d Type: %d\n", num, ev.type);
            }

            printf("\n");
            num++;
        }
        uint16_t delay = read_delay(mus);
        printf("Delay: %d\n", delay);
    }

done:
    mus_free(mus);
    return 0;
}
