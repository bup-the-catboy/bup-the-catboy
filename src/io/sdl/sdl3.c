#ifdef SDL_VERSION_3

#include <SDL3/SDL.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "io/io.h"
#include "io/sdlgfx.h"

struct SDLWindowGfx {
    enum SDL_GFX_API api;
    void* window;
    void* gfx_handle;
};

SDL_AudioStream* audio_stream;
SDL_Joystick* joystick;
SDL_JoystickID* joysticks;
int joystick_count;

void controller_init() {
    SDL_Init(SDL_INIT_JOYSTICK);
    joysticks = SDL_GetJoysticks(&joystick_count);
    printf("%d controllers found\n", joystick_count);
    if (joystick_count > 0) {
        controller_select(0);
    }
}

int controller_count() {
    return joystick_count;
}

void controller_select(int index) {
    if (joystick) SDL_CloseJoystick(joystick);
    joystick = SDL_OpenJoystick(joysticks[index]);
    printf("Controller %d connected: %s\n", index, controller_current_name());
}

const char* controller_name(int index) {
    return SDL_GetJoystickNameForID(joysticks[index]);
}

const char* controller_current_name() {
    return SDL_GetJoystickName(joystick);
}

bool controller_key_down(int keycode) {
    return SDL_GetKeyboardState(NULL)[keycode];
}

bool controller_button_down(int button) {
    if (!joystick) return false;
    return SDL_GetJoystickButton(joystick, button);
}

bool controller_mouse_down(int button) {
    return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(button);
}

void controller_mouse_pos(int* x, int* y) {
    float X, Y;
    SDL_GetMouseState(&X, &Y);
    *x = X;
    *y = Y;
}

float controller_get_axis(int index) {
    if (!joystick) return 0;
    SDL_PumpEvents();
    return SDL_GetJoystickAxis(joystick, index);
}

int controller_num_axes() {
    if (!joystick) return 0;
    return SDL_GetNumJoystickAxes(joystick);
}

int controller_num_buttons() {
    if (!joystick) return 0;
    return SDL_GetNumJoystickButtons(joystick);
}

void controller_deinit() {
    if (joystick) SDL_CloseJoystick(joystick);
}

void audio_backend_open() {
    SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, AUDIO_SAMPLE_RATE };
    audio_stream = SDL_CreateAudioStream(&spec, &spec);
}

void audio_backend_queue(short* samples, int num_samples) {
    if (SDL_GetAudioStreamQueued(audio_stream) > 0) {
        int wait_time = num_samples / (float)AUDIO_SAMPLE_RATE * 1000;
        if (wait_time > 0) {
            SDL_Delay(wait_time);
            return;
        }
    }
    SDL_PutAudioStreamData(audio_stream, samples, num_samples * sizeof(short));
    SDL_Delay(num_samples / (float)AUDIO_SAMPLE_RATE * 1000);
}

void audio_backend_close() {
    SDL_DestroyAudioStream(audio_stream);
}

uint64_t ticks() {
    return SDL_GetTicks();
}

void sync(uint64_t start, int ms) {
    Uint64 end = ticks();
    Uint64 time = end - start;
    Sint64 wait_time = ms - time;
    if (wait_time <= 0) return;
    sleep_ms(wait_time);
}

void sleep_ms(uint64_t ms) {
    SDL_Delay(ms);
}

void io_deinit() {
    SDL_Quit();
}

bool requested_quit() {
    SDL_PumpEvents();
    return SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_EVENT_QUIT, SDL_EVENT_QUIT) > 0;
}

void sdl_window_init(const char* title, int width, int height, enum SDL_GFX_API gfx_api, void** window, void** gfx_handle) {
    SDL_Init(SDL_INIT_VIDEO);
    int flags = SDL_WINDOW_RESIZABLE;
    if (gfx_api == SDL_GFX_API_OPENGL) {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        flags |= SDL_WINDOW_OPENGL;
    }
    struct SDLWindowGfx* w = *window = malloc(sizeof(struct SDLWindowGfx));
    w->window = SDL_CreateWindow(title, width, height, flags);
    SDL_ShowWindow(w->window);
    switch (gfx_api) {
        case SDL_GFX_API_SDLRENDERER:
            w->gfx_handle = SDL_CreateRenderer(w->window, NULL);
            SDL_SetRenderDrawBlendMode(w->gfx_handle, SDL_BLENDMODE_BLEND);
            printf("Currently using render driver: %s\n", SDL_GetRendererName(w->gfx_handle));
            printf("There is/are %d available:\n", SDL_GetNumRenderDrivers());
            for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
                printf(" - %s\n", SDL_GetRenderDriver(i));
            }
            break;
        case SDL_GFX_API_OPENGL:
            w->gfx_handle = SDL_GL_CreateContext(w->window);
            break;
    }
    *gfx_handle = w->gfx_handle;
}

void sdl_window_deinit(void* window) {
    struct SDLWindowGfx* w = window;
    switch (w->api) {
        case SDL_GFX_API_SDLRENDERER:
            SDL_DestroyRenderer(w->gfx_handle);
            break;
        case SDL_GFX_API_OPENGL:
            SDL_GL_DestroyContext(w->gfx_handle);
            break;
    }
    free(w);
}

void sdl_window_size(void* window, int* width, int* height) {
    SDL_GetWindowSize(((struct SDLWindowGfx*)window)->window, width, height);
}

void sdl_opengl_flush(void* window) {
    SDL_GL_SwapWindow(((struct SDLWindowGfx*)window)->window);
}

void sdl_renderer_set_color(void* renderer, int r, int g, int b, int a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void sdl_renderer_clear(void* renderer) {
    SDL_RenderClear(renderer);
}

void sdl_renderer_fill_rect(void* renderer, float x, float y, float w, float h) {
    SDL_FRect rect = (SDL_FRect){ .x = x, .y = y, .w = w, .h = h };
    SDL_RenderFillRect(renderer, &rect);
}

void sdl_renderer_draw_texture(void* renderer, void* texture, int srcX, int srcY, int srcW, int srcH, float dstX, float dstY, float dstW, float dstH, bool flip_x, bool flip_y) {
    SDL_FlipMode flip = 0;
    if (flip_x) flip |= SDL_FLIP_HORIZONTAL;
    if (flip_y) flip |= SDL_FLIP_VERTICAL;
    SDL_FRect src = (SDL_FRect){ .x = srcX, .y = srcY, .w = srcW, .h = srcH };
    SDL_FRect dst = (SDL_FRect){ .x = dstX, .y = dstY, .w = dstW, .h = dstH };
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    SDL_SetTextureColorMod(texture, r, g, b);
    SDL_SetTextureAlphaMod(texture, a);
    SDL_RenderTextureRotated(renderer, texture, &src, &dst, 0, NULL, flip);
}

void sdl_renderer_flush(void* renderer) {
    SDL_RenderPresent(renderer);
}

void* sdl_texture_create(void* renderer, void* texture_data, int width, int height) {
    // uncomment for funny results
    /*SDL_Surface* surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA8888, texture_data, 4 * width);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return tex;*/

    SDL_Surface* surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_ABGR8888, texture_data, 4 * width);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surface);
    return tex;
}

void sdl_texture_size(void* texture, int* width, int* height) {
    float w, h;
    SDL_GetTextureSize(texture, &w, &h);
    *width  = w;
    *height = h;
}

#endif