#include "headerfile/GAME.h"
#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/COLLISION.h"
#include "headerfile/UIELEMENTS.h"
#include "headerfile/COMBAT.h"
#include "headerfile/WORLD.h"
#include "headerfile/PARTICLE.h"
#include "headerfile/CRAFT.h"
#include "headerfile/SOUND.h"

static const SDL_Color titleColor = { 255, 200, 90, 255 };
static const SDL_Color textColor = { 255, 240, 215, 255 };
static const SDL_Color hintColor = { 210, 180, 140, 255 };

typedef struct {
	t_Player* player;
	t_Map* map;
	t_Camera* camera;
	t_EnemyPool* enemies;
	t_ProjectilePool* shots;
	t_inventory* inventory;
	t_World* world;
	t_CraftMenu* craft;
	t_Uihealth* health;
	GAME_SCREEN screen;
	int bestScore;
	bool newBest;
}t_Game;

static void ResetGame(t_Game* game)
{
	generateTerrain(game->map);
	generateOBJ(game->map);
	ResetPlayer(game->player);
	ClearEnemies(game->enemies);
	ClearProjectiles(game->shots);
	ClearInventory(game->inventory);
	ClearParticles();
	ResetWorld(game->world, game->map);
	game->craft->open = false;
	game->newBest = false;
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
			if (game->craft->open && e->key.keysym.sym == SDLK_ESCAPE)
				game->craft->open = false;
			else
				game->screen = SCREEN_PAUSED;
			return;
		case SDLK_c:
		case SDLK_TAB:
			game->craft->open = !game->craft->open;
			return;
		case SDLK_e:
		case SDLK_f:
			FeedSeed(game->world, game->map, game->player, game->inventory);
			return;
		default:
			break;
		}
	}
	if (HandleCraftEvent(e, game->craft, game->inventory, game->camera->Zoom))
		return;
	updateInventorySelection(e, game->inventory);

	if (e->type == SDL_MOUSEBUTTONDOWN && game->player->alive)
	{
		ScreenToWorld(e->button.x, e->button.y, *game->camera, &worldX, &worldY);
		if (e->button.button == SDL_BUTTON_LEFT)
		{
			// a click hits a leaf spirit first, otherwise it mines what is under the cursor
			if (!PlayerMeleeAttack(game->enemies, game->player, game->inventory, worldX, worldY))
				InteractPOBJ(e, game->map, game->player, game->inventory, *game->camera);
		}
		else if (e->button.button == SDL_BUTTON_RIGHT)
		{
			ThrowEmber(game->shots, game->player, game->inventory, worldX, worldY);
		}
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
	case SCREEN_GAMEOVER:
		if (e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_r || e->key.keysym.sym == SDLK_RETURN))
		{
			ResetGame(game);
			game->screen = SCREEN_PLAYING;
		}
		else if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE)
		{
			game->screen = SCREEN_TITLE;
		}
		break;
	}
}

static void UpdatePlaying(t_Game* game, float deltaTime)
{
	t_Player* player = game->player;
	t_World* world = game->world;

	if (player->alive)
		UpdatePlayerPos(player, PLAYER_SPEED, deltaTime);
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++) {
			if (game->map->objectMap[y][x].TYPE != NONE_OBJECT && areEntitiesColliding(&player->entity, &game->map->objectMap[y][x].entity))
				{
					resolveCollision(&player->entity, &game->map->objectMap[y][x].entity);
				}
		}
	}

	UpdateWorld(world, game->map, game->enemies, player, deltaTime);
	UpdateEnemies(game->enemies, player, game->map, game->inventory, world->windX, world->windY,
		GetSeedLightRadius(world), GetSeedBurnDps(world), deltaTime);
	UpdateProjectiles(game->shots, game->enemies, game->map, game->inventory, world->windX, world->windY, deltaTime);
	UpdateOBJState(game->map, game->inventory);
	UpdatePlayerLife(world, game->map, player, deltaTime);
	updateHeartStates(game->health, player->entity);

	if (GetSeed(game->map)->entity.hp <= 0)
	{
		game->screen = SCREEN_GAMEOVER;
		game->craft->open = false;
		world->bannerTimer = 0;
		if (world->nightsSurvived > game->bestScore)
		{
			game->bestScore = world->nightsSurvived;
			game->newBest = true;
			SaveBestScore(BEST_SCORE_FILE, game->bestScore);
		}
	}
}

static void RenderOverlay(int w, int h, Uint8 alpha)
{
	SDL_Rect full = { 0, 0, w, h };
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 20, 8, 10, alpha);
	SDL_RenderFillRect(renderer, &full);
}

