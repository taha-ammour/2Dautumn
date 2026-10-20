#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/UIELEMENTS.h"

#define SWING_COOLDOWN 320
#define SWING_REACH 12.0f
#define SWING_RADIUS 10.0f
#define SWING_ARC_RANGE 26.0f
#define SWING_ARC_ANGLE 1.2f
#define CREATURE_RESPAWN_TIME 60.0f
#define CREATURE_CALM_DISTANCE 180.0f

typedef struct {
	t_Ennemy enemies[MAX_ENEMIES];
	SDL_Texture* textures[LEAF_KIND_COUNT]; // one recolored sheet per kind
}t_EnemyPool;

void InitEnemyPool(t_EnemyPool* pool, const char* imageFile);
void ClearEnemies(t_EnemyPool* pool);
void PopulateCreatures(t_EnemyPool* pool, const t_Map* map);
void CalmCreatures(t_EnemyPool* pool);

void UpdatePlayerPos(t_Player* p, const t_Map* map, float speed, float deltaTime);
void UpdatePlayerHealth(t_Player* p, float deltaTime);
void DamagePlayer(t_Player* player, const t_Map* map, double dmg, float fromX, float fromY);
void PlayerSwing(t_EnemyPool* pool, t_Player* player, t_Map* map, t_inventory* inv, float dirX, float dirY);

void UpdateEnemies(t_EnemyPool* pool, t_Player* player, const t_Map* map, float windX, float windY, float deltaTime);
