#ifdef RENDERER_SDL_RENDERER

#include "io/io.h"
#include "io/sdlgfx.h"

#include <GL/gl.h>

#include <endianness.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void* window;
void* renderer;
struct Texture* current_texture;
float res_width, res_height;
float win_width, win_height;
float view_width, view_height;
int scissor_x, scissor_y, scissor_w, scissor_h;
float target_fps;

struct Texture* graphics_load_texture(unsigned char* buf, size_t len) {
    struct Texture* tex = malloc(sizeof(struct Texture));
    unsigned char* image = stbi_load_from_memory(buf, len, &tex->width, &tex->height, NULL, STBI_rgb_alpha);
    tex->texture_handle = sdl_texture_create(renderer, image, tex->width, tex->height);
    stbi_image_free(image);
    return tex;
}

void graphics_init(const char* window_name, int width, int height) {
    sdl_window_init(window_name, width, height, SDL_GFX_API_SDLRENDERER, &window, &renderer);
}

void graphics_set_resolution(float width, float height) {
    res_width  = width;
    res_height = height;
}

void graphics_start_frame() {
    int width, height;
    sdl_window_size(window, &width, &height);
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
    sdl_renderer_set_color(renderer, 127, 127, 127, 255);
    sdl_renderer_clear(renderer);
    sdl_renderer_set_color(renderer, 255, 255, 255, 255);
}

void graphics_end_frame() {
    sdl_renderer_set_color(renderer, 0, 0, 0, 255);
    sdl_renderer_fill_rect(renderer, 0, 0, win_width, scissor_y);
    sdl_renderer_fill_rect(renderer, 0, 0, scissor_x, win_height);
    sdl_renderer_fill_rect(renderer, 0, win_height - scissor_y, win_width, scissor_y);
    sdl_renderer_fill_rect(renderer, win_width - scissor_x, 0, win_width, scissor_y);
    sdl_renderer_flush(renderer);
}

void graphics_get_size(int* width, int* height) {
    sdl_window_size(window, width, height);
}

void graphics_select_texture(struct Texture* texture) {
    current_texture = texture;
}

void graphics_draw(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, uint32_t color) {
    float x = round(x1 * (scissor_w / res_width)  + scissor_x);
    float y = round(y1 * (scissor_h / res_height) + scissor_y);
    float w = round((x2 - x1) * (scissor_w / res_width));
    float h = round((y2 - y1) * (scissor_h / res_height));
    sdl_renderer_set_color(renderer, color >> 24, color >> 16, color >> 8, color >> 0);
    if (current_texture == NULL) {
        if (w < 0) w = -w;
        if (h < 0) h = -h;
        sdl_renderer_fill_rect(renderer, x, y, w, h);
    }
    else {
        int tex_w, tex_h;
        bool flip_x = w < 0, flip_y = h < 0;
        if (flip_x) {
            w = -w;
            x -= w;
        }
        if (flip_y) {
            h = -h;
            y -= h;
        }
        void* tex = current_texture->texture_handle;
        sdl_texture_size(tex, &tex_w, &tex_h);
        sdl_renderer_draw_texture(renderer, tex,
            round(u1 * tex_w),
            round(v1 * tex_h),
            round((u2 - u1) * tex_w),
            round((v2 - v1) * tex_h),
            x, y, w, h, flip_x, flip_y
        );
    }
}

void graphics_deinit() {
    sdl_window_deinit(window);
}

void graphics_flush(bool redraw) {}
void graphics_register_shader(const char* name, const char* shader) {}
void graphics_push_shader(const char* name) {}
void graphics_pop_shader() {}
void graphics_pop_all_shaders() {}
void graphics_shader_set_int(const char* name, int value) {}
void graphics_shader_set_float(const char* name, float value) {}

#endif