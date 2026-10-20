#include "headerfile/WORLD.h"
#include "headerfile/PARTICLE.h"
#include "headerfile/SOUND.h"

static const SDL_Color white = { 255, 255, 255, 255 };
static const SDL_Color duskColor = { 255, 165, 115, 255 };
static const SDL_Color nightColor = { 55, 60, 115, 255 };
static const SDL_Color dawnColor = { 255, 185, 170, 255 };
static const SDL_Color seedGlow = { 255, 205, 120, 255 };
static const SDL_Color bannerColor = { 255, 225, 160, 255 };

typedef enum {
	EDGE_WEST,
	EDGE_EAST,
	EDGE_NORTH,
	EDGE_SOUTH,
}MAP_EDGE;

static const char* edgeNames[] = { "west", "east", "north", "south" };

static SDL_Texture* createLightTexture(int size)
{
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Texture* texture;
	float radius = size * 0.5f;

	if (surface == NULL)
		return NULL;
	Uint32* pixels = (Uint32*)surface->pixels;
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			float dx = (x + 0.5f - radius) / radius;
			float dy = (y + 0.5f - radius) / radius;
			float falloff = 1.0f - SDL_sqrtf(dx * dx + dy * dy);
			if (falloff < 0)
				falloff = 0;
			Uint8 v = (Uint8)(255 * falloff * falloff);
			pixels[y * (surface->pitch / 4) + x] = SDL_MapRGBA(surface->format, v, v, v, 255);
		}
	}
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);
	return texture;
}

void InitWorld(t_World* world, t_Camera* camera)
{
	world->lightTex = createLightTexture(64);
	world->lightMapW = camera->viewport.w;
	world->lightMapH = camera->viewport.h;
	world->lightMap = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, world->lightMapW, world->lightMapH);
	if (world->lightMap == NULL)
	{
		fprintf(stderr, "Light map not created, nights will stay bright: %s\n", SDL_GetError());
	}
	else
	{
		SDL_SetTextureBlendMode(world->lightMap, SDL_BLENDMODE_MOD);
	}
}

void ShowBanner(t_World* world, const char* text)
{
	snprintf(world->banner, sizeof(world->banner), "%s", text);
	world->bannerTimer = 4.0f;
}

static void startPhase(t_World* world, WORLD_PHASE phase, float length)
{
	world->phase = phase;
	world->phaseLength = length;
	world->phaseTimer = length;
}

static void pickWindTarget(t_World* world, float minStrength, float maxStrength)
{
	float angle = RandomFloat(0.0f, 6.2831f);
	float strength = RandomFloat(minStrength, maxStrength);
	world->windTargetX = SDL_cosf(angle) * strength;
	world->windTargetY = SDL_sinf(angle) * strength * 0.6f;
	world->windTimer = RandomFloat(10.0f, 18.0f);
}

void ResetWorld(t_World* world, t_Map* map)
{
	char text[64];

	startPhase(world, PHASE_DAY, FIRST_DAY_LENGTH);
	world->day = 1;
	world->nightsSurvived = 0;
	world->toSpawn = 0;
	world->enemiesLeft = 0;
	world->spawnTimer = 0;
	world->daySpawnTimer = 20.0f;
	world->seedLevel = 0;
	world->seedMaxHp = SEED_BASE_HP;
	world->seedFuel = 20.0f;
	GetSeed(map)->entity.hp = world->seedMaxHp;

	pickWindTarget(world, 5.0f, WIND_MAX * 0.6f);
	world->windX = world->windTargetX;
	world->windY = world->windTargetY;

	snprintf(text, sizeof(text), "Day 1 - gather before the leaves fall");
	ShowBanner(world, text);
}

void UpdateWind(t_World* world, float deltaTime)
{
	world->windTimer -= deltaTime;
	if (world->windTimer <= 0)
	{
		if (world->phase == PHASE_NIGHT)
			pickWindTarget(world, WIND_MAX * 0.5f, WIND_MAX);
		else
			pickWindTarget(world, 0.0f, WIND_MAX * 0.6f);
	}
	world->windX += (world->windTargetX - world->windX) * SDL_min(1.0f, deltaTime * 0.5f);
	world->windY += (world->windTargetY - world->windY) * SDL_min(1.0f, deltaTime * 0.5f);
}

