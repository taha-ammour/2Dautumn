#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "headerfile/ITEM.h"
#include "headerfile/ENTITY.h"
#include "headerfile/GAME.h"


typedef enum {
	FULL,
	MID,
	EMPTY
}STATE;

typedef struct {
	SDL_Rect rect;
	SDL_Texture* tex;
	STATE state;
	int number;
	int hpshow;
}t_Uihealth;


typedef struct {
	t_inv_slot slots[7];
	SDL_Texture* tex;
	SDL_Texture* itemTexture;
	TTF_Font* font;
	SDL_Color textColor;
} t_inventory;

void InitInventory(const char* filename, const char* filenameitemtex,const char* fontpath, t_inventory* inventory);
void InitHealth(const char* filename, t_Entity* numHearts,t_Uihealth** health);

void addItemToInventory(t_inventory* inventory, t_item newItem);

void updateInventorySelection(SDL_Event* event, t_inventory* inventory);
void updateHeartStates(t_Uihealth* healths, t_Entity entity);

void renderInv(t_inventory* inv);
void renderhealth(t_Player p, t_Uihealth* health);