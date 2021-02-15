#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "mus.h"
#include "midi.h"
#include "mus2midi.h"

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

    midi_t *test = midi_loadf("test.mid");
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
    midi_t *midi = midi_loadf("D_E1M1.mid");
    midi_write(midi, "test.mid");
    midi_t *test = midi_loadf("test.mid");

    if (memcmp(midi, test, MIDI_HEADER_SIZE) == 0) {
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

void test_play_midi_file() {
    // Init SDL2 and SDL2_mixer
    // initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_INIT could not initialize audio\n");
    }

    //Initialize SDL_mixer
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
    }
    Mix_Music *m = Mix_LoadMUS("test.mid");
    if (m == NULL) {
        printf("Mix_LoadMUS(\"...\"): %s\n", Mix_GetError());
    }
    Mix_PlayMusic(m, -1);
    SDL_Delay(10000);
    Mix_FreeMusic(m);
    m = NULL;
    Mix_Quit();
    SDL_Quit();
}

void test_play_midi(midi_t *midi) {
    // Init SDL2 and SDL2_mixer
    // initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_INIT could not initialize audio\n");
    }

    //Initialize SDL_mixer
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
    }

    SDL_RWops *mem = SDL_RWFromConstMem(midi, MIDI_HEADER_SIZE+ntohl(midi->track_len));
    if (mem == NULL) {
        printf( "Could not SDL_RW: %s\n", SDL_GetError() );
    }
    Mix_Music *music = Mix_LoadMUS_RW(mem, 0);
    if (music == NULL) {
        printf("Mix_LoadMUS(\"...\"): %s\n", Mix_GetError());
    }

    // Play the music
    Mix_PlayMusic(music, 1);
    while (Mix_PlayingMusic() == 1) {}

    // Cleanup
    Mix_FreeMusic(music);
    music = NULL;
    Mix_Quit();
    SDL_Quit();
}


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
    }

    test_mus_load();
    return 0;
    midi_t *midi = mus_to_midi("D_E1M1.mus");
    test_play_midi(midi);
    midi_free(midi);
    return 0;
}
