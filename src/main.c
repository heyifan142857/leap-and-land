#include <main.h>

#undef main

static void init_app(void);
static void quit_app(void);
static void init_keyboard(void);
static void quit_keyboard(void);
static void cleanup_music(Mix_Music **music);

App app;
int next = -1;


int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    init_app();
    init_keyboard();
    srand((unsigned int)time(NULL));

    app.window = SDL_CreateWindow("Leap And Land",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,800,600,SDL_WINDOW_RESIZABLE);
    if (app.window == NULL) {
        HANDLE_ERROR("Create window");
    }

    app.renderer = SDL_CreateRenderer(app.window,-1,SDL_RENDERER_ACCELERATED);
    if (app.renderer == NULL) {
        HANDLE_ERROR("Create renderer");
    }

    while(!app.keyboard[SDL_SCANCODE_ESCAPE]){
        Mix_Music *bgm = Mix_LoadMUS("./res/music/C418 - Minecraft.mp3");
        if (bgm == NULL) {
            fprintf(stderr, "Mix_LoadMUS failed: %s\n", Mix_GetError());
            break;
        }

        if (Mix_PlayMusic(bgm,-1) == -1) {
            fprintf(stderr, "Mix_PlayMusic failed: %s\n", Mix_GetError());
            cleanup_music(&bgm);
            break;
        }

        do_menu_logic();
        cleanup_music(&bgm);

        if(app.keyboard[SDL_SCANCODE_ESCAPE]){
            break;
        }

        if(next == 0){
            while(!app.keyboard[SDL_SCANCODE_ESCAPE]){
                do_game_logic();
            }
        }else{
            do_help_logic();
        }
    }

    display_quit_cache();
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    quit_keyboard();

    quit_app();

    return 0;
}

static void init_app(void){
    const int image_flags = IMG_INIT_JPG | IMG_INIT_PNG;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO)<0){
        HANDLE_ERROR("Init SDL");
    }
    if((IMG_Init(image_flags) & image_flags) != image_flags){
        HANDLE_ERROR("Init Image");
    }
    if((Mix_Init(MIX_INIT_MP3) & MIX_INIT_MP3) != MIX_INIT_MP3){
        HANDLE_ERROR("Init Mixer");
    }
    init_Audio();
    if(TTF_Init()==-1){
        HANDLE_ERROR("Init TTF");
    }
}

static void quit_app(void){
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

static void init_keyboard(void){
    app.keyboard = calloc(SDL_NUM_SCANCODES, sizeof(bool));
    if (app.keyboard == NULL) {
        fprintf(stderr, "Failed to allocate keyboard state.\n");
        exit(EXIT_FAILURE);
    }
}

static void quit_keyboard(void){
    free(app.keyboard);
    app.keyboard = NULL;
}

static void cleanup_music(Mix_Music **music){
    if (music == NULL || *music == NULL) {
        return;
    }

    Mix_HaltMusic();
    Mix_FreeMusic(*music);
    *music = NULL;
}
