#include "headerfile/GAME.h"
#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/UIELEMENTS.h"
#include "headerfile/COMBAT.h"
#include "headerfile/WORLD.h"
#include "headerfile/PARTICLE.h"
#include "headerfile/CRAFT.h"
#include "headerfile/SOUND.h"

#define MAX_DRAWABLES 768
#define PLAYER_WAKE_TIME 3.0f
#define BRIDGE_REACH 40.0f

static const SDL_Color titleColor = { 255, 200, 90, 255 };
static const SDL_Color textColor = { 255, 240, 215, 255 };
static const SDL_Color hintColor = { 210, 180, 140, 255 };

typedef struct {
	t_Player* player;
	t_Map* map;
	t_Camera* camera;
	t_EnemyPool* enemies;
	t_inventory* inventory;
	t_World* world;
	t_CraftMenu* craft;
	t_Uihealth* health;
	GAME_SCREEN screen;
	int bestDay;
	bool newBest;
	bool showMap;
	bool mouseHeld;
	bool endingShown;
	float endingDelay;
}t_Game;

// anything drawn in the world, sorted by its feet so you can walk behind trees
typedef struct {
	int sortY;
	const t_Object* object;
	const t_Ennemy* enemy;
	const t_Player* player;
}t_Drawable;

static t_Drawable drawables[MAX_DRAWABLES];

static void SpawnAmbientMotes(t_Game* game, float deltaTime);

static void ResetGame(t_Game* game)
{
	t_Map* map = game->map;

	generateTerrain(map);
	generateOBJ(map);
	ResetPlayer(game->player, (float)(map->startX * TILE_SIZE), (float)(map->startY * TILE_SIZE));
	PopulateCreatures(game->enemies, map);
	ClearInventory(game->inventory);
	ClearParticles();
	ResetWorld(game->world, map);
	SnapCamera(*game->player, game->camera);
	InitLeaves(*game->camera);
	RevealAround(map, game->player->entity.posX + 8, game->player->entity.posY + 8);
	game->craft->open = false;
	game->showMap = false;
	game->mouseHeld = false;
	game->newBest = false;
	game->endingShown = false;
	game->endingDelay = 0;
}

static void UseItem(t_Game* game, int worldX, int worldY)
{
	t_Player* player = game->player;
	float px = player->entity.posX + 8, py = player->entity.posY + 10;

	if (TryPlantSeed(game->world, game->map, player, game->inventory))
		return;
	if (GetSelectedItem(game->inventory) != PLANK_TEX)
		return;
	// planks become a bridge on the water tile you point at, if it is close enough
	float dx = worldX - px, dy = worldY - py;
	if (dx * dx + dy * dy > BRIDGE_REACH * BRIDGE_REACH)
	{
		SpawnFloatText(px, py - 16, "Too far away", (SDL_Color){ 255, 200, 160, 255 });
		return;
	}
	if (PlaceBridge(game->map, worldX / TILE_SIZE, worldY / TILE_SIZE))
	{
		RemoveItem(game->inventory, PLANK_TEX, 1);
		SpawnBurst((float)(worldX / TILE_SIZE * TILE_SIZE + 8), (float)(worldY / TILE_SIZE * TILE_SIZE + 8), (SDL_Color){ 149, 113, 40, 255 }, 12, 40.0f);
		PlaySfx(SFX_BRIDGE);
	}
}

static void HandlePlayingEvent(t_Game* game, SDL_Event* e)
{
	int worldX, worldY;

	if (e->type == SDL_KEYDOWN && !e->key.repeat)
	{
		switch (e->key.keysym.sym)
		{
		case SDLK_ESCAPE:
		case SDLK_p:
			if (game->craft->open || game->showMap)
			{
				game->craft->open = false;
				game->showMap = false;
			}
			else
				game->screen = SCREEN_PAUSED;
			return;
		case SDLK_c:
		case SDLK_TAB:
			game->craft->open = !game->craft->open;
			game->showMap = false;
			return;
		case SDLK_m:
			game->showMap = !game->showMap;
			game->craft->open = false;
			return;
		case SDLK_e:
			TryPlantSeed(game->world, game->map, game->player, game->inventory);
			return;
		default:
			break;
		}
	}
	if (HandleCraftEvent(e, game->craft, game->inventory, game->camera->Zoom))
		return;
	updateInventorySelection(e, game->inventory);

	if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT)
		game->mouseHeld = false;
	if (e->type == SDL_MOUSEBUTTONDOWN && !game->craft->open && !game->showMap)
	{
		ScreenToWorld(e->button.x, e->button.y, *game->camera, &worldX, &worldY);
		if (e->button.button == SDL_BUTTON_LEFT)
			game->mouseHeld = true;
		else if (e->button.button == SDL_BUTTON_RIGHT)
			UseItem(game, worldX, worldY);
	}
}

