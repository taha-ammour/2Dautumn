#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define WINDOWWIDTH 1024
#define WINDOWHEIGHT 600

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern bool quit;

void Initgame();
float CalculateDeltaTime(Uint32* prevTime);