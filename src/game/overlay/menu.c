#include "menu.h"

#include <stdlib.h>
#include <string.h>

#include "game/input.h"
#include "game/level.h"
#include "game/overlay/transition.h"
#include "game/savefile.h"
#include "game/entities/functions.h"
#include "io/io.h"
#include "main.h"
#include "io/assets/assets.h"
#include "font/font.h"
#include "math_util.h"

#include <lunarengine.h>

typedef void(*ButtonAction)(int selected_index);

#define DEFAULT_MENU_WIDTH 192
#define MENU_PADDING 4
#define MENU_SPACING 4
#define MENU_CURSOR_HEIGHT 16
#define MENU_ANIM_DELAY 15

enum MenuItemType {
    ItemType_Button,
    ItemType_Slider,
    ItemType_InputBind,
    ItemType_Separator,
    ItemType_Image,
};

enum AnchorType {
    AnchorType_TopLeft,
    AnchorType_BottomRight,
    AnchorType_Center
};

enum MenuAnimationState {
    AnimState_Idle,
    AnimState_Push1,
    AnimState_Push2,
    AnimState_Pop1,
    AnimState_Pop2,
};

struct MenuItem {
    enum MenuItemType type;
    union {
        const char* label;
        const char* imgpath;
    };
    union {
        int input_index;
        ButtonAction button_action;
        struct {
            int srcx, srcy, srcw, srch;
            float dstx, dsty, dstw, dsth;
        } image;
        struct {
            int  min;
            int  max;
            int* val;
        } slider;
    };
};

struct MenuMeta {
    int x, y;
    int width;
    bool follow;
    void(*update)();
};

struct MenuList {
    union {
        struct MenuItem* item;
        struct MenuMeta* meta;
    };
    struct MenuList* next;
    struct MenuList* frst;
};

struct MenuStack {
    struct MenuList* menu;
    struct MenuStack* parent;
};

struct MenuStack* menu_stack = NULL;
struct MenuList* menus = NULL;

int selected_item_index = 0;
int visual_cursor_index = 0;
float visual_cursor_pos = 0;
struct MenuItem* selected_item = NULL;

enum MenuAnimationState anim_state = AnimState_Idle;
int anim_timer = 0;
int anim_arg = 0;

#ifndef NO_VSCODE
#define NO_VSCODE
#endif

#define POS_LEFT
#define POS_TOP
#define POS_CENTER | (1 << 30)
#define POS_RIGHT  | (1 << 29)
#define POS_BOTTOM | (1 << 29)
#define APPEND ({                                           \
    struct MenuList* next = malloc(sizeof(struct MenuList)); \
    item = malloc(sizeof(struct MenuItem));                   \
    curr->next = next;                                         \
    next->item = item;                                          \
    next->next = NULL;                                           \
    next->frst = curr->frst;                                      \
    curr = next;                                                   \
    item;                                                           \
})
#define BUTTON(func)          item->type = ItemType_Button;    item->button_action = func;
#define SLIDER(ptr, min, max) item->type = ItemType_Slider;    item->slider.val    = ptr; item->slider.min = min; item->slider.max = max;
#define INPUTBIND(id)         item->type = ItemType_InputBind; item->input_index   = id;
#define SEPARATOR()           item->type = ItemType_Separator;
#define IMGSIZE(_srcx, _srcy, _srcw, _srch, _dstx, _dsty, _dstw, _dsth) item->type = ItemType_Image;       \
    item->image.srcx = _srcx; item->image.srcy = _srcy; item->image.srcw = _srcw; item->image.srch = _srch; \
    item->image.dstx = _dstx; item->image.dsty = _dsty; item->image.dstw = _dstw; item->image.dsth = _dsth;
#define ITEM(lbl, value) \
    APPEND;               \
    item->label = lbl;     \
    value
