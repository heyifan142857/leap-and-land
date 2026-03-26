//
// Created by user on 2024/1/17.
//
#include <errno.h>
#include <string.h>

#include <game.h>

#define BEST_SCORE_FILE_PATH "./best_score.txt"

static void do_game_input(void);
static void draw_game(void);
static void do_fps(void);
static double speed_calculator_h(Uint32 time);
static int speed_calculator_y(int type, int size);
static bool init_blocks(void);
static void quit_blocks(void);
static void draw_blocks(LinkedList list);
static const char *generate_random_block_asset(int *block_shape);
static double difficulty_random_len(int diff);
static double difficulty_random_size(int diff);
static void do_auto(void);
static void do_score(void);
static void do_Moist(void);
static void start_gathering_audio(void);
static void update_gathering_audio(void);
static void stop_gathering_audio(void);
static SDL_Texture *create_scaled_texture(const char *file_path, int size);
static Blocks *create_block_node(int size, int len, const char *texture_path, int block_shape);
static Blocks *create_initial_block(void);
static Blocks *create_dynamic_block(void);
static void destroy_block(Blocks *target);
static void append_block(Blocks *new_block);
static void remove_head_block(void);
static bool advance_blocks(int steps);
static void finish_landing(void);
static bool load_chunks(void);
static void free_chunks(void);
static void trigger_fail(void);
static void load_best_score(void);
static void save_best_score(void);
static void submit_score_if_eligible(void);

static Uint32 tick;
static Uint32 start_time;
static Uint32 current_time;
static Uint32 tnt_time;
static Uint32 elapsed_time;
static Uint32 reminder;

static LinkedList blocks_list;

static int scores;
static int difficulty;
static bool fail;
static bool restart;
static bool automatic;
static bool assisted_mode_used;
static bool broke_best_score;
static bool RKeyPressed = false;
static Kun kun;
static bool gathering = false;
static bool jumping = false;
static Mix_Chunk *chunk_1;
static Mix_Chunk *chunk_2;
static Mix_Chunk *chunk_fail;
static Mix_Chunk *chunk_jump;
static Mix_Chunk *chunk_tnt;
static double percentage = 0.0;
static int channel_1 = -1;
static int channel_2 = -1;
static int best_score = -1;
static bool best_score_loaded;

static const char *const block_paths[] = {
    "./res/img/blocks_game/005.png",
    "./res/img/blocks_game/015.png",
    "./res/img/blocks_game/Block_of_Diamond_JE5_BE3.png",
    "./res/img/blocks_game/Cobblestone_JE5_BE3.png",
    "./res/img/blocks_game/Crafting_Table_JE4_BE3.png",
    "./res/img/blocks_game/Glass_JE4_BE2.png",
    "./res/img/blocks_game/Grass_Block_JE7_BE6.png",
    "./res/img/blocks_game/Oak_Planks_JE6_BE3.png",
    "./res/img/blocks_game/Dirt_JE2_BE2.png",
    "./res/img/blocks_game/Block_of_Bamboo_JE3_BE2.png",
    "./res/img/blocks_game/Oak_Leaves_JE4.png"
};

static const char *const dish_paths[] = {
    "./res/img/blocks_game/OIG2.1JM78Zo-removebg-preview.png",
    "./res/img/blocks_game/OIG2-removebg-preview.png"
};

static const char *const tnt_paths[] = {
    "./res/img/blocks_game/TNT_JE3_BE2.png"
};

static const char *const moist_paths[] = {
    "./res/img/blocks_game/Moist_Farmland_JE4_BE2.png"
};

static const char *const dirt_paths[] = {
    "./res/img/blocks_game/Dirt_JE2_BE2.png"
};

void do_game_logic(void){
    scores = 0;
    difficulty = 0;
    fail = false;
    restart = false;
    automatic = false;
    assisted_mode_used = false;
    broke_best_score = false;
    RKeyPressed = false;
    gathering = false;
    jumping = false;
    percentage = 0.0;
    elapsed_time = 0;
    tnt_time = 0;
    channel_1 = -1;
    channel_2 = -1;
    reminder = SDL_GetTicks();

    app.keyboard[SDL_SCANCODE_SPACE] = false;

    init_kun(&kun);
    if (!init_blocks()) {
        quit_kun(&kun);
        return;
    }

    if (!load_chunks()) {
        quit_blocks();
        quit_kun(&kun);
        return;
    }

    while(!app.keyboard[SDL_SCANCODE_ESCAPE] && !restart){
        do_game_input();
        draw_game();
        do_fps();
        difficulty = scores / 300;
    }

    quit_kun(&kun);
    quit_blocks();
    free_chunks();
}

