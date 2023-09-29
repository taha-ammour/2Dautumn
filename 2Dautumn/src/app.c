#include "headerfile/GAME.h"
#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/COLLISION.h"
#include "headerfile/UIELEMENTS.h"


int main(int argc, char* argv[]) {
	t_Player player;
	t_Map Map;
	t_Camera camera;
	t_Ennemy ennemy;
	t_Uihealth* health;
	Uint32 prevTime = SDL_GetTicks();
	float deltaTime = 0.0f;
	Initgame();
	InitCamera(&camera);
	InitTerrain(&Map, "res/spring.png");
	InitEnnemy(&ennemy, "res/ennemysprite.png", 3, 200);
	InitPlayer(&player, "res/player2.png");
	InitHealth( "res/health.png", &player.entity, &health);
	while (!quit) {

		deltaTime = CalculateDeltaTime(&prevTime);

		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
			if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_k) {
					player.entity.hp -= 10;
					printf("Health : %d\n", player.entity.hp);
				}
				if (e.key.keysym.sym == SDLK_n) {
					player.entity.hp += 10;
					printf("Health : %d\n", player.entity.hp);
				}
			}
		}

		UpdateCameraPos(player, &camera);
		UpdatePlayerPos(&player, 150, deltaTime);
		updateEnemyPos(&ennemy.entity, &player.entity, 90, deltaTime);
		updateHeartStates(health, player.entity);
		if (areEntitiesColliding(&player.entity, &ennemy.entity))
		{
			resolveCollision(&player.entity, &ennemy.entity);
			ennemy.entity.state = ATTACK;
		}
		


		SDL_RenderClear(renderer);
		renderTilemap(&Map, camera);
		renderPlayer(&player, camera);
		renderEnnemy(&ennemy, camera);
		renderhealth(player, health);
		SDL_RenderPresent(renderer);
	}
	free(health);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}