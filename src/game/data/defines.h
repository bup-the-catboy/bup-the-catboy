#ifndef BTCB_DATA_DEFINES_H
#define BTCB_DATA_DEFINES_H

#define NO_VSCODE

#include <lunarengine.h>
#include <SDL2/SDL_keycode.h>

#include "game/entities/functions.h"
#include "game/tiles/functions.h"
#include "game/overlay/menu_functions.h"
#include "game/data.h"
#include "main.h"

// we trick vscode into giving a specific syntax highlight color 

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#define GAME_ELEMENT(_1, _2) static void CONCAT(_, __COUNTER__)() { int _1; _2 }

#define TILE(_1, _2) int _1; _2
#define UPDATE(_1) (void)(_1);
#define COLLISION(_1) (void)(_1);
#define TEXTURE(_1) (void)(_1);
#define FLAGS(_1) (void)(_1);
#define SIZE(_1, _2) (void)(_1); (void)(_2);
#define TILES_IN_ROW(_1) (void)(_1);
#define PALETTE(_1) (void)(TILE_PALETTE_##_1);
#define THEME_TILESET(_1, _2) (void)(TILESET_TYPE_##_1); (void)(TILESET_##_2);
#define SOLID()
#define LVLEDIT_HIDE()
#define LVLEDIT_TEXTURE(_1) (void)(_1);
#define LVLEDIT_PROPERTIES(...)

#define INT   0
#define BOOL  1
#define FLOAT 2

#define ENTITY(      _1, _2) GAME_ELEMENT(_1, _2)
#define TILESET(     _1, _2) GAME_ELEMENT(_1, _2)
#define THEME(       _1, _2) GAME_ELEMENT(_1, _2)
#define TILE_PALETTE(_1, _2) GAME_ELEMENT(_1, _2)
#define TILESET_TYPE(_     ) GAME_ELEMENT(_ ,   )

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

#define MENU(name, content) GAME_ELEMENT(name, content)
#define ITEM(label, val) (void)(label); val
#define POSITION(x, y) (void)(x); (void)(y);
#define MENU_WIDTH(w) (void)(w);
#define DYNAMIC(func) (void)(func);
#define FOLLOW()
#define IMAGE(path, srcx, srcy, srcw, srch, dstx, dsty, dstw, dsth) ITEM(path, IMGSIZE(srcx, srcy, srcw, srch, dstx, dsty, dstw, dsth))

#endif