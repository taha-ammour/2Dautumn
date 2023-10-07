#include "headerfile/MAP.h"
#include "headerfile/GAME.h"
#include <time.h>


void ResetRock(RespawnInfo* respawnInfo)
{
    t_Map* map = respawnInfo->map;
    int x = respawnInfo->x;
    int y = respawnInfo->y;
    if (map->objectMap[y][x].TYPE == NONE_OBJECT)
    {
        map->objectMap[y][x].entity.hp = 10;
        map->objectMap[y][x].LIFE = FULL;
        map->objectMap[y][x].TYPE = TREE_OBJECT;
        
    }
    
}

Uint32 RespawnRockCallback(Uint32 interval, void* param) {
    RespawnInfo* respawnInfo = (RespawnInfo*)param;
    ResetRock(respawnInfo);
    free(respawnInfo); 
    return 0;
}

void generateTerrain(t_Map* map)
{
	srand((unsigned)time(NULL));
    int x, y;


    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            map->tilemap[y][x] = (GRASS_TILE_END - rand()%3) + (NUMBER_TILE_ROW * (rand() % 3));
        }
    }

    int numRockClusters = 5;

    for (int cluster = 0; cluster < numRockClusters; cluster++) {
        int rockClusterWidth = 4;
        int rockClusterHeight = 4;

        
        int minDistanceFromMiddleX = MAP_WIDTH / 4; 
        int minDistanceFromMiddleY = MAP_HEIGHT / 4; 

        int clusterX, clusterY;

        do {
            clusterX = rand() % (MAP_WIDTH - rockClusterWidth - 2) + 1;
            clusterY = rand() % (MAP_HEIGHT - rockClusterHeight - 2) + 1; 
        } while (abs(clusterX - MAP_WIDTH / 2) < minDistanceFromMiddleX || abs(clusterY - MAP_HEIGHT / 2) < minDistanceFromMiddleY);

        for (y = clusterY; y < clusterY + rockClusterHeight; y++) {
            for (x = clusterX; x < clusterX + rockClusterWidth; x++) {
                map->tilemap[y][x] = ROCK_TILE + (NUMBER_TILE_ROW * (rand() % 3));
            }
        }
    }
    int midx= MAP_WIDTH/2;
    int midy = MAP_HEIGHT/2;
    map->tilemap[midy][midx] = GRASS_TILE_END - 1;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (x == 0)
            {
                map->tilemap[y][x] = (BORDER_TILE_END)+1 * NUMBER_TILE_ROW;
            }
            else if (x == MAP_WIDTH - 1)
            {
                map->tilemap[y][x] = (BORDER_TILE_START)+1 * NUMBER_TILE_ROW;
            }
            else if (y == 0)
            {
                map->tilemap[y][x] = (BORDER_TILE_END - 1) + 2 * NUMBER_TILE_ROW;
            }
            else if (y == MAP_HEIGHT - 1) {
                map->tilemap[y][x] = (BORDER_TILE_END - 1);
            }
            map->objectMap[y][x].TYPE = NONE_OBJECT;
        }
    }
}