#define POSITION(X, Y) curr->frst->meta->x = X; curr->frst->meta->y = Y;
#define UPDATE(func) curr->frst->meta->update = func;
#define MENU_WIDTH(w) curr->frst->meta->width = w;
#define FOLLOW() curr->frst->meta->follow = true;
#define DYNAMIC(func) curr = func(curr);
#define IMAGE(path, srcx, srcy, srcw, srch, dstx, dsty, dstw, dsth) ITEM(path, IMGSIZE(srcx, srcy, srcw, srch, dstx, dsty, dstw, dsth))
#define MENU(name, content)                     \
    curr = &menus[MENU_##name];                  \
    curr->meta = malloc(sizeof(struct MenuMeta)); \
    curr->meta->width = DEFAULT_MENU_WIDTH;        \
    curr->meta->x = 0;                              \
    curr->meta->y = 0;                               \
    curr->meta->follow = false;                       \
    curr->frst = curr;                                 \
    content

static void start_game() {
    load_menu(none);
    char name[32];
    snprintf(name, 32, "levels/level%d.lvl", 101 + savefile->map_id);
    load_level(GET_ASSET(struct Binary, name));
    set(find_entity_with_tag("player"), "curr_node", Int, savefile->map_node);
}

void menubtn_start(int selected_index) {
    start_transition(start_game, 60, LE_Direction_Up, cubic_in_out);
}

void menubtn_select_file(int selected_index) {
    push_menu(file_select);
}

void menubtn_settings(int selected_index) {
    push_menu(settings);
}

void menubtn_back(int selected_index) {
    pop_menu();
}

void menubtn_quit(int selected_index) {
    exit(0);
}

char file_names[NUM_SAVEFILES][sizeof("File 0000")];

struct MenuList* menudyn_file_buttons(struct MenuList* curr) {
    struct MenuItem* item = curr->item;
    for (int i = 0; i < NUM_SAVEFILES; i++) {
        sprintf(file_names[i], "File %d", i + 1);
        ITEM(file_names[i], BUTTON(menubtn_file_select));
    }
    return curr;
}

void update_visual_cursor_index() {
    struct MenuList* curr = menu_stack->menu->next;
    visual_cursor_index = 0;
    for (int i = 0; i < selected_item_index; i++) {
        if (curr->item->type != ItemType_Image) visual_cursor_index++;
        curr = curr->next;
    }
}

void cursor_move_down();
void cursor_move_up() {
    struct MenuList* curr = menu_stack->menu->next;
    selected_item_index--;
    if (selected_item_index < 0) {
        cursor_move_down();
        return;
    }
    else for (int i = 0; i < selected_item_index; i++) curr = curr->next;
    if (curr->item->type == ItemType_Separator || curr->item->type == ItemType_Image) cursor_move_up();
    selected_item = curr->item;
    update_visual_cursor_index();
}

void cursor_move_down() {
    struct MenuList* curr = menu_stack->menu->next;
    selected_item_index++;
    for (int i = 0; i < selected_item_index; i++) {
        if (!curr) break;
        curr = curr->next;
    }
    if (!curr) {
        cursor_move_up();
        return;
    }
    if (curr->item->type == ItemType_Separator || curr->item->type == ItemType_Image) cursor_move_down();
    selected_item = curr->item;
    update_visual_cursor_index();
}

void reset_cursor() {
    selected_item_index = -1;
    cursor_move_down();
    visual_cursor_pos = visual_cursor_index;
    struct MenuList* curr = menu_stack->menu->next;
    for (int i = 0; i < selected_item_index; i++) {
        curr = curr->next;
    }
    selected_item = curr->item;
}

void menu_init() {
    menus = malloc(sizeof(struct MenuList) * MENU_COUNT);
    struct MenuList* curr;
    struct MenuItem* item;
#include "game/data/menus.h"
}

#undef MENU
#define MENU(name, _) #name,
const char* menu_names[] = {
    "none",
#include "game/data/menus.h"
};

void _push_menu(int index) {
    anim_state = AnimState_Push1;
    anim_arg = index;
}

void pop_menu_multi(int count) {
    anim_state = AnimState_Pop1;
    anim_arg = count;
}

void pop_menu() {
    pop_menu_multi(1);
}

void pop_menu_impl() {
    if (!menu_stack) return;
    struct MenuStack* prev = menu_stack->parent;
    free(menu_stack);
    menu_stack = prev;
    if (menu_visible()) reset_cursor();
}

void _load_menu(int index) {
    while (menu_stack) {
        struct MenuStack* parent = menu_stack->parent;
        free(menu_stack);
        menu_stack = parent;
    }
    struct MenuStack* stack = malloc(sizeof(struct MenuStack));
    stack->menu = index == -1 ? NULL : &menus[index];
    stack->parent = menu_stack;
    menu_stack = stack;
    if (index != -1) reset_cursor();
}

float update_animation() {
    float translation = 0;
    float x = anim_timer / (float)MENU_ANIM_DELAY;
    if (anim_state == AnimState_Pop1)  translation = lerp(elastic_in (x),  0,  WIDTH);
    if (anim_state == AnimState_Pop2)  translation = lerp(elastic_out(x), -WIDTH,  0);
    if (anim_state == AnimState_Push1) translation = lerp(elastic_in (x),  0, -WIDTH);
    if (anim_state == AnimState_Push2) translation = lerp(elastic_out(x),  WIDTH,  0);
    if (anim_state != AnimState_Idle)  anim_timer++;
    if (anim_timer == MENU_ANIM_DELAY) {
        anim_timer = 0;
        if (anim_state == AnimState_Pop1) {
            anim_state = AnimState_Pop2;
            for (int i = 0; i < anim_arg; i++) {
                pop_menu_impl();
            }
        }
        else if (anim_state == AnimState_Push1) {
            anim_state = AnimState_Push2;
            _load_menu(anim_arg);
        }
        else anim_state = AnimState_Idle;
    }
    return translation;
}

void get_menu_bounds(int* px, int* py, int* pw, int* ph) {
    int x, y, w, h;
    w = menu_stack->menu->frst->meta->width;
    h = MENU_PADDING;
    struct MenuList* curr = menu_stack->menu->next;
    while (curr) {
        if (curr->item->type != ItemType_Image) h += 8 + MENU_SPACING;
        curr = curr->next;
    }
    h = h - MENU_SPACING + MENU_PADDING;
#define GET_ANCHOR(x) ((x) >> 29) & 0b11
#define GET_OFFSET(x) ((x) & ~((1 << 30) | (1 << 29)))
    int anchorX = GET_ANCHOR(menu_stack->menu->frst->meta->x);
    int anchorY = GET_ANCHOR(menu_stack->menu->frst->meta->y);
    int offsetX = GET_OFFSET(menu_stack->menu->frst->meta->x);
    int offsetY = GET_OFFSET(menu_stack->menu->frst->meta->y);
    if (anchorX == AnchorType_TopLeft)     x =                      offsetX;
    if (anchorY == AnchorType_TopLeft)     y =                      offsetY;
    if (anchorX == AnchorType_Center)      x = WIDTH  / 2 - w / 2 + offsetX;
    if (anchorY == AnchorType_Center)      y = HEIGHT / 2 - h / 2 + offsetY;
    if (anchorX == AnchorType_BottomRight) x = WIDTH      - w     - offsetX;
    if (anchorY == AnchorType_BottomRight) y = HEIGHT     - h     - offsetY;
    if (menu_stack->menu->frst->meta->follow) {
        y = 0;
        h = HEIGHT;
    }
    if (px) *px = x;
    if (py) *py = y;
    if (pw) *pw = w;
    if (ph) *ph = h;
}

bool render_menu(LE_DrawList* drawlist) {
    float offsetX = update_animation();
    if (!menu_visible()) return false;
    struct MenuList* curr = menu_stack->menu->next;
    int x, y, w, h;
    get_menu_bounds(&x, &y, &w, &h);
    x += offsetX;
    LE_DrawSetColor(drawlist, 0x0000007F);
    LE_DrawListAppend(drawlist, gfxcmd_texture(NULL), x, y, w, h, 0, 0, 0, 0);
    int text_pos = y + MENU_PADDING;
    visual_cursor_pos += (visual_cursor_index - visual_cursor_pos) / 5;
    float cursor_pos = y + MENU_PADDING + visual_cursor_pos * (8 + MENU_SPACING) + 4 - MENU_CURSOR_HEIGHT / 2.f;
    LE_DrawSetColor(drawlist, 0xFFFFFF7F); //                             ^ fontHeight/2
    LE_DrawListAppend(drawlist, gfxcmd_texture(NULL), x, cursor_pos, w, MENU_CURSOR_HEIGHT, 0, 0, 0, 0);
    LE_DrawSetColor(drawlist, 0xFFFFFFFF);
    while (curr) {
        struct MenuItem* item = curr->item;
        switch (item->type) {
            case ItemType_Image:
                LE_DrawListAppend(drawlist, gfxcmd_texture(item->imgpath),
                    item->image.dstx + offsetX, item->image.dsty, item->image.dstw, item->image.dsth,
                    item->image.srcx,           item->image.srcy, item->image.srcw, item->image.srch
                );
                break;
            case ItemType_Button:
                render_text(drawlist, x + MENU_PADDING, text_pos, "%s", curr->item->label);
                text_pos += 8 + MENU_SPACING;
                break;
            case ItemType_Separator: {
                int len = strlen(curr->item->label);
                render_text(drawlist, x + (w - len * 8) / 2.f, text_pos, "%s", curr->item->label);
                text_pos += 8 + MENU_SPACING;
                break;
            } break;
            default:
                break;
        }
        curr = curr->next;
    }
    if (anim_state == AnimState_Idle) {
        if (is_button_pressed(BUTTON_MOVE_UP))   cursor_move_up();
        if (is_button_pressed(BUTTON_MOVE_DOWN)) cursor_move_down();
        if (is_button_pressed(BUTTON_JUMP) && selected_item->type == ItemType_Button) selected_item->button_action(selected_item_index);
    }
    return true;
}

bool menu_visible() {
    if (!menu_stack) return false;
    if (!menu_stack->menu) return false;
    return true;
}