static void RenderScreens(t_Game* game)
{
	int w = game->camera->viewport.w;
	int h = game->camera->viewport.h;
	char text[64];

	switch (game->screen)
	{
	case SCREEN_TITLE:
		RenderOverlay(w, h, 150);
		renderText(uiFontBig, "Autumn Harvest", w / 2, 30, titleColor, ALIGN_CENTER);
		renderText(uiFont, "Guard the Heart Seed from the leaf spirits", w / 2, 54, textColor, ALIGN_CENTER);
		renderText(uiFont, "WASD move    Left click mine / strike    Right click throw ember", w / 2, 82, hintColor, ALIGN_CENTER);
		renderText(uiFont, "C craft    E feed wood to the seed    1-7 / wheel select", w / 2, 94, hintColor, ALIGN_CENTER);
		renderText(uiFont, "The leaves ride the wind: watch where it blows from", w / 2, 112, hintColor, ALIGN_CENTER);
		if (game->bestScore > 0)
		{
			snprintf(text, sizeof(text), "Best: %d nights survived", game->bestScore);
			renderText(uiFont, text, w / 2, 134, textColor, ALIGN_CENTER);
		}
		if ((SDL_GetTicks() / 500) % 2 == 0)
			renderText(uiFont, "Press ENTER to begin", w / 2, 156, titleColor, ALIGN_CENTER);
		break;
	case SCREEN_PAUSED:
		RenderOverlay(w, h, 140);
		renderText(uiFontBig, "Paused", w / 2, h / 2 - 30, titleColor, ALIGN_CENTER);
		renderText(uiFont, "ESC to resume    Q to quit to title", w / 2, h / 2, textColor, ALIGN_CENTER);
		break;
	case SCREEN_GAMEOVER:
		RenderOverlay(w, h, 170);
		renderText(uiFontBig, "The Heart Seed withered", w / 2, h / 2 - 40, titleColor, ALIGN_CENTER);
		snprintf(text, sizeof(text), "Nights survived: %d", game->world->nightsSurvived);
		renderText(uiFont, text, w / 2, h / 2 - 10, textColor, ALIGN_CENTER);
		if (game->newBest)
			snprintf(text, sizeof(text), "New best!");
		else
			snprintf(text, sizeof(text), "Best: %d", game->bestScore);
		renderText(uiFont, text, w / 2, h / 2 + 2, game->newBest ? titleColor : textColor, ALIGN_CENTER);
		renderText(uiFont, "R to try again    ESC for title", w / 2, h / 2 + 24, hintColor, ALIGN_CENTER);
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
	t_ProjectilePool* shots = malloc(sizeof(t_ProjectilePool));
	t_World* world = malloc(sizeof(t_World));
	t_CraftMenu craft;
	t_Uihealth* health;
	t_inventory* playerInventory = (t_inventory*)malloc(sizeof(t_inventory));
	Uint32 prevTime;
	float deltaTime = 0.0f;
	if (player == NULL || enemies == NULL || shots == NULL || world == NULL || camera == NULL || Map == NULL || playerInventory == NULL) {
		fprintf(stderr, "Failed to allocate memory for the game.\n");
		return 1;
	}
	Initgame();
	if (window == NULL || renderer == NULL) {
		fprintf(stderr, "Failed to create the window: %s\n", SDL_GetError());
		return 1;
	}
	InitSound();
	InitUIFonts("fonts/arial.ttf");
	InitCamera(camera);
	InitTerrain(Map, "res/spring.png", "res/rock.png");
	InitEnemyPool(enemies, "res/ennemysprite.png");
	InitProjectiles(shots, "res/attack.png");
	InitPlayer(player, "res/player.png", 3 , 200);
	InitHealth( "res/health.png", &player->entity, &health);
	InitInventory("res/inv.png","res/items.png","fonts/arial.ttf", playerInventory);
	InitWorld(world, camera);
	InitCraftMenu(&craft, camera->viewport.w, camera->viewport.h);
	InitLeaves();

	t_Game game = { player, Map, camera, enemies, shots, playerInventory, world, &craft, health,
		SCREEN_TITLE, LoadBestScore(BEST_SCORE_FILE), false };
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

		if (game.screen == SCREEN_PLAYING)
		{
			UpdatePlaying(&game, deltaTime);
		}
		else if (game.screen == SCREEN_TITLE)
		{
			UpdateWind(world, deltaTime);
		}
		if (game.screen != SCREEN_PAUSED)
		{
			UpdateParticles(world->windX, world->windY, deltaTime);
		}
		UpdateCameraPos(*player, camera, deltaTime);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		renderTilemap(Map, *camera);
		renderSeedAura(world, Map, *camera);
		renderOBJmap(Map, *camera);
		renderEnemies(enemies, *camera);
		renderPlayer(player, *camera);
		renderProjectiles(shots, *camera);
		renderLeaves(*camera);
		renderParticles(*camera);
		renderLighting(world, Map, player, shots, *camera);
		renderFloatTexts(*camera);
		if (game.screen != SCREEN_TITLE)
		{
			renderhealth(*player, health);
			renderInv(playerInventory);
			renderWorldHud(world, Map, player);
			renderCraftMenu(&craft, playerInventory);
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
	free(shots);
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