void generateOBJ(t_Map* map, const char* filepathobj)
{
    int y, x;
    map->OBJtex = loadTexture(filepathobj);

    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (map->objectMap[y][x].TYPE == NONE_OBJECT &&
                (map->tilemap[y][x] == ROCK_TILE || map->tilemap[y][x] == ROCK_TILE + NUMBER_TILE_ROW || map->tilemap[y][x] == ROCK_TILE + NUMBER_TILE_ROW * 2))
            {
                if (rand() % 2 == 0) {
                    map->objectMap[y][x].TYPE = ROCK_OBJECT;
                    map->objectMap[y][x].LIFE = FULL;
                    map->objectMap[y][x].entity.hp = 20;
                    map->objectMap[y][x].entity.rect.x = x * TILE_SIZE;
                    map->objectMap[y][x].entity.rect.y = y * TILE_SIZE;
                    map->objectMap[y][x].entity.rect.w = TILE_SIZE;
                    map->objectMap[y][x].entity.rect.h = TILE_SIZE;
                }
            }
             if (map->objectMap[y][x].TYPE == NONE_OBJECT &&
                map->tilemap[y][x] == GRASS_TILE_START ) {
                 if (rand()% 4 == 0)
                 {
                    map->objectMap[y][x].TYPE = TREE_OBJECT;
                    map->objectMap[y][x].LIFE = FULL;
                    map->objectMap[y][x].entity.hp = 10;
                    map->objectMap[y][x].entity.rect.x = x * TILE_SIZE;
                    map->objectMap[y][x].entity.rect.y = y * TILE_SIZE;
                    map->objectMap[y][x].entity.rect.w = TILE_SIZE;
                    map->objectMap[y][x].entity.rect.h = TILE_SIZE;
                 }
                 
            }

        }
    }
    map->objectMap[MAP_HEIGHT/2][MAP_WIDTH/2].TYPE = BASEPOINT_OBJECT;
    map->objectMap[MAP_HEIGHT / 2][MAP_WIDTH / 2].LIFE = FULL;
    map->objectMap[MAP_HEIGHT / 2][MAP_WIDTH / 2].entity.hp = 50;
    map->objectMap[MAP_HEIGHT / 2][MAP_WIDTH / 2].entity.rect.x = (MAP_WIDTH / 2) * TILE_SIZE;
    map->objectMap[MAP_HEIGHT / 2][MAP_WIDTH / 2].entity.rect.y = (MAP_HEIGHT / 2) * TILE_SIZE;
    map->objectMap[MAP_HEIGHT / 2][MAP_WIDTH / 2].entity.rect.w = TILE_SIZE;
    map->objectMap[MAP_HEIGHT / 2][MAP_WIDTH / 2].entity.rect.h = TILE_SIZE;
}


void InitTerrain(t_Map* map, const char* filepath, const char* filepathobj) {
	map->texture = loadTexture(filepath);
	generateTerrain(map);
    generateOBJ(map, filepathobj);
}


void InteractPOBJ(SDL_Event* e,t_Map* map, t_Player* player, t_inventory* inv, t_Camera camera)
{
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT)
    {
        int breakDistance = 2;

        int mouseX = e->button.x/3 + camera.viewport.x;
        int mouseY = e->button.y/3 + camera.viewport.y;

        int playerTileX = player->entity.rect.x / TILE_SIZE;
        int playerTileY = player->entity.rect.y / TILE_SIZE;

        for (int y = playerTileY - breakDistance; y <= playerTileY + breakDistance; y++) {
            for (int x = playerTileX - breakDistance; x <= playerTileX + breakDistance; x++) {
                if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
                    if (map->objectMap[y][x].TYPE == ROCK_OBJECT) {
                        if (map->objectMap[y][x].entity.hp > 0) {
                            SDL_Rect objectRect = map->objectMap[y][x].entity.rect;
                            SDL_Rect playerRect = { player->entity.rect.x, player->entity.rect.y, player->entity.rect.w, player->entity.rect.h };
                            float distance = CalcDistance(objectRect, playerRect);

                            if (distance <= TILE_SIZE * breakDistance) {
                                if (mouseX >= objectRect.x && mouseX <= objectRect.x + objectRect.w &&
                                    mouseY >= objectRect.y && mouseY <= objectRect.y + objectRect.h) {

                                    map->objectMap[y][x].entity.hp -= player->attackdmg;
                                    
                                }
                            }
                        }
                        else {

                           

                        }
                    }
                    else if (map->objectMap[y][x].TYPE == TREE_OBJECT) {
                        if (map->objectMap[y][x].entity.hp > 0) {
                            SDL_Rect objectRect = map->objectMap[y][x].entity.rect;
                            SDL_Rect playerRect = { player->entity.rect.x, player->entity.rect.y, player->entity.rect.w, player->entity.rect.h };
                            float distance = CalcDistance(objectRect, playerRect);

                            if (distance <= TILE_SIZE * breakDistance) {
                                if (mouseX >= objectRect.x && mouseX <= objectRect.x + objectRect.w &&
                                    mouseY >= objectRect.y && mouseY <= objectRect.y + objectRect.h) {

                                    map->objectMap[y][x].entity.hp -= player->attackdmg;

                                }
                            }
                        }
                        else {

                        }
                    }
                }
            }
        }
    }
    

}

