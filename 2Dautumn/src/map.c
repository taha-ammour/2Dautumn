#include "headerfile/MAP.h"
#include "headerfile/GAME.h"
#include "headerfile/PARTICLE.h"
#include "headerfile/SOUND.h"

// world layout, in tiles
#define CAMP_X 24
#define CAMP_Y 40
#define HILLS_WALL_X 66
#define HILLS_WALL_Y 47
#define GLADE_WALL_X 104
#define GLADE_WALL_Y 18
#define BOULDER_GAP_Y 31
#define GRANITE_GAP_X 114
#define LAKE_X 88.0f
#define LAKE_Y 64.0f
#define LAKE_RX 22.0f
#define LAKE_RY 9.0f
#define ISLAND_RX 8.5f
#define ISLAND_RY 4.5f
#define SOIL_X 115
#define SOIL_Y 8

static const t_ObjectInfo objectInfo[OBJECT_TYPE_COUNT] = {
    /* NONE     */ { 0, false, -1, NONE_TEX, 0, 0, 0, NULL },
    /* TREE     */ { 8, true, 0, WOOD_TEX, 1, 2, 45000, NULL },
    /* ROCK     */ { 10, true, 1, STONE_TEX, 1, 2, 60000, "Too hard for bare hands. Craft a pickaxe!" },
    /* BOULDER  */ { 8, true, 1, STONE_TEX, 1, 1, 0, "A heavy boulder. A pickaxe could break it." },
    /* GRANITE  */ { 14, true, 2, STONE_TEX, 2, 2, 0, "Solid granite. Only a stone pickaxe will do." },
    /* LEAFPILE */ { 3, false, 0, LEAVES_TEX, 1, 3, 40000, NULL },
    /* MOSS     */ { 6, true, 0, MOSS_TEX, 1, 1, 60000, NULL },
    /* CLIFF    */ { 1, true, -1, NONE_TEX, 0, 0, 0, NULL },
    /* SIGN     */ { 1, true, -1, NONE_TEX, 0, 0, 0, NULL },
    /* SAPLING  */ { 1, true, -1, NONE_TEX, 0, 0, 0, NULL },
};

static const char* regionNames[REGION_COUNT] = {
    "Maple Meadow",
    "Whispering Wood",
    "Rocky Hills",
    "Mirror Pond",
    "Willow Isle",
    "Sunlit Glade",
};

static const char* signTexts[] = {
    "Find a sunny home for the Heart Seed before winter comes.",
    "Whispering Wood: leaf spirits live here. They are shy unless you hit them.",
    "Rocky Hills ahead. Boulders block the way - a pickaxe could break them.",
    "Willow Isle lies across the water. Planks could make a bridge.",
    "Beyond this granite lies the Sunlit Glade.",
};

const t_ObjectInfo* GetObjectInfo(OBJECT_TYPE type)
{
    return &objectInfo[type];
}

const char* GetRegionName(REGION region)
{
    return regionNames[region];
}

const char* GetSignText(int index)
{
    return signTexts[index];
}

static Uint32 tileColor(const t_Map* map, int x, int y)
{
    const t_Object* obj = &map->objectMap[y][x];
    if (!map->explored[y][x])
        return 0xFF1E0E10;
    if (obj->TYPE != NONE_OBJECT && obj->entity.hp > 0)
    {
        switch (obj->TYPE)
        {
        case TREE_OBJECT: return 0xFF5F521A;
        case ROCK_OBJECT: return 0xFF8A9C73;
        case BOULDER_OBJECT: return 0xFFB08250;
        case GRANITE_OBJECT: return 0xFF96A0B2;
        case LEAFPILE_OBJECT: return 0xFFD66E1E;
        case MOSS_OBJECT: return 0xFF60A03C;
        case CLIFF_OBJECT: return 0xFF303032;
        case SIGN_OBJECT: return 0xFFE8D8A0;
        case SAPLING_OBJECT: return 0xFFFFE58C;
        default: break;
        }
    }
    switch (map->ground[y][x])
    {
    case GROUND_FOREST: return 0xFF7A8020;
    case GROUND_ROCK: return 0xFF46554A;
    case GROUND_PATH: return 0xFF96713C;
    case GROUND_WATER: return 0xFF3E6E8C;
    case GROUND_BRIDGE: return 0xFF957128;
    case GROUND_SOIL: return 0xFFFFE58C;
    case GROUND_FLOWERS: return 0xFFC0B848;
    default: return 0xFFA4A51E;
    }
}

