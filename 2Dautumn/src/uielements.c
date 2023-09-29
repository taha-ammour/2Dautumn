#include "UIELEMENTS.h"
#include "GAME.h"



void InitHealth(const char* filename, t_Entity* numHearts, t_Uihealth** health)
{
	int maxHealth = numHearts->hp;
	int numHeartsNeeded = (maxHealth + 25 - 1) / 25;
	printf("hello : %d", numHeartsNeeded);
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
			(*health)[i].hpshow = numHearts->hp;
			maxHealth = 0;
		}
		(*health)[i].tex = loadTexture(filename);
		(*health)[i].STATE = FULL;
		(*health)[i].number = numHeartsNeeded;
	}

}

void updateHeartStates(t_Uihealth* healths, t_Entity entity)
{
	int currentHealth = entity.hp;

	for (int i = 0; i < healths[0].number; i++) {
		if (currentHealth >= 25) {
			healths[i].hpshow = 25;
			healths[i].STATE = FULL;
		}
		else if (currentHealth >= 25 / 2) {
			healths[i].hpshow = currentHealth;
			healths[i].STATE = FULL;
		}
		else if (currentHealth > 0) {
			healths[i].hpshow = currentHealth;
			healths[i].STATE = MID;
		}
		else {
			healths[i].hpshow = 0;
			healths[i].STATE = EMPTY;
		}

		currentHealth -= 25;
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
		SDL_Rect srcrect = { health[i].STATE * hearthsize,0, hearthsize, hearthsize };
		
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