void UpdateOBJState(t_Map* map,t_inventory* inv)
{
    
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            int health_obj = (int)map->objectMap[y][x].entity.hp;
            if (map->objectMap[y][x].TYPE == ROCK_OBJECT)
            {
                if (health_obj > 18) 
                {
                    map->objectMap[y][x].LIFE = FULL;
                }
                else if (health_obj > 13)
                {
                    map->objectMap[y][x].LIFE = MID;
                }
                else if (health_obj > 7)
                {
                    map->objectMap[y][x].LIFE = EMPTY;
                }
                else if(health_obj <= 0)
                {
                    t_item newitem = generateItemOnDestroy(STONE_TEX, MATERIAL);
                    addItemToInventory(inv, newitem);
                    map->objectMap[y][x].TYPE = NONE_OBJECT;

                }
            }
            else if (map->objectMap[y][x].TYPE == TREE_OBJECT) {
                if (health_obj > 8)
                {
                    map->objectMap[y][x].LIFE = FULL;
                }
                else if (health_obj > 5)
                {
                    map->objectMap[y][x].LIFE = MID;
                }
                else if (health_obj > 2)
                {
                    map->objectMap[y][x].LIFE = EMPTY;
                }
                else if (health_obj <= 0)
                {
                    t_item newitem = generateItemOnDestroy(WOOD_TEX, MATERIAL);
                    addItemToInventory(inv, newitem);
                    map->objectMap[y][x].TYPE = NONE_OBJECT;
                    Uint32 respawnTime = SDL_GetTicks() + 30000;
                    RespawnInfo* respawnInfo = (RespawnInfo*)malloc(sizeof(RespawnInfo));
                    if (respawnInfo == NULL) {
                    }
                    respawnInfo->x = x;
                    respawnInfo->y = y;
                    respawnInfo->map = map;

                    SDL_TimerID timerID = SDL_AddTimer(30000, RespawnRockCallback, respawnInfo);

                }
            }
            else if (map->objectMap[y][x].TYPE == BASEPOINT_OBJECT) {
                if (health_obj > 40)
                {
                    map->objectMap[y][x].LIFE = FULL;
                }
                else if (health_obj > 20)
                {
                    map->objectMap[y][x].LIFE = MID;
                }
                else if (health_obj > 10)
                {
                    map->objectMap[y][x].LIFE = EMPTY;
                }
               
            }
        }
    }
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


void renderOBJmap(t_Map* map, t_Camera camera)
{
    int x, y;
    SDL_Rect srcRect, destrect;

    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++) {
            int objectType = map->objectMap[y][x].TYPE;

            if (objectType == ROCK_OBJECT && map->objectMap[y][x].entity.hp > 0) {
                srcRect.x = map->objectMap[y][x].LIFE * TILE_SIZE;
                srcRect.y = 0;
                srcRect.w = TILE_SIZE;
                srcRect.h = TILE_SIZE;

                destrect.x = map->objectMap[y][x].entity.rect.x - camera.viewport.x;
                destrect.y = map->objectMap[y][x].entity.rect.y - camera.viewport.y;
                destrect.w = TILE_SIZE;
                destrect.h = TILE_SIZE;

                SDL_RenderCopy(renderer, map->OBJtex, &srcRect, &destrect);

            }
             if (objectType == TREE_OBJECT && map->objectMap[y][x].entity.hp > 0) {
                srcRect.x = map->objectMap[y][x].LIFE * TILE_SIZE;
                srcRect.y = 1 * TILE_SIZE;
                srcRect.w = TILE_SIZE;
                srcRect.h = TILE_SIZE;

                destrect.x = map->objectMap[y][x].entity.rect.x - camera.viewport.x;
                destrect.y = map->objectMap[y][x].entity.rect.y - camera.viewport.y;
                destrect.w = TILE_SIZE;
                destrect.h = TILE_SIZE;

                SDL_RenderCopy(renderer, map->OBJtex, &srcRect, &destrect);

            }
             if (objectType == BASEPOINT_OBJECT && map->objectMap[y][x].entity.hp > 0) {
                srcRect.x = map->objectMap[y][x].LIFE * TILE_SIZE;
                srcRect.y = 2* TILE_SIZE;
                srcRect.w = TILE_SIZE;
                srcRect.h = TILE_SIZE;

                destrect.x = map->objectMap[y][x].entity.rect.x - camera.viewport.x;
                destrect.y = map->objectMap[y][x].entity.rect.y - camera.viewport.y;
                destrect.w = TILE_SIZE;
                destrect.h = TILE_SIZE;

                SDL_RenderCopy(renderer, map->OBJtex, &srcRect, &destrect);
            }
           

        }
    }
}
