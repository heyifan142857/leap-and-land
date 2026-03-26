//
// Created by user on 2024/1/18.
//
#include <utils/audio.h>

void init_Audio(void) {
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) < 0) {
        fprintf(stderr, "Mix_OpenAudio failed: %s\n", Mix_GetError());
        exit(EXIT_FAILURE);
    }
}
