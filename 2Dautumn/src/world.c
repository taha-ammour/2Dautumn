#include "headerfile/WORLD.h"
#include "headerfile/PARTICLE.h"
#include "headerfile/SOUND.h"

static const SDL_Color white = { 255, 255, 255, 255 };
static const SDL_Color gold = { 255, 215, 120, 255 };
static const SDL_Color cream = { 255, 240, 215, 255 };
static const SDL_Color seedGlow = { 255, 229, 140, 255 };

static const char* regionFlavor[REGION_COUNT] = {
	"Home camp. The Heart Seed is warm in your pocket.",
	"Leaf spirits rustle between the trees.",
	"Cold stone and stubborn rocks.",
	"Still water mirrors the falling leaves.",
	"Moss grows thick on the old stones.",
	"Sunlight pours in. This is the place.",
};

static const char* questText[QUEST_DONE + 1] = {
	"Chop trees for wood, then craft a Wood Pick (C)",
	"Break the boulders east of camp to reach the Rocky Hills",
	"Mine stone in the hills and craft a Stone Pick",
	"Craft planks and bridge the pond to Willow Isle",
	"Gather moss from the mossy stones on Willow Isle",
	"Craft Compost from 4 leaves and 1 moss",
	"Break the granite wall in the hills to find the Sunlit Glade",
	"Plant the Heart Seed in the glowing soil (E)",
	"The Heart Seed is planted. Explore as you like!",
};

void ShowBanner(t_World* world, const char* title, const char* subtitle)
{
	snprintf(world->banner, sizeof(world->banner), "%s", title);
	snprintf(world->bannerSub, sizeof(world->bannerSub), "%s", subtitle);
	world->bannerTimer = 4.0f;
}

static void pickWindTarget(t_World* world)
{
	float angle = RandomFloat(0.0f, 6.2831f);
	float strength = RandomFloat(4.0f, WIND_MAX);
	world->windTargetX = SDL_cosf(angle) * strength;
	world->windTargetY = SDL_sinf(angle) * strength * 0.5f;
	world->windTimer = RandomFloat(10.0f, 20.0f);
}

void ResetWorld(t_World* world, t_Map* map)
{
	world->time = 0;
	world->day = 1;
	for (int i = 0; i < REGION_COUNT; i++)
		world->visited[i] = false;
	world->currentRegion = REGION_MEADOW;
	world->visited[REGION_MEADOW] = true;
	world->winter = false;
	world->finished = false;
	world->finishDay = 0;
	world->quest = QUEST_PICK;
	pickWindTarget(world);
	world->windX = world->windTargetX;
	world->windY = world->windTargetY;
	(void)map;
	ShowBanner(world, "Maple Meadow", "Autumn, day 1. Find a home for the Heart Seed before winter.");
}

void UpdateWind(t_World* world, float deltaTime)
{
	world->windTimer -= deltaTime;
	if (world->windTimer <= 0)
		pickWindTarget(world);
	world->windX += (world->windTargetX - world->windX) * SDL_min(1.0f, deltaTime * 0.4f);
	world->windY += (world->windTargetY - world->windY) * SDL_min(1.0f, deltaTime * 0.4f);
}

static QUEST_STEP currentQuest(const t_World* world, const t_Map* map, const t_inventory* inv)
{
	bool hasStonePick = CountItem(inv, STONE_PICK_TEX) > 0;
	bool hasPick = hasStonePick || CountItem(inv, WOOD_PICK_TEX) > 0;
	bool hasCompost = CountItem(inv, COMPOST_TEX) > 0;
	bool hasMoss = hasCompost || CountItem(inv, MOSS_TEX) > 0;

	if (map->planted)
		return QUEST_DONE;
	if (!hasPick && !world->visited[REGION_HILLS])
		return QUEST_PICK;
	if (!world->visited[REGION_HILLS])
		return QUEST_HILLS;
	if (!hasStonePick && !world->visited[REGION_GLADE])
		return QUEST_STONE_PICK;
	if (!hasMoss)
		return world->visited[REGION_ISLAND] ? QUEST_MOSS : QUEST_BRIDGE;
	if (!hasCompost)
		return QUEST_COMPOST;
	if (!world->visited[REGION_GLADE])
		return QUEST_GLADE;
	return QUEST_PLANT;
}

