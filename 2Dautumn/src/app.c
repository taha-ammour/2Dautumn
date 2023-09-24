#include "headerfile/GAME.h"
#include "headerfile/ENTITY.h"


int main(int argc, char* argv[]) {
	t_Player player;
	initPlayer(&player, "res/player.png");
	Initgame();
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
		}
		SDL_RenderClear(renderer);
		renderPlayer(&player);
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}