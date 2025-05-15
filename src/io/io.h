#ifndef BTCB_IO_H
#define BTCB_IO_H

#include <stdint.h>
#include <stddef.h>

#include "assets/assets.h"
#include "audio/audio.h"

#define MOUSE_LEFT   1
#define MOUSE_MIDDLE 2
#define MOUSE_RIGHT  3

#define TICKS_PER_SEC 1000.

typedef void(*GfxCmdCustom)(void* param, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color);

struct GfxCommand {
    GfxCmdCustom callback;
    void* param;
    bool eternal, important;
};

void gfxcmd_process(void* resource, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color);
struct GfxCommand* gfxcmd_texture(const char* path);
struct GfxCommand* gfxcmd_custom(GfxCmdCustom func, void* param);
struct GfxCommand* gfxcmd_eternal(struct GfxCommand* cmd);
struct GfxCommand* gfxcmd_important(struct GfxCommand* cmd);

struct Texture* graphics_load_texture(unsigned char* buf, size_t len);
void graphics_init(const char* window_name, int width, int height);
void graphics_set_resolution(float width, float height);
void graphics_start_frame();
void graphics_end_frame();
void graphics_get_size(int* width, int* height);
void graphics_select_texture(struct Texture* texture);
void graphics_draw(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, uint32_t color);
void graphics_deinit();
void graphics_flush(bool redraw);
void graphics_register_shader(const char* name, const char* shader);
void graphics_push_shader(const char* name);
void graphics_pop_shader();
void graphics_pop_all_shaders();
void graphics_shader_set_float(const char* name, float value);
void graphics_shader_set_int(const char* name, int value);

void        controller_init();
int         controller_count();
void        controller_select(int index);
const char* controller_name(int index);
const char* controller_current_name();
bool        controller_key_down(int keycode);
bool        controller_button_down(int button);
bool        controller_mouse_down(int button);
void        controller_mouse_pos(int* x, int* y);
float       controller_get_axis(int index);
int         controller_num_axes();
int         controller_num_buttons();
void        controller_deinit();

uint64_t ticks();
void sync(uint64_t start, int ms);
void sleep_ms(uint64_t ns);

bool requested_quit();
void io_deinit();

void audio_backend_open();
void audio_backend_queue(short* samples, int num_samples);
void audio_backend_close();

#endif