void UpdateWorld(t_World* world, t_Map* map, t_Player* player, t_inventory* inv, float deltaTime)
{
	UpdateWind(world, deltaTime);
	if (world->bannerTimer > 0)
		world->bannerTimer -= deltaTime;

	// the season only runs until the seed has found its home
	if (!world->finished)
	{
		world->time += deltaTime;
		int day = (int)(world->time / DAY_LENGTH) + 1;
		if (day != world->day && day <= SEASON_DAYS)
		{
			char text[96];
			world->day = day;
			if (day == SEASON_DAYS)
				snprintf(text, sizeof(text), "Winter arrives tonight. Hurry!");
			else if (day == SNOW_FROM_DAY)
				snprintf(text, sizeof(text), "The first snowflakes are falling. %d days until winter.", SEASON_DAYS - day + 1);
			else
				snprintf(text, sizeof(text), "%d days until winter.", SEASON_DAYS - day + 1);
			ShowBanner(world, day == SEASON_DAYS ? "Last day of autumn" : "A new day", text);
		}
		if (world->time >= SEASON_DAYS * DAY_LENGTH)
			world->winter = true;
	}

	if (player->alive)
	{
		REGION region = GetRegionAt(map, player->entity.posX + 8, player->entity.posY + 12);
		RevealAround(map, player->entity.posX + 8, player->entity.posY + 8);
		if (region != world->currentRegion)
		{
			world->currentRegion = region;
			if (!world->visited[region])
			{
				world->visited[region] = true;
				ShowBanner(world, GetRegionName(region), regionFlavor[region]);
				PlaySfx(SFX_DISCOVER);
			}
		}
	}
	world->quest = currentQuest(world, map, inv);
}

bool TryPlantSeed(t_World* world, t_Map* map, t_Player* player, t_inventory* inv)
{
	float sx = map->soilX * TILE_SIZE + 8.0f;
	float sy = map->soilY * TILE_SIZE + 8.0f;
	float dx = player->entity.posX + 8 - sx;
	float dy = player->entity.posY + 10 - sy;

	if (map->planted || !player->alive || dx * dx + dy * dy > PLANT_RANGE * PLANT_RANGE)
		return false;
	if (!RemoveItem(inv, COMPOST_TEX, 1))
	{
		SpawnFloatText(sx, sy - 12, "The soil needs compost first", (SDL_Color){ 255, 200, 160, 255 });
		return false;
	}
	PlantSeed(map);
	world->finished = true;
	world->finishDay = world->day;
	world->quest = QUEST_DONE;
	SpawnBurst(sx, sy, seedGlow, 40, 90.0f);
	SpawnBurst(sx, sy, (SDL_Color){ 164, 165, 30, 255 }, 20, 60.0f);
	AddScreenShake(2.0f);
	PlaySfx(SFX_PLANT);
	return true;
}

float GetSnowAmount(const t_World* world)
{
	float snowStart = (SNOW_FROM_DAY - 1) * DAY_LENGTH;
	float end = SEASON_DAYS * DAY_LENGTH;
	if (world->finished || world->time < snowStart)
		return 0.0f;
	return SDL_min(1.0f, (world->time - snowStart) / (end - snowStart)) * 0.8f;
}

static SDL_Color lerpColor(SDL_Color a, SDL_Color b, float t)
{
	SDL_Color c;
	t = SDL_clamp(t, 0.0f, 1.0f);
	c.r = (Uint8)(a.r + (b.r - a.r) * t);
	c.g = (Uint8)(a.g + (b.g - a.g) * t);
	c.b = (Uint8)(a.b + (b.b - a.b) * t);
	c.a = 255;
	return c;
}

