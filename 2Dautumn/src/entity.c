#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/ANIMATE.h"
#include "headerfile/COLLISION.h"
#include "headerfile/GAME.h"
#include "headerfile/COMBAT.h"

static float screenShake = 0.0f;

typedef struct {
    double hp;
    double attackdmg;
    int defence;
    float speed;
    int num_of_drop;
    SDL_Color tint;
}t_EnemyStats;

static const t_EnemyStats enemyStats[LEAF_KIND_COUNT] = {
    /* LEAF_ORANGE  */ { 25, 5, 1, 52, 1, { 255, 255, 255, 255 } },
    /* LEAF_RED     */ { 16, 4, 0, 85, 1, { 255, 120, 110, 255 } },
    /* LEAF_GOLD    */ { 30, 7, 1, 44, 2, { 255, 235, 90, 255 } },
    /* LEAF_CRIMSON */ { 60, 10, 3, 32, 3, { 220, 90, 200, 255 } },
};


void SetEntityPos(t_Entity* e, float x, float y)
{
    e->posX = x;
    e->posY = y;
    SyncEntityRect(e);
}

void SyncEntityRect(t_Entity* e)
{
    e->rect.x = (int)SDL_floorf(e->posX + 0.5f);
    e->rect.y = (int)SDL_floorf(e->posY + 0.5f);
}

void InitPlayer(t_Player* p, const char* imageFile, int nFrames, int speedF)
{
    p->entity.anim.animated = true;
    p->entity.anim.frames = nFrames;
    p->entity.anim.speedframe = speedF;
    p->texture = loadTexture(imageFile);

    if (!p->texture)
    {
          fprintf(stderr, "texture not loading: %s, Error: %s\n", imageFile, SDL_GetError());
    }
    ResetPlayer(p);
}

void ResetPlayer(t_Player* p)
{
    p->entity.hp = PLAYER_MAX_HP;
    p->attackdmg = 1.0f;
    p->defence = 0;
    p->entity.state = IDLE;
    p->entity.hurtTime = 0;
    p->attackTime = 0;
    p->flip = false;
    p->alive = true;
    p->respawnTimer = 0;

    p->entity.rect.w = TILE_SIZE;
    p->entity.rect.h = TILE_SIZE;
    SetEntityPos(&p->entity, (float)(BASEPOINT_X * TILE_SIZE), (float)(BASEPOINT_Y * TILE_SIZE + 32));
}

void InitEnnemy(t_Ennemy* e, SDL_Texture* texture, ENEMY_KIND kind, float x, float y)
{
    const t_EnemyStats* stats = &enemyStats[kind];

    e->entity.anim.animated = true;
    e->entity.anim.frames = 3;
    e->entity.anim.speedframe = 200;

    e->kind = kind;
    e->alive = true;
    e->entity.hp = stats->hp;
    e->maxHp = stats->hp;
    e->drop = true;
    e->num_of_drop = stats->num_of_drop;
    e->attackdmg = stats->attackdmg;
    e->defence = stats->defence;
    e->speed = stats->speed * RandomFloat(0.9f, 1.1f);
    e->attackCooldown = 0;
    e->flip = false;
    e->entity.hurtTime = 0;

    e->entity.state = IDLE;
    e->texture = texture;

    e->entity.rect.w = TILE_SIZE;
    e->entity.rect.h = TILE_SIZE;
    SetEntityPos(&e->entity, x, y);
}

SDL_Color GetEnemyTint(ENEMY_KIND kind)
{
    return enemyStats[kind].tint;
}

void InitCamera(t_Camera* camera) {
    camera->Zoom =3.0f;
    camera->viewport.x = 0;
    camera->viewport.y = 0;
    camera->viewport.w = (int)SDL_ceilf(WINDOWWIDTH / camera->Zoom);
    camera->viewport.h = (int)SDL_ceilf(WINDOWHEIGHT / camera->Zoom);
}

void AddScreenShake(float amount)
{
    screenShake += amount;
    if (screenShake > 6.0f)
        screenShake = 6.0f;
}

void UpdateCameraPos(t_Player p, t_Camera* camera, float deltaTime) {
    int maxX = MAP_WIDTH * TILE_SIZE - camera->viewport.w;
    int maxY = MAP_HEIGHT * TILE_SIZE - camera->viewport.h;

    camera->viewport.x = p.entity.rect.x + p.entity.rect.w / 2 - camera->viewport.w / 2;
    camera->viewport.y = p.entity.rect.y + p.entity.rect.h / 2 - camera->viewport.h / 2;

    if (camera->viewport.x < 0)
        camera->viewport.x = 0;
    if (camera->viewport.y < 0)
        camera->viewport.y = 0;
    if (camera->viewport.x > maxX)
        camera->viewport.x = maxX;
    if (camera->viewport.y > maxY)
        camera->viewport.y = maxY;

    if (screenShake > 0.0f)
    {
        camera->viewport.x += (int)RandomFloat(-screenShake, screenShake);
        camera->viewport.y += (int)RandomFloat(-screenShake, screenShake);
        screenShake -= deltaTime * 20.0f;
        if (screenShake < 0.0f)
            screenShake = 0.0f;
    }

    SDL_RenderSetScale(renderer, camera->Zoom, camera->Zoom);
}

