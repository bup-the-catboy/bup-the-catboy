#ifndef NO_VSCODE

#include <lunarengine.h>
#include <SDL2/SDL_keycode.h>

#include "game/entities/functions.h"
#include "game/tiles/functions.h"

// we trick vscode into giving a specific syntax highlight color 

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#define GAME_ELEMENT(_1, _2) static void CONCAT(_, __COUNTER__)() { int _1; _2 }

#define UPDATE(_1) (void)(_1);
#define COLLISION(_1) (void)(_1);
#define TEXTURE(_1) (void)(_1);
#define FLAGS(_1) (void)(_1);
#define SIZE(_1, _2) (void)(_1); (void)(_2);
#define TILES_IN_ROW(_1) (void)(_1);
#define SOLID()
#define LVLEDIT_HIDE()

#define ENTITY( _1, _2) GAME_ELEMENT(_1, _2)
#define TILESET(_1, _2) GAME_ELEMENT(_1, _2)
#define TILE(   _1, _2) GAME_ELEMENT(_1, _2)

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

#endif
