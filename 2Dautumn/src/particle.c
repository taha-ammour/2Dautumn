#include "headerfile/PARTICLE.h"
#include "headerfile/MAP.h"
#include "headerfile/UIELEMENTS.h"

static t_Particle particles[MAX_PARTICLES];
static t_FloatText floatTexts[MAX_FLOAT_TEXTS];
static t_Leaf leaves[MAX_LEAVES];

static const SDL_Color leafColors[] = {
	{ 214, 110, 30, 255 },
	{ 190, 60, 30, 255 },
	{ 232, 180, 40, 255 },
	{ 150, 70, 30, 255 },
	{ 200, 80, 120, 255 },
};

void ClearParticles(void)
{
	for (int i = 0; i < MAX_PARTICLES; i++)
		particles[i].active = false;
	for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
		floatTexts[i].active = false;
}

void SpawnBurst(float x, float y, SDL_Color color, int count, float speed)
{
	for (int i = 0; i < MAX_PARTICLES && count > 0; i++)
	{
		t_Particle* p = &particles[i];
		if (p->active)
			continue;
		float angle = RandomFloat(0.0f, 6.2831f);
		float force = RandomFloat(0.3f, 1.0f) * speed;
		p->active = true;
		p->x = x;
		p->y = y;
		p->vx = SDL_cosf(angle) * force;
		p->vy = SDL_sinf(angle) * force - speed * 0.3f;
		p->maxLife = RandomFloat(0.3f, 0.7f);
		p->life = p->maxLife;
		p->color = color;
		p->size = rand() % 2 + 1;
		count--;
	}
}

void SpawnFloatText(float x, float y, const char* text, SDL_Color color)
{
	for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
	{
		t_FloatText* t = &floatTexts[i];
		if (t->active)
			continue;
		t->active = true;
		t->x = x;
		t->y = y;
		t->life = 1.0f;
		t->color = color;
		snprintf(t->text, sizeof(t->text), "%s", text);
		return;
	}
}

void InitLeaves(void)
{
	for (int i = 0; i < MAX_LEAVES; i++)
	{
		leaves[i].x = RandomFloat(0, (float)(MAP_WIDTH * TILE_SIZE));
		leaves[i].y = RandomFloat(0, (float)(MAP_HEIGHT * TILE_SIZE));
		leaves[i].sway = RandomFloat(0.0f, 6.2831f);
		leaves[i].fall = RandomFloat(6.0f, 14.0f);
		leaves[i].color = leafColors[rand() % (sizeof(leafColors) / sizeof(leafColors[0]))];
	}
}

void UpdateParticles(float windX, float windY, float deltaTime)
{
	float worldW = (float)(MAP_WIDTH * TILE_SIZE);
	float worldH = (float)(MAP_HEIGHT * TILE_SIZE);

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		t_Particle* p = &particles[i];
		if (!p->active)
			continue;
		p->life -= deltaTime;
		if (p->life <= 0)
		{
			p->active = false;
			continue;
		}
		p->vy += 120.0f * deltaTime;
		p->vx += windX * deltaTime;
		p->x += p->vx * deltaTime;
		p->y += p->vy * deltaTime;
	}

	for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
	{
		t_FloatText* t = &floatTexts[i];
		if (!t->active)
			continue;
		t->life -= deltaTime;
		t->y -= 14.0f * deltaTime;
		if (t->life <= 0)
			t->active = false;
	}

	for (int i = 0; i < MAX_LEAVES; i++)
	{
		t_Leaf* l = &leaves[i];
		l->sway += deltaTime * 2.0f;
		l->x += (windX * 1.4f + SDL_sinf(l->sway) * 8.0f) * deltaTime;
		l->y += (l->fall + windY * 1.4f) * deltaTime;
		// wrap around the map so the air is never empty
		if (l->x < 0) l->x += worldW;
		if (l->x >= worldW) l->x -= worldW;
		if (l->y < 0) l->y += worldH;
		if (l->y >= worldH) l->y -= worldH;
	}
}

void renderLeaves(t_Camera camera)
{
	for (int i = 0; i < MAX_LEAVES; i++)
	{
		t_Leaf* l = &leaves[i];
		int x = (int)l->x - camera.viewport.x;
		int y = (int)l->y - camera.viewport.y;
		if (x < -2 || y < -2 || x > camera.viewport.w || y > camera.viewport.h)
			continue;
		// leaves flip between flat and edge-on as they tumble
		SDL_Rect r = { x, y, SDL_sinf(l->sway) > 0 ? 2 : 1, 1 };
		SDL_SetRenderDrawColor(renderer, l->color.r, l->color.g, l->color.b, 255);
		SDL_RenderFillRect(renderer, &r);
	}
}

void renderParticles(t_Camera camera)
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		t_Particle* p = &particles[i];
		if (!p->active)
			continue;
		SDL_Rect r = { (int)p->x - camera.viewport.x, (int)p->y - camera.viewport.y, p->size, p->size };
		Uint8 alpha = (Uint8)(255 * (p->life / p->maxLife));
		SDL_SetRenderDrawColor(renderer, p->color.r, p->color.g, p->color.b, alpha);
		SDL_RenderFillRect(renderer, &r);
	}
}

void renderFloatTexts(t_Camera camera)
{
	for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
	{
		t_FloatText* t = &floatTexts[i];
		if (!t->active)
			continue;
		renderText(uiFont, t->text, (int)t->x - camera.viewport.x, (int)t->y - camera.viewport.y, t->color, ALIGN_CENTER);
	}
}