static void refreshMinimapTile(t_Map* map, int x, int y)
{
    map->minimapPixels[y * MAP_WIDTH + x] = tileColor(map, x, y);
    map->minimapDirty = true;
}

static void setObject(t_Map* map, int x, int y, OBJECT_TYPE type)
{
    t_Object* obj = &map->objectMap[y][x];
    obj->TYPE = type;
    obj->respawnType = type;
    obj->LIFE = FULL;
    obj->entity.hp = objectInfo[type].maxHp;
    obj->entity.hurtTime = 0;
    obj->respawnAt = 0;
    obj->variant = rand() % 3;
    obj->entity.rect.x = x * TILE_SIZE;
    obj->entity.rect.y = y * TILE_SIZE;
    obj->entity.rect.w = TILE_SIZE;
    obj->entity.rect.h = TILE_SIZE;
    obj->entity.posX = (float)obj->entity.rect.x;
    obj->entity.posY = (float)obj->entity.rect.y;
}

static float ellipse(float x, float y, float cx, float cy, float rx, float ry)
{
    return ((x - cx) / rx) * ((x - cx) / rx) + ((y - cy) / ry) * ((y - cy) / ry);
}

static REGION regionFor(int x, int y)
{
    float lake = ellipse(x + 0.5f, y + 0.5f, LAKE_X, LAKE_Y, LAKE_RX, LAKE_RY);
    float forestEdge = 22.0f + 2.0f * SDL_sinf(x * 0.35f) + 1.5f * SDL_sinf(x * 0.11f + 1.0f);

    if (x >= GLADE_WALL_X + 2 && y < GLADE_WALL_Y)
        return REGION_GLADE;
    if (x >= HILLS_WALL_X && y < HILLS_WALL_Y + 2)
        return REGION_HILLS;
    if (ellipse(x + 0.5f, y + 0.5f, LAKE_X, LAKE_Y, ISLAND_RX, ISLAND_RY) <= 1.0f)
        return REGION_ISLAND;
    if (lake <= 1.9f)
        return REGION_POND;
    if (y < forestEdge && x < HILLS_WALL_X)
        return REGION_FOREST;
    return REGION_MEADOW;
}

static void setPath(t_Map* map, int x0, int y0, int x1, int y1)
{
    for (int y = SDL_min(y0, y1); y <= SDL_max(y0, y1); y++)
        for (int x = SDL_min(x0, x1); x <= SDL_max(x0, x1); x++)
            for (int dy = 0; dy < 2; dy++)
                for (int dx = 0; dx < 2; dx++)
                    if (y + dy < MAP_HEIGHT && x + dx < MAP_WIDTH && map->ground[y + dy][x + dx] != GROUND_WATER)
                        map->ground[y + dy][x + dx] = GROUND_PATH;
}

