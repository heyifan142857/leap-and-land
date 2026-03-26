//
// Created by user on 2024/1/18.
//
#include <utils/display.h>
#include <string.h>

typedef struct ImageCacheEntry {
    char *file_path;
    SDL_Texture *texture;
    int width;
    int height;
    Uint64 last_used;
    struct ImageCacheEntry *next;
} ImageCacheEntry;

typedef struct FontCacheEntry {
    char *file_path;
    int size;
    TTF_Font *font;
    Uint64 last_used;
    struct FontCacheEntry *next;
} FontCacheEntry;

typedef struct TextCacheEntry {
    char *font_path;
    char *text;
    int size;
    Uint8 r;
    Uint8 g;
    Uint8 b;
    SDL_Texture *texture;
    int width;
    int height;
    Uint64 last_used;
    struct TextCacheEntry *next;
} TextCacheEntry;

#define IMAGE_CACHE_LIMIT 32
#define FONT_CACHE_LIMIT 16
#define TEXT_CACHE_LIMIT 128

static ImageCacheEntry *image_cache_head;
static FontCacheEntry *font_cache_head;
static TextCacheEntry *text_cache_head;
static int image_cache_count;
static int font_cache_count;
static int text_cache_count;
static Uint64 cache_tick;

static Uint64 next_cache_tick(void);
static char *duplicate_string(const char *source);
static void destroy_image_entry(ImageCacheEntry *entry);
static void destroy_font_entry(FontCacheEntry *entry);
static void destroy_text_entry(TextCacheEntry *entry);
static void evict_oldest_image_entry(void);
static void evict_oldest_font_entry(void);
static void evict_oldest_text_entry(void);
static ImageCacheEntry *find_image_entry(const char *file_path);
static FontCacheEntry *find_font_entry(const char *file_path, int size);
static TextCacheEntry *find_text_entry(const char *file_path, const char *text, int size, Uint8 r, Uint8 g, Uint8 b);
static ImageCacheEntry *load_image_entry(const char *file_path);
static FontCacheEntry *load_font_entry(const char *file_path, int size);
static TextCacheEntry *load_text_entry(const char *file_path, const char *text, int size, Uint8 r, Uint8 g, Uint8 b);
static ImageCacheEntry *get_image_entry(const char *file_path);
static FontCacheEntry *get_font_entry(const char *file_path, int size);
static TextCacheEntry *get_text_entry(const char *file_path, const char *text, int size, Uint8 r, Uint8 g, Uint8 b);

void display_quit_cache(void) {
    while (text_cache_head != NULL) {
        TextCacheEntry *next = text_cache_head->next;
        destroy_text_entry(text_cache_head);
        text_cache_head = next;
    }
    while (font_cache_head != NULL) {
        FontCacheEntry *next = font_cache_head->next;
        destroy_font_entry(font_cache_head);
        font_cache_head = next;
    }
    while (image_cache_head != NULL) {
        ImageCacheEntry *next = image_cache_head->next;
        destroy_image_entry(image_cache_head);
        image_cache_head = next;
    }

    image_cache_count = 0;
    font_cache_count = 0;
    text_cache_count = 0;
    cache_tick = 0;
}

void display_image(const char* filePath, int x, int y) {
    ImageCacheEntry *entry = get_image_entry(filePath);
    SDL_Rect rect_image;

    if (entry == NULL) {
        return;
    }

    rect_image = (SDL_Rect){.x = x, .y = y, .w = entry->width, .h = entry->height};
    SDL_RenderCopy(app.renderer, entry->texture, NULL, &rect_image);
}

void display_font(const char* filePath,const char* text,int size,int r,int g,int b,int x,int y){
    display_font_alpha(filePath, text, size, r, g, b, 255, x, y);
}

void display_font_alpha(const char* filePath,const char* text,int size,int r,int g,int b,Uint8 alpha,int x,int y){
    TextCacheEntry *entry = get_text_entry(filePath, text, size, (Uint8)r, (Uint8)g, (Uint8)b);
    SDL_Rect rect;

    if (entry == NULL) {
        return;
    }

    if (SDL_SetTextureAlphaMod(entry->texture, alpha) < 0) {
        fprintf(stderr, "SDL_SetTextureAlphaMod failed: %s\n", SDL_GetError());
    }
    rect = (SDL_Rect){.x = x, .y = y, .w = entry->width, .h = entry->height};
    SDL_RenderCopy(app.renderer, entry->texture, NULL, &rect);
}

static Uint64 next_cache_tick(void) {
    cache_tick += 1;
    return cache_tick;
}

