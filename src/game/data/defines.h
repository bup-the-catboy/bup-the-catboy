#ifndef BTCB_DATA_DEFINES_H
#define BTCB_DATA_DEFINES_H

#define NO_VSCODE

#include <lunarengine.h>

#include "game/entities/functions.h"
#include "game/tiles/functions.h"
#include "game/overlay/menu_functions.h"
#include "main.h"

// this file is completely unnecessary
// but it gives syntax errors for missing data entries
// like a tile palette for the PALETTE command in tilesets.h

// its complicated but its also funny lmao :3

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#define GAME_ELEMENT(id, _1, _2) static void id##__##_1() { int _1; _2 } static int id##_##_1;

#define TILE(_1, _2) int _1; _2
#define UPDATE(_1) (void)(_1);
#define COLLISION(_1) (void)(_1);
#define TEXTURE(_1) (void)(_1);
#define FLAGS(_1) (void)(_1);
#define PRIORITY(_1) (void)(_1);
#define SIZE(_1, _2) (void)(_1); (void)(_2);
#define TILES_IN_ROW(_1) (void)(_1);
#define DEFAULT_PROPERTY(_1, _2, _3) (void)(_1); (void)(LE_EntityProperty){ .as##_2 = _3 };
#define PALETTE(_1) (void)(TILE_PALETTE_##_1);
#define THEME_TILESET(_1, _2) (void)(TILESET_TYPE_##_1); (void)(TILESET_##_2);
#define SOLID()
#define LVLEDIT_HIDE()
#define LVLEDIT_TEXTURE(_1) (void)(_1);
#define LVLEDIT_PROPERTIES(...)
#define LVLEDIT_CROP(x, y, w, h) (void)(x); (void)(y); (void)(w); (void)(h);

#define INT   0
#define BOOL  1
#define FLOAT 2

#define ENTITY(      _1, _2) GAME_ELEMENT(ENTITY_BUILDER, _1, _2)
#define TILESET(     _1, _2) GAME_ELEMENT(TILESET,        _1, _2)
#define THEME(       _1, _2) GAME_ELEMENT(THEME,          _1, _2)
#define TILE_PALETTE(_1, _2) GAME_ELEMENT(TILE_PALETTE,   _1, _2)
#define TILESET_TYPE(_     ) GAME_ELEMENT(TILESET_TYPE,   _ ,   )

#define ENUM(id)

#define INPUT(_)      static int _                        = 0;
#define KEYMAP(_)     static int CONCAT(__K, __COUNTER__) = _;
#define MOUSEBTN(_)   static int CONCAT(__M, __COUNTER__) = _;
#define CONTROLLER(_) static int CONCAT(__C, __COUNTER__) = _;
#define JOYSTICK(_)   static int CONCAT(__J, __COUNTER__) = _;

#define A       1
#define B       0
#define X       3
#define Y       2
#define L       4
#define R       5
#define ZL      6
#define ZR      7
#define PLUS    9
#define MINUS   8
#define D_UP    11
#define D_LEFT  13
#define D_DOWN  12
#define D_RIGHT 14

#define LEFT  *4+0
#define RIGHT *4+1
#define UP    *4+2
#define DOWN  *4+3

#define MUSIC(_1, _2) static char* _1 = (char*)_2;

#define SIMPLE_ANIMATED_TEXTURE(_1, _2, ...) (void)(_1); (void)(_2);
#define SIMPLE_STATIONARY_TEXTURE(_1) SIMPLE_ANIMATED_TEXTURE(1, 1, _1)

#define BUTTON(func)          (void)(func);
#define SLIDER(ptr, min, max) (void)(ptr); (void)(min); (void)(max);
#define INPUTBIND(id)         (void)(id);
#define SEPARATOR()
#define IMGSIZE(srcx, srcy, srcw, srch, dstx, dsty, dstw, dsth) \
    (void)(srcx); (void)(srcy); (void)(srcw); (void)(srch);      \
    (void)(dstx); (void)(dsty); (void)(dstw); (void)(dsth);

#define POS_LEFT
#define POS_TOP
#define POS_CENTER
#define POS_RIGHT
#define POS_BOTTOM

#define MENU(name, content) GAME_ELEMENT(MENU, name, content)
#define ITEM(label, val) (void)(label); val
#define POSITION(x, y) (void)(x); (void)(y);
#define MENU_WIDTH(w) (void)(w);
#define DYNAMIC(func) (void)(func);
#define FOLLOW()
#define IMAGE(path, srcx, srcy, srcw, srch, dstx, dsty, dstw, dsth) ITEM(path, IMGSIZE(srcx, srcy, srcw, srch, dstx, dsty, dstw, dsth))

#define POWERUP(name, extends) GAME_ELEMENT(POWERUP, name, (void)(powerup_##name##_update); (void)(POWERUP_##extends);)

static int POWERUP__ = -1;

#endif