void generateTerrain(t_Map* map)
{
    int x, y;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            REGION region = regionFor(x, y);
            map->region[y][x] = region;
            map->explored[y][x] = false;
            map->tilemap[y][x] = (GRASS_TILE_END - rand()%3) + (NUMBER_TILE_ROW * (rand() % 3));

            switch (region)
            {
            case REGION_FOREST: map->ground[y][x] = GROUND_FOREST; break;
            case REGION_HILLS:
                map->ground[y][x] = GROUND_ROCK;
                map->tilemap[y][x] = ROCK_TILE + (NUMBER_TILE_ROW * (rand() % 3));
                break;
            case REGION_GLADE: map->ground[y][x] = rand() % 3 == 0 ? GROUND_FLOWERS : GROUND_GRASS; break;
            case REGION_POND:
                map->ground[y][x] = ellipse(x + 0.5f, y + 0.5f, LAKE_X, LAKE_Y, LAKE_RX, LAKE_RY) <= 1.0f ? GROUND_WATER : GROUND_GRASS;
                break;
            default: map->ground[y][x] = GROUND_GRASS; break;
            }
            // every cell gets a valid (empty) object so collisions never read garbage
            setObject(map, x, y, NONE_OBJECT);
        }
    }

    // trails from camp to the hills and to the pond
    setPath(map, CAMP_X, CAMP_Y, 44, CAMP_Y);
    setPath(map, 44, BOULDER_GAP_Y, 44, CAMP_Y);
    setPath(map, 44, BOULDER_GAP_Y, HILLS_WALL_X - 1, BOULDER_GAP_Y);
    setPath(map, 30, CAMP_Y, 30, 52);
    setPath(map, 30, 52, 63, 52);

    map->ground[SOIL_Y][SOIL_X] = GROUND_SOIL;
    for (y = SOIL_Y - 2; y <= SOIL_Y + 2; y++)
        for (x = SOIL_X - 2; x <= SOIL_X + 2; x++)
            if (map->ground[y][x] != GROUND_SOIL)
                map->ground[y][x] = GROUND_FLOWERS;

    map->startX = CAMP_X;
    map->startY = CAMP_Y;
    map->soilX = SOIL_X;
    map->soilY = SOIL_Y;
    map->bouldersX = HILLS_WALL_X;
    map->bouldersY = BOULDER_GAP_Y;
    map->graniteX = GRANITE_GAP_X;
    map->graniteY = GLADE_WALL_Y;
    map->islandX = (int)LAKE_X;
    map->islandY = (int)LAKE_Y;
    map->planted = false;
}

static bool nearTile(int x, int y, int cx, int cy, int radius)
{
    return abs(x - cx) <= radius && abs(y - cy) <= radius;
}

static void placeSign(t_Map* map, int x, int y, int text)
{
    setObject(map, x, y, SIGN_OBJECT);
    map->objectMap[y][x].variant = text;
}

