#include "headerfile/MAP.h"
#include "headerfile/GAME.h"
#include <time.h>


void generateTerrain(t_Map* map)
{
    int x, y;

    srand((unsigned)time(NULL));

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (x == 0 )
            {
                map->tilemap[y][x] = (BORDER_TILE_END ) + 1 * NUMBER_TILE_ROW;
            }
            else if (x == MAP_WIDTH - 1)
            {
                map->tilemap[y][x] = (BORDER_TILE_START) + 1 * NUMBER_TILE_ROW ;
            }
            else if (y == 0)
            {
                map->tilemap[y][x] = (BORDER_TILE_END - 1) + 2 * NUMBER_TILE_ROW;
            }
            else if ( y == MAP_HEIGHT - 1) {
                map->tilemap[y][x] = (BORDER_TILE_END - 1);
            }
            else {
                // For non-border tiles, assign other tile types based on conditions
                int randomNumber = rand() % 100; // Random number between 0 and 99
                int randtilenum = rand() % 3 ;

                if (randomNumber < 40) {
                    map->tilemap[y][x] = GRASS_TILE_END + ( NUMBER_TILE_ROW * randtilenum); // Assign 0 for grass
                }
                else if (randomNumber < 90) {
                    map->tilemap[y][x] = GRASS_TILE_START + (NUMBER_TILE_ROW * randtilenum); // Assign 0 for grass

                }
                else {
                    map->tilemap[y][x] = ROCK_TILE + (NUMBER_TILE_ROW * randtilenum); // Assign 2 for rock
                }
            }
        }
    }
}


void InitTerrain(t_Map* map, const char* filepath) {
	map->texture = loadTexture(filepath);
	generateTerrain(map);
}

void renderTilemap(t_Map* map, t_Camera camera)
{
	int x, y;

	SDL_Rect srcRect, destrect, position;
	for (y = 0; y < MAP_HEIGHT; y++)
	{
		for (x = 0; x < MAP_WIDTH; x++) {
			int tileIndex = map->tilemap[y][x];
			srcRect.x = (tileIndex % NUMBER_TILE_ROW) * TILE_SIZE;
			srcRect.y = (tileIndex / NUMBER_TILE_ROW) * TILE_SIZE;
			srcRect.w = TILE_SIZE;
			srcRect.h = TILE_SIZE;

            position.x = x * TILE_SIZE;
            position.y = y * TILE_SIZE;

			destrect.x = position.x - camera.viewport.x;
			destrect.y = position.y - camera.viewport.y;

			destrect.w = TILE_SIZE;
			destrect.h = TILE_SIZE;

			SDL_RenderCopy(renderer, map->texture, &srcRect, &destrect);
		}
	}
}
