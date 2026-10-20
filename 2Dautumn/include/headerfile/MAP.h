#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "headerfile/GAME.h"
#include "headerfile/ENTITY.h"
#include "headerfile/UIELEMENTS.h"

#define TILE_SIZE 16
#define NUMBER_TILE_ROW 7
#define MAP_WIDTH 128
#define MAP_HEIGHT 80
#define REVEAL_RADIUS 9
#define MAX_SIGNS 8

// spring.png tile indices
typedef enum {
	BORDER_TILE_START = 0,
	BORDER_TILE_END = 2,
	GRASS_TILE_START = 3,
	GRASS_TILE_END = 5,
	ROCK_TILE = 6,
} TILES;

// terrain.png tile indices
typedef enum {
	TERRAIN_WATER = 0,
	TERRAIN_PATH = 2,
	TERRAIN_BRIDGE = 3,
	TERRAIN_SOIL = 4,
	TERRAIN_FLOWERS = 5,
} TERRAIN_TILES;

typedef enum {
	GROUND_GRASS,
	GROUND_FOREST,
	GROUND_ROCK,
	GROUND_PATH,
	GROUND_WATER,
	GROUND_BRIDGE,
	GROUND_SOIL,
	GROUND_FLOWERS,
} GROUND;

typedef enum {
	REGION_MEADOW,
	REGION_FOREST,
	REGION_HILLS,
	REGION_POND,
	REGION_ISLAND,
	REGION_GLADE,
	REGION_COUNT,
} REGION;

typedef enum {
	NONE_OBJECT,
	TREE_OBJECT,
	ROCK_OBJECT,
	BOULDER_OBJECT,
	GRANITE_OBJECT,
	LEAFPILE_OBJECT,
	MOSS_OBJECT,
	CLIFF_OBJECT,
	SIGN_OBJECT,
	SAPLING_OBJECT,
	OBJECT_TYPE_COUNT,
} OBJECT_TYPE;

typedef struct {
	OBJECT_TYPE TYPE;
	STATE LIFE;
	t_Entity entity;
	Uint32 respawnAt; // 0 when nothing has to grow back here
	OBJECT_TYPE respawnType;
	int variant;      // sprite variant, or the text index of a sign
} t_Object;

typedef struct {
	TILES tile;
	SDL_Texture* texture;
	int tilemap[MAP_HEIGHT][MAP_WIDTH];
	GROUND ground[MAP_HEIGHT][MAP_WIDTH];
	REGION region[MAP_HEIGHT][MAP_WIDTH];
	bool explored[MAP_HEIGHT][MAP_WIDTH];
	t_Object objectMap[MAP_HEIGHT][MAP_WIDTH];
	SDL_Texture* OBJtex;     // rock.png: rocks, trees, the heart seed
	SDL_Texture* extraTex;   // objects.png: boulders, granite, leaf piles...
	SDL_Texture* terrainTex; // terrain.png: water, path, bridge, soil, flowers

	SDL_Texture* minimapTex;
	Uint32 minimapPixels[MAP_HEIGHT * MAP_WIDTH];
	bool minimapDirty;

	int startX, startY; // camp, where the player wakes up
	int soilX, soilY;   // where the heart seed wants to be planted
	int bouldersX, bouldersY;
	int graniteX, graniteY;
	int islandX, islandY;
	bool planted;
}t_Map;

typedef struct {
	int maxHp;
	bool solid;
	int pickTier;        // 0 = anything breaks it, 1 = wood pick, 2 = stone pick, -1 = unbreakable
	ITEM_TEX drop;
	int dropMin, dropMax;
	Uint32 respawnTime;  // 0 = never grows back
	const char* needText;
}t_ObjectInfo;

const t_ObjectInfo* GetObjectInfo(OBJECT_TYPE type);
const char* GetRegionName(REGION region);
const char* GetSignText(int index);

void generateTerrain(t_Map* map);
void generateOBJ(t_Map* map);

void InitTerrain(t_Map* map, const char* filepath, const char* filepathobj, const char* filepathextra, const char* filepathterrain);
void ScreenToWorld(int screenX, int screenY, t_Camera camera, int* worldX, int* worldY);

bool IsSolidAt(const t_Map* map, float x, float y);
bool MoveWithCollision(const t_Map* map, t_Entity* e, float dx, float dy);
REGION GetRegionAt(const t_Map* map, float x, float y);
t_Object* FindObjectNear(t_Map* map, float x, float y, float radius);
bool PlaceBridge(t_Map* map, int tileX, int tileY);
void PlantSeed(t_Map* map);

void RevealAround(t_Map* map, float x, float y);
void UpdateOBJState(t_Map* map, const t_Player* player);

void renderTilemap(t_Map* map, t_Camera camera);
void renderObject(t_Map* map, const t_Object* obj, t_Camera camera);
void renderMinimap(t_Map* map, const t_Player* player, SDL_Rect dest, bool showLabels);
