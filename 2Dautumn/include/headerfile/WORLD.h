#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/COMBAT.h"

#define DAY_LENGTH 70.0f
#define FIRST_DAY_LENGTH 90.0f
#define DUSK_LENGTH 6.0f
#define DAWN_LENGTH 5.0f
#define NIGHT_MIN_LENGTH 20.0f
#define SEED_BASE_HP 100.0
#define SEED_HP_PER_LEVEL 20.0
#define SEED_FEED_RANGE 40.0f
#define SEED_MAX_FUEL 60.0f
#define WIND_MAX 35.0f

typedef enum {
	PHASE_DAY,
	PHASE_DUSK,
	PHASE_NIGHT,
	PHASE_DAWN,
}WORLD_PHASE;

typedef struct {
	WORLD_PHASE phase;
	float phaseTimer;
	float phaseLength;
	int day;
	int nightsSurvived;

	float windX, windY;
	float windTargetX, windTargetY;
	float windTimer;

	int toSpawn;
	int enemiesLeft;
	float spawnTimer;
	float daySpawnTimer;

	int seedLevel;
	double seedMaxHp;
	float seedFuel;

	SDL_Texture* lightTex;
	SDL_Texture* lightMap;
	int lightMapW, lightMapH;

	char banner[64];
	float bannerTimer;
}t_World;

void InitWorld(t_World* world, t_Camera* camera);
void ResetWorld(t_World* world, t_Map* map);
void ShowBanner(t_World* world, const char* text);

void UpdateWind(t_World* world, float deltaTime);
void UpdateWorld(t_World* world, t_Map* map, t_EnemyPool* enemies, t_Player* player, float deltaTime);
void UpdatePlayerLife(t_World* world, t_Map* map, t_Player* player, float deltaTime);

float GetSeedLightRadius(const t_World* world);
float GetSeedBurnDps(const t_World* world);
bool FeedSeed(t_World* world, t_Map* map, t_Player* player, t_inventory* inv);

void renderSeedAura(const t_World* world, t_Map* map, t_Camera camera);
void renderLighting(t_World* world, t_Map* map, t_Player* player, t_ProjectilePool* shots, t_Camera camera);
void renderWorldHud(const t_World* world, t_Map* map, const t_Player* player);