// leaf spirits are blown in, so they enter from the edge the wind comes from
static MAP_EDGE getUpwindEdge(const t_World* world)
{
	if (SDL_fabsf(world->windX) < 2.0f && SDL_fabsf(world->windY) < 2.0f)
		return (MAP_EDGE)(rand() % 4);
	if (SDL_fabsf(world->windX) >= SDL_fabsf(world->windY))
		return world->windX > 0 ? EDGE_WEST : EDGE_EAST;
	return world->windY > 0 ? EDGE_NORTH : EDGE_SOUTH;
}

static void spawnAtEdge(t_EnemyPool* enemies, ENEMY_KIND kind, MAP_EDGE edge)
{
	float maxX = (float)((MAP_WIDTH - 2) * TILE_SIZE);
	float maxY = (float)((MAP_HEIGHT - 2) * TILE_SIZE);
	float x, y;

	switch (edge)
	{
	case EDGE_WEST: x = TILE_SIZE; y = RandomFloat(TILE_SIZE, maxY); break;
	case EDGE_EAST: x = maxX; y = RandomFloat(TILE_SIZE, maxY); break;
	case EDGE_NORTH: x = RandomFloat(TILE_SIZE, maxX); y = TILE_SIZE; break;
	default: x = RandomFloat(TILE_SIZE, maxX); y = maxY; break;
	}
	SpawnEnemy(enemies, kind, x, y);
}

static ENEMY_KIND chooseEnemyKind(int night)
{
	int weights[LEAF_KIND_COUNT] = { 6, night >= 2 ? 3 : 0, night >= 3 ? 2 : 0, night >= 4 ? 1 + (night - 4) : 0 };
	int total = 0;

	for (int i = 0; i < LEAF_KIND_COUNT; i++)
		total += weights[i];
	int roll = rand() % total;
	for (int i = 0; i < LEAF_KIND_COUNT; i++)
	{
		if (roll < weights[i])
			return (ENEMY_KIND)i;
		roll -= weights[i];
	}
	return LEAF_ORANGE;
}

static void updateSeedLook(t_World* world, t_Object* seed)
{
	double ratio = seed->entity.hp / world->seedMaxHp;
	if (ratio > 0.66)
		seed->LIFE = FULL;
	else if (ratio > 0.33)
		seed->LIFE = MID;
	else
		seed->LIFE = EMPTY;
}

