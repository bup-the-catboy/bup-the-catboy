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

#define INPUT(_) static int _
#define KEYMAP(_) = _;
#define MOUSEBTN(_) = _;

#define MUSIC(_1, _2) static char* _1 = (char*)_2;

#define SIMPLE_ANIMATED_TEXTURE(_1, _2, ...) (void)(_1); (void)(_2);
#define SIMPLE_STATIONARY_TEXTURE(_1) SIMPLE_ANIMATED_TEXTURE(1, 1, _1)

#endif
