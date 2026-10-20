#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PLAYER_MAX_HP 100
#define PLAYER_SPEED 95.0f
#define PLAYER_IFRAMES 900
#define PLAYER_REGEN_DELAY 5000
#define PLAYER_REGEN_RATE 4.0
#define ATTACK_ANIM_TIME 250

#define MAX_ENEMIES 64


extern SDL_Window* window;
extern SDL_Renderer* renderer;

typedef enum {
	IDLE,
	WALK,
	ATTACK,
}ENTITY_STATES;

// the leaf spirits, each one tinted from the ennemy sprite
typedef enum {
	LEAF_ORANGE,
	LEAF_RED,
	LEAF_GOLD,
	LEAF_CRIMSON,
	LEAF_KIND_COUNT,
}ENEMY_KIND;

// leaf spirits are peaceful until someone hits them
typedef enum {
	MOOD_IDLE,
	MOOD_WANDER,
	MOOD_ANGRY,
	MOOD_FLEE,
}ENEMY_MOOD;

typedef struct {
	bool animated;
	int frames;
	int speedframe;
}t_SpriteAnimation;

typedef struct {
	double hp;
	t_SpriteAnimation anim;
	SDL_Rect rect;
	ENTITY_STATES state;
	float posX, posY; // precise position, rect is rounded from it
	Uint32 hurtTime;  // last time this entity took damage
}t_Entity;

typedef struct {
	t_Entity entity;
	double attackdmg;
	int defence;
	SDL_Texture* texture;
	Uint32 attackTime;
	bool flip;
	float faceX, faceY; // last walking direction, used by the keyboard swing
	bool alive;
	float respawnTimer;
}t_Player;

typedef struct {
	t_Entity entity;
	double attackdmg;
	int defence;
	bool drop;
	int num_of_drop;
	SDL_Texture* texture;
	ENEMY_KIND kind;
	bool alive;
	double maxHp;
	float speed;
	float attackCooldown;
	bool flip;
	ENEMY_MOOD mood;
	float moodTimer;
	float homeX, homeY;
	float targetX, targetY;
	float respawnTimer;
}t_Ennemy;

typedef struct {
	float Zoom;
	SDL_Rect viewport;
	float x, y; // smoothed position of the top-left corner
}t_Camera;

typedef void (*AnimationFunction)(const t_Entity*, SDL_Rect*);

void SetEntityPos(t_Entity* e, float x, float y);
void SyncEntityRect(t_Entity* e);

void InitPlayer(t_Player* p, const char* imageFile, int nFrames, int speedF);
void ResetPlayer(t_Player* p, float x, float y);
void InitEnnemy(t_Ennemy* e, SDL_Texture* texture, ENEMY_KIND kind, float x, float y);
void InitCamera(t_Camera* camera);

void AddScreenShake(float amount);
void SnapCamera(t_Player p, t_Camera* camera);
void UpdateCameraPos(t_Player p, t_Camera* camera, float deltaTime);

SDL_Color GetEnemyTint(ENEMY_KIND kind);

void renderPlayer(const t_Player* p, t_Camera camera);
void renderEnnemy(const t_Ennemy* e, t_Camera camera);