static void do_game_input(void){
    SDL_Event event;

    while (SDL_PollEvent(&event)){
        do_event(event);
    }

    if (app.keyboard[SDL_SCANCODE_R]) {
        if (!RKeyPressed) {
            RKeyPressed = true;
            automatic = !automatic;
            if (automatic) {
                assisted_mode_used = true;
            }
        }
    } else {
        RKeyPressed = false;
    }

    if(!jumping){
        if(!automatic || fail){
            if(app.keyboard[SDL_SCANCODE_SPACE]){
                reminder = SDL_GetTicks();
                if(fail){
                    restart = true;
                }else if(!gathering){
                    start_gathering_audio();
                }else{
                    update_gathering_audio();
                    if(percentage < 1.0){
                        percentage = (SDL_GetTicks() - tick) / LEN_CHUNK;
                    }else{
                        percentage = 1.0;
                    }
                }
            }

            if(!app.keyboard[SDL_SCANCODE_SPACE] && gathering){
                Uint32 duration;

                reminder = SDL_GetTicks();
                duration = SDL_GetTicks() - tick;
                tick = SDL_GetTicks();
                stop_gathering_audio();
                printf("Press the space bar for %u\n",duration);

                gathering = false;
                kun.dh = speed_calculator_h(duration);
                jumping = true;
                percentage = 0.0;
                start_time = SDL_GetTicks();
                current_time = SDL_GetTicks();
            }

            if(blocks_list.middle != NULL && blocks_list.middle->shape == 2 && (SDL_GetTicks() - tnt_time) > 3384){
                trigger_fail();
            }
        }else if(!fail){
            do_auto();
        }
    }else{
        Blocks *next_block;
        Blocks *next_next_block;

        current_time = SDL_GetTicks();
        elapsed_time = current_time - start_time;

        kun.h += kun.dh * 0.001 * elapsed_time;
        kun.dh -= KUN_ACCELERATION * 0.001 * elapsed_time;
        for (Blocks *p = blocks_list.head; p != NULL; p = p->after) {
            p->x -= KUN_SPEED * 0.001 * elapsed_time;
        }

        if(kun.h >= 0){
            return;
        }

        reminder = SDL_GetTicks();
        next_block = (blocks_list.middle != NULL) ? blocks_list.middle->after : NULL;
        next_next_block = (next_block != NULL) ? next_block->after : NULL;

        if(blocks_list.middle != NULL
           && blocks_list.middle->x <= 432.0
           && blocks_list.middle->x + blocks_list.middle->size >= 358
           && !fail){
            finish_landing();
        }else if(next_block != NULL
                 && next_block->x <= 432.0
                 && next_block->x + next_block->size >= 358
                 && !fail){
            if (advance_blocks(1)) {
                finish_landing();
            } else {
                app.keyboard[SDL_SCANCODE_ESCAPE] = true;
            }
        }else if(next_next_block != NULL
                 && next_next_block->x <= 432.0
                 && next_next_block->x + next_next_block->size >= 358
                 && !fail){
            if (advance_blocks(2)) {
                finish_landing();
            } else {
                app.keyboard[SDL_SCANCODE_ESCAPE] = true;
            }
        }else{
            trigger_fail();
            if(kun.h <= -400){
                kun.h = -400;
                jumping = false;
            }
        }
    }
}

