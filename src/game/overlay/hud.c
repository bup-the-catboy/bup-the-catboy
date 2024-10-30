#include "font/font.h"
#include "game/input.h"
#include "game/level.h"
#include "game/savefile.h"
#include "main.h"
#include "math_util.h"

#include <stdarg.h>

#define INIT 1

#define BUMP_HEIGHT 2
#define HIDDEN_OPACITY 100
#define SHOWN_OPACITY 255
#define SPACING 12
#define DISTANCE_FROM_TOP 8
#define HIDDEN_POS -40
#define SHOWN_POS 8
#define FADE_SPEED 10
#define FADE_DELAY 60
#define SHOW_DELAY 300
#define STANDING_DELAY 30
#define NUM_CAT_COINS 3
#define CAT_COIN_DISTANCE 8
#define CAT_COIN_SPACING 20
#define CAT_COIN_POPOUT_SCALE 1.5f
#define CAT_COIN_POPOUT_DELAY 30
#define CAT_COIN_GLINT_FRAMES 8
#define CAT_COIN_GLINT_DELAY 3
#define CAT_COIN_HIDDEN_POS -24
#define CAT_COIN_COLLECTED CHAR_CATCOIN_TLF CHAR_CATCOIN_TRF "\n" CHAR_CATCOIN_BLF CHAR_CATCOIN_BRF
#define CAT_COIN_MISSING   CHAR_CATCOIN_TLO CHAR_CATCOIN_TRO "\n" CHAR_CATCOIN_BLO CHAR_CATCOIN_BRO

struct HUDElement {
    struct {
        int x, y, w, h;
    } box;
    int color;
    int hide_x;
    int init;
    float x, y;
    float dst_x, dst_y;
    int show_timer;
    int bump_timer;
    int value;
    int opacity;
};

int standing_timer = 0;
int cat_coin_collect_timers[NUM_CAT_COINS] = { -2 };
const char* cat_coin_glint[] = {
    "\x00\x00\n\x00\x00",
    "\x8C\x8D\n\x8E\x8F",
    "\x90\x91\n\x92\x93",
    "\x94\x95\n\x96\x97",
    "\x98\x99\n\x9A\x9B",
    "\x9C\x9D\n\x9E\x9F",
    "\xA0\xA1\n\xA2\xA3",
    "\x00\x00\n\x00\x00",
};

struct HUDElement hud_elements[] = {
    { { -100,       -100, 156, 144 }, 0x00FF00,          HIDDEN_POS, INIT },
    { { -100,       -100, 156, 144 }, 0xFFFF00,          HIDDEN_POS, INIT },
    { { WIDTH - 80, -100, 180, 140 }, 0xFFFFFF, CAT_COIN_HIDDEN_POS, INIT }
};

#define HUD_COLOR_MOD "${#%%06x}"
#define HUD_OPAC      "${%%%%%%d}"

bool approach(int* out, int val, int spd) {
    if (*out == val) return false;
    if (*out < val) {
        (*out) += spd;
        if (*out > val) *out = val;
    }
    if (*out > val) {
        (*out) -= spd;
        if (*out < val) *out = val;
    }
    return true;
}

unsigned int interpolate_color(unsigned int from, unsigned int to, float x) {
    float fr = (from >> 16) & 0xFF;
    float fg = (from >>  8) & 0xFF;
    float fb = (from >>  0) & 0xFF;
    float tr = (to   >> 16) & 0xFF;
    float tg = (to   >>  8) & 0xFF;
    float tb = (to   >>  0) & 0xFF;
    float r  = (tr - fr) * x + fr;
    float g  = (tg - fg) * x + fg;
    float b  = (tb - fb) * x + fb;
    return
        ((unsigned int)(unsigned char)r << 16) |
        ((unsigned int)(unsigned char)g <<  8) |
        ((unsigned int)(unsigned char)b <<  0);
}

float suggest_y_pos(struct HUDElement* element) {
    int shown = 0;
    for (int i = 0; i < sizeof(hud_elements) / sizeof(*hud_elements); i++) {
        if (&hud_elements[i] == element) break;
        if (hud_elements[i].dst_x == hud_elements[i].hide_x && fabsf(hud_elements[i].dst_x - hud_elements[i].x) < 1) continue;
        shown++;
    }
    return DISTANCE_FROM_TOP + shown * SPACING;
}

void _show_hud_element(struct HUDElement* element) {
    element->show_timer = 0;
    element->dst_x = SHOWN_POS;
    element->dst_y = element->y = suggest_y_pos(element);
}

