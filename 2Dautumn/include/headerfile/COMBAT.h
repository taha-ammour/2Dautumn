#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/UIELEMENTS.h"

#define MELEE_RANGE 30
#define MELEE_COOLDOWN 300
#define EMBER_SPEED 170.0f
#define EMBER_LIFE 1.2f
#define EMBER_DAMAGE 10.0
#define SEED_ATTACK_RANGE 14.0f

typedef struct {
	t_Ennemy enemies[MAX_ENEMIES];
	SDL_Texture* texture;
}t_EnemyPool;

typedef struct {
	t_Projectile shots[MAX_PROJECTILES];
	SDL_Texture* texture;
}t_ProjectilePool;

void InitEnemyPool(t_EnemyPool* pool, const char* imageFile);
void ClearEnemies(t_EnemyPool* pool);
t_Ennemy* SpawnEnemy(t_EnemyPool* pool, ENEMY_KIND kind, float x, float y);
int CountAliveEnemies(const t_EnemyPool* pool);

void DamagePlayer(t_Player* player, double dmg, float fromX, float fromY);
void DamageEnemy(t_EnemyPool* pool, t_Ennemy* e, double dmg, t_inventory* inv);
bool PlayerMeleeAttack(t_EnemyPool* pool, t_Player* player, t_inventory* inv, int worldX, int worldY);

void UpdateEnemies(t_EnemyPool* pool, t_Player* player, t_Map* map, t_inventory* inv,
	float windX, float windY, float lightRadius, float burnDps, float deltaTime);

void InitProjectiles(t_ProjectilePool* pool, const char* imageFile);
void ClearProjectiles(t_ProjectilePool* pool);
bool ThrowEmber(t_ProjectilePool* pool, t_Player* player, t_inventory* inv, int worldX, int worldY);
void UpdateProjectiles(t_ProjectilePool* pool, t_EnemyPool* enemies, t_Map* map, t_inventory* inv,
	float windX, float windY, float deltaTime);

void renderEnemies(const t_EnemyPool* pool, t_Camera camera);
void renderProjectiles(const t_ProjectilePool* pool, t_Camera camera);
