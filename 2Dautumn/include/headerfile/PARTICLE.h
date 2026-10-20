#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "headerfile/ENTITY.h"

#define MAX_PARTICLES 512
#define MAX_FLOAT_TEXTS 32
#define MAX_LEAVES 160

typedef struct {
	float x, y;
	float vx, vy;
	float life, maxLife;
	SDL_Color color;
	int size;
	bool active;
}t_Particle;

typedef struct {
	float x, y;
	float life;
	char text[24];
	SDL_Color color;
	bool active;
}t_FloatText;

// falling autumn leaves, pushed around by the wind
typedef struct {
	float x, y;
	float sway;
	float fall;
	SDL_Color color;
}t_Leaf;

void ClearParticles(void);
void SpawnBurst(float x, float y, SDL_Color color, int count, float speed);
void SpawnFloatText(float x, float y, const char* text, SDL_Color color);

void InitLeaves(void);
void UpdateParticles(float windX, float windY, float deltaTime);

void renderLeaves(t_Camera camera);
void renderParticles(t_Camera camera);
void renderFloatTexts(t_Camera camera);
