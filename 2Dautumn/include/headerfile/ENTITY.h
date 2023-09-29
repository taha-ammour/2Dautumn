#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BASEPOINT_X 400
#define BASEPOINT_Y 300
#define PLAYER_RADIUS 30
#define ENEMY_RADIUS 20
#define CHASE_DISTANCE 100


extern SDL_Window* window;
extern SDL_Renderer* renderer;

typedef enum {
	IDLE,
	WALK,
	ATTACK,
}ENTITY_STATES;

typedef struct {
	bool animated;
	int frames;
	int speedframe;
}t_SpriteAnimation;

typedef struct {
	int hp;
	t_SpriteAnimation anim;
	SDL_Rect rect;
	ENTITY_STATES state;
}t_Entity;

typedef struct {
	t_Entity entity;
	double attackdmg;
	int defence;
	SDL_Texture* texture;
}t_Player;

typedef struct {
	t_Entity entity;
	double attackdmg;
	int defence;
	bool drop;
	int num_of_drop;
	SDL_Texture* texture;
}t_Ennemy;

typedef struct {
	float Zoom;
	SDL_Rect viewport;
}t_Camera;

typedef void (*AnimationFunction)(const t_Entity*, SDL_Rect*);

void InitPlayer(t_Player* p, const char* imageFile);
void InitEnnemy(t_Ennemy* e, const char* imageFile, int nFrames, int speedF);
void InitCamera(t_Camera* camera);

void UpdateCameraPos(t_Player p, t_Camera* camera);
void UpdatePlayerPos(t_Player* p,float distance, float deltaTime);
void updateEnemyPos(t_Entity* ennemy, t_Entity* player, float speed, float deltaTime);


void renderPlayer(const t_Player* p, t_Camera camera);
void renderEnnemy(const t_Ennemy* e, t_Camera camera);