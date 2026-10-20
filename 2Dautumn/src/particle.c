#include "headerfile/PARTICLE.h"
#include "headerfile/MAP.h"
#include "headerfile/UIELEMENTS.h"
#include "headerfile/SOUND.h"

static t_Particle particles[MAX_PARTICLES];
static t_FloatText floatTexts[MAX_FLOAT_TEXTS];
static t_Leaf leaves[MAX_LEAVES];
static t_Pickup pickups[MAX_PICKUPS];
static t_Swing swings[MAX_SWINGS];

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
	for (int i = 0; i < MAX_PICKUPS; i++)
		pickups[i].active = false;
	for (int i = 0; i < MAX_SWINGS; i++)
		swings[i].active = false;
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
		p->gravity = 120.0f;
		count--;
	}
}

// a slow glowing speck that floats upwards
void SpawnMote(float x, float y, SDL_Color color)
{
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		t_Particle* p = &particles[i];
		if (p->active)
			continue;
		p->active = true;
		p->x = x;
		p->y = y;
		p->vx = RandomFloat(-4.0f, 4.0f);
		p->vy = RandomFloat(-10.0f, -4.0f);
		p->maxLife = RandomFloat(1.5f, 2.5f);
		p->life = p->maxLife;
		p->color = color;
		p->size = 1;
		p->gravity = 0.0f;
		return;
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
		t->life = 1.2f;
		t->color = color;
		snprintf(t->text, sizeof(t->text), "%s", text);
		return;
	}
}

void SpawnPickup(float x, float y, ITEM_TEX tex, int count)
{
	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		t_Pickup* p = &pickups[i];
		if (p->active)
			continue;
		float angle = RandomFloat(0.0f, 6.2831f);
		p->active = true;
		p->x = x;
		p->y = y;
		p->z = 2.0f;
		p->vx = SDL_cosf(angle) * RandomFloat(15.0f, 35.0f);
		p->vy = SDL_sinf(angle) * RandomFloat(10.0f, 25.0f);
		p->vz = RandomFloat(70.0f, 100.0f);
		p->tex = tex;
		p->count = count;
		p->age = 0;
		return;
	}
}

void SpawnSwing(float x, float y, float angle)
{
	for (int i = 0; i < MAX_SWINGS; i++)
	{
		if (swings[i].active)
			continue;
		swings[i].active = true;
		swings[i].x = x;
		swings[i].y = y;
		swings[i].angle = angle;
		swings[i].life = 0.18f;
		return;
	}
}

void InitLeaves(t_Camera camera)
{
	for (int i = 0; i < MAX_LEAVES; i++)
	{
		leaves[i].x = camera.viewport.x + RandomFloat(0, (float)camera.viewport.w);
		leaves[i].y = camera.viewport.y + RandomFloat(0, (float)camera.viewport.h);
		leaves[i].sway = RandomFloat(0.0f, 6.2831f);
		leaves[i].fall = RandomFloat(6.0f, 14.0f);
		leaves[i].color = leafColors[rand() % (sizeof(leafColors) / sizeof(leafColors[0]))];
	}
}

void UpdateParticles(float windX, float windY, float deltaTime)
{
	(void)windY;
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
		p->vy += p->gravity * deltaTime;
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
		t->y -= 12.0f * deltaTime;
		if (t->life <= 0)
			t->active = false;
	}

	for (int i = 0; i < MAX_SWINGS; i++)
	{
		if (swings[i].active)
		{
			swings[i].life -= deltaTime;
			if (swings[i].life <= 0)
				swings[i].active = false;
		}
	}
}

void UpdateLeaves(t_Camera camera, float windX, float windY, float snow, float deltaTime)
{
	float left = (float)camera.viewport.x - 8;
	float top = (float)camera.viewport.y - 8;
	float w = (float)camera.viewport.w + 16;
	float h = (float)camera.viewport.h + 16;
	int flakes = (int)(snow * MAX_LEAVES);

	for (int i = 0; i < MAX_LEAVES; i++)
	{
		t_Leaf* l = &leaves[i];
		bool flake = i < flakes;
		l->sway += deltaTime * 2.0f;
		l->x += (windX * (flake ? 0.6f : 1.4f) + SDL_sinf(l->sway) * 8.0f) * deltaTime;
		l->y += (l->fall * (flake ? 1.8f : 1.0f) + windY * 1.4f) * deltaTime;
		// keep the air around the camera full: whatever drifts out comes back on the other side
		while (l->x < left) l->x += w;
		while (l->x >= left + w) l->x -= w;
		while (l->y < top) l->y += h;
		while (l->y >= top + h) l->y -= h;
	}
}

