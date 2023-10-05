#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "headerfile/GAME.h"
#include "headerfile/ENTITY.h"
#include "headerfile/UIELEMENTS.h"

#define TILE_SIZE 16
#define NUMBER_TILE_ROW 7
#define MAP_WIDTH (WINDOWWIDTH / TILE_SIZE)
#define MAP_HEIGHT (WINDOWHEIGHT / TILE_SIZE)

typedef enum {
	BORDER_TILE_START = 0,
	BORDER_TILE_END = 2,
	GRASS_TILE_START = 3,
	GRASS_TILE_END = 5,
	ROCK_TILE = 6,
} TILES;

typedef enum {
	NONE_OBJECT,
	TREE_OBJECT,
	ROCK_OBJECT,
	BASEPOINT_OBJECT,
} OBJECT_TYPE;


typedef struct {
	OBJECT_TYPE TYPE;
	STATE LIFE;
	t_Entity entity;
} t_Object;

typedef struct {
	TILES tile;
	SDL_Texture* texture;
	int tilemap[MAP_HEIGHT][MAP_WIDTH];
	t_Object objectMap[MAP_HEIGHT][MAP_WIDTH];
	SDL_Texture* OBJtex;

}t_Map;

typedef struct {
	int x;
	int y;
	t_Map* map;
} RespawnInfo;


void ResetRock(RespawnInfo* respawnInfo);

Uint32 RespawnRockCallback(Uint32 interval, void* param);

void generateTerrain(t_Map* map);
void generateOBJ(t_Map* map, const char* filepathobj);

void InitTerrain(t_Map* map, const char* filepath, const char* filepathobj);

void InteractPOBJ(SDL_Event* e,t_Map* map, t_Player* player,t_inventory* inv, t_Camera camera);
void UpdateOBJState(t_Map* map, t_inventory* inv);

void renderTilemap(t_Map* map, t_Camera camera);
void renderOBJmap(t_Map* map, t_Camera camera);