static char *duplicate_string(const char *source) {
    char *copy;
    size_t length;

    if (source == NULL) {
        return NULL;
    }

    length = strlen(source) + 1;
    copy = malloc(length);
    if (copy == NULL) {
        fprintf(stderr, "Failed to allocate cached string.\n");
        return NULL;
    }

    memcpy(copy, source, length);
    return copy;
}

static void destroy_image_entry(ImageCacheEntry *entry) {
    if (entry == NULL) {
        return;
    }

    free(entry->file_path);
    SDL_DestroyTexture(entry->texture);
    free(entry);
}

static void destroy_font_entry(FontCacheEntry *entry) {
    if (entry == NULL) {
        return;
    }

    free(entry->file_path);
    TTF_CloseFont(entry->font);
    free(entry);
}

static void destroy_text_entry(TextCacheEntry *entry) {
    if (entry == NULL) {
        return;
    }

    free(entry->font_path);
    free(entry->text);
    SDL_DestroyTexture(entry->texture);
    free(entry);
}

static void evict_oldest_image_entry(void) {
    ImageCacheEntry *current = image_cache_head;
    ImageCacheEntry *oldest = image_cache_head;
    ImageCacheEntry *previous = NULL;
    ImageCacheEntry *oldest_previous = NULL;

    if (image_cache_head == NULL) {
        return;
    }

    while (current != NULL) {
        if (current->last_used < oldest->last_used) {
            oldest = current;
            oldest_previous = previous;
        }
        previous = current;
        current = current->next;
    }

    if (oldest_previous == NULL) {
        image_cache_head = oldest->next;
    } else {
        oldest_previous->next = oldest->next;
    }

    destroy_image_entry(oldest);
    image_cache_count -= 1;
}

static void evict_oldest_font_entry(void) {
    FontCacheEntry *current = font_cache_head;
    FontCacheEntry *oldest = font_cache_head;
    FontCacheEntry *previous = NULL;
    FontCacheEntry *oldest_previous = NULL;

    if (font_cache_head == NULL) {
        return;
    }

    while (current != NULL) {
        if (current->last_used < oldest->last_used) {
            oldest = current;
            oldest_previous = previous;
        }
        previous = current;
        current = current->next;
    }

    if (oldest_previous == NULL) {
        font_cache_head = oldest->next;
    } else {
        oldest_previous->next = oldest->next;
    }

    destroy_font_entry(oldest);
    font_cache_count -= 1;
}

static void evict_oldest_text_entry(void) {
    TextCacheEntry *current = text_cache_head;
    TextCacheEntry *oldest = text_cache_head;
    TextCacheEntry *previous = NULL;
    TextCacheEntry *oldest_previous = NULL;

    if (text_cache_head == NULL) {
        return;
    }

    while (current != NULL) {
        if (current->last_used < oldest->last_used) {
            oldest = current;
            oldest_previous = previous;
        }
        previous = current;
        current = current->next;
    }

    if (oldest_previous == NULL) {
        text_cache_head = oldest->next;
    } else {
        oldest_previous->next = oldest->next;
    }

    destroy_text_entry(oldest);
    text_cache_count -= 1;
}

static ImageCacheEntry *find_image_entry(const char *file_path) {
    for (ImageCacheEntry *entry = image_cache_head; entry != NULL; entry = entry->next) {
        if (strcmp(entry->file_path, file_path) == 0) {
            entry->last_used = next_cache_tick();
            return entry;
        }
    }

    return NULL;
}

static FontCacheEntry *find_font_entry(const char *file_path, int size) {
    for (FontCacheEntry *entry = font_cache_head; entry != NULL; entry = entry->next) {
        if (entry->size == size && strcmp(entry->file_path, file_path) == 0) {
            entry->last_used = next_cache_tick();
            return entry;
        }
    }

    return NULL;
}

static TextCacheEntry *find_text_entry(const char *file_path, const char *text, int size, Uint8 r, Uint8 g, Uint8 b) {
    for (TextCacheEntry *entry = text_cache_head; entry != NULL; entry = entry->next) {
        if (entry->size == size
            && entry->r == r
            && entry->g == g
            && entry->b == b
            && strcmp(entry->font_path, file_path) == 0
            && strcmp(entry->text, text) == 0) {
            entry->last_used = next_cache_tick();
            return entry;
        }
    }

    return NULL;
}

