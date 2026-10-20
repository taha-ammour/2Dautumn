#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/UIELEMENTS.h"

#define DAY_LENGTH 120.0f
#define SEASON_DAYS 10
#define SNOW_FROM_DAY 8
#define WIND_MAX 30.0f
#define PLANT_RANGE 22.0f

// the steps towards planting the heart seed, the first unmet one is shown
typedef enum {
	QUEST_PICK,
	QUEST_HILLS,
	QUEST_STONE_PICK,
	QUEST_BRIDGE,
	QUEST_MOSS,
	QUEST_COMPOST,
	QUEST_GLADE,
	QUEST_PLANT,
	QUEST_DONE,
}QUEST_STEP;

typedef struct {
	float time; // seconds since the season started
	int day;
	bool visited[REGION_COUNT];
	REGION currentRegion;
	bool winter;
	bool finished;
	int finishDay;
	QUEST_STEP quest;

	float windX, windY;
	float windTargetX, windTargetY;
	float windTimer;

	char banner[48];
	char bannerSub[96];
	float bannerTimer;
}t_World;

void ResetWorld(t_World* world, t_Map* map);
void ShowBanner(t_World* world, const char* title, const char* subtitle);

void UpdateWind(t_World* world, float deltaTime);
void UpdateWorld(t_World* world, t_Map* map, t_Player* player, t_inventory* inv, float deltaTime);
bool TryPlantSeed(t_World* world, t_Map* map, t_Player* player, t_inventory* inv);
float GetSnowAmount(const t_World* world);

void renderSeasonTint(const t_World* world, t_Camera camera);
bool renderSignBubble(t_Map* map, const t_Player* player, t_Camera camera);
void renderWorldHud(const t_World* world, t_Map* map, const t_Player* player, t_Camera camera, bool hideBanner);