static void draw_game(void){
    SDL_RenderClear(app.renderer);

    display_image("./res/img/menu_background_1073x600.jpg",0,0);
    draw_blocks(blocks_list);
    draw_kun(&kun);

    if(!fail){
        char score_text[20];

        snprintf(score_text, sizeof(score_text), "Scores: %d", scores);
        display_font("./res/font/Gugi-Regular.ttf",score_text,54,0,0,0,10,10);
        if(difficulty == 0){
            display_font("./res/font/Gugi-Regular.ttf","difficulty: Easy",54,0,0,0,10,80);
        }else if(difficulty == 1){
            display_font("./res/font/Gugi-Regular.ttf","difficulty: Medium",54,0,0,0,10,80);
        }else{
            display_font("./res/font/Gugi-Regular.ttf","difficulty: Hard",54,0,0,0,10,80);
        }
    }else{
        char scores_text[32];
        char best_text[64];

        display_font("./res/font/Gugi-Regular.ttf","KUNKUN DIED!",72,0,0,0,120,100);

        snprintf(scores_text, sizeof(scores_text), "The scores is %d", scores);
        display_font("./res/font/Gugi-Regular.ttf",scores_text,42,0,0,0,120,200);
        if (assisted_mode_used) {
            display_font(
                "./res/font/Peralta-Regular.ttf",
                "Score not counted because assisted mode was used.",
                24,
                140,
                20,
                20,
                120,
                260
            );
        } else if (broke_best_score) {
            display_font(
                "./res/font/Peralta-Regular.ttf",
                "You set a new best score!",
                28,
                0,
                120,
                0,
                120,
                260
            );
        } else if (best_score >= 0) {
            snprintf(best_text, sizeof(best_text), "Best score: %d", best_score);
            display_font("./res/font/Peralta-Regular.ttf", best_text, 28, 40, 40, 40, 120, 260);
        } else {
            display_font("./res/font/Peralta-Regular.ttf", "No best score yet", 28, 40, 40, 40, 120, 260);
        }
        display_font("./res/font/Gugi-Regular.ttf","press SPACE to try again",30,0,0,0,130,510);
    }

    if(gathering){
        SDL_Rect bgRect = {348, 358, WIDTH + 4, 9};
        int barWidth = (int)(percentage * WIDTH);
        SDL_Rect progressBarRect = {350, 360, barWidth, 5};

        SDL_SetRenderDrawColor(app.renderer, 160, 160, 160, 255);
        SDL_RenderFillRect(app.renderer, &bgRect);

        if(difficulty == 0){
            SDL_SetRenderDrawColor(app.renderer, 0, 153, 0, 255);
        }else if(difficulty == 1){
            SDL_SetRenderDrawColor(app.renderer, 0, 0, 204, 255);
        }else{
            SDL_SetRenderDrawColor(app.renderer, 204, 0, 0, 255);
        }

        SDL_RenderFillRect(app.renderer, &progressBarRect);
    }

    if(!jumping && !fail && !automatic && (SDL_GetTicks() - reminder) > 4000){
        double alpha = fabsf(sinf((float)((SDL_GetTicks() - tick) / 1500.0)));

        display_font_alpha("./res/font/Peralta-Regular.ttf", "Press and hold SPACE to jump", 36, 102, 0, 102, (Uint8)(255 * alpha), 90, 200);
        display_font_alpha("./res/font/Peralta-Regular.ttf", "Or press R for free jump mode", 36, 102, 0, 102, (Uint8)(255 * alpha), 90, 250);
    }

    if(automatic && !fail){
        double alpha = fabsf(sinf((float)((SDL_GetTicks() - reminder) / 1500.0)));

        display_font_alpha("./res/font/Peralta-Regular.ttf", "Free Jump Mode", 36, 102, 0, 102, (Uint8)(255 * alpha), 200, 200);
        display_font_alpha("./res/font/Peralta-Regular.ttf", "press R to quit", 36, 102, 0, 102, (Uint8)(255 * alpha), 200, 250);
    }

    SDL_RenderPresent(app.renderer);

    if(jumping){
        start_time = current_time;

        if (elapsed_time < ANIMATION_DELAY) {
            SDL_Delay(ANIMATION_DELAY - elapsed_time);
        }
    }
}

static void do_fps(void){
    // todo
}

static double speed_calculator_h(Uint32 time){
    double speed;

    if(time <= LEN_CHUNK){
        speed = (double)time;
    }else{
        speed = LEN_CHUNK;
    }

    return 10 * sqrt(speed);
}

static bool init_blocks(void){
    blocks_list = (LinkedList){0};

    for (int i = 0; i < NUM_BLOCKS; ++i) {
        Blocks *new_block = create_initial_block();
        if (new_block == NULL) {
            quit_blocks();
            return false;
        }

        append_block(new_block);
    }

    blocks_list.middle = blocks_list.head;
    for (int i = 0; i < NUM_BLOCKS / 2; ++i) {
        blocks_list.middle = blocks_list.middle->after;
    }

    if (blocks_list.middle != NULL) {
        blocks_list.middle->x = 400 - 0.5 * blocks_list.middle->size;
    }

    return true;
}

static void quit_blocks(void){
    Blocks *current = blocks_list.head;

    while (current != NULL) {
        Blocks *next = current->after;
        destroy_block(current);
        current = next;
    }

    blocks_list = (LinkedList){0};
}

