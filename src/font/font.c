#include <SDL2/SDL.h>
#include <lunarengine.h>
#include <math.h>

#include "main.h"
#include "assets/assets.h"
#include "game/level.h"

#define M_2PI 6.283185307179586

struct TextGraph {
    bool wavy;
    bool gay;
    int color;
    int wave_size;
    int wave_length;
    int gay_length;
    float gay_speed;
    float wave_speed;
    float scale;
    float opacity;
    int spacing;
    int textptr;
    char* text;
    struct TextGraph* next;
};

int clamp(int x, int min, int max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

float wrap(float x, float min, float max) {
    float range = max - min;
    while (x < min) x += range;
    while (x > max) x -= range;
    return x;
}

void _append(struct TextGraph* curr, char c) {
    if (!curr->text) {
        curr->text = malloc(8192);
        memset(curr->text, 0, 8192);
    }
    curr->text[curr->textptr++] = c;
}

unsigned int _read_color(const char* string, int* ptr) {
    unsigned int value;
    char c;
    while ((c = string[(*ptr)++])) {
        int num = -1;
        if (c >= '0' && c <= '9') num = c - '0';
        if (c >= 'A' && c <= 'F') num = c - 'A' + 10;
        if (c >= 'a' && c <= 'f') num = c - 'a' + 10;
        if (num == -1) break;
        value <<= 4;
        value += num;
    }
    (*ptr)--;
    return value;
}

int _read_number(const char* string, int* ptr) {
    int value = 0;
    char c;
    while ((c = string[(*ptr)++])) {
        if (c < '0' || c > '9') break;
        value *= 10;
        value += c - '0';
    }
    (*ptr)--;
    return value;
}

#define read_color _read_color(string, &ptr)
#define read_number _read_number(string, &ptr)
#define append(c) _append(curr, c)
#define nextchar      \
    c = string[ptr++]; \
    if (c == 0) break

struct TextGraph* parse_text_graph(const char* string) {
    struct TextGraph* text = malloc(sizeof(struct TextGraph));
    struct TextGraph* curr = text;
    memset(text, 0, sizeof(struct TextGraph));
    curr->color = 0xFFFFFF;
    curr->scale = 1;
    curr->opacity = 1;
    curr->spacing = 8;
    int ptr = 0;
    char c;
    while ((c = string[ptr++])) {
        if (c == '$') {
            nextchar;
            if (c == '$') append('$');
            else if (c == '{') {
                if (curr->text) {
                    curr->next = malloc(sizeof(struct TextGraph));
                    memcpy(curr->next, curr, sizeof(struct TextGraph));
                    curr = curr->next;
                    curr->next = NULL;
                    curr->text = NULL;
                    curr->textptr = 0;
                    curr->spacing = 8;
                }
                nextchar;
                if (c == '^') {
                    curr->scale = read_number / 100.f;
                    nextchar;
                }
                else if (c == '%') {
                    curr->opacity = read_number / 100.f;
                    nextchar;
                }
                else if (c == '#') {
                    curr->gay = false;
                    curr->color = read_color;
                    nextchar;
                }
                else if (c == '_') {
                    curr->spacing = read_number;
                    append(' ');
                    nextchar;
                }
                else if (c == '&') {
                    curr->gay = true;
                    curr->gay_length = 16;
                    curr->gay_speed = 1;
                    bool left = false;
                    while ((c = string[ptr++])) {
                        if (c == '}') break;
                        if (c == '=') curr->gay_speed  = read_number / 100.f;
                        if (c == '-') curr->gay_length = read_number;
                        if (c == '<') left = true;
                    }
                    if (left) curr->gay_speed *= -1;
                }
                else if (c == '~') {
                    curr->wavy = true;
                    curr->wave_size = 2;
                    curr->wave_length = 8;
                    curr->wave_speed = 1;
                    while ((c = string[ptr++])) {
                        if (c == '}') break;
                        if (c == '^') curr->wave_size   = read_number;
                        if (c == '-') curr->wave_length = read_number;
                        if (c == '=') curr->wave_speed  = read_number / 100.f;
                    }
                }
                else if (c == '!') {
                    while ((c = string[ptr++])) {
                        if (c == '}') break;
                        if (c == '#' || c == '*') curr->color = 0xFFFFFF;
                        if (c == '^' || c == '*') curr->scale = 1;
                        if (c == '%' || c == '*') curr->opacity = 1;
                        if (c == '&' || c == '*') curr->gay = false;
                        if (c == '~' || c == '*') curr->wavy = false;
                    }
                }
            }
            else {
                append('$');
                append(c);
            }
        }
        else append(c);
    }
    return text;
}

void free_text_graph(struct TextGraph* text) {
    if (text->text) free(text->text);
    if (text->next) free_text_graph(text->next);
    free(text);
}

void render_text_graph(LE_DrawList* dl, float x, float y, struct TextGraph* text) {
    SDL_Texture* texture = GET_ASSET(SDL_Texture, "images/font.png");
    struct TextGraph* curr = text;
    char c;
    int ptr;
    int curr_glyph = 0;
    while (curr) {
        ptr = 0;
        if (!curr->text) break;
        while ((c = curr->text[ptr++])) {
            float wave = 0;
            int alpha = clamp((int)(curr->opacity * 255), 0, 255);
            unsigned int color;
            if (curr->wavy) wave = sin((global_timer * curr->wave_speed) / -60.f * M_2PI + (curr_glyph % curr->wave_length) / (float)curr->wave_length * M_2PI) * curr->wave_size * curr->scale;
            if (curr->gay) {
                float h = (global_timer * curr->gay_speed) / -60.f + (curr_glyph % curr->gay_length) / (float)curr->gay_length;
                float s = 1;
                float v = 1;
                h = wrap(h, 0, 1);

                // https://github.com/Inseckto/HSV-to-RGB/blob/master/HSV2RGB.c
                int i = floor(h * 6);
                float f = h * 6 - i;
                float p = v * (1 - s);
                float q = v * (1 - f * s);
                float t = v * (1 - (1 - f) * s);
                float r, g, b;
                switch (i % 6) {
                    case 0: r = v, g = t, b = p; break;
                    case 1: r = q, g = v, b = p; break;
                    case 2: r = p, g = v, b = t; break;
                    case 3: r = p, g = q, b = v; break;
                    case 4: r = t, g = p, b = v; break;
                    case 5: r = v, g = p, b = q; break;
                }

                color = ((int)(r * 255) << 24) | ((int)(g * 255) << 16) | ((int)(b * 255) << 8) | (alpha);
            }
            else color = (curr->color << 8) | (alpha);
            int index = (unsigned char)c - 32;
            int srcX = index % 12 * 10;
            int srcY = index / 12 * 10;
            LE_DrawSetColor(dl, color);
            LE_DrawListAppend(dl, texture, x, y + wave, curr->scale * 10, curr->scale * 10, srcX, srcY, 10, 10);
            x += curr->scale * curr->spacing;
            curr->spacing = 8;
            curr_glyph++;
        }
        curr = curr->next;
    }
}

void render_text(LE_DrawList* dl, float x, float y, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char formatted[8192];
    vsnprintf(formatted, 8192, fmt, args);
    va_end(args);

    unsigned int prev = LE_DrawGetColor(dl);
    struct TextGraph* text = parse_text_graph(formatted);
    render_text_graph(dl, x, y, text);
    free_text_graph(text);
    LE_DrawSetColor(dl, prev);
}