//
// Created by user on 2024/1/18.
//
#include <kun.h>

void init_kun(Kun *kun){
    SDL_Surface *surface_kun = IMG_Load("./res/img/kunkun_100x100.png");
    if (surface_kun == NULL) {
        fprintf(stderr, "IMG_Load failed for kunkun texture: %s\n", IMG_GetError());
        kun->texture = NULL;
        kun->h = KUN_INIT_H;
        kun->dh = 0;
        return;
    }

    kun->texture = SDL_CreateTextureFromSurface(app.renderer, surface_kun);
    SDL_FreeSurface(surface_kun);
    kun->h = KUN_INIT_H;
    kun->dh = 0;
}

void do_kun(Kun kun,Uint32 time){
    kun.h += kun.dh * 0.001 * time;
    kun.dh -= KUN_ACCELERATION * 0.001 * time;
}

void draw_kun(Kun *kun){
    if (kun->texture == NULL) {
        return;
    }

    SDL_Rect rect_image = {.x = 350, .y = (int)(380 - kun->h)};
    SDL_QueryTexture(kun->texture, NULL, NULL, &rect_image.w, &rect_image.h);
    SDL_RenderCopy(app.renderer, kun->texture, NULL, &rect_image);
}

void quit_kun(Kun *kun){
    SDL_DestroyTexture(kun->texture);
    kun->texture = NULL;
}
