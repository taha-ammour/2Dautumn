#include "headerfile/MAP.h"
#include "headerfile/GAME.h"
#include "headerfile/PARTICLE.h"
#include "headerfile/SOUND.h"
#include <time.h>

static const SDL_Color rockChipColor = { 120, 130, 110, 255 };
static const SDL_Color woodChipColor = { 150, 110, 40, 255 };

static void setObject(t_Map* map, int x, int y, OBJECT_TYPE type, double hp)
{
    t_Object* obj = &map->objectMap[y][x];
    obj->TYPE = type;
    obj->LIFE = FULL;
    obj->entity.hp = hp;
    obj->respawnAt = 0;
    obj->entity.rect.x = x * TILE_SIZE;
    obj->entity.rect.y = y * TILE_SIZE;
    obj->entity.rect.w = TILE_SIZE;
    obj->entity.rect.h = TILE_SIZE;
    obj->entity.posX = (float)obj->entity.rect.x;
    obj->entity.posY = (float)obj->entity.rect.y;
}

void RespawnTree(t_Map* map, int x, int y)
{
    if (map->objectMap[y][x].TYPE == NONE_OBJECT)
    {
        setObject(map, x, y, TREE_OBJECT, 10);
    }
}

void generateTerrain(t_Map* map)
{
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
            // every cell gets a valid (empty) object so collisions never read garbage
            setObject(map, x, y, NONE_OBJECT, 0);
        }
    }
}

void generateOBJ(t_Map* map)
{
    int y, x;

    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++) {
            // keep a clearing around the heart seed
            if (abs(x - MAP_WIDTH / 2) <= SEED_CLEARING && abs(y - MAP_HEIGHT / 2) <= SEED_CLEARING)
                continue;
            if (map->objectMap[y][x].TYPE == NONE_OBJECT &&
                (map->tilemap[y][x] == ROCK_TILE || map->tilemap[y][x] == ROCK_TILE + NUMBER_TILE_ROW || map->tilemap[y][x] == ROCK_TILE + NUMBER_TILE_ROW * 2))
            {
                if (rand() % 2 == 0) {
                    setObject(map, x, y, ROCK_OBJECT, 20);
                }
            }
             if (map->objectMap[y][x].TYPE == NONE_OBJECT &&
                map->tilemap[y][x] == GRASS_TILE_START ) {
                 if (rand()% 4 == 0)
                 {
                    setObject(map, x, y, TREE_OBJECT, 10);
                 }

            }

        }
    }
    setObject(map, MAP_WIDTH / 2, MAP_HEIGHT / 2, BASEPOINT_OBJECT, 100);
}


void InitTerrain(t_Map* map, const char* filepath, const char* filepathobj) {
	map->texture = loadTexture(filepath);
    map->OBJtex = loadTexture(filepathobj);
	generateTerrain(map);
    generateOBJ(map);
}

t_Object* GetSeed(t_Map* map)
{
    return &map->objectMap[MAP_HEIGHT / 2][MAP_WIDTH / 2];
}

void ScreenToWorld(int screenX, int screenY, t_Camera camera, int* worldX, int* worldY)
{
    *worldX = (int)(screenX / camera.Zoom) + camera.viewport.x;
    *worldY = (int)(screenY / camera.Zoom) + camera.viewport.y;
}