void renderSeasonTint(const t_World* world, t_Camera camera)
{
	float season = world->time / (SEASON_DAYS * DAY_LENGTH);
	float dayPhase = SDL_fmodf(world->time, DAY_LENGTH) / DAY_LENGTH;
	SDL_Color tint;

	// golden early autumn, deeper amber later, cold blue just before winter
	if (season < 0.6f)
		tint = lerpColor(white, (SDL_Color){ 255, 242, 220, 255 }, season / 0.6f);
	else
		tint = lerpColor((SDL_Color){ 255, 242, 220, 255 }, (SDL_Color){ 200, 215, 245, 255 }, (season - 0.6f) / 0.4f);
	// each day ends in a soft evening glow, then morning comes back
	if (!world->finished && dayPhase > 0.8f)
	{
		float evening = dayPhase < 0.95f ? (dayPhase - 0.8f) / 0.15f : (1.0f - dayPhase) / 0.05f;
		tint = lerpColor(tint, (SDL_Color){ 230, 190, 175, 255 }, evening * 0.8f);
	}
	if (tint.r == 255 && tint.g == 255 && tint.b == 255)
		return;

	SDL_Rect full = { 0, 0, camera.viewport.w, camera.viewport.h };
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MOD);
	SDL_SetRenderDrawColor(renderer, tint.r, tint.g, tint.b, 255);
	SDL_RenderFillRect(renderer, &full);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

// a speech-bubble style panel with wrapped text, anchored above a point in world units
static void renderBubble(const char* text, int centerX, int bottomY, int wrapWidth)
{
	float scaleX, scaleY;
	SDL_RenderGetScale(renderer, &scaleX, &scaleY);
	SDL_Surface* surface = TTF_RenderUTF8_Solid_Wrapped(uiFontSmall, text, cream, (Uint32)(wrapWidth * scaleX));
	if (surface == NULL)
		return;
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	int padding = 10;
	SDL_Rect box = { (int)(centerX * scaleX) - surface->w / 2 - padding, (int)(bottomY * scaleY) - surface->h - padding * 2,
		surface->w + padding * 2, surface->h + padding * 2 };
	box.x = SDL_clamp(box.x, 4, WINDOWWIDTH - box.w - 4);
	box.y = SDL_max(box.y, 4);
	SDL_Rect textRect = { box.x + padding, box.y + padding, surface->w, surface->h };

	SDL_RenderSetScale(renderer, 1.0f, 1.0f);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 30, 14, 16, 225);
	SDL_RenderFillRect(renderer, &box);
	SDL_SetRenderDrawColor(renderer, 150, 100, 45, 255);
	SDL_RenderDrawRect(renderer, &box);
	SDL_RenderCopy(renderer, texture, NULL, &textRect);
	SDL_RenderSetScale(renderer, scaleX, scaleY);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

bool renderSignBubble(t_Map* map, const t_Player* player, t_Camera camera)
{
	int tx = (int)((player->entity.posX + 8) / TILE_SIZE);
	int ty = (int)((player->entity.posY + 8) / TILE_SIZE);

	if (!player->alive)
		return false;
	for (int y = ty - 2; y <= ty + 2; y++)
	{
		for (int x = tx - 2; x <= tx + 2; x++)
		{
			if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT || map->objectMap[y][x].TYPE != SIGN_OBJECT)
				continue;
			float dx = x * TILE_SIZE + 8 - (player->entity.posX + 8);
			float dy = y * TILE_SIZE + 8 - (player->entity.posY + 8);
			if (dx * dx + dy * dy > 26 * 26)
				continue;
			renderBubble(GetSignText(map->objectMap[y][x].variant), x * TILE_SIZE + 8 - camera.viewport.x, y * TILE_SIZE - camera.viewport.y - 2, 150);
			return true;
		}
	}
	return false;
}

static bool questTarget(const t_World* world, const t_Map* map, float* x, float* y)
{
	int tx, ty;
	switch (world->quest)
	{
	case QUEST_HILLS: tx = map->bouldersX; ty = map->bouldersY; break;
	case QUEST_BRIDGE:
	case QUEST_MOSS: tx = map->islandX; ty = map->islandY; break;
	case QUEST_GLADE: tx = map->graniteX; ty = map->graniteY; break;
	case QUEST_PLANT: tx = map->soilX; ty = map->soilY; break;
	default: return false;
	}
	*x = tx * TILE_SIZE + 8.0f;
	*y = ty * TILE_SIZE + 8.0f;
	return true;
}