static void draw_blocks(LinkedList list){
    if (list.middle == NULL) {
        return;
    }

    SDL_Rect rect_middle = {
        .x = (int)(list.middle->x),
        .y = speed_calculator_y(list.middle->shape, list.middle->size)
    };

    SDL_QueryTexture(list.middle->texture, NULL, NULL, &rect_middle.w, &rect_middle.h);
    SDL_RenderCopy(app.renderer, list.middle->texture, NULL, &rect_middle);

    for (Blocks *p = list.middle->before; p != NULL ; p = p->before) {
        SDL_Rect rect;

        p->x = p->after->x - (p->len + p->size);
        rect = (SDL_Rect){.x = (int)(p->x), .y = speed_calculator_y(p->shape, p->size)};
        SDL_QueryTexture(p->texture, NULL, NULL, &rect.w, &rect.h);
        SDL_RenderCopy(app.renderer, p->texture, NULL, &rect);
    }

    for (Blocks *p = list.middle->after; p != NULL; p = p->after) {
        SDL_Rect rect;

        p->x = p->before->x + (p->before->len + p->before->size);
        rect = (SDL_Rect){.x = (int)(p->x), .y = speed_calculator_y(p->shape, p->size)};
        SDL_QueryTexture(p->texture, NULL, NULL, &rect.w, &rect.h);
        SDL_RenderCopy(app.renderer, p->texture, NULL, &rect);
    }
}

static const char *generate_random_block_asset(int *block_shape) {
    int random = rand() % 20;

    if(difficulty < 2){
        if(random < 15){
            *block_shape = 0;
            return block_paths[rand() % (sizeof(block_paths) / sizeof(block_paths[0]))];
        }
        if(random < 18){
            *block_shape = 1;
            return dish_paths[rand() % (sizeof(dish_paths) / sizeof(dish_paths[0]))];
        }

        *block_shape = 3;
        return moist_paths[0];
    }

    if(random < 10){
        *block_shape = 0;
        return block_paths[rand() % (sizeof(block_paths) / sizeof(block_paths[0]))];
    }
    if(random < 13){
        *block_shape = 1;
        return dish_paths[rand() % (sizeof(dish_paths) / sizeof(dish_paths[0]))];
    }
    if(random < 18){
        *block_shape = 2;
        return tnt_paths[0];
    }

    *block_shape = 3;
    return moist_paths[0];
}

static int speed_calculator_y(int type,int size){
    if(type == 0 || type == 2 || type == 3){
        return 480 - (int)(size * 0.25);
    }

    return 480 - (int)(size * 0.5);
}

static double difficulty_random_len(int diff){
    if(diff == 0){
        return ((double)rand() / RAND_MAX) * (2.5 - 0.7) + 0.7;
    }

    return ((double)rand() / RAND_MAX) * (2.5 - 1.3) + 1.3;
}

static double difficulty_random_size(int diff){
    if(diff == 0){
        return ((double)rand() / RAND_MAX) * (1.5 - 1.0) + 1.0;
    }
    if(diff == 1){
        return ((double)rand() / RAND_MAX) * (1.2 - 0.7) + 0.7;
    }

    return ((double)rand() / RAND_MAX) * (1.0 - 0.5) + 0.5;
}

static void do_auto(void){
    double len = blocks_list.middle->x + blocks_list.middle->len + blocks_list.middle->size
               + blocks_list.middle->after->size * 0.5 - 400;
    double time_jump = len / KUN_SPEED;
    double speed_jump = time_jump * 0.5 * KUN_ACCELERATION;
    double gathering_time = speed_jump * speed_jump / 100.0;

    if(!gathering){
        start_gathering_audio();
    }else{
        update_gathering_audio();
        if(percentage < 1.0){
            percentage = (SDL_GetTicks() - tick) / LEN_CHUNK;
        }else{
            percentage = 1.0;
        }
    }

    if((SDL_GetTicks() - tick) > gathering_time){
        Uint32 duration = SDL_GetTicks() - tick;

        tick = SDL_GetTicks();
        stop_gathering_audio();
        printf("Press the space bar for %u\n",duration);

        gathering = false;
        kun.dh = speed_calculator_h(duration);
        jumping = true;
        percentage = 0.0;
        start_time = SDL_GetTicks();
        current_time = SDL_GetTicks();
    }

    if(blocks_list.middle->shape == 2 && (SDL_GetTicks() - tnt_time) > 3384){
        trigger_fail();
    }
}

