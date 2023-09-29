#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "headerfile/ENTITY.h"

int areEntitiesColliding(const t_Entity* entity1, const t_Entity* entity2);
void resolveCollision(t_Entity* entity1, t_Entity* entity2);
