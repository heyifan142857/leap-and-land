//
// Created by user on 2024/1/16.
//
#include <menu.h>

static void draw_menu(void);
static void init_widgets(void);
static void do_menu_input(SDL_Event event);
static void quit_widgets(void);
static void do_widgets(void);
static void pre_widget(void);
static void next_widget(void);
static void act_widget(void);
static void action_start(void);
static void action_help(void);
static void action_quit(void);

static Widget *widgets;
static int selection;
static Mix_Chunk *shift_chunk;
static Mix_Chunk *select_chunk;

void do_menu_logic(){
    next = -1;
    printf("Opening menu\n");
    init_widgets();

    SDL_Event event;
    while (!app.keyboard[SDL_SCANCODE_ESCAPE] && SDL_WaitEvent(&event) && next == -1){
        do_menu_input(event);

        do_widgets();

        draw_menu();
    }
    quit_widgets();
}

static void draw_menu(){
    SDL_RenderClear(app.renderer);

    display_image("./res/img/menu_background_1073x600.jpg",0,0);
    display_image("./res/img/chicken41x50.png",widgets[selection].x - 60,widgets[selection].y - 10);
    display_image("./res/img/blocks_menu/1.png",50,320);
    display_image("./res/img/kunkun_300x300.png",50,100);

    display_font("./res/font/ZenTokyoZoo-Regular.ttf","LEAP AND LAND",72,64,64,64,90,60);

    for (int i = 0; i < NUM_WIDGETS; ++i) {
        display_font("./res/font/Peralta-Regular.ttf",widgets[i].text,36,207,142,107,widgets[i].x,widgets[i].y);
    }

    SDL_RenderPresent(app.renderer);
}

static void init_widgets() {
    widgets = malloc(NUM_WIDGETS * sizeof (Widget));
    if (widgets == NULL) {
        fprintf(stderr, "Failed to allocate menu widgets.\n");
        exit(EXIT_FAILURE);
    }

    widgets[0] = (Widget) {"start game",WIDGET_X,WIDGET_Y_TOP,action_start};
    widgets[1] = (Widget) {"help",WIDGET_X,WIDGET_Y_TOP+WIDGET_Y_GAP,action_help};
    widgets[2] = (Widget) {"exit game",WIDGET_X,WIDGET_Y_TOP+2*WIDGET_Y_GAP,action_quit};
    selection = 0;
    printf("select at %s\n",widgets[selection].text);
    shift_chunk = Mix_LoadWAV("./res/chunk/chicken.mp3");
    select_chunk = Mix_LoadWAV("./res/chunk/select.mp3");
}

static void do_menu_input(SDL_Event event){
    do_event(event);
}

static void quit_widgets(){
    free(widgets);
    Mix_FreeChunk(shift_chunk);
    Mix_FreeChunk(select_chunk);
}

static void do_widgets(){
    if(app.keyboard[SDL_SCANCODE_UP] || app.keyboard[SDL_SCANCODE_LEFT] || app.keyboard[SDL_SCANCODE_W]){
        printf("select at %s\n",widgets[selection].text);
        pre_widget();
        Mix_PlayChannel(-1,shift_chunk,0);
    }
    if(app.keyboard[SDL_SCANCODE_DOWN] || app.keyboard[SDL_SCANCODE_RIGHT] || app.keyboard[SDL_SCANCODE_S]){
        printf("select at %s\n",widgets[selection].text);
        next_widget();
        Mix_PlayChannel(-1,shift_chunk,0);
    }
    if(app.keyboard[SDL_SCANCODE_SPACE] || app.keyboard[SDL_SCANCODE_RETURN]){
        printf("select at %s\n",widgets[selection].text);
        act_widget();
        Mix_PlayChannel(-1,select_chunk,0);
        SDL_Delay(100);
        next = selection;
    }
}

static void pre_widget(){
    selection = (selection - 1 + NUM_WIDGETS) % NUM_WIDGETS;
}

static void next_widget(){
    selection = (selection + 1) % NUM_WIDGETS;
}

static void act_widget(){
    void (*action)() = widgets[selection].action;
    if(action){
        action();
    }
}

static void action_start(){
    printf("START GAME\n");
}

static void action_help(){
    printf("ENTER HELP\n");
}

static void action_quit(){
    printf("EXIT GAME\n");
    app.keyboard[SDL_SCANCODE_ESCAPE] = true;
}
