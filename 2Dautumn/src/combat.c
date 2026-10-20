#include "headerfile/COMBAT.h"
#include "headerfile/COLLISION.h"
#include "headerfile/PARTICLE.h"
#include "headerfile/SOUND.h"

static const SDL_Color damageTextColor = { 255, 90, 70, 255 };
static const SDL_Color hitTextColor = { 255, 230, 160, 255 };
static const SDL_Color emberColor = { 255, 150, 40, 255 };
static const SDL_Color smokeColor = { 90, 80, 80, 255 };

static Uint32 lastNoStickWarning = 0;

void InitEnemyPool(t_EnemyPool* pool, const char* imageFile)
{
	pool->texture = loadTexture(imageFile);
	ClearEnemies(pool);
}

void ClearEnemies(t_EnemyPool* pool)
{
	for (int i = 0; i < MAX_ENEMIES; i++)
		pool->enemies[i].alive = false;
}

t_Ennemy* SpawnEnemy(t_EnemyPool* pool, ENEMY_KIND kind, float x, float y)
{
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		if (!pool->enemies[i].alive)
		{
			InitEnnemy(&pool->enemies[i], pool->texture, kind, x, y);
			return &pool->enemies[i];
		}
	}
	return NULL;
}

int CountAliveEnemies(const t_EnemyPool* pool)
{
	int count = 0;
	for (int i = 0; i < MAX_ENEMIES; i++)
		if (pool->enemies[i].alive)
			count++;
	return count;
}

// orange leaves are drawn untinted, so their bursts get their own color
static SDL_Color burstColor(ENEMY_KIND kind)
{
	return kind == LEAF_ORANGE ? emberColor : GetEnemyTint(kind);
}

static float centerX(const t_Entity* e) { return e->posX + e->rect.w * 0.5f; }
static float centerY(const t_Entity* e) { return e->posY + e->rect.h * 0.5f; }

static void knockback(t_Entity* e, float fromX, float fromY, float force)
{
	float dx = centerX(e) - fromX;
	float dy = centerY(e) - fromY;
	float dist = SDL_sqrtf(dx * dx + dy * dy);
	if (dist < 0.01f)
		return;
	e->posX += dx / dist * force;
	e->posY += dy / dist * force;
	SyncEntityRect(e);
}

void DamagePlayer(t_Player* player, double dmg, float fromX, float fromY)
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
	knockback(&player->entity, fromX, fromY, 10.0f);

	snprintf(text, sizeof(text), "-%d", (int)dmg);
	SpawnFloatText(centerX(&player->entity), player->entity.posY - 4, text, damageTextColor);
	AddScreenShake(3.0f);
	PlaySfx(SFX_HURT);
}

static void giveDrop(t_inventory* inv, ITEM_TEX itemTex, int count, float x, float y)
{
	char text[24];
	if (count <= 0)
		return;
	addItemToInventory(inv, makeItem(itemTex, MATERIAL, count));
	snprintf(text, sizeof(text), "+%d %s", count, GetItemName(itemTex));
	SpawnFloatText(x, y, text, hitTextColor);
}

static void killEnemy(t_EnemyPool* pool, t_Ennemy* e, t_inventory* inv)
{
	float x = centerX(&e->entity);
	float y = centerY(&e->entity);

	e->alive = false;
	SpawnBurst(x, y, burstColor(e->kind), 18, 70.0f);
	PlaySfx(SFX_ENEMY_DIE);

	// leaf spirits leave twigs behind, the big ones carry wood
	if (e->drop)
	{
		giveDrop(inv, STICK_TEX, rand() % (e->num_of_drop + 1), x, y - 8);
		if (e->kind == LEAF_CRIMSON)
			giveDrop(inv, WOOD_TEX, 2, x, y - 16);
	}

	// golden leaves burst into two smaller ones
	if (e->kind == LEAF_GOLD)
	{
		for (int i = 0; i < 2; i++)
		{
			t_Ennemy* child = SpawnEnemy(pool, LEAF_ORANGE, e->entity.posX + (i == 0 ? -6.0f : 6.0f), e->entity.posY);
			if (child != NULL)
			{
				child->entity.hp = 10;
				child->maxHp = 10;
				child->drop = false;
			}
		}
	}
}

void DamageEnemy(t_EnemyPool* pool, t_Ennemy* e, double dmg, t_inventory* inv)
{
	char text[16];

	if (!e->alive)
		return;
	dmg -= e->defence;
	if (dmg < 1)
		dmg = 1;
	e->entity.hp -= dmg;
	e->entity.hurtTime = SDL_GetTicks();

	snprintf(text, sizeof(text), "%d", (int)dmg);
	SpawnFloatText(centerX(&e->entity), e->entity.posY - 2, text, hitTextColor);
	if (e->entity.hp <= 0)
		killEnemy(pool, e, inv);
}