void generateOBJ(t_Map* map)
{
    int y, x;

    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++) {
            REGION region = map->region[y][x];
            GROUND ground = map->ground[y][x];
            int roll = rand() % 100;
            bool wall = x < 2 || y < 2 || x >= MAP_WIDTH - 2 || y >= MAP_HEIGHT - 2 ||
                (x >= HILLS_WALL_X && x < HILLS_WALL_X + 2 && y < HILLS_WALL_Y + 2) ||
                (y >= HILLS_WALL_Y && y < HILLS_WALL_Y + 2 && x >= HILLS_WALL_X) ||
                (x >= GLADE_WALL_X && x < GLADE_WALL_X + 2 && y < GLADE_WALL_Y + 2) ||
                (y >= GLADE_WALL_Y && y < GLADE_WALL_Y + 2 && x >= GLADE_WALL_X);

            if (wall)
            {
                // the two gates: boulders into the hills, granite into the glade
                if (x >= HILLS_WALL_X && x < HILLS_WALL_X + 2 && y >= BOULDER_GAP_Y && y < BOULDER_GAP_Y + 2)
                    setObject(map, x, y, BOULDER_OBJECT);
                else if (y >= GLADE_WALL_Y && y < GLADE_WALL_Y + 2 && x >= GRANITE_GAP_X && x < GRANITE_GAP_X + 2)
                    setObject(map, x, y, GRANITE_OBJECT);
                else
                    setObject(map, x, y, CLIFF_OBJECT);
                continue;
            }
            if (ground == GROUND_WATER || ground == GROUND_PATH || ground == GROUND_SOIL ||
                nearTile(x, y, CAMP_X, CAMP_Y, 3) || nearTile(x, y, SOIL_X, SOIL_Y, 2) ||
                nearTile(x, y, HILLS_WALL_X + 2, BOULDER_GAP_Y, 2) || nearTile(x, y, GRANITE_GAP_X, GLADE_WALL_Y + 3, 2))
                continue;

            switch (region)
            {
            case REGION_FOREST:
                if (roll < 34) setObject(map, x, y, TREE_OBJECT);
                else if (roll < 40) setObject(map, x, y, LEAFPILE_OBJECT);
                break;
            case REGION_HILLS:
                if (roll < 10) setObject(map, x, y, ROCK_OBJECT);
                else if (roll < 12) setObject(map, x, y, CLIFF_OBJECT);
                break;
            case REGION_ISLAND:
                if (roll < 8) setObject(map, x, y, MOSS_OBJECT);
                else if (roll < 14) setObject(map, x, y, TREE_OBJECT);
                else if (roll < 18) setObject(map, x, y, LEAFPILE_OBJECT);
                break;
            case REGION_GLADE:
                if (roll < 5) setObject(map, x, y, TREE_OBJECT);
                break;
            default:
                if (roll < 6) setObject(map, x, y, TREE_OBJECT);
                else if (roll < 9) setObject(map, x, y, LEAFPILE_OBJECT);
                else if (roll < 10) setObject(map, x, y, ROCK_OBJECT);
                break;
            }
        }
    }

    // the island always has some moss to find
    setObject(map, (int)LAKE_X - 3, (int)LAKE_Y - 1, MOSS_OBJECT);
    setObject(map, (int)LAKE_X + 2, (int)LAKE_Y + 1, MOSS_OBJECT);

    placeSign(map, CAMP_X + 2, CAMP_Y - 2, 0);
    placeSign(map, 32, 24, 1);
    placeSign(map, HILLS_WALL_X - 3, BOULDER_GAP_Y - 1, 2);
    placeSign(map, 63, 54, 3);
    placeSign(map, GRANITE_GAP_X - 2, GLADE_WALL_Y + 3, 4);

    for (y = 0; y < MAP_HEIGHT; y++)
        for (x = 0; x < MAP_WIDTH; x++)
            map->minimapPixels[y * MAP_WIDTH + x] = tileColor(map, x, y);
    map->minimapDirty = true;
}


void InitTerrain(t_Map* map, const char* filepath, const char* filepathobj, const char* filepathextra, const char* filepathterrain) {
	map->texture = loadTexture(filepath);
    map->OBJtex = loadTexture(filepathobj);
    map->extraTex = loadTexture(filepathextra);
    map->terrainTex = loadTexture(filepathterrain);
    map->minimapTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, MAP_WIDTH, MAP_HEIGHT);
	generateTerrain(map);
    generateOBJ(map);
}

void ScreenToWorld(int screenX, int screenY, t_Camera camera, int* worldX, int* worldY)
{
    *worldX = (int)(screenX / camera.Zoom) + camera.viewport.x;
    *worldY = (int)(screenY / camera.Zoom) + camera.viewport.y;
}

bool IsSolidAt(const t_Map* map, float x, float y)
{
    int tx = (int)SDL_floorf(x / TILE_SIZE);
    int ty = (int)SDL_floorf(y / TILE_SIZE);

    if (tx < 0 || ty < 0 || tx >= MAP_WIDTH || ty >= MAP_HEIGHT)
        return true;
    if (map->ground[ty][tx] == GROUND_WATER)
        return true;

    const t_Object* obj = &map->objectMap[ty][tx];
    if (obj->TYPE == NONE_OBJECT || obj->entity.hp <= 0 || !objectInfo[obj->TYPE].solid)
        return false;
    // only the base of an object blocks, so you can walk behind its top
    float localX = x - tx * TILE_SIZE;
    float localY = y - ty * TILE_SIZE;
    return localX >= 2 && localX < 14 && localY >= 6;
}

static bool feetBlocked(const t_Map* map, float x, float y)
{
    return IsSolidAt(map, x + 4, y + 11) || IsSolidAt(map, x + 12, y + 11) ||
        IsSolidAt(map, x + 4, y + 15) || IsSolidAt(map, x + 12, y + 15);
}

