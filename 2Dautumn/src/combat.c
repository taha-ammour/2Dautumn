#include "headerfile/COMBAT.h"
#include "headerfile/PARTICLE.h"
#include "headerfile/SOUND.h"

static const SDL_Color damageTextColor = { 255, 90, 70, 255 };
static const SDL_Color hitTextColor = { 255, 230, 160, 255 };
static const SDL_Color hintColor = { 255, 220, 170, 255 };
static const SDL_Color leafBurst = { 214, 110, 30, 255 };

static Uint32 lastHintTime = 0;

typedef struct {
	float hueShift;  // degrees
	float saturation;
	float value;
}t_Recolor;

static const t_Recolor kindColors[LEAF_KIND_COUNT] = {
	/* LEAF_ORANGE  */ { 0, 1.0f, 1.0f },
	/* LEAF_RED     */ { -22, 1.1f, 0.95f },
	/* LEAF_GOLD    */ { 20, 1.0f, 1.15f },
	/* LEAF_CRIMSON */ { -65, 0.9f, 0.9f },
};

static void recolorPixel(Uint8* r, Uint8* g, Uint8* b, const t_Recolor* recolor)
{
	float rf = *r / 255.0f, gf = *g / 255.0f, bf = *b / 255.0f;
	float maxc = SDL_max(rf, SDL_max(gf, bf));
	float minc = SDL_min(rf, SDL_min(gf, bf));
	float delta = maxc - minc;
	float h = 0, s = maxc > 0 ? delta / maxc : 0, v = maxc;

	// greys (outline, shadow) keep their color
	if (delta < 0.08f)
		return;
	if (maxc == rf) h = 60.0f * SDL_fmodf((gf - bf) / delta, 6.0f);
	else if (maxc == gf) h = 60.0f * ((bf - rf) / delta + 2.0f);
	else h = 60.0f * ((rf - gf) / delta + 4.0f);

	h = SDL_fmodf(h + recolor->hueShift + 360.0f, 360.0f);
	s = SDL_min(1.0f, s * recolor->saturation);
	v = SDL_min(1.0f, v * recolor->value);

	float c = v * s;
	float x = c * (1.0f - SDL_fabsf(SDL_fmodf(h / 60.0f, 2.0f) - 1.0f));
	float m = v - c;
	float rr = 0, gg = 0, bb = 0;
	if (h < 60) { rr = c; gg = x; }
	else if (h < 120) { rr = x; gg = c; }
	else if (h < 180) { gg = c; bb = x; }
	else if (h < 240) { gg = x; bb = c; }
	else if (h < 300) { rr = x; bb = c; }
	else { rr = c; bb = x; }
	*r = (Uint8)((rr + m) * 255);
	*g = (Uint8)((gg + m) * 255);
	*b = (Uint8)((bb + m) * 255);
}

void InitEnemyPool(t_EnemyPool* pool, const char* imageFile)
{
	SDL_Surface* loaded = IMG_Load(imageFile);
	SDL_Surface* source = loaded ? SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0) : NULL;

	SDL_FreeSurface(loaded);
	for (int kind = 0; kind < LEAF_KIND_COUNT; kind++)
	{
		pool->textures[kind] = NULL;
		if (source == NULL)
			continue;
		SDL_Surface* copy = SDL_DuplicateSurface(source);
		Uint8* pixels = (Uint8*)copy->pixels;
		for (int y = 0; y < copy->h; y++)
		{
			for (int x = 0; x < copy->w; x++)
			{
				Uint8* p = pixels + y * copy->pitch + x * 4;
				if (p[3] != 0)
					recolorPixel(&p[0], &p[1], &p[2], &kindColors[kind]);
			}
		}
		pool->textures[kind] = SDL_CreateTextureFromSurface(renderer, copy);
		SDL_FreeSurface(copy);
	}
	if (source == NULL)
		fprintf(stderr, "Image not loading: %s, Error: %s\n", imageFile, IMG_GetError());
	SDL_FreeSurface(source);
	ClearEnemies(pool);
}

void ClearEnemies(t_EnemyPool* pool)
{
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		pool->enemies[i].alive = false;
		pool->enemies[i].respawnTimer = 0;
	}
}

static t_Ennemy* spawnEnemy(t_EnemyPool* pool, ENEMY_KIND kind, float x, float y)
{
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		t_Ennemy* e = &pool->enemies[i];
		if (!e->alive && e->respawnTimer <= 0)
		{
			InitEnnemy(e, pool->textures[kind], kind, x, y);
			return e;
		}
	}
	return NULL;
}

