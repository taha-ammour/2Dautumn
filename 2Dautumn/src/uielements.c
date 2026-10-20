#include "headerfile/UIELEMENTS.h"

TTF_Font* uiFont = NULL;
TTF_Font* uiFontSmall = NULL;
TTF_Font* uiFontBig = NULL;

void InitUIFonts(const char* fontpath)
{
	uiFont = TTF_OpenFont(fontpath, 24);
	uiFontSmall = TTF_OpenFont(fontpath, 18);
	uiFontBig = TTF_OpenFont(fontpath, 56);
	if (uiFont == NULL || uiFontSmall == NULL || uiFontBig == NULL)
	{
		fprintf(stderr, "Font not loading: %s, Error: %s\n", fontpath, TTF_GetError());
	}
}

static void drawSurface(SDL_Surface* surface, int x, int y)
{
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect rect = { x, y, surface->w, surface->h };
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
}

// x and y are in world-scaled (logical) units, the glyphs are drawn at screen resolution
void renderText(TTF_Font* font, const char* text, int x, int y, SDL_Color color, TEXT_ALIGN align)
{
	SDL_Color shadow = { 30, 14, 16, 255 };
	float scaleX, scaleY;

	if (font == NULL || text == NULL || text[0] == '\0')
		return;
	SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text, color);
	SDL_Surface* shadowSurface = TTF_RenderUTF8_Solid(font, text, shadow);
	if (surface == NULL || shadowSurface == NULL)
	{
		SDL_FreeSurface(surface);
		SDL_FreeSurface(shadowSurface);
		return;
	}
	SDL_RenderGetScale(renderer, &scaleX, &scaleY);
	int px = (int)(x * scaleX);
	int py = (int)(y * scaleY);
	if (align == ALIGN_CENTER)
		px -= surface->w / 2;
	else if (align == ALIGN_RIGHT)
		px -= surface->w;

	SDL_RenderSetScale(renderer, 1.0f, 1.0f);
	drawSurface(shadowSurface, px + 2, py + 2);
	drawSurface(surface, px, py);
	SDL_RenderSetScale(renderer, scaleX, scaleY);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(shadowSurface);
}

int TextWidth(TTF_Font* font, const char* text)
{
	int w = 0, h = 0;
	float scaleX, scaleY;
	if (font == NULL || TTF_SizeUTF8(font, text, &w, &h) != 0)
		return 0;
	SDL_RenderGetScale(renderer, &scaleX, &scaleY);
	return (int)(w / scaleX);
}

void renderPanel(SDL_Rect rect, Uint8 alpha)
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 30, 14, 16, alpha);
	SDL_RenderFillRect(renderer, &rect);
	SDL_SetRenderDrawColor(renderer, 150, 100, 45, alpha);
	SDL_RenderDrawRect(renderer, &rect);
}


void InitInventory(const char* filename, const char* filenameitemtex, const char* fontpath, t_inventory* inventory)
{
	ClearInventory(inventory);

	SDL_Color white = { 255,255,255,255 };
	inventory->textColor = white;
	inventory->font = TTF_OpenFont(fontpath, 8);
	if (inventory->font == NULL)
	{
		fprintf(stderr, "Font not loading: %s, Error: %s\n", fontpath, TTF_GetError());
	}
	inventory->tex = loadTexture(filename);
	inventory->itemTexture = loadTexture(filenameitemtex);
}

void ClearInventory(t_inventory* inventory)
{
	for (int i = 0; i < INV_SLOTS; i++) {
		inventory->slots[i].selected = i == 0;
		inventory->slots[i].item.itemCount = 0;
		inventory->slots[i].item.TYPE = NONE;
		inventory->slots[i].item.TEX = NONE_TEX;
	}
}

void InitHealth(const char* filename, t_Entity* numHearts, t_Uihealth** health)
{
	int maxHealth = (int)numHearts->hp;
	int numHeartsNeeded = (maxHealth + 25 - 1) / 25;
	SDL_Texture* heartTexture = loadTexture(filename);
	*health = (t_Uihealth*)malloc(sizeof(t_Uihealth) * numHeartsNeeded);
	if (*health == NULL) {
		fprintf(stderr, "Memory allocation failed for healths.\n");
		exit(1); 
	}
	for (int i = 0; i < numHeartsNeeded; i++) {
		if (maxHealth >= 25) {
			(*health)[i].hpshow = 25;
			maxHealth -= 25;
		}
		else {
			(*health)[i].hpshow = (int)numHearts->hp;
			maxHealth = 0;
		}
		(*health)[i].tex = heartTexture;
		(*health)[i].state = FULL;
		(*health)[i].number = numHeartsNeeded;
	}

}

bool addItemToInventory(t_inventory* inventory, t_item newItem)
{
	for (int i = 0; i < INV_SLOTS; i++) {
		if (inventory->slots[i].item.TEX == newItem.TEX) {
			inventory->slots[i].item.itemCount += newItem.itemCount;
			
			return true; 
		}
	}

	for (int i = 0; i < INV_SLOTS; i++) {
		if (inventory->slots[i].item.TEX == NONE_TEX) {
			inventory->slots[i].item = newItem;
			return true; 
		}
	}
	return false;
}

int CountItem(const t_inventory* inventory, ITEM_TEX itemTex)
{
	int count = 0;
	for (int i = 0; i < INV_SLOTS; i++) {
		if (inventory->slots[i].item.TEX == itemTex) {
			count += inventory->slots[i].item.itemCount;
		}
	}
	return count;
}