static ImageCacheEntry *load_image_entry(const char *file_path) {
    SDL_Surface *surface_image = IMG_Load(file_path);
    SDL_Texture *texture_image;
    ImageCacheEntry *entry;

    if (surface_image == NULL) {
        fprintf(stderr, "IMG_Load failed for %s: %s\n", file_path, IMG_GetError());
        return NULL;
    }

    texture_image = SDL_CreateTextureFromSurface(app.renderer, surface_image);
    if (texture_image == NULL) {
        fprintf(stderr, "SDL_CreateTextureFromSurface failed for %s: %s\n", file_path, SDL_GetError());
        SDL_FreeSurface(surface_image);
        return NULL;
    }

    entry = calloc(1, sizeof(ImageCacheEntry));
    if (entry == NULL) {
        fprintf(stderr, "Failed to allocate image cache entry.\n");
        SDL_DestroyTexture(texture_image);
        SDL_FreeSurface(surface_image);
        return NULL;
    }

    entry->texture = texture_image;
    entry->file_path = duplicate_string(file_path);
    if (entry->file_path == NULL) {
        destroy_image_entry(entry);
        SDL_FreeSurface(surface_image);
        return NULL;
    }

    entry->width = surface_image->w;
    entry->height = surface_image->h;
    entry->last_used = next_cache_tick();
    entry->next = image_cache_head;
    image_cache_head = entry;
    image_cache_count += 1;

    SDL_FreeSurface(surface_image);
    return entry;
}

static FontCacheEntry *load_font_entry(const char *file_path, int size) {
    FontCacheEntry *entry = calloc(1, sizeof(FontCacheEntry));

    if (entry == NULL) {
        fprintf(stderr, "Failed to allocate font cache entry.\n");
        return NULL;
    }

    entry->file_path = duplicate_string(file_path);
    if (entry->file_path == NULL) {
        free(entry);
        return NULL;
    }

    entry->font = TTF_OpenFont(file_path, size);
    if (entry->font == NULL) {
        fprintf(stderr, "TTF_OpenFont failed for %s: %s\n", file_path, TTF_GetError());
        destroy_font_entry(entry);
        return NULL;
    }

    entry->size = size;
    entry->last_used = next_cache_tick();
    entry->next = font_cache_head;
    font_cache_head = entry;
    font_cache_count += 1;
    return entry;
}

static TextCacheEntry *load_text_entry(const char *file_path, const char *text, int size, Uint8 r, Uint8 g, Uint8 b) {
    FontCacheEntry *font_entry = get_font_entry(file_path, size);
    SDL_Color color = {r, g, b, 255};
    SDL_Surface *surface;
    SDL_Texture *texture;
    TextCacheEntry *entry;

    if (font_entry == NULL) {
        return NULL;
    }

    surface = TTF_RenderText_Blended(font_entry->font, text, color);
    if (surface == NULL) {
        fprintf(stderr, "TTF_RenderText_Blended failed: %s\n", TTF_GetError());
        return NULL;
    }

    texture = SDL_CreateTextureFromSurface(app.renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "SDL_CreateTextureFromSurface failed for text '%s': %s\n", text, SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    entry = calloc(1, sizeof(TextCacheEntry));
    if (entry == NULL) {
        fprintf(stderr, "Failed to allocate text cache entry.\n");
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        return NULL;
    }

    entry->texture = texture;
    entry->font_path = duplicate_string(file_path);
    entry->text = duplicate_string(text);
    if (entry->font_path == NULL || entry->text == NULL) {
        destroy_text_entry(entry);
        SDL_FreeSurface(surface);
        return NULL;
    }

    entry->size = size;
    entry->r = r;
    entry->g = g;
    entry->b = b;
    entry->width = surface->w;
    entry->height = surface->h;
    entry->last_used = next_cache_tick();
    entry->next = text_cache_head;
    text_cache_head = entry;
    text_cache_count += 1;

    SDL_FreeSurface(surface);
    return entry;
}

static ImageCacheEntry *get_image_entry(const char *file_path) {
    ImageCacheEntry *entry = find_image_entry(file_path);

    if (entry != NULL) {
        return entry;
    }

    if (image_cache_count >= IMAGE_CACHE_LIMIT) {
        evict_oldest_image_entry();
    }

    return load_image_entry(file_path);
}

static FontCacheEntry *get_font_entry(const char *file_path, int size) {
    FontCacheEntry *entry = find_font_entry(file_path, size);

    if (entry != NULL) {
        return entry;
    }

    if (font_cache_count >= FONT_CACHE_LIMIT) {
        evict_oldest_font_entry();
    }

    return load_font_entry(file_path, size);
}

static TextCacheEntry *get_text_entry(const char *file_path, const char *text, int size, Uint8 r, Uint8 g, Uint8 b) {
    TextCacheEntry *entry = find_text_entry(file_path, text, size, r, g, b);

    if (entry != NULL) {
        return entry;
    }

    if (text_cache_count >= TEXT_CACHE_LIMIT) {
        evict_oldest_text_entry();
    }

    return load_text_entry(file_path, text, size, r, g, b);
}
