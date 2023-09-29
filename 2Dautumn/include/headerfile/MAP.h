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

typedef struct {
	TILES tile;
	SDL_Texture* texture;
	int tilemap[MAP_HEIGHT][MAP_WIDTH];
}t_Map;



void generateTerrain(t_Map* map);
void InitTerrain(t_Map* map, const char* filepath);
void renderTilemap(t_Map* map, t_Camera camera);