static void do_score(void){
    scores += blocks_list.middle->before->len / 10;
    if(blocks_list.middle->size < 80){
        scores += 20;
    }
    if(blocks_list.middle->before->shape == 2){
        scores += 50;
    }
}

static void do_Moist(void){
    SDL_Texture *new_texture = create_scaled_texture(dirt_paths[0], blocks_list.middle->size);

    if (new_texture == NULL) {
        return;
    }

    SDL_DestroyTexture(blocks_list.middle->texture);
    blocks_list.middle->texture = new_texture;
}

static void start_gathering_audio(void){
    tick = SDL_GetTicks();
    gathering = true;
    channel_1 = Mix_PlayChannel(-1, chunk_1, 0);
    channel_2 = -1;
}

static void update_gathering_audio(void){
    bool first_chunk_finished;

    if (channel_1 >= 0) {
        first_chunk_finished = !Mix_Playing(channel_1);
    } else {
        // Fall back to the measured chunk length if SDL_mixer did not hand out a channel.
        first_chunk_finished = (SDL_GetTicks() - tick) >= LEN_CHUNK;
    }

    if (first_chunk_finished && (channel_2 < 0 || !Mix_Playing(channel_2))) {
        channel_2 = Mix_PlayChannel(-1, chunk_2, -1);
    }
}

static void stop_gathering_audio(void){
    if (channel_1 >= 0) {
        Mix_FadeOutChannel(channel_1, 700);
    }
    if (channel_2 >= 0) {
        Mix_FadeOutChannel(channel_2, 10);
    }

    channel_1 = -1;
    channel_2 = -1;
}

