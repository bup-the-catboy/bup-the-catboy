#ifndef BTCB_SDL_H

#include <stdbool.h>

enum SDL_GFX_API {
    SDL_GFX_API_SDLRENDERER,
    SDL_GFX_API_OPENGL,
};

void  sdl_window_init(const char* title, int width, int height, enum SDL_GFX_API gfx_api, void** window, void** gfx_handle);
void  sdl_window_deinit(void* window);
void  sdl_window_size(void* window, int* width, int* height);
void  sdl_opengl_flush(void* window);
void  sdl_renderer_set_color(void* renderer, int r, int g, int b, int a);
void  sdl_renderer_clear(void* renderer);
void  sdl_renderer_fill_rect(void* renderer, float x, float y, float w, float h);
void  sdl_renderer_draw_texture(void* renderer, void* texture, int srcX, int srcY, int srcW, int srcH, float dstX, float dstY, float dstW, float dstH, bool flip_x, bool flip_y);
void  sdl_renderer_flush(void* renderer);
void* sdl_texture_create(void* renderer, void* texture_data, int width, int height);
void  sdl_texture_size(void* texture, int* width, int* height);

#endif