bool InteractPOBJ(SDL_Event* e,t_Map* map, t_Player* player, t_inventory* inv, t_Camera camera)
{
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT)
    {
        int breakDistance = 2;
        int mouseX, mouseY;
        ScreenToWorld(e->button.x, e->button.y, camera, &mouseX, &mouseY);

        int playerTileX = player->entity.rect.x / TILE_SIZE;
        int playerTileY = player->entity.rect.y / TILE_SIZE;

        for (int y = playerTileY - breakDistance; y <= playerTileY + breakDistance; y++) {
            for (int x = playerTileX - breakDistance; x <= playerTileX + breakDistance; x++) {
                if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
                    continue;
                t_Object* obj = &map->objectMap[y][x];
                if ((obj->TYPE != ROCK_OBJECT && obj->TYPE != TREE_OBJECT) || obj->entity.hp <= 0)
                    continue;

                SDL_Rect objectRect = obj->entity.rect;
                float distance = CalcDistance(objectRect, player->entity.rect);

                if (distance <= TILE_SIZE * breakDistance &&
                    mouseX >= objectRect.x && mouseX <= objectRect.x + objectRect.w &&
                    mouseY >= objectRect.y && mouseY <= objectRect.y + objectRect.h) {

                    // trees are soft, rocks really need a pickaxe
                    double baseDamage = obj->TYPE == TREE_OBJECT ? 2.0 : 1.0;
                    obj->entity.hp -= baseDamage * player->attackdmg * GetMiningMultiplier(GetSelectedItem(inv));
                    player->attackTime = SDL_GetTicks();
                    SpawnBurst(objectRect.x + 8.0f, objectRect.y + 8.0f,
                        obj->TYPE == ROCK_OBJECT ? rockChipColor : woodChipColor, 5, 40.0f);
                    PlaySfx(SFX_HARVEST);
                    return true;
                }
            }
        }
    }
    return false;
}

static void harvestObject(t_Map* map, t_inventory* inv, int x, int y, ITEM_TEX itemTex, SDL_Color color)
{
    t_Object* obj = &map->objectMap[y][x];
    t_item newitem = generateItemOnDestroy(itemTex, MATERIAL);
    char text[24];

    addItemToInventory(inv, newitem);
    snprintf(text, sizeof(text), "+%d %s", newitem.itemCount, GetItemName(itemTex));
    SpawnFloatText(obj->entity.rect.x + 8.0f, (float)obj->entity.rect.y, text, (SDL_Color){ 255, 240, 200, 255 });
    SpawnBurst(obj->entity.rect.x + 8.0f, obj->entity.rect.y + 8.0f, color, 14, 60.0f);
    obj->TYPE = NONE_OBJECT;
}

void UpdateOBJState(t_Map* map,t_inventory* inv)
{
    Uint32 now = SDL_GetTicks();

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            t_Object* obj = &map->objectMap[y][x];
            int health_obj = (int)obj->entity.hp;
            if (obj->TYPE == ROCK_OBJECT)
            {
                if (health_obj > 18)
                {
                    obj->LIFE = FULL;
                }
                else if (health_obj > 13)
                {
                    obj->LIFE = MID;
                }
                else if (health_obj > 7)
                {
                    obj->LIFE = EMPTY;
                }
                else if(obj->entity.hp <= 0)
                {
                    harvestObject(map, inv, x, y, STONE_TEX, rockChipColor);
                }
            }
            else if (obj->TYPE == TREE_OBJECT) {
                if (health_obj > 8)
                {
                    obj->LIFE = FULL;
                }
                else if (health_obj > 5)
                {
                    obj->LIFE = MID;
                }
                else if (health_obj > 2)
                {
                    obj->LIFE = EMPTY;
                }
                else if (obj->entity.hp <= 0)
                {
                    harvestObject(map, inv, x, y, WOOD_TEX, woodChipColor);
                    obj->respawnAt = now + TREE_RESPAWN_TIME;
                }
            }
            else if (obj->TYPE == NONE_OBJECT && obj->respawnAt != 0 && now >= obj->respawnAt) {
                RespawnTree(map, x, y);
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
            t_Object* obj = &map->objectMap[y][x];
            int row;

            if (obj->entity.hp <= 0)
                continue;
            if (obj->TYPE == ROCK_OBJECT)
                row = 0;
            else if (obj->TYPE == TREE_OBJECT)
                row = 1;
            else if (obj->TYPE == BASEPOINT_OBJECT)
                row = 2;
            else
                continue;

            srcRect.x = obj->LIFE * TILE_SIZE;
            srcRect.y = row * TILE_SIZE;
            srcRect.w = TILE_SIZE;
            srcRect.h = TILE_SIZE;

            destrect.x = obj->entity.rect.x - camera.viewport.x;
            destrect.y = obj->entity.rect.y - camera.viewport.y;
            destrect.w = TILE_SIZE;
            destrect.h = TILE_SIZE;

            SDL_RenderCopy(renderer, map->OBJtex, &srcRect, &destrect);
        }
    }
}
