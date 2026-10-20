#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

// every sound is synthesized at startup, no audio file is needed
typedef enum {
	SFX_HIT,
	SFX_HARVEST,
	SFX_CLINK,
	SFX_PICKUP,
	SFX_HURT,
	SFX_ENEMY_DIE,
	SFX_CRAFT,
	SFX_BRIDGE,
	SFX_DISCOVER,
	SFX_PLANT,
	SFX_SWING,
	SFX_COUNT,
}SFX;

void InitSound(void);
void PlaySfx(SFX sfx);
void FreeSound(void);
