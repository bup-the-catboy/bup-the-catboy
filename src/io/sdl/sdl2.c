#ifdef SDL_VERSION_2

#include <SDL2/SDL.h>

#include <stdio.h>
#include <time.h>

#include "io/io.h"
#include "io/sdlgfx.h"
#include "io/audio/audio.h"

struct SDLWindowGfx {
    enum SDL_GFX_API api;
    void* window;
    void* gfx_handle;
};

SDL_Joystick* joystick;

void controller_init() {
    SDL_Init(SDL_INIT_JOYSTICK);
    printf("%d controllers found\n", controller_count());
    if (controller_count() > 0) {
        controller_select(0);
    }
}

int controller_count() {
    return SDL_NumJoysticks();
}

void controller_select(int index) {
    if (joystick) SDL_JoystickClose(joystick);
    joystick = SDL_JoystickOpen(index);
    printf("Controller %d connected: %s\n", index, controller_current_name());
}

const char* controller_name(int index) {
    return SDL_JoystickNameForIndex(index);
}

const char* controller_current_name() {
    return SDL_JoystickName(joystick);
}

bool controller_key_down(int keycode) {
    return SDL_GetKeyboardState(NULL)[keycode];
}

bool controller_button_down(int button) {
    if (!joystick) return false;
    return SDL_JoystickGetButton(joystick, button);
}

bool controller_mouse_down(int button) {
    return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button);
}

void controller_mouse_pos(int* x, int* y) {
    SDL_GetMouseState(x, y);
}

float controller_get_axis(int index) {
    if (!joystick) return 0;
    SDL_PumpEvents();
    return SDL_JoystickGetAxis(joystick, index);
}

int controller_num_axes() {
    if (!joystick) return 0;
    return SDL_JoystickNumAxes(joystick);
}

int controller_num_buttons() {
    if (!joystick) return 0;
    return SDL_JoystickNumButtons(joystick);
}

void controller_deinit() {
    if (joystick) SDL_JoystickClose(joystick);
}

void audio_backend_open() {
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec spec;
    spec.freq = AUDIO_SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples = 1024;
    spec.callback = NULL;
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
}

void audio_backend_queue(short* samples, int num_samples) {
    while (SDL_GetQueuedAudioSize(1) > sizeof(short) * num_samples) {
        SDL_Delay(1);
    }
    SDL_QueueAudio(1, samples, sizeof(short) * num_samples);
    SDL_Delay(1);
}

void audio_backend_close() {
    SDL_CloseAudio();
}

uint64_t ticks() {
    return SDL_GetTicks64();
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
    return SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_QUIT, SDL_QUIT) > 0;
}

void sdl_window_init(const char* title, int width, int height, enum SDL_GFX_API gfx_api, void** window, void** gfx_handle) {
    SDL_Init(SDL_INIT_VIDEO);
    if (getenv("WAYLAND_DISPLAY")) SDL_SetHint("SDL_VIDEODRIVER", "wayland"); // run in native wayland when on wayland
    int flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
    if (gfx_api == SDL_GFX_API_OPENGL) {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        flags |= SDL_WINDOW_OPENGL;
    }
    struct SDLWindowGfx* w = *window = malloc(sizeof(struct SDLWindowGfx));
    w->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    switch (gfx_api) {
        case SDL_GFX_API_SDLRENDERER:
            w->gfx_handle = SDL_CreateRenderer(w->window, -1, SDL_RENDERER_ACCELERATED);
            SDL_SetRenderDrawBlendMode(w->gfx_handle, SDL_BLENDMODE_BLEND);
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
            SDL_GL_DeleteContext(w->gfx_handle);
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
    SDL_RenderFillRectF(renderer, &rect);
}

void sdl_renderer_draw_texture(void* renderer, void* texture, int srcX, int srcY, int srcW, int srcH, float dstX, float dstY, float dstW, float dstH, bool flip_x, bool flip_y) {
    SDL_RendererFlip flip = 0;
    if (flip_x) flip |= SDL_FLIP_HORIZONTAL;
    if (flip_y) flip |= SDL_FLIP_VERTICAL;
    SDL_Rect  src = (SDL_Rect) { .x = srcX, .y = srcY, .w = srcW, .h = srcH };
    SDL_FRect dst = (SDL_FRect){ .x = dstX, .y = dstY, .w = dstW, .h = dstH };
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    SDL_SetTextureColorMod(texture, r, g, b);
    SDL_SetTextureAlphaMod(texture, a);
    SDL_RenderCopyExF(renderer, texture, &src, &dst, 0, NULL, flip);
}

void sdl_renderer_flush(void* renderer) {
    SDL_RenderPresent(renderer);
}

void* sdl_texture_create(void* renderer, void* texture_data, int width, int height) {
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(texture_data, width, height, 32, 4 * width,
#ifdef IS_BIG_ENDIAN
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#else
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#endif
    );
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

void sdl_texture_size(void* texture, int* width, int* height) {
    SDL_QueryTexture(texture, NULL, NULL, width, height);
}

#endif