void UpdateWorld(t_World* world, t_Map* map, t_EnemyPool* enemies, t_Player* player, float deltaTime)
{
	t_Object* seed = GetSeed(map);
	char text[64];

	UpdateWind(world, deltaTime);
	world->phaseTimer -= deltaTime;
	if (world->bannerTimer > 0)
		world->bannerTimer -= deltaTime;

	if (world->phase == PHASE_DAY && seed->entity.hp > 0 && seed->entity.hp < world->seedMaxHp)
	{
		seed->entity.hp += (0.3 + 0.1 * world->seedLevel) * deltaTime;
		if (seed->entity.hp > world->seedMaxHp)
			seed->entity.hp = world->seedMaxHp;
	}
	if (world->phase == PHASE_NIGHT && world->seedFuel > 0)
	{
		world->seedFuel -= 0.5f * deltaTime;
		if (world->seedFuel < 0)
			world->seedFuel = 0;
	}
	updateSeedLook(world, seed);

	switch (world->phase)
	{
	case PHASE_DAY:
		// a few stray leaves wander in during the day
		world->daySpawnTimer -= deltaTime;
		if (world->daySpawnTimer <= 0)
		{
			if (CountAliveEnemies(enemies) < 2)
				spawnAtEdge(enemies, LEAF_ORANGE, getUpwindEdge(world));
			world->daySpawnTimer = 25.0f;
		}
		if (world->phaseTimer <= 0)
		{
			startPhase(world, PHASE_DUSK, DUSK_LENGTH);
			// the night wind builds up during dusk, so its direction is readable before the wave
			pickWindTarget(world, WIND_MAX * 0.5f, WIND_MAX);
			ShowBanner(world, "Dusk... the wind is rising");
		}
		break;
	case PHASE_DUSK:
		if (world->phaseTimer <= 0)
		{
			startPhase(world, PHASE_NIGHT, NIGHT_MIN_LENGTH);
			world->toSpawn = 3 + 2 * world->day;
			world->spawnTimer = 1.0f;
			snprintf(text, sizeof(text), "Night %d - they ride the wind from the %s", world->day, edgeNames[getUpwindEdge(world)]);
			ShowBanner(world, text);
			PlaySfx(SFX_NIGHT);
		}
		break;
	case PHASE_NIGHT:
		world->spawnTimer -= deltaTime;
		if (world->toSpawn > 0 && world->spawnTimer <= 0)
		{
			spawnAtEdge(enemies, chooseEnemyKind(world->day), getUpwindEdge(world));
			world->toSpawn--;
			world->spawnTimer = RandomFloat(0.8f, 1.8f);
		}
		if (world->phaseTimer <= 0 && world->toSpawn == 0 && CountAliveEnemies(enemies) == 0)
		{
			world->nightsSurvived++;
			world->seedLevel++;
			world->seedMaxHp += SEED_HP_PER_LEVEL;
			seed->entity.hp = SDL_min(world->seedMaxHp, seed->entity.hp + world->seedMaxHp * 0.3);
			if (player->alive)
				player->entity.hp = PLAYER_MAX_HP;
			startPhase(world, PHASE_DAWN, DAWN_LENGTH);
			snprintf(text, sizeof(text), "Dawn - the Heart Seed grows (Lv %d)", world->seedLevel);
			ShowBanner(world, text);
			SpawnBurst(seed->entity.rect.x + 8.0f, seed->entity.rect.y + 8.0f, seedGlow, 30, 80.0f);
			PlaySfx(SFX_DAWN);
		}
		break;
	case PHASE_DAWN:
		if (world->phaseTimer <= 0)
		{
			world->day++;
			startPhase(world, PHASE_DAY, DAY_LENGTH);
			snprintf(text, sizeof(text), "Day %d", world->day);
			ShowBanner(world, text);
		}
		break;
	}
	world->enemiesLeft = world->toSpawn + CountAliveEnemies(enemies);
}

void UpdatePlayerLife(t_World* world, t_Map* map, t_Player* player, float deltaTime)
{
	t_Object* seed = GetSeed(map);

	if (player->alive && player->entity.hp <= 0)
	{
		// falling costs the seed part of its life: it regrows the stump
		player->alive = false;
		player->respawnTimer = PLAYER_RESPAWN_TIME;
		seed->entity.hp -= world->seedMaxHp * 0.2;
		SpawnBurst(player->entity.posX + 8, player->entity.posY + 8, (SDL_Color){ 150, 110, 40, 255 }, 24, 70.0f);
		AddScreenShake(5.0f);
		ShowBanner(world, "You fell! The Heart Seed regrows you...");
	}
	if (!player->alive && seed->entity.hp > 0)
	{
		player->respawnTimer -= deltaTime;
		if (player->respawnTimer <= 0)
		{
			ResetPlayer(player);
			player->entity.hp = PLAYER_MAX_HP * 0.6;
			player->entity.hurtTime = SDL_GetTicks();
			SpawnBurst(player->entity.posX + 8, player->entity.posY + 8, seedGlow, 20, 50.0f);
		}
	}
}

float GetSeedLightRadius(const t_World* world)
{
	return 30.0f + 8.0f * world->seedLevel + world->seedFuel * 0.6f;
}

float GetSeedBurnDps(const t_World* world)
{
	return 3.0f + 1.0f * world->seedLevel;
}

bool FeedSeed(t_World* world, t_Map* map, t_Player* player, t_inventory* inv)
{
	t_Object* seed = GetSeed(map);
	float dist = CalcDistance(seed->entity.rect, player->entity.rect);
	float x = seed->entity.rect.x + 8.0f;
	float y = (float)seed->entity.rect.y;

	if (!player->alive || dist > SEED_FEED_RANGE)
		return false;
	if (!RemoveItem(inv, WOOD_TEX, 1))
	{
		SpawnFloatText(x, y - 6, "Feed it wood", (SDL_Color){ 255, 90, 70, 255 });
		return false;
	}
	world->seedFuel = SDL_min(SEED_MAX_FUEL, world->seedFuel + 15.0f);
	seed->entity.hp = SDL_min(world->seedMaxHp, seed->entity.hp + 8.0);
	SpawnBurst(x, y + 8, seedGlow, 16, 50.0f);
	SpawnFloatText(x, y - 6, "+glow", seedGlow);
	PlaySfx(SFX_FEED);
	return true;
}