static void HandleEvent(t_Game* game, SDL_Event* e)
{
	if (e->type == SDL_QUIT) {
		quit = 1;
		return;
	}
	switch (game->screen)
	{
	case SCREEN_TITLE:
		if ((e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_RETURN || e->key.keysym.sym == SDLK_SPACE)) ||
			e->type == SDL_MOUSEBUTTONDOWN)
		{
			ResetGame(game);
			game->screen = SCREEN_PLAYING;
		}
		else if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE)
		{
			quit = 1;
		}
		break;
	case SCREEN_PLAYING:
		HandlePlayingEvent(game, e);
		break;
	case SCREEN_PAUSED:
		if (e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_ESCAPE || e->key.keysym.sym == SDLK_p))
			game->screen = SCREEN_PLAYING;
		else if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_q)
			game->screen = SCREEN_TITLE;
		break;
	case SCREEN_ENDING:
		if (e->type != SDL_KEYDOWN)
			break;
		if (game->world->finished && e->key.keysym.sym == SDLK_RETURN)
			game->screen = SCREEN_PLAYING;
		else if (!game->world->finished && (e->key.keysym.sym == SDLK_r || e->key.keysym.sym == SDLK_RETURN))
		{
			ResetGame(game);
			game->screen = SCREEN_PLAYING;
		}
		else if (e->key.keysym.sym == SDLK_ESCAPE)
			game->screen = SCREEN_TITLE;
		break;
	}
}

static void UpdatePlaying(t_Game* game, float deltaTime)
{
	t_Player* player = game->player;
	t_World* world = game->world;
	int mouseX, mouseY, worldX, worldY;

	// the crafting menu and the map pause the world
	if (game->craft->open || game->showMap)
		return;

	if (player->alive)
	{
		UpdatePlayerPos(player, game->map, PLAYER_SPEED, deltaTime);
		UpdatePlayerHealth(player, deltaTime);
		const Uint8* keys = SDL_GetKeyboardState(NULL);
		if (game->mouseHeld)
		{
			// hold the button to keep swinging towards the cursor
			SDL_GetMouseState(&mouseX, &mouseY);
			ScreenToWorld(mouseX, mouseY, *game->camera, &worldX, &worldY);
			PlayerSwing(game->enemies, player, game->map, game->inventory,
				worldX - (player->entity.posX + 8), worldY - (player->entity.posY + 9));
		}
		else if (keys[SDL_SCANCODE_SPACE])
		{
			PlayerSwing(game->enemies, player, game->map, game->inventory, 0, 0);
		}
	}

	UpdateEnemies(game->enemies, player, game->map, world->windX, world->windY, deltaTime);
	UpdateOBJState(game->map, player);
	UpdatePickups(player, game->inventory, deltaTime);
	UpdateWorld(world, game->map, player, game->inventory, deltaTime);
	updateHeartStates(game->health, player->entity);
	SpawnAmbientMotes(game, deltaTime);

	if (player->alive && player->entity.hp <= 0)
	{
		// no game over: the stump just wakes up back at camp
		player->alive = false;
		player->respawnTimer = PLAYER_WAKE_TIME;
		SpawnBurst(player->entity.posX + 8, player->entity.posY + 8, (SDL_Color){ 150, 110, 40, 255 }, 24, 70.0f);
		AddScreenShake(4.0f);
		CalmCreatures(game->enemies);
	}
	else if (!player->alive)
	{
		player->respawnTimer -= deltaTime;
		if (player->respawnTimer <= 0)
		{
			ResetPlayer(player, (float)(game->map->startX * TILE_SIZE), (float)(game->map->startY * TILE_SIZE));
			player->entity.hurtTime = SDL_GetTicks();
			SnapCamera(*player, game->camera);
		}
	}

	if (world->winter)
	{
		game->screen = SCREEN_ENDING;
	}
	else if (world->finished && !game->endingShown)
	{
		// let the sapling sparkle for a moment before the ending card
		game->endingDelay += deltaTime;
		if (game->endingDelay > 2.5f)
		{
			game->endingShown = true;
			game->screen = SCREEN_ENDING;
			if (game->bestDay == 0 || world->finishDay < game->bestDay)
			{
				game->bestDay = world->finishDay;
				game->newBest = true;
				SaveBestScore(BEST_SCORE_FILE, game->bestDay);
			}
		}
	}
}

