#include "headerfile/UIELEMENTS.h"


void InitInventory(const char* filename, const char* filenameitemtex, const char* fontpath, t_inventory* inventory)
{
	inventory->slots[0].selected = true;
	inventory->slots[0].item.itemCount = 0;
	inventory->slots[0].item.TYPE = NONE;
	inventory->slots[0].item.TEX = NONE_TEX;
	for (int i = 1; i < 7; i++) {
		inventory->slots[i].selected = false;
		inventory->slots[i].item.itemCount = 0;
		inventory->slots[i].item.TYPE = NONE;
		inventory->slots[i].item.TEX = NONE_TEX;
	}
	
	SDL_Color white = { 255,255,255,255 };
	inventory->textColor = white;
	inventory->font = TTF_OpenFont(fontpath, 8);
	if (inventory->font == NULL)
	{
		printf("not working :(");
	}
	inventory->tex = loadTexture(filename);
	inventory->itemTexture = loadTexture(filenameitemtex);
}

void InitHealth(const char* filename, t_Entity* numHearts, t_Uihealth** health)
{
	int maxHealth = (int)numHearts->hp;
	int numHeartsNeeded = (maxHealth + 25 - 1) / 25;
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
		(*health)[i].tex = loadTexture(filename);
		(*health)[i].state = FULL;
		(*health)[i].number = numHeartsNeeded;
	}

}

void addItemToInventory(t_inventory* inventory, t_item newItem)
{
	for (int i = 0; i < 7; i++) {
		if (inventory->slots[i].item.TEX == newItem.TEX) {
			inventory->slots[i].item.itemCount += newItem.itemCount;
			
			return; 
		}
	}

	for (int i = 0; i < 7; i++) {
		if (inventory->slots[i].item.TEX == NONE_TEX) {
			inventory->slots[i].item = newItem;
			return; 
		}
	}
}

void updateInventorySelection(SDL_Event* event, t_inventory* inventory)
{
	if (event->type == SDL_KEYDOWN) {
		if (event->key.keysym.sym >= SDLK_1 && event->key.keysym.sym <= SDLK_7) {
			int slotIndex = event->key.keysym.sym - SDLK_1;
			if (slotIndex >= 0 && slotIndex < 7) {
				for (int i = 0; i < 7; i++) {
					inventory->slots[i].selected = false;
				}
				inventory->slots[slotIndex].selected = true;
			}
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

void renderInv(t_inventory* inv)
{
	int invsize = 16;
	int invspace = 14;
	for (int i = 0; i < 7; i++)
	{
		SDL_Rect srcrect = {inv->slots[i].selected * invsize, 0, invsize,invsize };
		
		SDL_Rect destrect = { (int)((int64_t)WINDOWWIDTH / 9 - 3 + i * ((int64_t)invsize + invspace) * 0.5),(int)(WINDOWHEIGHT/4 + invsize * 1.5 ), (int)(invsize ),(int)(invsize)};
		SDL_RenderCopy(renderer, inv->tex, &srcrect, &destrect);
		if (inv->slots[i].item.TYPE != NONE) {
			SDL_Rect srcrect1 = { inv->slots[i].item.TEX * invsize, 0, invsize, invsize};
			SDL_Rect destrect1 = { (int)((int64_t)WINDOWWIDTH / 9 - 3 + i * ((int64_t)invsize + invspace) * 0.5),(int)(WINDOWHEIGHT / 4 + invsize * 1.5), (int)(invsize),(int)(invsize) };
			SDL_RenderCopy(renderer, inv->itemTexture, &srcrect1, &destrect1);

			char text[5];
			snprintf(text, sizeof(text), "%d", inv->slots[i].item.itemCount);
			SDL_Surface* surface = TTF_RenderText_Solid(inv->font, text, inv->textColor);
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, surface);
			
			SDL_Rect textrect = { destrect1.x + 8,destrect1.y + 8,surface->w, surface->h };
			SDL_FreeSurface(surface);
			SDL_RenderCopy(renderer, textTexture, NULL, &textrect);
			SDL_DestroyTexture(textTexture);
		}
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