void UpdatePlayerPos(t_Player* p , float distance, float deltaTime) {
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
    p->entity.posX += dirX * distance * deltaTime;
    p->entity.posY += dirY * distance * deltaTime;

    if (dirX < 0)
        p->flip = true;
    else if (dirX > 0)
        p->flip = false;

    if (SDL_GetTicks() - p->attackTime < ATTACK_ANIM_TIME)
    {
        p->entity.state = ATTACK;
    }
    else if (dirX == 0 && dirY == 0)
    {
        p->entity.state = IDLE;
    }
    else {
        p->entity.state = WALK;
    }
    if (p->entity.posX < 16)
        p->entity.posX = 16;
    if (p->entity.posX > MAP_WIDTH * 16 - 32)
        p->entity.posX = MAP_WIDTH * 16 - 32;
    if (p->entity.posY < 16)
        p->entity.posY = 16;
    if (p->entity.posY > MAP_HEIGHT * 16 - 32)
        p->entity.posY = MAP_HEIGHT * 16 - 32;
    SyncEntityRect(&p->entity);
}

void updateEnemyPos(t_Ennemy* ennemy, t_Entity* player, SDL_Rect target, float windX, float windY, float speedScale, float deltaTime)
{
    t_Entity* e = &ennemy->entity;
    float distance = player != NULL ? CalcDistance(player->rect, e->rect) : 1e9f;
    float distanceOBJ = CalcDistance(target, e->rect);
    float velocitx = 0, velocity = 0;
    float speed = ennemy->speed * speedScale;
    // red leaves are drawn to the player, crimson ones only care about the seed
    float chaseDistance = ennemy->kind == LEAF_RED ? CHASE_DISTANCE * 1.4f :
        ennemy->kind == LEAF_CRIMSON ? 0.0f : (float)CHASE_DISTANCE;

    if (distance < chaseDistance && distance > 1.0f)
    {
        velocitx = (player->rect.x - e->rect.x) / distance * speed;
        velocity = (player->rect.y - e->rect.y) / distance * speed;
        e->state = WALK;
    }
    else if(distanceOBJ > SEED_ATTACK_RANGE){
        velocitx = (target.x - e->rect.x) / distanceOBJ * speed;
        velocity = (target.y - e->rect.y) / distanceOBJ * speed;
        e->state = WALK;
    }
    else
    {
        e->state = ATTACK;
    }

    // leaves are light, the wind carries them along
    if (e->state == WALK)
    {
        velocitx += windX * 0.5f;
        velocity += windY * 0.5f;
    }
    if (velocitx < 0)
        ennemy->flip = true;
    else if (velocitx > 0)
        ennemy->flip = false;

    e->posX += velocitx * deltaTime;
    e->posY += velocity * deltaTime;

    if (e->posX < 0)
        e->posX = 0;
    if (e->posX > MAP_WIDTH * TILE_SIZE - TILE_SIZE)
        e->posX = (float)(MAP_WIDTH * TILE_SIZE - TILE_SIZE);
    if (e->posY < 0)
        e->posY = 0;
    if (e->posY > MAP_HEIGHT * TILE_SIZE - TILE_SIZE)
        e->posY = (float)(MAP_HEIGHT * TILE_SIZE - TILE_SIZE);
    SyncEntityRect(e);
}

static bool isFlashing(const t_Entity* e, Uint32 duration)
{
    Uint32 since = SDL_GetTicks() - e->hurtTime;
    return e->hurtTime != 0 && since < duration && (since / 80) % 2 == 0;
}

void renderPlayer(const t_Player* p, t_Camera camera) {
    SDL_Rect srcrect = { 0, 0, TILE_SIZE, TILE_SIZE };
    if (!p->alive)
        return;
    if (p->entity.anim.animated)
    {
        animationFunctions[p->entity.state](&(p->entity), &srcrect);
    }
    SDL_Rect destRect = { p->entity.rect.x - camera.viewport.x, p->entity.rect.y - camera.viewport.y, p->entity.rect.w, p->entity.rect.h};
    if (isFlashing(&p->entity, PLAYER_IFRAMES))
        SDL_SetTextureColorMod(p->texture, 255, 90, 90);
    SDL_RenderCopyEx(renderer, p->texture, &srcrect, &destRect, 0, NULL, p->flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    SDL_SetTextureColorMod(p->texture, 255, 255, 255);
}

void renderEnnemy(const t_Ennemy* e, t_Camera camera) {
    SDL_Rect srcrect = { 0, 0, TILE_SIZE, TILE_SIZE };
    SDL_Color tint = GetEnemyTint(e->kind);
    if (e->entity.anim.animated) {

        animationFunctions[e->entity.state](&(e->entity), &srcrect);
    }
    SDL_Rect destRect = { e->entity.rect.x - camera.viewport.x, e->entity.rect.y - camera.viewport.y, e->entity.rect.w, e->entity.rect.h };
    SDL_RendererFlip flip = e->flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_SetTextureColorMod(e->texture, tint.r, tint.g, tint.b);
    SDL_RenderCopyEx(renderer, e->texture, &srcrect, &destRect, 0, NULL, flip);
    if (isFlashing(&e->entity, 240))
    {
        // second additive pass makes the leaf flash bright when hit
        SDL_SetTextureBlendMode(e->texture, SDL_BLENDMODE_ADD);
        SDL_RenderCopyEx(renderer, e->texture, &srcrect, &destRect, 0, NULL, flip);
        SDL_SetTextureBlendMode(e->texture, SDL_BLENDMODE_BLEND);
    }
    SDL_SetTextureColorMod(e->texture, 255, 255, 255);

    // small health bar once the leaf is damaged
    if (e->entity.hp < e->maxHp)
    {
        SDL_Rect back = { destRect.x + 2, destRect.y - 3, 12, 2 };
        SDL_Rect front = back;
        front.w = (int)(12 * (e->entity.hp / e->maxHp));
        SDL_SetRenderDrawColor(renderer, 40, 10, 15, 220);
        SDL_RenderFillRect(renderer, &back);
        SDL_SetRenderDrawColor(renderer, 240, 120, 40, 255);
        SDL_RenderFillRect(renderer, &front);
    }
}
