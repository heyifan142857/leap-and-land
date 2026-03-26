//
// Created by user on 2024/1/16.
//
#include "utils/input.h"

static void do_quit(SDL_QuitEvent quit);
static void do_keydown(SDL_KeyboardEvent key);
static void do_keyup(SDL_KeyboardEvent key);
static void do_mousebuttondown(SDL_MouseButtonEvent key);
static void do_mousebuttonup(SDL_MouseButtonEvent key);

void do_event(SDL_Event event){
    switch (event.type) {
        case SDL_QUIT:
            do_quit(event.quit);
            printf("Exit.");
            return;
        case SDL_MOUSEBUTTONDOWN:
            do_mousebuttondown(event.button);
            break;
        case SDL_MOUSEBUTTONUP:
            do_mousebuttonup(event.button);
            break;
        case SDL_KEYDOWN:
            do_keydown(event.key);
            break;
        case SDL_KEYUP:
            do_keyup(event.key);
            break;
        default:
            break;
    }
    fflush(stdout);
}

static void do_quit(SDL_QuitEvent quit){
    (void)quit;
    app.keyboard[SDL_SCANCODE_ESCAPE] = true;
}


void do_keydown(SDL_KeyboardEvent key){
    app.keyboard[key.keysym.scancode] = true;
    printf("key down: %s\n", SDL_GetKeyName(key.keysym.sym));
}

void do_keyup(SDL_KeyboardEvent key){
    app.keyboard[key.keysym.scancode] = false;
    printf("key up: %s\n", SDL_GetKeyName(key.keysym.sym));
}

static void do_mousebuttondown(SDL_MouseButtonEvent key){
    printf("Button down at (%d, %d)\n",key.x,key.y);
}
static void do_mousebuttonup(SDL_MouseButtonEvent key){
    printf("Button up at (%d, %d)\n",key.x,key.y);
}