bool RemoveItem(t_inventory* inventory, ITEM_TEX itemTex, int count)
{
	if (CountItem(inventory, itemTex) < count)
		return false;
	for (int i = 0; i < INV_SLOTS && count > 0; i++) {
		t_item* item = &inventory->slots[i].item;
		if (item->TEX != itemTex)
			continue;
		int taken = item->itemCount < count ? item->itemCount : count;
		item->itemCount -= taken;
		count -= taken;
		// an empty slot is freed for another item
		if (item->itemCount <= 0) {
			item->itemCount = 0;
			item->TYPE = NONE;
			item->TEX = NONE_TEX;
		}
	}
	return true;
}

ITEM_TEX GetSelectedItem(const t_inventory* inventory)
{
	for (int i = 0; i < INV_SLOTS; i++) {
		if (inventory->slots[i].selected) {
			return inventory->slots[i].item.TEX;
		}
	}
	return NONE_TEX;
}

void renderItemIcon(const t_inventory* inv, ITEM_TEX itemTex, SDL_Rect dest)
{
	if (itemTex == NONE_TEX)
		return;
	SDL_Rect srcrect = { itemTex * 16, 0, 16, 16 };
	SDL_RenderCopy(renderer, inv->itemTexture, &srcrect, &dest);
}

void updateInventorySelection(SDL_Event* event, t_inventory* inventory)
{
	if (event->type == SDL_MOUSEWHEEL && event->wheel.y != 0) {
		int current = 0;
		for (int i = 0; i < INV_SLOTS; i++) {
			if (inventory->slots[i].selected)
				current = i;
			inventory->slots[i].selected = false;
		}
		current = (current + (event->wheel.y > 0 ? INV_SLOTS - 1 : 1)) % INV_SLOTS;
		inventory->slots[current].selected = true;
	}
	if (event->type == SDL_KEYDOWN) {
		SDL_Keycode key = event->key.keysym.sym;
		int slotIndex = -1;
		if (key >= SDLK_1 && key <= SDLK_9)
			slotIndex = key - SDLK_1;
		else if (key == SDLK_0)
			slotIndex = 9;
		if (slotIndex >= 0 && slotIndex < INV_SLOTS) {
			for (int i = 0; i < INV_SLOTS; i++) {
				inventory->slots[i].selected = false;
			}
			inventory->slots[slotIndex].selected = true;
		}
	}

}

void updateHeartStates(t_Uihealth* healths, t_Entity entity)
{
	int currentHealth = (int)entity.hp;

	for (int i = 0; i < healths[0].number; i++) {
		if (currentHealth >= 25) {
			healths[i].hpshow = 25;
			healths[i].state = FULL;
		}
		else if (currentHealth >= 25 / 2) {
			healths[i].hpshow = currentHealth;
			healths[i].state = FULL;
		}
		else if (currentHealth > 0) {
			healths[i].hpshow = currentHealth;
			healths[i].state = MID;
		}
		else {
			healths[i].hpshow = 0;
			healths[i].state = EMPTY;
		}

		currentHealth -= 25;
	}
}

void renderInv(t_inventory* inv, int screenW, int screenH)
{
	int invsize = 16;
	int step = 18;
	int startX = (screenW - INV_SLOTS * step) / 2;
	int y = screenH - invsize - 5;
	char text[8];

	for (int i = 0; i < INV_SLOTS; i++)
	{
		SDL_Rect srcrect = { inv->slots[i].selected * invsize, 0, invsize, invsize };
		SDL_Rect destrect = { startX + i * step, y, invsize, invsize };
		SDL_RenderCopy(renderer, inv->tex, &srcrect, &destrect);
		if (inv->slots[i].item.TYPE != NONE) {
			renderItemIcon(inv, inv->slots[i].item.TEX, destrect);
			if (inv->slots[i].item.itemCount > 1) {
				snprintf(text, sizeof(text), "%d", inv->slots[i].item.itemCount);
				renderText(uiFontSmall, text, destrect.x + invsize - 1, destrect.y + invsize - 8, inv->textColor, ALIGN_RIGHT);
			}
		}
	}
	// name of the item in hand, just above the hotbar
	ITEM_TEX selected = GetSelectedItem(inv);
	if (selected != NONE_TEX)
	{
		renderText(uiFontSmall, GetItemName(selected), screenW / 2, y - 9, inv->textColor, ALIGN_CENTER);
	}
}

void renderhealth(t_Player p, t_Uihealth* health)
{
	int hearthsize = 16;
	int heartSpacing = 5;
	int heartsPerLine = 7; 
	int currentLine = 0;

	for (int i = 0; i < health->number; i++)
	{
		SDL_Rect srcrect = { health[i].state * hearthsize,0, hearthsize, hearthsize };
		
		SDL_Rect destRect = { (int)(10 + (i % heartsPerLine) * (hearthsize + heartSpacing) * 0.5), (int)(10 + currentLine * (hearthsize + heartSpacing) * 0.5), (int)(hearthsize * 0.5), (int)(hearthsize * 0.5)};
		destRect.x *= (int)1.0;
		destRect.y *= (int)1.0;
		destRect.w *= (int)1.0;
		destRect.h *= (int)1.0;
		SDL_RenderCopy(renderer, health[i].tex, &srcrect, &destRect);
		if ((i + 1) % heartsPerLine == 0) {
			currentLine++;
		}
	}
}