static bool freeSpot(const t_Map* map, int x, int y)
{
	float px = (float)(x * TILE_SIZE), py = (float)(y * TILE_SIZE);
	return !IsSolidAt(map, px + 4, py + 11) && !IsSolidAt(map, px + 12, py + 11) &&
		!IsSolidAt(map, px + 4, py + 15) && !IsSolidAt(map, px + 12, py + 15);
}

static void spawnInRegion(t_EnemyPool* pool, const t_Map* map, REGION region, ENEMY_KIND kind, int count)
{
	for (int tries = 0; tries < 2000 && count > 0; tries++)
	{
		int x = rand() % MAP_WIDTH;
		int y = rand() % MAP_HEIGHT;
		// keep the camp itself quiet
		if (map->region[y][x] != region || !freeSpot(map, x, y) || (abs(x - map->startX) < 8 && abs(y - map->startY) < 8))
			continue;
		spawnEnemy(pool, kind, (float)(x * TILE_SIZE), (float)(y * TILE_SIZE));
		count--;
	}
}

void PopulateCreatures(t_EnemyPool* pool, const t_Map* map)
{
	ClearEnemies(pool);
	spawnInRegion(pool, map, REGION_FOREST, LEAF_ORANGE, 10);
	spawnInRegion(pool, map, REGION_MEADOW, LEAF_ORANGE, 5);
	spawnInRegion(pool, map, REGION_HILLS, LEAF_RED, 8);
	spawnInRegion(pool, map, REGION_POND, LEAF_GOLD, 6);
	spawnInRegion(pool, map, REGION_ISLAND, LEAF_CRIMSON, 3);
	spawnInRegion(pool, map, REGION_GLADE, LEAF_GOLD, 2);
}

void CalmCreatures(t_EnemyPool* pool)
{
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		if (pool->enemies[i].alive && pool->enemies[i].mood != MOOD_IDLE)
		{
			pool->enemies[i].mood = MOOD_IDLE;
			pool->enemies[i].moodTimer = 1.0f;
		}
	}
}

static float centerX(const t_Entity* e) { return e->posX + e->rect.w * 0.5f; }
static float centerY(const t_Entity* e) { return e->posY + e->rect.h * 0.5f; }

static void knockback(const t_Map* map, t_Entity* e, float fromX, float fromY, float force)
{
	float dx = centerX(e) - fromX;
	float dy = centerY(e) - fromY;
	float dist = SDL_sqrtf(dx * dx + dy * dy);
	if (dist < 0.01f)
		return;
	MoveWithCollision(map, e, dx / dist * force, dy / dist * force);
}

static void showHint(float x, float y, const char* text)
{
	Uint32 now = SDL_GetTicks();
	if (now - lastHintTime < 1200)
		return;
	lastHintTime = now;
	SpawnFloatText(x, y, text, hintColor);
}

void UpdatePlayerPos(t_Player* p, const t_Map* map, float distance, float deltaTime) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    float dirX = 0, dirY = 0;

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
        dirY -= 1;
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
        dirY += 1;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
        dirX -= 1;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
        dirX += 1;

    // same speed when walking diagonally
    if (dirX != 0 && dirY != 0)
    {
        dirX *= 0.7071f;
        dirY *= 0.7071f;
    }
    MoveWithCollision(map, &p->entity, dirX * distance * deltaTime, dirY * distance * deltaTime);

    if (dirX != 0 || dirY != 0)
    {
        p->faceX = dirX;
        p->faceY = dirY;
    }
    if (dirX < 0)
        p->flip = true;
    else if (dirX > 0)
        p->flip = false;

    if (SDL_GetTicks() - p->attackTime < ATTACK_ANIM_TIME)
        p->entity.state = ATTACK;
    else if (dirX == 0 && dirY == 0)
        p->entity.state = IDLE;
    else
        p->entity.state = WALK;
}

void UpdatePlayerHealth(t_Player* p, float deltaTime)
{
	// out of a fight for a while, the stump slowly mends
	if (p->alive && p->entity.hp < PLAYER_MAX_HP && SDL_GetTicks() - p->entity.hurtTime > PLAYER_REGEN_DELAY)
	{
		p->entity.hp += PLAYER_REGEN_RATE * deltaTime;
		if (p->entity.hp > PLAYER_MAX_HP)
			p->entity.hp = PLAYER_MAX_HP;
	}
}

void DamagePlayer(t_Player* player, const t_Map* map, double dmg, float fromX, float fromY)
{
	Uint32 now = SDL_GetTicks();
	char text[16];

	if (!player->alive || (player->entity.hurtTime != 0 && now - player->entity.hurtTime < PLAYER_IFRAMES))
		return;
	dmg -= player->defence;
	if (dmg < 1)
		dmg = 1;
	player->entity.hp -= dmg;
	player->entity.hurtTime = now;
	knockback(map, &player->entity, fromX, fromY, 8.0f);

	snprintf(text, sizeof(text), "-%d", (int)dmg);
	SpawnFloatText(centerX(&player->entity), player->entity.posY - 4, text, damageTextColor);
	AddScreenShake(3.0f);
	PlaySfx(SFX_HURT);
}