bool MoveWithCollision(const t_Map* map, t_Entity* e, float dx, float dy)
{
    bool moved = false;
    // never trap something that is already overlapping (a tree regrew on it, a bridge...)
    bool stuck = feetBlocked(map, e->posX, e->posY);

    if (dx != 0 && (stuck || !feetBlocked(map, e->posX + dx, e->posY)))
    {
        e->posX += dx;
        moved = true;
    }
    if (dy != 0 && (stuck || !feetBlocked(map, e->posX, e->posY + dy)))
    {
        e->posY += dy;
        moved = true;
    }
    SyncEntityRect(e);
    return moved;
}

REGION GetRegionAt(const t_Map* map, float x, float y)
{
    int tx = SDL_clamp((int)(x / TILE_SIZE), 0, MAP_WIDTH - 1);
    int ty = SDL_clamp((int)(y / TILE_SIZE), 0, MAP_HEIGHT - 1);
    return map->region[ty][tx];
}

t_Object* FindObjectNear(t_Map* map, float x, float y, float radius)
{
    t_Object* best = NULL;
    float bestDist = radius;
    int tx = (int)(x / TILE_SIZE);
    int ty = (int)(y / TILE_SIZE);

    for (int cy = ty - 1; cy <= ty + 1; cy++)
    {
        for (int cx = tx - 1; cx <= tx + 1; cx++)
        {
            if (cx < 0 || cy < 0 || cx >= MAP_WIDTH || cy >= MAP_HEIGHT)
                continue;
            t_Object* obj = &map->objectMap[cy][cx];
            if (obj->TYPE == NONE_OBJECT || obj->entity.hp <= 0)
                continue;
            // distance from the point to the object's tile
            float nx = SDL_clamp(x, (float)obj->entity.rect.x, (float)(obj->entity.rect.x + TILE_SIZE));
            float ny = SDL_clamp(y, (float)obj->entity.rect.y, (float)(obj->entity.rect.y + TILE_SIZE));
            float dist = SDL_sqrtf((x - nx) * (x - nx) + (y - ny) * (y - ny));
            if (dist <= bestDist)
            {
                bestDist = dist;
                best = obj;
            }
        }
    }
    return best;
}

bool PlaceBridge(t_Map* map, int tileX, int tileY)
{
    if (tileX < 0 || tileY < 0 || tileX >= MAP_WIDTH || tileY >= MAP_HEIGHT || map->ground[tileY][tileX] != GROUND_WATER)
        return false;
    map->ground[tileY][tileX] = GROUND_BRIDGE;
    refreshMinimapTile(map, tileX, tileY);
    return true;
}

void PlantSeed(t_Map* map)
{
    setObject(map, map->soilX, map->soilY, SAPLING_OBJECT);
    map->objectMap[map->soilY][map->soilX].variant = 1;
    map->objectMap[map->soilY][map->soilX].respawnAt = SDL_GetTicks() + 1800;
    map->planted = true;
    refreshMinimapTile(map, map->soilX, map->soilY);
}

void RevealAround(t_Map* map, float x, float y)
{
    int tx = (int)(x / TILE_SIZE);
    int ty = (int)(y / TILE_SIZE);

    for (int cy = ty - REVEAL_RADIUS; cy <= ty + REVEAL_RADIUS; cy++)
    {
        for (int cx = tx - REVEAL_RADIUS; cx <= tx + REVEAL_RADIUS; cx++)
        {
            if (cx < 0 || cy < 0 || cx >= MAP_WIDTH || cy >= MAP_HEIGHT || map->explored[cy][cx])
                continue;
            if ((cx - tx) * (cx - tx) + (cy - ty) * (cy - ty) > REVEAL_RADIUS * REVEAL_RADIUS)
                continue;
            map->explored[cy][cx] = true;
            refreshMinimapTile(map, cx, cy);
        }
    }
}