void UpdatePickups(t_Player* player, t_inventory* inv, float deltaTime)
{
	float px = player->entity.posX + 8.0f;
	float py = player->entity.posY + 10.0f;
	char text[32];

	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		t_Pickup* p = &pickups[i];
		if (!p->active)
			continue;
		p->age += deltaTime;

		// a little hop when it pops out
		p->vz -= 320.0f * deltaTime;
		p->z += p->vz * deltaTime;
		if (p->z < 0)
		{
			p->z = 0;
			p->vz = -p->vz * 0.35f;
		}

		float dx = px - p->x, dy = py - p->y;
		float dist = SDL_sqrtf(dx * dx + dy * dy);
		if (player->alive && p->age > 0.35f && dist < 40.0f)
		{
			float pull = 60.0f + (40.0f - dist) * 6.0f;
			p->vx = dx / (dist + 0.01f) * pull;
			p->vy = dy / (dist + 0.01f) * pull;
			if (dist < 6.0f && addItemToInventory(inv, makeItem(p->tex, GetItemType(p->tex), p->count)))
			{
				snprintf(text, sizeof(text), "+%d %s", p->count, GetItemName(p->tex));
				SpawnFloatText(px, py - 14, text, (SDL_Color){ 255, 240, 200, 255 });
				PlaySfx(SFX_PICKUP);
				p->active = false;
				continue;
			}
		}
		else
		{
			p->vx *= 1.0f - SDL_min(1.0f, deltaTime * 4.0f);
			p->vy *= 1.0f - SDL_min(1.0f, deltaTime * 4.0f);
		}
		p->x += p->vx * deltaTime;
		p->y += p->vy * deltaTime;
	}
}

void renderLeaves(t_Camera camera, float snow)
{
	int flakes = (int)(snow * MAX_LEAVES);
	for (int i = 0; i < MAX_LEAVES; i++)
	{
		t_Leaf* l = &leaves[i];
		int x = (int)l->x - camera.viewport.x;
		int y = (int)l->y - camera.viewport.y;
		if (i < flakes)
		{
			SDL_Rect r = { x, y, i % 3 == 0 ? 2 : 1, i % 3 == 0 ? 2 : 1 };
			SDL_SetRenderDrawColor(renderer, 245, 248, 255, 255);
			SDL_RenderFillRect(renderer, &r);
			continue;
		}
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

void renderSwings(t_Camera camera)
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	for (int i = 0; i < MAX_SWINGS; i++)
	{
		t_Swing* s = &swings[i];
		if (!s->active)
			continue;
		float t = 1.0f - s->life / 0.18f;
		Uint8 alpha = (Uint8)(255 * (1.0f - t * 0.7f));
		// a crescent sweeping across the facing direction
		for (int step = 0; step <= 10; step++)
		{
			float a = s->angle - 1.1f + 2.2f * step / 10.0f;
			if (a > s->angle - 1.1f + 2.2f * (t + 0.3f))
				break;
			for (float r = 8.0f; r <= 15.0f; r += 1.0f)
			{
				SDL_Rect dot = { (int)(s->x + SDL_cosf(a) * r) - camera.viewport.x, (int)(s->y + SDL_sinf(a) * r) - camera.viewport.y, 1, 1 };
				SDL_SetRenderDrawColor(renderer, 255, 250, 225, r > 13.0f || r < 10.0f ? alpha / 2 : alpha);
				SDL_RenderFillRect(renderer, &dot);
			}
		}
	}
}

void renderPickups(const t_inventory* inv, t_Camera camera)
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		t_Pickup* p = &pickups[i];
		if (!p->active)
			continue;
		int x = (int)p->x - camera.viewport.x;
		int y = (int)p->y - camera.viewport.y;
		float bob = p->age > 0.6f ? SDL_sinf(p->age * 5.0f) * 1.0f : 0.0f;
		SDL_Rect shadow = { x - 3, y + 3, 6, 2 };
		SDL_SetRenderDrawColor(renderer, 42, 54, 54, 110);
		SDL_RenderFillRect(renderer, &shadow);
		SDL_Rect icon = { x - 6, (int)(y - 8 - p->z + bob), 12, 12 };
		renderItemIcon(inv, p->tex, icon);
	}
}

void renderFloatTexts(t_Camera camera)
{
	for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
	{
		t_FloatText* t = &floatTexts[i];
		if (!t->active)
			continue;
		renderText(uiFontSmall, t->text, (int)t->x - camera.viewport.x, (int)t->y - camera.viewport.y, t->color, ALIGN_CENTER);
	}
}