static void killEnemy(t_Ennemy* e)
{
	float x = centerX(&e->entity);
	float y = centerY(&e->entity);
	SDL_Color tint = GetEnemyTint(e->kind);

	e->alive = false;
	e->respawnTimer = CREATURE_RESPAWN_TIME;
	SpawnBurst(x, y, tint, 18, 70.0f);
	PlaySfx(SFX_ENEMY_DIE);
	if (e->drop)
	{
		SpawnPickup(x, y, LEAVES_TEX, 1 + rand() % e->num_of_drop);
		if (rand() % 3 == 0)
			SpawnPickup(x, y, STICK_TEX, 1);
	}
}

static void damageEnemy(t_Ennemy* e, double dmg)
{
	char text[16];

	dmg -= e->defence;
	if (dmg < 1)
		dmg = 1;
	e->entity.hp -= dmg;
	e->entity.hurtTime = SDL_GetTicks();
	snprintf(text, sizeof(text), "%d", (int)dmg);
	SpawnFloatText(centerX(&e->entity), e->entity.posY - 2, text, hitTextColor);

	if (e->entity.hp <= 0)
	{
		killEnemy(e);
		return;
	}
	// a hurt leaf fights back, a badly hurt one runs away
	if (e->entity.hp < e->maxHp * 0.3)
	{
		e->mood = MOOD_FLEE;
		e->moodTimer = 3.0f;
	}
	else
	{
		e->mood = MOOD_ANGRY;
		e->moodTimer = 8.0f;
	}
}

void PlayerSwing(t_EnemyPool* pool, t_Player* player, t_Map* map, t_inventory* inv, float dirX, float dirY)
{
	Uint32 now = SDL_GetTicks();
	float len = SDL_sqrtf(dirX * dirX + dirY * dirY);
	ITEM_TEX held = GetSelectedItem(inv);
	bool hitCreature = false;

	if (!player->alive || now - player->attackTime < SWING_COOLDOWN)
		return;
	if (len < 0.01f)
	{
		dirX = player->faceX;
		dirY = player->faceY;
		len = SDL_sqrtf(dirX * dirX + dirY * dirY);
	}
	dirX /= len;
	dirY /= len;

	float px = centerX(&player->entity);
	float py = centerY(&player->entity) + 1;
	float hitX = px + dirX * SWING_REACH;
	float hitY = py + dirY * SWING_REACH;

	player->attackTime = now;
	if (SDL_fabsf(dirX) > 0.2f)
		player->flip = dirX < 0;
	SpawnSwing(px, py, SDL_atan2f(dirY, dirX));
	PlaySfx(SFX_SWING);

	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		t_Ennemy* e = &pool->enemies[i];
		if (!e->alive)
			continue;
		// anything inside the crescent in front of you gets hit
		float dx = centerX(&e->entity) - px;
		float dy = centerY(&e->entity) - py;
		float dist = SDL_sqrtf(dx * dx + dy * dy);
		if (dist > SWING_ARC_RANGE)
			continue;
		if (dist > 6.0f && SDL_acosf(SDL_clamp((dx * dirX + dy * dirY) / dist, -1.0f, 1.0f)) > SWING_ARC_ANGLE)
			continue;
		knockback(map, &e->entity, px, py, 7.0f);
		SpawnBurst(centerX(&e->entity), centerY(&e->entity), GetEnemyTint(e->kind), 6, 50.0f);
		damageEnemy(e, GetWeaponDamage(held) * player->attackdmg);
		hitCreature = true;
	}
	if (hitCreature)
	{
		AddScreenShake(1.5f);
		PlaySfx(SFX_HIT);
		return;
	}

	t_Object* obj = FindObjectNear(map, hitX, hitY, SWING_RADIUS);
	if (obj == NULL)
		return;
	const t_ObjectInfo* info = GetObjectInfo(obj->TYPE);
	float ox = obj->entity.rect.x + 8.0f;
	float oy = (float)obj->entity.rect.y;
	if (info->pickTier < 0)
		return;
	if (GetPickTier(held) < info->pickTier)
	{
		obj->entity.hurtTime = now;
		showHint(ox, oy - 4, info->needText);
		PlaySfx(SFX_CLINK);
		return;
	}

	double base = obj->TYPE == TREE_OBJECT || obj->TYPE == MOSS_OBJECT ? 2.0 : (obj->TYPE == LEAFPILE_OBJECT ? 3.0 : 1.0);
	obj->entity.hp -= base * player->attackdmg * GetMiningMultiplier(held);
	obj->entity.hurtTime = now;
	SpawnBurst(ox, oy + 8, obj->TYPE == LEAFPILE_OBJECT ? leafBurst : (SDL_Color){ 200, 190, 160, 255 }, 5, 40.0f);
	AddScreenShake(0.6f);
	PlaySfx(SFX_HARVEST);
}

