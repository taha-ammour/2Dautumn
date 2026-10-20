#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "headerfile/ENTITY.h"
#include "headerfile/UIELEMENTS.h"

#define MAX_PARTICLES 512
#define MAX_FLOAT_TEXTS 32
#define MAX_LEAVES 140
#define MAX_PICKUPS 64
#define MAX_SWINGS 8

typedef struct {
	float x, y;
	float vx, vy;
	float life, maxLife;
	SDL_Color color;
	int size;
	float gravity;
	bool active;
}t_Particle;

typedef struct {
	float x, y;
	float life;
	char text[32];
	SDL_Color color;
	bool active;
}t_FloatText;

// falling autumn leaves (and later snow), pushed around by the wind
typedef struct {
	float x, y;
	float sway;
	float fall;
	SDL_Color color;
}t_Leaf;

// items that pop out of what you break and fly to you
typedef struct {
	float x, y, z;
	float vx, vy, vz;
	ITEM_TEX tex;
	int count;
	float age;
	bool active;
}t_Pickup;

typedef struct {
	float x, y;
	float angle;
	float life;
	bool active;
}t_Swing;

void ClearParticles(void);
void SpawnBurst(float x, float y, SDL_Color color, int count, float speed);
void SpawnMote(float x, float y, SDL_Color color);
void SpawnFloatText(float x, float y, const char* text, SDL_Color color);
void SpawnPickup(float x, float y, ITEM_TEX tex, int count);
void SpawnSwing(float x, float y, float angle);

void InitLeaves(t_Camera camera);
void UpdateParticles(float windX, float windY, float deltaTime);
void UpdateLeaves(t_Camera camera, float windX, float windY, float snow, float deltaTime);
void UpdatePickups(t_Player* player, t_inventory* inv, float deltaTime);

void renderLeaves(t_Camera camera, float snow);
void renderParticles(t_Camera camera);
void renderSwings(t_Camera camera);
void renderPickups(const t_inventory* inv, t_Camera camera);
void renderFloatTexts(t_Camera camera);
