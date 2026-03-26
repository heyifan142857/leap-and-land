//
// Created by user on 2024/1/19.
//
#include <help.h>

static void do_help_input(void);
static void draw_help(void);

static double elapsedSeconds;
static Uint32 tick;

void do_help_logic(){
    app.keyboard[SDL_SCANCODE_SPACE] = false;

    tick = SDL_GetTicks();
    elapsedSeconds = 0.0;

    Mix_Music *bgm = Mix_LoadMUS("./res/music/help.mp3");
    if (bgm != NULL) {
        Mix_PlayMusic(bgm,0);
    }

    while(!app.keyboard[SDL_SCANCODE_ESCAPE] && next != 0){
        do_help_input();

        draw_help();
    }

    Mix_HaltMusic();
    if (bgm != NULL) {
        Mix_FreeMusic(bgm);
    }
}

static void do_help_input(){
    SDL_Event event;

    while (SDL_PollEvent(&event)){
        do_event(event);
    }

    if(app.keyboard[SDL_SCANCODE_SPACE]){
        next = 0;
    }

}



static void draw_help(){
    SDL_RenderClear(app.renderer);

    display_image("./res/img/menu_background_1073x600.jpg",0,0);
    display_image("./res/img/help.png",20,60);

    display_font_alpha(
        "./res/font/Peralta-Regular.ttf",
        "press SPACE to go back to the menu",
        36,
        0,
        102,
        0,
        (Uint8)(255 * fabsf(sinf((float)(elapsedSeconds)))),
        30,
        450
    );
    elapsedSeconds = (SDL_GetTicks() - tick) / 1500.0;

    SDL_RenderPresent(app.renderer);
}