void hud_update_element(LE_Entity* player, struct HUDElement* element, int target_value) {
    bool hide = false; // todo
    int target_opacity = hide ? HIDDEN_OPACITY : SHOWN_OPACITY;
    element->dst_y = suggest_y_pos(element);
    if (element->init) {
        element->init = 0;
        element->value = target_value;
        element->x = element->dst_x = SHOWN_POS;
        element->y = element->dst_y;
        element->show_timer = 0;
        element->bump_timer = FADE_DELAY;
        element->opacity = SHOWN_OPACITY;
    }
    element->bump_timer++;
    element->show_timer++;
    approach(&element->opacity, target_opacity, FADE_SPEED);
    if (approach(&element->value, target_value, 1)) {
        _show_hud_element(element);
        element->bump_timer = 0;
    }
    if (element->show_timer == SHOW_DELAY && standing_timer < STANDING_DELAY) element->dst_x = element->hide_x;
    element->x += (element->dst_x - element->x) / 10;
    element->y += (element->dst_y - element->y) / 10;
}

void hud_update(LE_Entity* player) {
    if (fabsf(player->velX) + fabsf(player->velY) < 0.05f) standing_timer++;
    else standing_timer = 0;
    for (int i = 0; i < sizeof(hud_elements) / sizeof(*hud_elements); i++) {
        if (hud_elements[i].show_timer < SHOW_DELAY) continue;
        if (standing_timer == 0) hud_elements[i].dst_x = hud_elements[i].hide_x;
        if (standing_timer == STANDING_DELAY) {
            hud_elements[i].dst_x = SHOWN_POS;
            hud_elements[i].y = hud_elements[i].dst_y = suggest_y_pos(&hud_elements[i]);
        }
    }
    hud_update_element(player, &hud_elements[0], savefile->lives);
    hud_update_element(player, &hud_elements[1], savefile->coins);
    hud_update_element(player, &hud_elements[2], 0);
}

void render_hud_element(LE_DrawList* drawlist, struct HUDElement* element, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char formatted[1024];
    vsnprintf(formatted, 1024, fmt, args);
    va_end(args);
    float y = element->y - max(0, min(-abs(element->bump_timer - BUMP_HEIGHT) + BUMP_HEIGHT, BUMP_HEIGHT));
    unsigned int color = interpolate_color(element->color, 0xFFFFFF, clamp(element->bump_timer / (float)FADE_DELAY, 0, 1));
    render_text(drawlist, element->x, y, formatted, element->opacity * 100 / 255, color);
}

void render_cat_coins(LE_DrawList* drawlist) {
    int render_order[NUM_CAT_COINS];
    for (int i = NUM_CAT_COINS - 1; i >= 0; i--) {
        render_order[i] = i;
        if (cat_coin_collect_timers[0] == -2) cat_coin_collect_timers[i] = -1;
        if (cat_coin_collect_timers[i] != -1) cat_coin_collect_timers[i]++;
        bool collected = cat_coin_collect_timers[i] >= 0;
        if (!collected && (savefile->level_flags[curr_level_id] & (1 << i))) {
            _show_hud_element(&hud_elements[2]);
            cat_coin_collect_timers[i] = 0;
            collected = true;
        }
    }
    for (int i = 0; i < NUM_CAT_COINS; i++) {
        for (int j = i + 1; j < NUM_CAT_COINS; j++) {
            if (cat_coin_collect_timers[i] > cat_coin_collect_timers[j]) {
                int tmp = render_order[i];
                render_order[i] = render_order[j];
                render_order[i] = tmp;
            }
        }
    }
    for (int j = 0; j < NUM_CAT_COINS; j++) {
        int i = render_order[j];
        bool collected = cat_coin_collect_timers[i] >= 0;
        float scale = collected && cat_coin_collect_timers[i] < CAT_COIN_POPOUT_DELAY
            ? sin(cat_coin_collect_timers[i] / (float)CAT_COIN_POPOUT_DELAY * M_PI) * (CAT_COIN_POPOUT_SCALE - 1) + 1
            : 1.0f;
        float x = WIDTH - CAT_COIN_DISTANCE - (NUM_CAT_COINS - i) * CAT_COIN_SPACING - (scale - 1) * 19 / 2;
        float y = hud_elements[2].x - (scale - 1) * 19 / 2;
        render_text(drawlist, x, y, "${^%d}%s", (int)(scale * 100), collected ? CAT_COIN_COLLECTED : CAT_COIN_MISSING);
        render_text(drawlist, x, y, "%s", cat_coin_glint[(int)clamp((cat_coin_collect_timers[i] - CAT_COIN_POPOUT_DELAY) / (float)CAT_COIN_GLINT_DELAY, 0, CAT_COIN_GLINT_FRAMES - 1)]);
    }
}

void render_hud(LE_DrawList* drawlist) {
    render_hud_element(drawlist, &hud_elements[0], HUD_OPAC CHAR_LIVES HUD_COLOR_MOD "*%02d",                                      savefile->lives);
    render_hud_element(drawlist, &hud_elements[1], HUD_OPAC "%c"       HUD_COLOR_MOD "*%02d", CHAR_COINS[(global_timer / 10) % 4], savefile->coins);
    render_cat_coins(drawlist);
}

void show_hud_element(int id) {
    _show_hud_element(&hud_elements[id]);
}
