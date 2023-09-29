#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "headerfile/ENTITY.h"

void attackAnimation(const t_Entity* e, SDL_Rect* srcrect);
void idleAnimation(const t_Entity* e, SDL_Rect* srcrect);
void walkAnimation(const t_Entity* e, SDL_Rect* srcrect);

AnimationFunction animationFunctions[];