bool PlayerMeleeAttack(t_EnemyPool* pool, t_Player* player, t_inventory* inv, int worldX, int worldY)
{
	Uint32 now = SDL_GetTicks();
	t_Ennemy* target = NULL;
	float best = 14.0f;
	float px = centerX(&player->entity);
	float py = centerY(&player->entity);

	if (!player->alive || now - player->attackTime < MELEE_COOLDOWN)
		return false;

	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		t_Ennemy* e = &pool->enemies[i];
		if (!e->alive)
			continue;
		float ex = centerX(&e->entity);
		float ey = centerY(&e->entity);
		float toClick = SDL_sqrtf((ex - worldX) * (ex - worldX) + (ey - worldY) * (ey - worldY));
		float toPlayer = SDL_sqrtf((ex - px) * (ex - px) + (ey - py) * (ey - py));
		if (toClick < best && toPlayer <= MELEE_RANGE)
		{
			best = toClick;
			target = e;
		}
	}
	if (target == NULL)
		return false;

	player->attackTime = now;
	player->flip = centerX(&target->entity) < px;
	knockback(&target->entity, px, py, 8.0f);
	SpawnBurst(centerX(&target->entity), centerY(&target->entity), burstColor(target->kind), 6, 50.0f);
	AddScreenShake(1.5f);
	PlaySfx(SFX_HIT);
	DamageEnemy(pool, target, GetWeaponDamage(GetSelectedItem(inv)) * player->attackdmg, inv);
	return true;
}

void UpdateEnemies(t_EnemyPool* pool, t_Player* player, t_Map* map, t_inventory* inv,
	float windX, float windY, float lightRadius, float burnDps, float deltaTime)
{
	t_Object* seed = GetSeed(map);
	float seedX = seed->entity.rect.x + TILE_SIZE * 0.5f;
	float seedY = seed->entity.rect.y + TILE_SIZE * 0.5f;

	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		t_Ennemy* e = &pool->enemies[i];
		if (!e->alive)
			continue;

		float dx = centerX(&e->entity) - seedX;
		float dy = centerY(&e->entity) - seedY;
		bool inLight = dx * dx + dy * dy < lightRadius * lightRadius;

		// the glow of the heart seed slows and scorches dry leaves
		if (inLight)
		{
			e->entity.hp -= burnDps * deltaTime;
			if (rand() % 12 == 0)
				SpawnBurst(centerX(&e->entity), e->entity.posY, rand() % 2 ? smokeColor : emberColor, 1, 20.0f);
			if (e->entity.hp <= 0)
			{
				killEnemy(pool, e, inv);
				continue;
			}
		}

		updateEnemyPos(e, player->alive ? &player->entity : NULL, seed->entity.rect, windX, windY, inLight ? 0.6f : 1.0f, deltaTime);

		e->attackCooldown -= deltaTime;
		if (e->entity.state == ATTACK && e->attackCooldown <= 0 && seed->entity.hp > 0)
		{
			seed->entity.hp -= e->attackdmg * 0.5;
			seed->entity.hurtTime = SDL_GetTicks();
			e->attackCooldown = 1.0f;
			SpawnBurst(seedX, seedY, (SDL_Color){ 230, 220, 90, 255 }, 4, 40.0f);
		}

		if (player->alive && areEntitiesColliding(&player->entity, &e->entity))
		{
			DamagePlayer(player, e->attackdmg, centerX(&e->entity), centerY(&e->entity));
			// both bounce apart so the leaf never hides under the player
			knockback(&e->entity, centerX(&player->entity), centerY(&player->entity), 4.0f);
		}
	}

	// keep the swarm from collapsing into a single pile
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		if (!pool->enemies[i].alive)
			continue;
		for (int j = i + 1; j < MAX_ENEMIES; j++)
		{
			if (pool->enemies[j].alive)
				separateEntities(&pool->enemies[i].entity, &pool->enemies[j].entity);
		}
	}
}

void InitProjectiles(t_ProjectilePool* pool, const char* imageFile)
{
	pool->texture = loadTexture(imageFile);
	ClearProjectiles(pool);
}

void ClearProjectiles(t_ProjectilePool* pool)
{
	for (int i = 0; i < MAX_PROJECTILES; i++)
		pool->shots[i].active = false;
}