static SDL_Texture *create_scaled_texture(const char *file_path, int size){
    SDL_Surface *surface = IMG_Load(file_path);
    SDL_Surface *scaled_surface = NULL;
    SDL_Texture *texture = NULL;
    int width;
    int height;

    if (surface == NULL) {
        fprintf(stderr, "IMG_Load failed for %s: %s\n", file_path, IMG_GetError());
        return NULL;
    }

    width = surface->w * size / 100;
    height = surface->h * size / 100;
    if (width <= 0) {
        width = 1;
    }
    if (height <= 0) {
        height = 1;
    }

    scaled_surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        width,
        height,
        surface->format->BitsPerPixel,
        surface->format->format
    );
    if (scaled_surface == NULL) {
        fprintf(stderr, "SDL_CreateRGBSurfaceWithFormat failed: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }

    if (SDL_BlitScaled(surface, NULL, scaled_surface, NULL) < 0) {
        fprintf(stderr, "SDL_BlitScaled failed: %s\n", SDL_GetError());
        SDL_FreeSurface(scaled_surface);
        SDL_FreeSurface(surface);
        return NULL;
    }

    texture = SDL_CreateTextureFromSurface(app.renderer, scaled_surface);
    if (texture == NULL) {
        fprintf(stderr, "SDL_CreateTextureFromSurface failed for %s: %s\n", file_path, SDL_GetError());
    }

    SDL_FreeSurface(scaled_surface);
    SDL_FreeSurface(surface);
    return texture;
}

static Blocks *create_block_node(int size, int len, const char *texture_path, int block_shape){
    Blocks *new_block = malloc(sizeof(Blocks));

    if (new_block == NULL) {
        fprintf(stderr, "Failed to allocate block node.\n");
        return NULL;
    }

    new_block->texture = create_scaled_texture(texture_path, size);
    if (new_block->texture == NULL) {
        free(new_block);
        return NULL;
    }

    new_block->shape = block_shape;
    new_block->x = 0.0;
    new_block->size = size;
    new_block->len = len;
    new_block->before = NULL;
    new_block->after = NULL;
    return new_block;
}

static Blocks *create_initial_block(void){
    int block_shape;
    int size = (int)(100 * (((double)rand() / RAND_MAX) * (1.5 - 0.7) + 0.7));
    int len = (int)(100 * (((double)rand() / RAND_MAX) * (2.5 - 0.7) + 0.7));
    const char *texture_path = generate_random_block_asset(&block_shape);

    return create_block_node(size, len, texture_path, block_shape);
}

static Blocks *create_dynamic_block(void){
    int block_shape;
    int size = (int)(100 * difficulty_random_size(difficulty));
    int len = (int)(100 * difficulty_random_len(difficulty));
    const char *texture_path = generate_random_block_asset(&block_shape);

    return create_block_node(size, len, texture_path, block_shape);
}

static void destroy_block(Blocks *target){
    if (target == NULL) {
        return;
    }

    SDL_DestroyTexture(target->texture);
    free(target);
}

static void append_block(Blocks *new_block){
    if (new_block == NULL) {
        return;
    }

    if (blocks_list.tail == NULL) {
        blocks_list.head = new_block;
        blocks_list.tail = new_block;
        return;
    }

    blocks_list.tail->after = new_block;
    new_block->before = blocks_list.tail;
    blocks_list.tail = new_block;
}

static void remove_head_block(void){
    Blocks *old_head = blocks_list.head;

    if (old_head == NULL) {
        return;
    }

    blocks_list.head = old_head->after;
    if (blocks_list.head != NULL) {
        blocks_list.head->before = NULL;
    } else {
        blocks_list.tail = NULL;
    }

    destroy_block(old_head);
}

static bool advance_blocks(int steps){
    for (int i = 0; i < steps; ++i) {
        Blocks *new_block = create_dynamic_block();
        if (new_block == NULL) {
            return false;
        }

        append_block(new_block);
        blocks_list.middle = blocks_list.middle->after;
        remove_head_block();
        do_score();
    }

    return true;
}

static void finish_landing(void){
    printf("LAND\n");
    jumping = false;
    kun.h = 0;
    kun.dh = 0;

    if(blocks_list.middle->shape == 2){
        Mix_PlayChannel(-1,chunk_tnt,0);
        tnt_time = SDL_GetTicks();
    }else{
        Mix_PlayChannel(-1,chunk_jump,0);
    }

    if(blocks_list.middle->shape == 3){
        do_Moist();
    }
}

static bool load_chunks(void){
    chunk_1 = Mix_LoadWAV("./res/chunk/chunk_1.mp3");
    chunk_2 = Mix_LoadWAV("./res/chunk/chunk_2.mp3");
    chunk_fail = Mix_LoadWAV("./res/chunk/fail.mp3");
    chunk_jump = Mix_LoadWAV("./res/chunk/jump.mp3");
    chunk_tnt = Mix_LoadWAV("./res/chunk/tnt.mp3");

    if (chunk_1 == NULL || chunk_2 == NULL || chunk_fail == NULL || chunk_jump == NULL || chunk_tnt == NULL) {
        fprintf(stderr, "Mix_LoadWAV failed: %s\n", Mix_GetError());
        free_chunks();
        return false;
    }

    return true;
}

static void free_chunks(void){
    if (chunk_1 != NULL) {
        Mix_FreeChunk(chunk_1);
        chunk_1 = NULL;
    }
    if (chunk_2 != NULL) {
        Mix_FreeChunk(chunk_2);
        chunk_2 = NULL;
    }
    if (chunk_fail != NULL) {
        Mix_FreeChunk(chunk_fail);
        chunk_fail = NULL;
    }
    if (chunk_jump != NULL) {
        Mix_FreeChunk(chunk_jump);
        chunk_jump = NULL;
    }
    if (chunk_tnt != NULL) {
        Mix_FreeChunk(chunk_tnt);
        chunk_tnt = NULL;
    }
}

static void trigger_fail(void){
    if (fail) {
        return;
    }

    printf("YOU LOSE\n");
    fail = true;
    submit_score_if_eligible();
    Mix_PlayChannel(-1, chunk_fail, 0);
}

static void load_best_score(void){
    FILE *file;

    if (best_score_loaded) {
        return;
    }

    best_score_loaded = true;
    best_score = -1;
    file = fopen(BEST_SCORE_FILE_PATH, "r");
    if (file == NULL) {
        if (errno != ENOENT) {
            fprintf(stderr, "Failed to open %s: %s\n", BEST_SCORE_FILE_PATH, strerror(errno));
        }
        return;
    }

    if (fscanf(file, "%d", &best_score) != 1) {
        best_score = -1;
    }

    fclose(file);
}

static void save_best_score(void){
    FILE *file = fopen(BEST_SCORE_FILE_PATH, "w");

    if (file == NULL) {
        fprintf(stderr, "Failed to write %s: %s\n", BEST_SCORE_FILE_PATH, strerror(errno));
        return;
    }

    if (best_score >= 0) {
        fprintf(file, "%d\n", best_score);
    }

    fclose(file);
}

static void submit_score_if_eligible(void){
    load_best_score();
    broke_best_score = false;

    if (assisted_mode_used || scores <= 0) {
        return;
    }

    if (best_score < 0 || scores > best_score) {
        best_score = scores;
        broke_best_score = true;
        save_best_score();
    }
}