// a small golden arrow at the edge of the screen pointing to the current goal
static void renderQuestPointer(const t_World* world, const t_Map* map, t_Camera camera)
{
	float gx, gy;
	if (!questTarget(world, map, &gx, &gy))
		return;
	float sx = gx - camera.viewport.x;
	float sy = gy - camera.viewport.y;
	if (sx > 8 && sy > 8 && sx < camera.viewport.w - 8 && sy < camera.viewport.h - 8)
		return;

	float cx = camera.viewport.w / 2.0f, cy = camera.viewport.h / 2.0f;
	float dx = sx - cx, dy = sy - cy;
	float len = SDL_sqrtf(dx * dx + dy * dy);
	dx /= len;
	dy /= len;
	float t = SDL_min((cx - 14) / SDL_max(0.001f, SDL_fabsf(dx)), (cy - 14) / SDL_max(0.001f, SDL_fabsf(dy)));
	float px = cx + dx * t, py = cy + dy * t;
	float pulse = 1.0f + 0.15f * SDL_sinf(SDL_GetTicks() / 150.0f);

	SDL_Vertex v[3];
	SDL_Color c = { 255, 215, 120, 230 };
	v[0].position = (SDL_FPoint){ px + dx * 6 * pulse, py + dy * 6 * pulse };
	v[1].position = (SDL_FPoint){ px - dy * 4 - dx * 3, py + dx * 4 - dy * 3 };
	v[2].position = (SDL_FPoint){ px + dy * 4 - dx * 3, py - dx * 4 - dy * 3 };
	for (int i = 0; i < 3; i++)
	{
		v[i].color = c;
		v[i].tex_coord = (SDL_FPoint){ 0, 0 };
	}
	SDL_RenderGeometry(renderer, NULL, v, 3, NULL, 0);
}

void renderWorldHud(const t_World* world, t_Map* map, const t_Player* player, t_Camera camera, bool hideBanner)
{
	int screenW = camera.viewport.w;
	char text[96];
	float scaleX, scaleY;
	SDL_RenderGetScale(renderer, &scaleX, &scaleY);

	renderQuestPointer(world, map, camera);

	// current goal, under the hearts
	snprintf(text, sizeof(text), "%s", questText[world->quest]);
	SDL_Rect goalPanel = { 4, 17, TextWidth(uiFontSmall, text) + 8, 10 };
	renderPanel(goalPanel, 170);
	renderText(uiFontSmall, text, 8, 18, gold, ALIGN_LEFT);

	// minimap and calendar in the top right corner
	SDL_Rect mini = { WINDOWWIDTH - 170, 12, 160, 100 };
	renderMinimap(map, player, mini, false);
	if (world->finished)
		snprintf(text, sizeof(text), "Seed planted on day %d", world->finishDay);
	else
		snprintf(text, sizeof(text), "Autumn  -  day %d of %d", world->day, SEASON_DAYS);
	renderText(uiFontSmall, text, screenW - 4, (int)(118 / scaleY), cream, ALIGN_RIGHT);
	renderText(uiFontSmall, GetRegionName(world->currentRegion), screenW - 4, (int)(140 / scaleY), gold, ALIGN_RIGHT);

	if (!player->alive)
		renderText(uiFont, "You wake up back at camp...", screenW / 2, camera.viewport.h / 2 - 20, cream, ALIGN_CENTER);

	if (world->bannerTimer > 0 && !hideBanner)
	{
		Uint8 alpha = (Uint8)(200 * SDL_min(1.0f, world->bannerTimer));
		int width = SDL_max(TextWidth(uiFont, world->banner), TextWidth(uiFontSmall, world->bannerSub)) + 16;
		SDL_Rect panel = { screenW / 2 - width / 2, 44, width, 24 };
		renderPanel(panel, alpha);
		renderText(uiFont, world->banner, screenW / 2, 46, gold, ALIGN_CENTER);
		renderText(uiFontSmall, world->bannerSub, screenW / 2, 57, cream, ALIGN_CENTER);
	}
}
