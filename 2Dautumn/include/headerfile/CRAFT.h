#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "headerfile/ITEM.h"
#include "headerfile/UIELEMENTS.h"

#define RECIPE_COUNT 7

typedef struct {
	ITEM_TEX result;
	ITEM_TYPE type;
	int amount;
	ITEM_TEX cost1;
	int count1;
	ITEM_TEX cost2;
	int count2;
}t_Recipe;

typedef struct {
	bool open;
	int hovered;
	SDL_Rect rect;
}t_CraftMenu;

void InitCraftMenu(t_CraftMenu* menu, int screenW, int screenH);
bool CanCraft(const t_inventory* inv, const t_Recipe* recipe);
bool Craft(t_inventory* inv, const t_Recipe* recipe);
bool HandleCraftEvent(SDL_Event* e, t_CraftMenu* menu, t_inventory* inv, float zoom);
void renderCraftMenu(const t_CraftMenu* menu, const t_inventory* inv);
