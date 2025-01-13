#ifndef BTCB_IO_H
#define BTCB_IO_H

#include <stdint.h>
#include <stddef.h>

#include "assets/assets.h"
#include "audio/audio.h"

#define MOUSE_LEFT   1
#define MOUSE_MIDDLE 2
#define MOUSE_RIGHT  3

#define NS_TO_S .0000000001 // number of seconds in a nanosecond
#define S_TO_NS 1000000000. // number of nanoseconds in a second

#define TICKS_PER_SEC S_TO_NS

struct GfxResource* graphics_load_texture(unsigned char* buf, size_t len);
struct GfxResource* graphics_load_shader(const char* shader);
void graphics_init(const char* window_name, int width, int height);
void graphics_set_resolution(float width, float height);
void graphics_start_frame();
void graphics_end_frame();
void graphics_get_size(int* width, int* height);
void graphics_select_texture(struct GfxResource* texture);
void graphics_draw(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, uint32_t color);
void graphics_deinit();
void graphics_select_shader(struct GfxResource* shader);
void graphics_shader_set_float(const char* name, float value);
void graphics_shader_set_int(const char* name, int value);
struct GfxResource* graphics_dummy_shader();

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
void sleep_ns(uint64_t ns);

bool requested_quit();
void io_deinit();

void audio_backend_open();
void audio_backend_queue(short* samples, int num_samples);
void audio_backend_close();

#endif

