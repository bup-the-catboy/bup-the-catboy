#ifdef RENDERER_OPENGL_LEGACY

#include "io/io.h"
#include "main.h"

#include <SDL2/SDL.h>

#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Uint64 start_ticks;
SDL_Window* window;
SDL_GLContext* gl_context;
struct Texture* current_texture;
float res_width, res_height;
float win_width, win_height;
float view_width, view_height;
int scissor_x, scissor_y, scissor_w, scissor_h;

struct Texture* graphics_load_texture(unsigned char* buf, size_t len) {
    struct Texture* texture = malloc(sizeof(struct Texture));
    unsigned char* image = stbi_load_from_memory(buf, len, &texture->width, &texture->height, NULL, STBI_rgb_alpha);
    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
    texture->texture_handle = (void*)(uintptr_t)handle;
    return texture;
}

void graphics_init(const char* window_name, int width, int height) {
    SDL_SetHint("SDL_VIDEODRIVER", "wayland");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    gl_context = SDL_GL_CreateContext(window);
    glEnable(GL_SCISSOR_TEST);
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
    view_width  /= win_width;
    view_height /= win_height;
    glViewport(0, 0, width, height);
    glScissor(scissor_x, scissor_y, scissor_w, scissor_h);
    glClearColor(.5f, .5f, .5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    start_ticks = SDL_GetTicks64();
}

void graphics_end_frame(float fps) {
    glFlush();
    SDL_GL_SwapWindow(window);
    Uint64 end_ticks = SDL_GetTicks64();
    Uint64 frame_time = end_ticks - start_ticks;
    Sint64 wait_time = (1000 / fps) - frame_time;
    if (wait_time <= 0) return;
    SDL_Delay(wait_time);
}

void graphics_get_size(int* width, int* height) {
    SDL_GetWindowSize(window, width, height);
}

void graphics_select_texture(struct Texture* texture) {
    if (current_texture == texture) return;
    current_texture = texture;
    if (texture) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture ? (uintptr_t)texture->texture_handle : 0);
}

void graphics_draw(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, uint32_t color) {
    x1 = ((x1 / res_width)  * view_width  + (1 - view_width)  / 2) *  2 - 1;
    x2 = ((x2 / res_width)  * view_width  + (1 - view_width)  / 2) *  2 - 1;
    y1 = ((y1 / res_height) * view_height + (1 - view_height) / 2) * -2 + 1;
    y2 = ((y2 / res_height) * view_height + (1 - view_height) / 2) * -2 + 1;
    glColor4ub(color >> 24, color >> 16, color >> 8, color >> 0);
    glBegin(GL_QUADS);
    glTexCoord2f(u1, v1);
    glVertex2f(x1, y1);
    glTexCoord2f(u2, v1);
    glVertex2f(x2, y1);
    glTexCoord2f(u2, v2);
    glVertex2f(x2, y2);
    glTexCoord2f(u1, v2);
    glVertex2f(x1, y2);
    glEnd();
}

void graphics_deinit() {
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}

int graphics_load_shader(const char* shader) {
    return 0;
}

void graphics_select_shader(int shader) {}
void graphics_shader_set_int(const char* name, int value) {}
void graphics_shader_set_float(const char* name, float value) {}

#endif