static SDL_Color breakColor(OBJECT_TYPE type)
{
    switch (type)
    {
    case TREE_OBJECT: return (SDL_Color){ 150, 110, 40, 255 };
    case BOULDER_OBJECT: return (SDL_Color){ 176, 130, 80, 255 };
    case GRANITE_OBJECT: return (SDL_Color){ 150, 160, 178, 255 };
    case LEAFPILE_OBJECT: return (SDL_Color){ 214, 110, 30, 255 };
    case MOSS_OBJECT: return (SDL_Color){ 96, 140, 60, 255 };
    default: return (SDL_Color){ 120, 130, 110, 255 };
    }
}

static void breakObject(t_Map* map, int x, int y)
{
    t_Object* obj = &map->objectMap[y][x];
    const t_ObjectInfo* info = &objectInfo[obj->TYPE];
    float cx = obj->entity.rect.x + 8.0f;
    float cy = obj->entity.rect.y + 10.0f;
    int count = info->dropMin + (info->dropMax > info->dropMin ? rand() % (info->dropMax - info->dropMin + 1) : 0);

    if (count > 0)
        SpawnPickup(cx, cy, info->drop, count);
    if (obj->TYPE == LEAFPILE_OBJECT)
    {
        // leaf piles sometimes hide a twig
        if (rand() % 4 == 0)
            SpawnPickup(cx, cy, STICK_TEX, 1);
        SpawnBurst(cx, cy, (SDL_Color){ 190, 60, 30, 255 }, 10, 70.0f);
        SpawnBurst(cx, cy, (SDL_Color){ 232, 180, 40, 255 }, 10, 70.0f);
    }
    SpawnBurst(cx, cy, breakColor(obj->TYPE), 16, 60.0f);
    if (obj->TYPE == BOULDER_OBJECT || obj->TYPE == GRANITE_OBJECT)
        AddScreenShake(3.0f);

    obj->respawnType = obj->TYPE;
    obj->TYPE = NONE_OBJECT;
    obj->respawnAt = info->respawnTime != 0 ? SDL_GetTicks() + info->respawnTime : 0;
    refreshMinimapTile(map, x, y);
}

void UpdateOBJState(t_Map* map, const t_Player* player)
{
    Uint32 now = SDL_GetTicks();

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            t_Object* obj = &map->objectMap[y][x];
            if (obj->TYPE == NONE_OBJECT)
            {
                // regrow, but never right on top of the player
                if (obj->respawnAt != 0 && now >= obj->respawnAt && !SDL_HasIntersection(&obj->entity.rect, &player->entity.rect))
                {
                    setObject(map, x, y, obj->respawnType);
                    refreshMinimapTile(map, x, y);
                }
                continue;
            }
            if (obj->TYPE == SAPLING_OBJECT && obj->variant == 1 && now >= obj->respawnAt)
            {
                // the sprout shoots up into a golden tree
                obj->variant = 2;
                SpawnBurst(obj->entity.rect.x + 8.0f, obj->entity.rect.y + 4.0f, (SDL_Color){ 255, 229, 140, 255 }, 30, 70.0f);
                AddScreenShake(1.5f);
            }
            if (objectInfo[obj->TYPE].pickTier < 0)
                continue;
            if (obj->entity.hp <= 0)
            {
                breakObject(map, x, y);
                continue;
            }
            double ratio = obj->entity.hp / objectInfo[obj->TYPE].maxHp;
            obj->LIFE = ratio > 0.66 ? FULL : (ratio > 0.33 ? MID : EMPTY);
        }
    }
}

static void renderTerrainTile(t_Map* map, int index, SDL_Rect* dest)
{
    SDL_Rect src = { index * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE };
    SDL_RenderCopy(renderer, map->terrainTex, &src, dest);
}

static bool isWet(const t_Map* map, int x, int y)
{
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT)
        return true;
    return map->ground[y][x] == GROUND_WATER || map->ground[y][x] == GROUND_BRIDGE;
}

