#ifdef RENDERER_SDL_RENDERER

#include "io/io.h"

#include <SDL2/SDL.h>
#include <GL/gl.h>

#include <endianness.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

SDL_Window* window;
SDL_Renderer* renderer;
struct Texture* current_texture;
float res_width, res_height;
float win_width, win_height;
float view_width, view_height;
int scissor_x, scissor_y, scissor_w, scissor_h;
float target_fps;

struct Texture* graphics_load_texture(unsigned char* buf, size_t len) {
    struct Texture* texture = malloc(sizeof(struct Texture));
    unsigned char* image = stbi_load_from_memory(buf, len, &texture->width, &texture->height, NULL, STBI_rgb_alpha);
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(image, texture->width, texture->height, 32, 4 * texture->width,
#ifdef IS_BIG_ENDIAN
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#else
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#endif
    );
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    stbi_image_free(image);
    texture->texture_handle = tex;
    return texture;
}

void graphics_init(const char* window_name, int width, int height) {
    SDL_SetHint("SDL_VIDEODRIVER", "wayland");
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void graphics_set_resolution(float width, float height) {
    res_width  = width;
    res_height = height;
}

void graphics_start_frame() {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    win_width  = width;
    win_height = height;
    view_width  = win_width;
    view_height = win_height;
    float target_aspect_ratio = res_width / res_height;
    float aspect_ratio = win_width / win_height;
    if (target_aspect_ratio > aspect_ratio) view_height = width / target_aspect_ratio;
    else view_width = height * target_aspect_ratio;
    scissor_x = round((win_width  - view_width)  / 2);
    scissor_y = round((win_height - view_height) / 2);
    scissor_w = view_width;
    scissor_h = view_height;
    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void graphics_end_frame() {
    SDL_Rect rect;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    rect = (SDL_Rect){ 0, 0, win_width, scissor_y };
    SDL_RenderFillRect(renderer, &rect);
    rect = (SDL_Rect){ 0, 0, scissor_x, win_height };
    SDL_RenderFillRect(renderer, &rect);
    rect = (SDL_Rect){ 0, win_height - scissor_y, win_width, scissor_y };
    SDL_RenderFillRect(renderer, &rect);
    rect = (SDL_Rect){ win_width - scissor_x, 0, win_width, scissor_y };
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderPresent(renderer);
}

void graphics_get_size(int* width, int* height) {
    SDL_GetWindowSize(window, width, height);
}

void graphics_select_texture(struct Texture* texture) {
    current_texture = texture;
}

void graphics_draw(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, uint32_t color) {
    SDL_FRect dst = (SDL_FRect){
        .x = round(x1 * (scissor_w / res_width)  + scissor_x),
        .y = round(y1 * (scissor_h / res_height) + scissor_y),
        .w = round((x2 - x1) * (scissor_w / res_width)),
        .h = round((y2 - y1) * (scissor_h / res_height)),
    };
    SDL_SetRenderDrawColor(renderer, color >> 24, color >> 16, color >> 8, color >> 0);
    if (current_texture == NULL) {
        if (dst.w < 0) dst.w = -dst.w;
        if (dst.h < 0) dst.h = -dst.h;
        SDL_RenderFillRectF(renderer, &dst);
    }
    else {
        int w, h;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (dst.w < 0) {
            dst.w = -dst.w;
            dst.x -= dst.w;
            flip |= SDL_FLIP_HORIZONTAL;
        }
        if (dst.h < 0) {
            dst.h = -dst.h;
            dst.y -= dst.h;
            flip |= SDL_FLIP_VERTICAL;
        }
        SDL_Texture* tex = current_texture->texture_handle;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect src = (SDL_Rect){
            .x = round(u1 * w),
            .y = round(v1 * h),
            .w = round((u2 - u1) * w),
            .h = round((v2 - v1) * h),
        };
        SDL_RenderCopyExF(renderer, tex, &src, &dst, 0, NULL, flip);
    }
}

void graphics_deinit() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

int graphics_load_shader(const char* shader) {
    return 0;
}

void graphics_select_shader(int shader) {}
void graphics_shader_set_int(const char* name, int value) {}
void graphics_shader_set_float(const char* name, float value) {}

#endif