void renderSeedAura(const t_World* world, t_Map* map, t_Camera camera)
{
	t_Object* seed = GetSeed(map);
	SDL_Point points[64];
	float radius = GetSeedLightRadius(world);
	float cx = seed->entity.rect.x + 8.0f - camera.viewport.x;
	float cy = seed->entity.rect.y + 8.0f - camera.viewport.y;
	float spin = SDL_GetTicks() / 2000.0f;

	if (seed->entity.hp <= 0)
		return;
	// dotted ring showing where leaves start to burn
	for (int i = 0; i < 64; i++)
	{
		float a = spin + i * 6.2831f / 64.0f;
		points[i].x = (int)(cx + SDL_cosf(a) * radius);
		points[i].y = (int)(cy + SDL_sinf(a) * radius);
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, seedGlow.r, seedGlow.g, seedGlow.b, 110);
	SDL_RenderDrawPoints(renderer, points, 64);
}

static SDL_Color lerpColor(SDL_Color a, SDL_Color b, float t)
{
	SDL_Color c;
	if (t < 0) t = 0;
	if (t > 1) t = 1;
	c.r = (Uint8)(a.r + (b.r - a.r) * t);
	c.g = (Uint8)(a.g + (b.g - a.g) * t);
	c.b = (Uint8)(a.b + (b.b - a.b) * t);
	c.a = 255;
	return c;
}

static SDL_Color getAmbientColor(const t_World* world)
{
	float t = 1.0f - world->phaseTimer / world->phaseLength;
	switch (world->phase)
	{
	case PHASE_DUSK:
		return t < 0.5f ? lerpColor(white, duskColor, t * 2) : lerpColor(duskColor, nightColor, (t - 0.5f) * 2);
	case PHASE_NIGHT:
		return nightColor;
	case PHASE_DAWN:
		return t < 0.5f ? lerpColor(nightColor, dawnColor, t * 2) : lerpColor(dawnColor, white, (t - 0.5f) * 2);
	default:
		// the last seconds of the day already turn golden
		if (world->phaseTimer < 10.0f)
			return lerpColor(white, (SDL_Color){ 255, 235, 200, 255 }, 1.0f - world->phaseTimer / 10.0f);
		return white;
	}
}

static void drawLight(t_World* world, float x, float y, float radius, SDL_Color color)
{
	SDL_Rect dest = { (int)(x - radius), (int)(y - radius), (int)(radius * 2), (int)(radius * 2) };
	SDL_SetTextureColorMod(world->lightTex, color.r, color.g, color.b);
	SDL_RenderCopy(renderer, world->lightTex, NULL, &dest);
}

void renderLighting(t_World* world, t_Map* map, t_Player* player, t_ProjectilePool* shots, t_Camera camera)
{
	SDL_Color ambient = getAmbientColor(world);
	t_Object* seed = GetSeed(map);
	float flicker = 1.0f + 0.06f * SDL_sinf(SDL_GetTicks() / 140.0f) + 0.03f * SDL_sinf(SDL_GetTicks() / 53.0f);
	float scaleX, scaleY;

	if (world->lightMap == NULL || (ambient.r == 255 && ambient.g == 255 && ambient.b == 255))
		return;

	SDL_RenderGetScale(renderer, &scaleX, &scaleY);
	SDL_SetRenderTarget(renderer, world->lightMap);
	SDL_SetRenderDrawColor(renderer, ambient.r, ambient.g, ambient.b, 255);
	SDL_RenderClear(renderer);

	if (seed->entity.hp > 0)
	{
		drawLight(world, seed->entity.rect.x + 8.0f - camera.viewport.x, seed->entity.rect.y + 8.0f - camera.viewport.y,
			GetSeedLightRadius(world) * 1.5f * flicker, seedGlow);
	}
	if (player->alive)
	{
		drawLight(world, player->entity.rect.x + 8.0f - camera.viewport.x, player->entity.rect.y + 8.0f - camera.viewport.y,
			38.0f, (SDL_Color){ 150, 135, 115, 255 });
	}
	for (int i = 0; i < MAX_PROJECTILES; i++)
	{
		t_Projectile* shot = &shots->shots[i];
		if (shot->active)
			drawLight(world, shot->entity.rect.x + 4.0f - camera.viewport.x, shot->entity.rect.y + 4.0f - camera.viewport.y,
				22.0f * flicker, (SDL_Color){ 255, 140, 50, 255 });
	}

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderSetScale(renderer, scaleX, scaleY);
	SDL_Rect dest = { 0, 0, world->lightMapW, world->lightMapH };
	SDL_RenderCopy(renderer, world->lightMap, NULL, &dest);
}

