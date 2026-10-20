#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define WINDOWWIDTH 1024
#define WINDOWHEIGHT 600
#define BEST_SCORE_FILE "highscore.txt"

typedef enum {
	SCREEN_TITLE,
	SCREEN_PLAYING,
	SCREEN_PAUSED,
	SCREEN_GAMEOVER,
}GAME_SCREEN;

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern bool quit;

void Initgame();
SDL_Texture* loadTexture(const char* filename);
float CalculateDeltaTime(Uint32* prevTime);
float CalcDistance(SDL_Rect a, SDL_Rect b);
float RandomFloat(float min, float max);

int LoadBestScore(const char* filename);
void SaveBestScore(const char* filename, int score);
