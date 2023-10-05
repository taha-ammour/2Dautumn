#include "headerfile/GAME.h"

SDL_Window* window;
SDL_Renderer* renderer;

bool quit = false;

void Initgame()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		return;
	if (TTF_Init() != 0)
		return;
	IMG_Init(IMG_INIT_PNG);
	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	window = SDL_CreateWindow("Aut RPG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOWWIDTH, WINDOWHEIGHT, SDL_WINDOW_SHOWN );
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

	if (!window)
	{
		TTF_Quit();
		SDL_Quit();
		return;
	}
	
	if (!renderer)
	{
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return;
	}

}

SDL_Texture* loadTexture(const char* filename) {
	SDL_Texture* texture = NULL;
	SDL_Surface* surface = IMG_Load(filename);
	if (surface != NULL) {
		texture = SDL_CreateTextureFromSurface(renderer, surface);
		if (texture == NULL) {
			fprintf(stderr, "Unable to create texture from surface: %s\n", SDL_GetError());
		}
		SDL_FreeSurface(surface);
	}
	else {
		fprintf(stderr, "Image not loading: %s, Error: %s\n", filename, IMG_GetError());
	}
	return texture;
}


float CalculateDeltaTime(Uint32* prevTime) {
	Uint32 currentTime = SDL_GetTicks();
	float deltaTime = (float)(currentTime - *prevTime) / 1000.0f; // Convert to seconds
	*prevTime = currentTime;
	return deltaTime;
}

float CalcDistance(SDL_Rect a, SDL_Rect b)
{
	return sqrtf((float)((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)));
}
