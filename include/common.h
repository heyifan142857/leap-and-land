//
// Created by user on 2024/1/16.
//

#ifndef LEAP_AND_LAND_COMMON_H
#define LEAP_AND_LAND_COMMON_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#define HANDLE_ERROR(msg) printf(msg ": %s\n",SDL_GetError());\
exit(EXIT_FAILURE);

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool *keyboard;
} App;

extern int next;

extern App app;

#endif //LEAP_AND_LAND_COMMON_H
