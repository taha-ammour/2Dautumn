#include "headerfile/ANIMATE.h"
#include "headerfile/MAP.h"


void attackAnimation(const t_Entity* e, SDL_Rect* srcrect)
{
	Uint32 currentTime = SDL_GetTicks();
	int frameIndex = (currentTime / e->anim.speedframe) % e->anim.frames;

	srcrect->x = frameIndex * TILE_SIZE;
	srcrect->y = ATTACK * TILE_SIZE;
	srcrect->w = TILE_SIZE;
	srcrect->h = TILE_SIZE;

}

void idleAnimation(const t_Entity* e, SDL_Rect* srcrect)
{
	Uint32 currentTime = SDL_GetTicks();
	int frameIndex = (currentTime / e->anim.speedframe) % e->anim.frames;

	srcrect->x = frameIndex * TILE_SIZE;
	srcrect->y = IDLE * TILE_SIZE;
	srcrect->w = TILE_SIZE;
	srcrect->h = TILE_SIZE;
}

void walkAnimation(const t_Entity* e, SDL_Rect* srcrect)
{
	Uint32 currentTime = SDL_GetTicks();
	int frameIndex = (currentTime / e->anim.speedframe) % e->anim.frames;

	srcrect->x = frameIndex * TILE_SIZE;
	srcrect->y = WALK * TILE_SIZE;
	srcrect->w = TILE_SIZE;
	srcrect->h = TILE_SIZE;
}

AnimationFunction animationFunctions[] = {
	idleAnimation,
	walkAnimation,
	attackAnimation
	
};