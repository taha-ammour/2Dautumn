#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
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

#define INV_SLOTS 10

typedef struct {
	SDL_Rect rect;
	SDL_Texture* tex;
	STATE state;
	int number;
	int hpshow;
}t_Uihealth;


typedef struct {
	t_inv_slot slots[INV_SLOTS];
	SDL_Texture* tex;
	SDL_Texture* itemTexture;
	TTF_Font* font;
	SDL_Color textColor;
} t_inventory;

typedef enum {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
}TEXT_ALIGN;

// fonts are sized in screen pixels, text is drawn unscaled so it stays crisp
extern TTF_Font* uiFont;
extern TTF_Font* uiFontSmall;
extern TTF_Font* uiFontBig;

void InitUIFonts(const char* fontpath);
void renderText(TTF_Font* font, const char* text, int x, int y, SDL_Color color, TEXT_ALIGN align);
int TextWidth(TTF_Font* font, const char* text);
void renderPanel(SDL_Rect rect, Uint8 alpha);

void InitInventory(const char* filename, const char* filenameitemtex,const char* fontpath, t_inventory* inventory);
void InitHealth(const char* filename, t_Entity* numHearts,t_Uihealth** health);

void ClearInventory(t_inventory* inventory);
bool addItemToInventory(t_inventory* inventory, t_item newItem);
int CountItem(const t_inventory* inventory, ITEM_TEX itemTex);
bool RemoveItem(t_inventory* inventory, ITEM_TEX itemTex, int count);
ITEM_TEX GetSelectedItem(const t_inventory* inventory);
void renderItemIcon(const t_inventory* inv, ITEM_TEX itemTex, SDL_Rect dest);

void updateInventorySelection(SDL_Event* event, t_inventory* inventory);
void updateHeartStates(t_Uihealth* healths, t_Entity entity);

void renderInv(t_inventory* inv, int screenW, int screenH);
void renderhealth(t_Player p, t_Uihealth* health);