static int compareDrawables(const void* a, const void* b)
{
	return ((const t_Drawable*)a)->sortY - ((const t_Drawable*)b)->sortY;
}

static void RenderWorldObjects(t_Game* game)
{
	t_Camera camera = *game->camera;
	int count = 0;
	int x0 = SDL_max(0, camera.viewport.x / TILE_SIZE - 1);
	int y0 = SDL_max(0, camera.viewport.y / TILE_SIZE - 1);
	int x1 = SDL_min(MAP_WIDTH - 1, (camera.viewport.x + camera.viewport.w) / TILE_SIZE + 1);
	int y1 = SDL_min(MAP_HEIGHT - 1, (camera.viewport.y + camera.viewport.h) / TILE_SIZE + 1);

	for (int y = y0; y <= y1; y++)
	{
		for (int x = x0; x <= x1 && count < MAX_DRAWABLES - MAX_ENEMIES - 1; x++)
		{
			const t_Object* obj = &game->map->objectMap[y][x];
			if (obj->TYPE == NONE_OBJECT || obj->entity.hp <= 0)
				continue;
			// leaf piles lie flat on the ground, everything walks over them
			int sortY = obj->entity.rect.y + (obj->TYPE == LEAFPILE_OBJECT ? 0 : 15);
			drawables[count++] = (t_Drawable){ sortY, obj, NULL, NULL };
		}
	}
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		const t_Ennemy* e = &game->enemies->enemies[i];
		if (!e->alive || !SDL_HasIntersection(&e->entity.rect, &(SDL_Rect){ camera.viewport.x - 16, camera.viewport.y - 16, camera.viewport.w + 32, camera.viewport.h + 32 }))
			continue;
		drawables[count++] = (t_Drawable){ e->entity.rect.y + 15, NULL, e, NULL };
	}
	drawables[count++] = (t_Drawable){ game->player->entity.rect.y + 15, NULL, NULL, game->player };

	qsort(drawables, count, sizeof(t_Drawable), compareDrawables);
	for (int i = 0; i < count; i++)
	{
		if (drawables[i].object)
			renderObject(game->map, drawables[i].object, camera);
		else if (drawables[i].enemy)
			renderEnnemy(drawables[i].enemy, camera);
		else
			renderPlayer(drawables[i].player, camera);
	}
}

// holding planks shows where the next one would go
static void RenderPlankPreview(t_Game* game)
{
	int mouseX, mouseY, worldX, worldY;
	t_Player* player = game->player;

	if (game->screen != SCREEN_PLAYING || game->craft->open || game->showMap || !player->alive ||
		GetSelectedItem(game->inventory) != PLANK_TEX)
		return;
	SDL_GetMouseState(&mouseX, &mouseY);
	ScreenToWorld(mouseX, mouseY, *game->camera, &worldX, &worldY);
	int tx = worldX / TILE_SIZE, ty = worldY / TILE_SIZE;
	if (tx < 0 || ty < 0 || tx >= MAP_WIDTH || ty >= MAP_HEIGHT)
		return;
	float dx = worldX - (player->entity.posX + 8), dy = worldY - (player->entity.posY + 10);
	bool valid = game->map->ground[ty][tx] == GROUND_WATER && dx * dx + dy * dy <= BRIDGE_REACH * BRIDGE_REACH;

	SDL_Rect r = { tx * TILE_SIZE - game->camera->viewport.x, ty * TILE_SIZE - game->camera->viewport.y, TILE_SIZE, TILE_SIZE };
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	if (valid)
		SDL_SetRenderDrawColor(renderer, 160, 255, 140, 90);
	else
		SDL_SetRenderDrawColor(renderer, 255, 110, 90, 60);
	SDL_RenderFillRect(renderer, &r);
	SDL_SetRenderDrawColor(renderer, valid ? 200 : 255, valid ? 255 : 140, valid ? 190 : 120, 200);
	SDL_RenderDrawRect(renderer, &r);
}