bool ThrowEmber(t_ProjectilePool* pool, t_Player* player, t_inventory* inv, int worldX, int worldY)
{
	Uint32 now = SDL_GetTicks();
	float px = centerX(&player->entity);
	float py = centerY(&player->entity);
	float dx = worldX - px;
	float dy = worldY - py;
	float dist = SDL_sqrtf(dx * dx + dy * dy);

	if (!player->alive || dist < 1.0f)
		return false;
	if (CountItem(inv, STICK_TEX) <= 0)
	{
		if (now - lastNoStickWarning > 1500)
		{
			SpawnFloatText(px, player->entity.posY - 6, "Need a stick!", damageTextColor);
			lastNoStickWarning = now;
		}
		return false;
	}

	for (int i = 0; i < MAX_PROJECTILES; i++)
	{
		t_Projectile* shot = &pool->shots[i];
		if (shot->active)
			continue;
		RemoveItem(inv, STICK_TEX, 1);
		shot->active = true;
		shot->life = EMBER_LIFE;
		shot->velocityX = dx / dist * EMBER_SPEED;
		shot->velocityY = dy / dist * EMBER_SPEED;
		shot->entity.hp = 1;
		shot->entity.rect.w = 8;
		shot->entity.rect.h = 8;
		SetEntityPos(&shot->entity, px - 4, py - 4);
		player->attackTime = now;
		player->flip = dx < 0;
		PlaySfx(SFX_EMBER);
		return true;
	}
	return false;
}

void UpdateProjectiles(t_ProjectilePool* pool, t_EnemyPool* enemies, t_Map* map, t_inventory* inv,
	float windX, float windY, float deltaTime)
{
	for (int i = 0; i < MAX_PROJECTILES; i++)
	{
		t_Projectile* shot = &pool->shots[i];
		if (!shot->active)
			continue;

		// embers drift with the wind, so aim upwind
		shot->velocityX += windX * 1.5f * deltaTime;
		shot->velocityY += windY * 1.5f * deltaTime;
		shot->entity.posX += (float)shot->velocityX * deltaTime;
		shot->entity.posY += (float)shot->velocityY * deltaTime;
		SyncEntityRect(&shot->entity);
		shot->life -= deltaTime;
		if (rand() % 3 == 0)
			SpawnBurst(centerX(&shot->entity), centerY(&shot->entity), emberColor, 1, 10.0f);

		int tileX = (int)(centerX(&shot->entity) / TILE_SIZE);
		int tileY = (int)(centerY(&shot->entity) / TILE_SIZE);
		if (shot->life <= 0 || tileX < 0 || tileY < 0 || tileX >= MAP_WIDTH || tileY >= MAP_HEIGHT)
		{
			shot->active = false;
			continue;
		}
		t_Object* obj = &map->objectMap[tileY][tileX];
		if ((obj->TYPE == ROCK_OBJECT || obj->TYPE == TREE_OBJECT) && obj->entity.hp > 0)
		{
			SpawnBurst(centerX(&shot->entity), centerY(&shot->entity), emberColor, 6, 40.0f);
			shot->active = false;
			continue;
		}

		for (int j = 0; j < MAX_ENEMIES; j++)
		{
			t_Ennemy* e = &enemies->enemies[j];
			if (e->alive && areEntitiesColliding(&shot->entity, &e->entity))
			{
				SpawnBurst(centerX(&shot->entity), centerY(&shot->entity), emberColor, 10, 60.0f);
				knockback(&e->entity, centerX(&shot->entity) - (float)shot->velocityX, centerY(&shot->entity) - (float)shot->velocityY, 5.0f);
				PlaySfx(SFX_HIT);
				DamageEnemy(enemies, e, EMBER_DAMAGE, inv);
				shot->active = false;
				break;
			}
		}
	}
}

void renderEnemies(const t_EnemyPool* pool, t_Camera camera)
{
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		if (pool->enemies[i].alive)
			renderEnnemy(&pool->enemies[i], camera);
	}
}

void renderProjectiles(const t_ProjectilePool* pool, t_Camera camera)
{
	int frame = (SDL_GetTicks() / 80) % 3;
	SDL_Rect srcrect = { frame * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE };

	for (int i = 0; i < MAX_PROJECTILES; i++)
	{
		const t_Projectile* shot = &pool->shots[i];
		if (!shot->active)
			continue;
		SDL_Rect destrect = { shot->entity.rect.x - 1 - camera.viewport.x, shot->entity.rect.y - 1 - camera.viewport.y, 10, 10 };
		SDL_RenderCopy(renderer, pool->texture, &srcrect, &destrect);
	}
}
