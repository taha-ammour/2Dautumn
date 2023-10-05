#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ENTITY.h"

typedef enum {
    NONE = -1,
    MATERIAL,
    TRAPS,
    ITEMATT,
    MINE,
}ITEM_TYPE;

typedef enum {
    NONE_TEX = -1,
    STONE_TEX,
    WOOD_TEX,
    STICK_TEX,
    WOOD_PICK_TEX,
    WOOD_SWORD_TEX,
    STONE_PICK_TEX,
    STONE_SWORD_TEX,
}ITEM_TEX;

typedef struct {
    SDL_Rect rect;  
    char text[32];   
    SDL_Texture* texture;  
    SDL_bool visible;  
} t_Popup;

typedef struct {
    SDL_Rect rect;
    ITEM_TEX TEX;
    ITEM_TYPE TYPE;
    t_Popup pop;
    int itemCount;
}t_item;

typedef struct {
    t_item item;
    bool selected;
} t_inv_slot;

t_item generateItemOnDestroy(ITEM_TEX itemTex, ITEM_TYPE itemtype);