static void renderBar(int x, int y, int w, int h, float ratio, SDL_Color color)
{
	SDL_Rect back = { x, y, w, h };
	SDL_Rect front = { x + 1, y + 1, (int)((w - 2) * SDL_max(0.0f, SDL_min(1.0f, ratio))), h - 2 };
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 30, 14, 16, 220);
	SDL_RenderFillRect(renderer, &back);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
	SDL_RenderFillRect(renderer, &front);
}

void renderWorldHud(const t_World* world, t_Map* map, const t_Player* player)
{
	t_Object* seed = GetSeed(map);
	int screenW = world->lightMapW;
	int screenH = world->lightMapH;
	char text[64];

	// day / night clock
	if (world->phase == PHASE_NIGHT)
		snprintf(text, sizeof(text), "Night %d  -  %d leaves left", world->day, world->enemiesLeft);
	else if (world->phase == PHASE_DUSK)
		snprintf(text, sizeof(text), "Dusk %d", world->day);
	else if (world->phase == PHASE_DAWN)
		snprintf(text, sizeof(text), "Dawn %d", world->day + 1);
	else
		snprintf(text, sizeof(text), "Day %d", world->day);
	renderText(uiFont, text, screenW / 2, 4, white, ALIGN_CENTER);
	if (world->phase == PHASE_DAY)
		renderBar(screenW / 2 - 30, 15, 60, 3, world->phaseTimer / world->phaseLength, (SDL_Color){ 240, 190, 70, 255 });

	// heart seed health and glow
	snprintf(text, sizeof(text), "Heart Seed Lv %d", world->seedLevel);
	renderText(uiFont, text, screenW - 6, 4, seedGlow, ALIGN_RIGHT);
	renderBar(screenW - 66, 15, 60, 4, (float)(seed->entity.hp / world->seedMaxHp), (SDL_Color){ 200, 220, 80, 255 });
	renderBar(screenW - 66, 20, 60, 2, world->seedFuel / SEED_MAX_FUEL, (SDL_Color){ 255, 150, 50, 255 });

	// wind vane
	int cx = screenW - 14, cy = 34;
	float strength = SDL_sqrtf(world->windX * world->windX + world->windY * world->windY);
	renderText(uiFont, "wind", cx - 12, cy - 4, white, ALIGN_RIGHT);
	if (strength > 0.5f)
	{
		float dx = world->windX / strength, dy = world->windY / strength;
		float len = 3.0f + 6.0f * strength / WIND_MAX;
		int tipX = (int)(cx + dx * len), tipY = (int)(cy + dy * len);
		SDL_SetRenderDrawColor(renderer, 255, 225, 160, 255);
		SDL_RenderDrawLine(renderer, (int)(cx - dx * len), (int)(cy - dy * len), tipX, tipY);
		SDL_RenderDrawLine(renderer, tipX, tipY, (int)(tipX - dx * 3 - dy * 2), (int)(tipY - dy * 3 + dx * 2));
		SDL_RenderDrawLine(renderer, tipX, tipY, (int)(tipX - dx * 3 + dy * 2), (int)(tipY - dy * 3 - dx * 2));
	}

	if (!player->alive && seed->entity.hp > 0)
	{
		snprintf(text, sizeof(text), "Regrowing... %d", (int)SDL_ceilf(player->respawnTimer));
		renderText(uiFont, text, screenW / 2, screenH / 2 - 20, bannerColor, ALIGN_CENTER);
	}

	if (world->bannerTimer > 0)
	{
		SDL_Rect panel = { 40, 46, screenW - 80, 14 };
		renderPanel(panel, (Uint8)(180 * SDL_min(1.0f, world->bannerTimer)));
		renderText(uiFont, world->banner, screenW / 2, 48, bannerColor, ALIGN_CENTER);
	}
}