static void pickWanderTarget(t_Ennemy* e, const t_Map* map)
{
	for (int tries = 0; tries < 6; tries++)
	{
		float x = e->homeX + RandomFloat(-40.0f, 40.0f);
		float y = e->homeY + RandomFloat(-40.0f, 40.0f);
		if (!IsSolidAt(map, x + 8, y + 13))
		{
			e->targetX = x;
			e->targetY = y;
			e->mood = MOOD_WANDER;
			e->moodTimer = 4.0f;
			return;
		}
	}
	e->moodTimer = 1.0f;
}

void UpdateEnemies(t_EnemyPool* pool, t_Player* player, const t_Map* map, float windX, float windY, float deltaTime)
{
	float px = centerX(&player->entity);
	float py = centerY(&player->entity);

	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		t_Ennemy* e = &pool->enemies[i];
		if (!e->alive)
		{
			// leaf spirits come back home when nobody is watching
			if (e->respawnTimer > 0)
			{
				e->respawnTimer -= deltaTime;
				float hx = e->homeX - px, hy = e->homeY - py;
				if (e->respawnTimer <= 0 && hx * hx + hy * hy > 220.0f * 220.0f)
					InitEnnemy(e, pool->textures[e->kind], e->kind, e->homeX, e->homeY);
				else if (e->respawnTimer <= 0)
					e->respawnTimer = 5.0f;
			}
			continue;
		}

		float ex = centerX(&e->entity), ey = centerY(&e->entity);
		float dx = px - ex, dy = py - ey;
		float dist = SDL_sqrtf(dx * dx + dy * dy);
		float vx = 0, vy = 0;

		e->moodTimer -= deltaTime;
		e->attackCooldown -= deltaTime;

		switch (e->mood)
		{
		case MOOD_IDLE:
			e->entity.state = IDLE;
			// curious: they turn to look at you when you come close
			if (player->alive && dist < 32.0f)
				e->flip = dx < 0;
			if (e->moodTimer <= 0)
				pickWanderTarget(e, map);
			break;
		case MOOD_WANDER:
		{
			float tx = e->targetX - e->entity.posX, ty = e->targetY - e->entity.posY;
			float tdist = SDL_sqrtf(tx * tx + ty * ty);
			if (tdist < 2.0f || e->moodTimer <= 0)
			{
				e->mood = MOOD_IDLE;
				e->moodTimer = RandomFloat(1.0f, 3.5f);
				break;
			}
			vx = tx / tdist * e->speed * 0.45f;
			vy = ty / tdist * e->speed * 0.45f;
			break;
		}
		case MOOD_ANGRY:
			if (!player->alive || dist > CREATURE_CALM_DISTANCE || e->moodTimer <= 0)
			{
				e->mood = MOOD_IDLE;
				e->moodTimer = 2.0f;
				break;
			}
			if (dist > 1.0f)
			{
				vx = dx / dist * e->speed;
				vy = dy / dist * e->speed;
			}
			if (SDL_HasIntersection(&player->entity.rect, &e->entity.rect) && e->attackCooldown <= 0)
			{
				DamagePlayer(player, map, e->attackdmg, ex, ey);
				e->attackCooldown = 1.0f;
				knockback(map, &e->entity, px, py, 6.0f);
			}
			break;
		case MOOD_FLEE:
			if (e->moodTimer <= 0)
			{
				e->mood = MOOD_IDLE;
				e->moodTimer = 2.0f;
				e->homeX = e->entity.posX;
				e->homeY = e->entity.posY;
				break;
			}
			if (dist > 1.0f)
			{
				vx = -dx / dist * e->speed * 1.2f;
				vy = -dy / dist * e->speed * 1.2f;
			}
			break;
		}

		if (vx != 0 || vy != 0)
		{
			e->entity.state = WALK;
			if (SDL_fabsf(vx) > 1.0f)
				e->flip = vx < 0;
		}
		else
		{
			e->entity.state = IDLE;
		}
		// wandering leaves drift a little with the wind
		if (e->mood == MOOD_WANDER)
		{
			vx += windX * 0.15f;
			vy += windY * 0.15f;
		}
		if (!MoveWithCollision(map, &e->entity, vx * deltaTime, vy * deltaTime) && e->mood == MOOD_WANDER)
		{
			e->mood = MOOD_IDLE;
			e->moodTimer = RandomFloat(0.5f, 1.5f);
		}
	}
}