// golden specks drift up in the glade and around the planted seed
static void SpawnAmbientMotes(t_Game* game, float deltaTime)
{
	t_Camera* camera = game->camera;
	t_Map* map = game->map;
	SDL_Color glow = { 255, 229, 140, 255 };

	if (RandomFloat(0.0f, 1.0f) < deltaTime * 14.0f)
	{
		float x = camera->viewport.x + RandomFloat(0.0f, (float)camera->viewport.w);
		float y = camera->viewport.y + RandomFloat(0.0f, (float)camera->viewport.h);
		if (GetRegionAt(map, x, y) == REGION_GLADE)
			SpawnMote(x, y, glow);
	}
	if (map->planted && RandomFloat(0.0f, 1.0f) < deltaTime * 6.0f)
		SpawnMote(map->soilX * TILE_SIZE + RandomFloat(2.0f, 14.0f), map->soilY * TILE_SIZE + RandomFloat(0.0f, 10.0f), glow);
}

static void RenderOverlay(int w, int h, Uint8 alpha)
{
	SDL_Rect full = { 0, 0, w, h };
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 20, 8, 10, alpha);
	SDL_RenderFillRect(renderer, &full);
}

static void RenderBigMap(t_Game* game)
{
	int w = game->camera->viewport.w;
	int h = game->camera->viewport.h;
	SDL_Rect dest = { (WINDOWWIDTH - 640) / 2, 70, 640, 400 };

	RenderOverlay(w, h, 150);
	renderText(uiFont, "Map", w / 2, 6, titleColor, ALIGN_CENTER);
	renderMinimap(game->map, game->player, dest, true);
	renderText(uiFontSmall, "M or Esc to close", w / 2, 160, hintColor, ALIGN_CENTER);
}

static void RenderScreens(t_Game* game)
{
	int w = game->camera->viewport.w;
	int h = game->camera->viewport.h;
	char text[96];

	switch (game->screen)
	{
	case SCREEN_TITLE:
		RenderOverlay(w, h, 160);
		renderText(uiFontBig, "Autumn Harvest", w / 2, 20, titleColor, ALIGN_CENTER);
		renderText(uiFont, "Find a home for the Heart Seed before winter", w / 2, 46, textColor, ALIGN_CENTER);
		renderText(uiFontSmall, "WASD  move", w / 2, 72, hintColor, ALIGN_CENTER);
		renderText(uiFontSmall, "Left mouse (hold)  swing at what you point at   -   Space  swing forward", w / 2, 82, hintColor, ALIGN_CENTER);
		renderText(uiFontSmall, "Right mouse  place a plank on water   -   E  plant the seed", w / 2, 92, hintColor, ALIGN_CENTER);
		renderText(uiFontSmall, "C  craft   -   M  map   -   1-0 / wheel  pick an item   -   Esc  pause", w / 2, 102, hintColor, ALIGN_CENTER);
		renderText(uiFontSmall, "Leaf spirits are peaceful. They only fight back if you hit them.", w / 2, 120, textColor, ALIGN_CENTER);
		if (game->bestDay > 0)
		{
			snprintf(text, sizeof(text), "Best: seed planted on day %d", game->bestDay);
			renderText(uiFontSmall, text, w / 2, 138, textColor, ALIGN_CENTER);
		}
		if ((SDL_GetTicks() / 500) % 2 == 0)
			renderText(uiFont, "Press Enter to begin", w / 2, 160, titleColor, ALIGN_CENTER);
		break;
	case SCREEN_PAUSED:
		RenderOverlay(w, h, 140);
		renderText(uiFontBig, "Paused", w / 2, h / 2 - 30, titleColor, ALIGN_CENTER);
		renderText(uiFont, "Esc to resume    Q to quit to title", w / 2, h / 2 + 4, textColor, ALIGN_CENTER);
		break;
	case SCREEN_ENDING:
		RenderOverlay(w, h, 170);
		if (game->world->finished)
		{
			renderText(uiFontBig, "Spring will remember you", w / 2, h / 2 - 50, titleColor, ALIGN_CENTER);
			snprintf(text, sizeof(text), "The Heart Seed took root on day %d of %d.", game->world->finishDay, SEASON_DAYS);
			renderText(uiFont, text, w / 2, h / 2 - 16, textColor, ALIGN_CENTER);
			if (game->newBest)
				renderText(uiFont, "Your fastest yet!", w / 2, h / 2 - 2, titleColor, ALIGN_CENTER);
			else if (game->bestDay > 0)
			{
				snprintf(text, sizeof(text), "Best: day %d", game->bestDay);
				renderText(uiFont, text, w / 2, h / 2 - 2, textColor, ALIGN_CENTER);
			}
			renderText(uiFontSmall, "Enter  keep exploring      Esc  title", w / 2, h / 2 + 24, hintColor, ALIGN_CENTER);
		}
		else
		{
			renderText(uiFontBig, "Winter came", w / 2, h / 2 - 50, titleColor, ALIGN_CENTER);
			renderText(uiFont, "The Heart Seed sleeps under the snow, waiting for another autumn.", w / 2, h / 2 - 16, textColor, ALIGN_CENTER);
			renderText(uiFontSmall, "R  try again      Esc  title", w / 2, h / 2 + 10, hintColor, ALIGN_CENTER);
		}
		break;
	default:
		break;
	}
}


