#include "headerfile/CRAFT.h"
#include "headerfile/SOUND.h"

#define ROW_HEIGHT 18

// stone tools are upgrades: they use up the wooden one
static const t_Recipe recipes[RECIPE_COUNT] = {
	{ STICK_TEX, MATERIAL, 2, WOOD_TEX, 1, NONE_TEX, 0 },
	{ PLANK_TEX, MATERIAL, 2, WOOD_TEX, 1, NONE_TEX, 0 },
	{ WOOD_PICK_TEX, MINE, 1, WOOD_TEX, 2, STICK_TEX, 2 },
	{ WOOD_SWORD_TEX, ITEMATT, 1, WOOD_TEX, 2, STICK_TEX, 1 },
	{ STONE_PICK_TEX, MINE, 1, WOOD_PICK_TEX, 1, STONE_TEX, 3 },
	{ STONE_SWORD_TEX, ITEMATT, 1, WOOD_SWORD_TEX, 1, STONE_TEX, 3 },
	{ COMPOST_TEX, MATERIAL, 1, LEAVES_TEX, 4, MOSS_TEX, 1 },
};

void InitCraftMenu(t_CraftMenu* menu, int screenW, int screenH)
{
	menu->open = false;
	menu->hovered = -1;
	menu->rect.w = 170;
	menu->rect.h = 20 + RECIPE_COUNT * ROW_HEIGHT;
	menu->rect.x = (screenW - menu->rect.w) / 2;
	menu->rect.y = (screenH - menu->rect.h) / 2 - 10;
}

bool CanCraft(const t_inventory* inv, const t_Recipe* recipe)
{
	if (CountItem(inv, recipe->cost1) < recipe->count1)
		return false;
	if (recipe->cost2 != NONE_TEX && CountItem(inv, recipe->cost2) < recipe->count2)
		return false;
	return true;
}

bool Craft(t_inventory* inv, const t_Recipe* recipe)
{
	if (!CanCraft(inv, recipe))
		return false;
	RemoveItem(inv, recipe->cost1, recipe->count1);
	if (recipe->cost2 != NONE_TEX)
		RemoveItem(inv, recipe->cost2, recipe->count2);
	if (!addItemToInventory(inv, makeItem(recipe->result, recipe->type, recipe->amount)))
	{
		// no free slot: give the materials back
		addItemToInventory(inv, makeItem(recipe->cost1, GetItemType(recipe->cost1), recipe->count1));
		if (recipe->cost2 != NONE_TEX)
			addItemToInventory(inv, makeItem(recipe->cost2, GetItemType(recipe->cost2), recipe->count2));
		return false;
	}
	return true;
}

static int rowAt(const t_CraftMenu* menu, int x, int y)
{
	if (x < menu->rect.x || x >= menu->rect.x + menu->rect.w)
		return -1;
	int row = (y - menu->rect.y - 16) / ROW_HEIGHT;
	if (y < menu->rect.y + 16 || row < 0 || row >= RECIPE_COUNT)
		return -1;
	return row;
}

bool HandleCraftEvent(SDL_Event* e, t_CraftMenu* menu, t_inventory* inv, float zoom)
{
	if (!menu->open)
		return false;
	if (e->type == SDL_MOUSEMOTION)
	{
		menu->hovered = rowAt(menu, (int)(e->motion.x / zoom), (int)(e->motion.y / zoom));
		return false;
	}
	if (e->type == SDL_MOUSEBUTTONDOWN)
	{
		SDL_Point p = { (int)(e->button.x / zoom), (int)(e->button.y / zoom) };
		int row = rowAt(menu, p.x, p.y);
		if (row >= 0 && e->button.button == SDL_BUTTON_LEFT && Craft(inv, &recipes[row]))
			PlaySfx(SFX_CRAFT);
		// clicks on the panel never reach the world behind it
		return SDL_PointInRect(&p, &menu->rect);
	}
	return false;
}

static void renderCost(const t_inventory* inv, ITEM_TEX itemTex, int count, int x, int y, bool enough)
{
	char text[8];
	SDL_Color color = enough ? (SDL_Color){ 255, 255, 255, 255 } : (SDL_Color){ 255, 100, 80, 255 };
	SDL_Rect icon = { x, y, 12, 12 };

	renderItemIcon(inv, itemTex, icon);
	snprintf(text, sizeof(text), "%d", count);
	renderText(uiFontSmall, text, x + 12, y + 2, color, ALIGN_LEFT);
}

void renderCraftMenu(const t_CraftMenu* menu, const t_inventory* inv)
{
	SDL_Color title = { 255, 225, 160, 255 };

	if (!menu->open)
		return;
	renderPanel(menu->rect, 225);
	renderText(uiFontSmall, "Crafting  -  click to craft, C to close", menu->rect.x + menu->rect.w / 2, menu->rect.y + 4, title, ALIGN_CENTER);

	for (int i = 0; i < RECIPE_COUNT; i++)
	{
		const t_Recipe* recipe = &recipes[i];
		bool craftable = CanCraft(inv, recipe);
		SDL_Rect row = { menu->rect.x + 3, menu->rect.y + 16 + i * ROW_HEIGHT, menu->rect.w - 6, ROW_HEIGHT - 2 };
		SDL_Rect icon = { row.x + 2, row.y, 16, 16 };
		char name[32];

		if (i == menu->hovered)
		{
			SDL_SetRenderDrawColor(renderer, 120, 70, 30, craftable ? 200 : 90);
			SDL_RenderFillRect(renderer, &row);
		}
		renderItemIcon(inv, recipe->result, icon);
		if (recipe->amount > 1)
			snprintf(name, sizeof(name), "%s x%d", GetItemName(recipe->result), recipe->amount);
		else
			snprintf(name, sizeof(name), "%s", GetItemName(recipe->result));
		renderText(uiFontSmall, name, row.x + 20, row.y + 4, craftable ? title : (SDL_Color){ 150, 130, 110, 255 }, ALIGN_LEFT);

		renderCost(inv, recipe->cost1, recipe->count1, row.x + row.w - 50, row.y + 2, CountItem(inv, recipe->cost1) >= recipe->count1);
		if (recipe->cost2 != NONE_TEX)
			renderCost(inv, recipe->cost2, recipe->count2, row.x + row.w - 24, row.y + 2, CountItem(inv, recipe->cost2) >= recipe->count2);
	}
}