void renderTilemap(t_Map* map, t_Camera camera)
{
	int x0 = SDL_max(0, camera.viewport.x / TILE_SIZE - 1);
	int y0 = SDL_max(0, camera.viewport.y / TILE_SIZE - 1);
	int x1 = SDL_min(MAP_WIDTH - 1, (camera.viewport.x + camera.viewport.w) / TILE_SIZE + 1);
	int y1 = SDL_min(MAP_HEIGHT - 1, (camera.viewport.y + camera.viewport.h) / TILE_SIZE + 1);
	int waterFrame = (SDL_GetTicks() / 700) % 2;
	Uint32 now = SDL_GetTicks();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	for (int y = y0; y <= y1; y++)
	{
		for (int x = x0; x <= x1; x++) {
			SDL_Rect destrect = { x * TILE_SIZE - camera.viewport.x, y * TILE_SIZE - camera.viewport.y, TILE_SIZE, TILE_SIZE };
			int tileIndex = map->tilemap[y][x];
			SDL_Rect srcRect = { (tileIndex % NUMBER_TILE_ROW) * TILE_SIZE, (tileIndex / NUMBER_TILE_ROW) * TILE_SIZE, TILE_SIZE, TILE_SIZE };

			switch (map->ground[y][x])
			{
			case GROUND_FOREST:
				// the wood is shaded by its canopy
				SDL_SetTextureColorMod(map->texture, 205, 195, 158);
				SDL_RenderCopy(renderer, map->texture, &srcRect, &destrect);
				SDL_SetTextureColorMod(map->texture, 255, 255, 255);
				break;
			case GROUND_PATH: renderTerrainTile(map, TERRAIN_PATH, &destrect); break;
			case GROUND_FLOWERS: renderTerrainTile(map, TERRAIN_FLOWERS, &destrect); break;
			case GROUND_SOIL:
				renderTerrainTile(map, TERRAIN_SOIL, &destrect);
				if (!map->planted)
				{
					Uint8 glow = (Uint8)(90 + 70 * SDL_sinf(now / 300.0f));
					SDL_SetRenderDrawColor(renderer, 255, 229, 140, glow);
					SDL_RenderDrawRect(renderer, &destrect);
				}
				break;
			case GROUND_WATER:
			case GROUND_BRIDGE:
				renderTerrainTile(map, TERRAIN_WATER + waterFrame, &destrect);
				// light foam where the water meets the shore
				SDL_SetRenderDrawColor(renderer, 200, 225, 215, 200);
				if (!isWet(map, x, y - 1)) SDL_RenderDrawLine(renderer, destrect.x, destrect.y, destrect.x + 15, destrect.y);
				if (!isWet(map, x, y + 1)) SDL_RenderDrawLine(renderer, destrect.x, destrect.y + 15, destrect.x + 15, destrect.y + 15);
				if (!isWet(map, x - 1, y)) SDL_RenderDrawLine(renderer, destrect.x, destrect.y, destrect.x, destrect.y + 15);
				if (!isWet(map, x + 1, y)) SDL_RenderDrawLine(renderer, destrect.x + 15, destrect.y, destrect.x + 15, destrect.y + 15);
				if (map->ground[y][x] == GROUND_BRIDGE)
					renderTerrainTile(map, TERRAIN_BRIDGE, &destrect);
				break;
			default:
				SDL_RenderCopy(renderer, map->texture, &srcRect, &destrect);
				break;
			}
		}
	}
}