int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	srand((unsigned)time(NULL));
	t_Player* player = malloc( 1 *sizeof(t_Player));
	t_Map* Map = malloc(1 * sizeof(t_Map));
	t_Camera* camera = malloc(1 * sizeof(t_Camera));
	t_EnemyPool* enemies = malloc(sizeof(t_EnemyPool));
	t_World* world = malloc(sizeof(t_World));
	t_CraftMenu craft;
	t_Uihealth* health;
	t_inventory* playerInventory = (t_inventory*)malloc(sizeof(t_inventory));
	Uint32 prevTime;
	float deltaTime = 0.0f;
	if (player == NULL || enemies == NULL || world == NULL || camera == NULL || Map == NULL || playerInventory == NULL) {
		fprintf(stderr, "Failed to allocate memory for the game.\n");
		return 1;
	}
	Initgame();
	if (window == NULL || renderer == NULL) {
		fprintf(stderr, "Failed to create the window: %s\n", SDL_GetError());
		return 1;
	}
	InitSound();
	InitUIFonts("fonts/PixelifySans.ttf");
	InitCamera(camera);
	InitTerrain(Map, "res/spring.png", "res/rock.png", "res/objects.png", "res/terrain.png");
	InitEnemyPool(enemies, "res/ennemysprite.png");
	InitPlayer(player, "res/player.png", 3 , 200);
	InitHealth( "res/health.png", &player->entity, &health);
	InitInventory("res/inv.png","res/items.png","fonts/PixelifySans.ttf", playerInventory);
	InitCraftMenu(&craft, camera->viewport.w, camera->viewport.h);

	t_Game game = { player, Map, camera, enemies, playerInventory, world, &craft, health,
		SCREEN_TITLE, LoadBestScore(BEST_SCORE_FILE), false, false, false, false, 0 };
	ResetGame(&game);

	prevTime = SDL_GetTicks();
	while (!quit) {
		Uint32 frameStart = SDL_GetTicks();

		deltaTime = CalculateDeltaTime(&prevTime);
		// avoid huge jumps after the window was dragged or the game stalled
		if (deltaTime > 0.05f)
			deltaTime = 0.05f;

		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			HandleEvent(&game, &e);
		}

		bool frozen = game.screen == SCREEN_PAUSED || game.craft->open || game.showMap;
		if (game.screen == SCREEN_PLAYING)
			UpdatePlaying(&game, deltaTime);
		else if (game.screen != SCREEN_PAUSED)
			UpdateWind(world, deltaTime);
		if (!frozen)
		{
			UpdateParticles(world->windX, world->windY, deltaTime);
			UpdateLeaves(*camera, world->windX, world->windY, GetSnowAmount(world), deltaTime);
		}
		UpdateCameraPos(*player, camera, frozen ? 0.0f : deltaTime);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		renderTilemap(Map, *camera);
		RenderWorldObjects(&game);
		renderPickups(playerInventory, *camera);
		renderSwings(*camera);
		renderParticles(*camera);
		renderLeaves(*camera, GetSnowAmount(world));
		renderSeasonTint(world, *camera);
		renderFloatTexts(*camera);
		if (game.screen != SCREEN_TITLE)
		{
			RenderPlankPreview(&game);
			bool readingSign = game.screen == SCREEN_PLAYING && renderSignBubble(Map, player, *camera);
			renderhealth(*player, health);
			renderInv(playerInventory, camera->viewport.w, camera->viewport.h);
			renderWorldHud(world, Map, player, *camera, readingSign || game.screen == SCREEN_ENDING);
			renderCraftMenu(&craft, playerInventory);
			if (game.showMap)
				RenderBigMap(&game);
		}
		RenderScreens(&game);
		SDL_RenderPresent(renderer);

		// cap at ~60 fps when vsync is not available
		Uint32 frameTime = SDL_GetTicks() - frameStart;
		if (frameTime < 16)
			SDL_Delay(16 - frameTime);
	}


	FreeSound();
	free(playerInventory);
	free(health);
	free(enemies);
	free(world);
	free(camera);
	free(Map);
	free(player);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	Mix_CloseAudio();
	TTF_Quit();
	SDL_Quit();
	return 0;
}
