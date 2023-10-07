#include "headerfile/GAME.h"
#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/COLLISION.h"
#include "headerfile/UIELEMENTS.h"


int main(int argc, char* argv[]) {
	srand((unsigned)time(NULL));
	t_Player* player = malloc( 1 *sizeof(t_Player));
	t_Map* Map = malloc(1 * sizeof(t_Map));
	t_Camera* camera = malloc(1 * sizeof(t_Camera));
	t_Ennemy* ennemy = malloc(1 * sizeof(t_Ennemy));
	t_Uihealth* health;
	t_inventory* playerInventory = (t_inventory*)malloc(sizeof(t_inventory));
	Uint32 prevTime = SDL_GetTicks();
	float deltaTime = 0.0f;
	if (player == NULL || ennemy == NULL || camera == NULL || Map == NULL) {
		fprintf(stderr, "Failed to allocate memory for 'player'.\n");
		return 1; 
	}
	Initgame();
	InitCamera(camera);
	InitTerrain(Map, "res/spring.png", "res/rock.png");
	InitEnnemy(ennemy, "res/ennemysprite.png", 3, 200);
	InitPlayer(player, "res/player.png", 3 , 200);
	InitHealth( "res/health.png", &player->entity, &health);
	InitInventory("res/inv.png","res/items.png","fonts/arial.ttf", playerInventory);
	while (!quit) {

		deltaTime = CalculateDeltaTime(&prevTime);

		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
			updateInventorySelection(&e, playerInventory);
			InteractPOBJ(&e,Map, player, playerInventory,* camera);
		}

		UpdateCameraPos(*player, camera);
		UpdatePlayerPos(player, 150, deltaTime);
		updateEnemyPos(&ennemy->entity, &player->entity, 90, deltaTime);
		updateHeartStates(health, player->entity);
		UpdateOBJState(Map,playerInventory);
		if (areEntitiesColliding(&player->entity, &ennemy->entity))
		{
			resolveCollision(&ennemy->entity,&player->entity);
			ennemy->entity.state = ATTACK;
		}
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			for (int x = 0; x < MAP_WIDTH; x++) {
				if (areEntitiesColliding(&player->entity, &Map->objectMap[y][x].entity))
					{
						resolveCollision(&player->entity, &Map->objectMap[y][x].entity);
					}
			}
		}
		


		SDL_RenderClear(renderer);
		renderTilemap(Map, *camera);
		renderOBJmap(Map, *camera);
		renderPlayer(player, *camera);
		renderEnnemy(ennemy, *camera);
		renderhealth(*player, health);
		renderInv(playerInventory);
		SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}
	
	
	free(playerInventory);
	free(health);
	free(ennemy);
	free(camera);
	free(Map);
	free(player);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}