void renderObject(t_Map* map, const t_Object* obj, t_Camera camera)
{
    SDL_Texture* texture = map->extraTex;
    SDL_Rect srcRect = { obj->LIFE * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE };
    Uint32 sinceHit = SDL_GetTicks() - obj->entity.hurtTime;
    int wobble = obj->entity.hurtTime != 0 && sinceHit < 120 ? (sinceHit / 40) % 2 * 2 - 1 : 0;

    switch (obj->TYPE)
    {
    case TREE_OBJECT: texture = map->OBJtex; srcRect.y = 1 * TILE_SIZE; break;
    case ROCK_OBJECT: texture = map->OBJtex; srcRect.y = 0; break;
    case BOULDER_OBJECT: srcRect.y = 0; break;
    case GRANITE_OBJECT: srcRect.y = 1 * TILE_SIZE; break;
    case LEAFPILE_OBJECT: srcRect.y = 2 * TILE_SIZE; break;
    case MOSS_OBJECT: srcRect.y = 3 * TILE_SIZE; break;
    case CLIFF_OBJECT: srcRect.y = 4 * TILE_SIZE; srcRect.x = obj->variant * TILE_SIZE; break;
    case SIGN_OBJECT: srcRect.y = 5 * TILE_SIZE; srcRect.x = 0; break;
    case SAPLING_OBJECT: srcRect.y = 5 * TILE_SIZE; srcRect.x = obj->variant * TILE_SIZE; break;
    default: return;
    }

    SDL_Rect destrect = { obj->entity.rect.x - camera.viewport.x + wobble, obj->entity.rect.y - camera.viewport.y, TILE_SIZE, TILE_SIZE };
    SDL_RenderCopy(renderer, texture, &srcRect, &destrect);
    if (wobble != 0)
    {
        // brief bright flash when struck
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);
        SDL_SetTextureAlphaMod(texture, 120);
        SDL_RenderCopy(renderer, texture, &srcRect, &destrect);
        SDL_SetTextureAlphaMod(texture, 255);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
}

static const struct { REGION region; int x, y; } regionLabels[] = {
    { REGION_MEADOW, 24, 46 }, { REGION_FOREST, 28, 10 }, { REGION_HILLS, 84, 30 },
    { REGION_POND, 70, 56 }, { REGION_ISLAND, 88, 64 }, { REGION_GLADE, 115, 10 },
};

// dest is in screen pixels
void renderMinimap(t_Map* map, const t_Player* player, SDL_Rect dest, bool showLabels)
{
    float scaleX, scaleY;
    float tilesToPx = (float)dest.w / MAP_WIDTH;

    if (map->minimapTex == NULL)
        return;
    if (map->minimapDirty)
    {
        SDL_UpdateTexture(map->minimapTex, NULL, map->minimapPixels, MAP_WIDTH * sizeof(Uint32));
        map->minimapDirty = false;
    }
    SDL_RenderGetScale(renderer, &scaleX, &scaleY);
    SDL_RenderSetScale(renderer, 1.0f, 1.0f);

    SDL_Rect frame = { dest.x - 3, dest.y - 3, dest.w + 6, dest.h + 6 };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 30, 14, 16, 230);
    SDL_RenderFillRect(renderer, &frame);
    SDL_SetRenderDrawColor(renderer, 150, 100, 45, 255);
    SDL_RenderDrawRect(renderer, &frame);
    SDL_RenderCopy(renderer, map->minimapTex, NULL, &dest);

    if ((SDL_GetTicks() / 300) % 3 != 0)
    {
        int size = SDL_max(3, (int)tilesToPx);
        SDL_Rect dot = { dest.x + (int)((player->entity.posX + 8) / TILE_SIZE * tilesToPx) - size / 2,
            dest.y + (int)((player->entity.posY + 8) / TILE_SIZE * tilesToPx) - size / 2, size, size };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &dot);
    }
    SDL_RenderSetScale(renderer, scaleX, scaleY);

    if (showLabels)
    {
        for (size_t i = 0; i < sizeof(regionLabels) / sizeof(regionLabels[0]); i++)
        {
            if (!map->explored[regionLabels[i].y][regionLabels[i].x])
                continue;
            renderText(uiFontSmall, regionNames[regionLabels[i].region],
                (int)((dest.x + regionLabels[i].x * tilesToPx) / scaleX), (int)((dest.y + regionLabels[i].y * tilesToPx) / scaleY),
                (SDL_Color){ 255, 240, 215, 255 }, ALIGN_CENTER);
        }
    }
}
