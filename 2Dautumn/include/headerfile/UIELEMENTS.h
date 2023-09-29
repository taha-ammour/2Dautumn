#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ENTITY.h"

typedef enum  {
	FULL,
	MID,
	EMPTY
}HEARTSTATE;

typedef struct {
	SDL_Rect rect;
	SDL_Texture* tex;
	HEARTSTATE STATE;
	int number;
	int hpshow;
}t_Uihealth;

void InitHealth(const char* filename, t_Entity* numHearts,t_Uihealth** health);
void updateHeartStates(t_Uihealth* healths, t_Entity entity);
void renderhealth(t_Player p, t_Uihealth* health);