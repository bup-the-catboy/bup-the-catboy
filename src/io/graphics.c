#include "io.h"

#include <SDL2/SDL.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Uint64 start_ticks;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_GLContext* gl_context;

struct Texture* graphics_load_texture(unsigned char* buf, size_t len) {
    struct Texture* texture = malloc(sizeof(struct Texture));
    unsigned char* image = stbi_load_from_memory(buf, len, &texture->width, &texture->height, NULL, STBI_rgb_alpha);
    glGenTextures(1, (GLuint*)&texture->texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture->texture_handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
    return texture;
}

void graphics_init(const char* window_name, int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    gl_context = SDL_GL_CreateContext(window);
}

void graphics_update_viewport(float viewx, float viewy, float vieww, float viewh) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-viewx, vieww - viewx, viewh - viewy, -viewy, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void graphics_start_frame() {
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
    if (texture) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture ? texture->texture_handle : 0);
}

void graphics_draw(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, uint32_t color) {
    glColor4ub(color >> 24, color >> 16, color >> 8, color >> 0);
    glBegin(GL_QUADS);
    glTexCoord2f(x1, y1);
    glVertex2f(u1, v1);
    glTexCoord2f(x2, y1);
    glVertex2f(u2, v1);
    glTexCoord2f(x2, y2);
    glVertex2f(u2, v2);
    glTexCoord2f(x1, y2);
    glVertex2f(u1, v2);
    glEnd();
}

void graphics_deinit() {
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}