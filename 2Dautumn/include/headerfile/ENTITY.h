#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


extern SDL_Window* window;
extern SDL_Renderer* renderer;

typedef enum {
	ATTACK,
	IDLE,
}ENTITY_STATES;

typedef struct {
	int hp;
	SDL_Rect rect;
	ENTITY_STATES state;
}t_Entity;

typedef struct {
	t_Entity entity;
	double attackdmg;
	int defence;
	SDL_Texture* texture;
}t_Player;

typedef struct {
	t_Entity entity;
	double attackdmg;
	int defence;
	bool drop;
	int num_of_drop;
	SDL_Texture* texture;
}t_Ennemy;

SDL_Texture* loadTexture(const char* filename);
void initPlayer(t_Player* p, const char* imageFile);
void renderPlayer(const t_Player* p);
