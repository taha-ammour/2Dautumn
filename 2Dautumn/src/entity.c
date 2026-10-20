#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/ANIMATE.h"
#include "headerfile/GAME.h"

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
    /* LEAF_ORANGE  */ { 20, 5, 0, 40, 2, { 214, 110, 30, 255 } },
    /* LEAF_RED     */ { 26, 7, 1, 55, 2, { 200, 50, 40, 255 } },
    /* LEAF_GOLD    */ { 22, 5, 0, 45, 3, { 240, 200, 50, 255 } },
    /* LEAF_CRIMSON */ { 40, 9, 2, 35, 3, { 190, 60, 150, 255 } },
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
    ResetPlayer(p, 0, 0);
}

void ResetPlayer(t_Player* p, float x, float y)
{
    p->entity.hp = PLAYER_MAX_HP;
    p->attackdmg = 1.0f;
    p->defence = 0;
    p->entity.state = IDLE;
    p->entity.hurtTime = 0;
    p->attackTime = 0;
    p->flip = false;
    p->faceX = 0;
    p->faceY = 1;
    p->alive = true;
    p->respawnTimer = 0;

    p->entity.rect.w = TILE_SIZE;
    p->entity.rect.h = TILE_SIZE;
    SetEntityPos(&p->entity, x, y);
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
    e->flip = rand() % 2;
    e->entity.hurtTime = 0;
    e->mood = MOOD_IDLE;
    e->moodTimer = RandomFloat(0.5f, 2.5f);
    e->homeX = x;
    e->homeY = y;
    e->targetX = x;
    e->targetY = y;
    e->respawnTimer = 0;

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
    camera->x = 0;
    camera->y = 0;
}

void AddScreenShake(float amount)
{
    screenShake += amount;
    if (screenShake > 6.0f)
        screenShake = 6.0f;
}

static void cameraTarget(t_Player p, t_Camera* camera, float* x, float* y)
{
    float maxX = (float)(MAP_WIDTH * TILE_SIZE - camera->viewport.w);
    float maxY = (float)(MAP_HEIGHT * TILE_SIZE - camera->viewport.h);
    *x = SDL_clamp(p.entity.posX + 8 - camera->viewport.w / 2.0f, 0.0f, maxX);
    *y = SDL_clamp(p.entity.posY + 8 - camera->viewport.h / 2.0f, 0.0f, maxY);
}

void SnapCamera(t_Player p, t_Camera* camera)
{
    cameraTarget(p, camera, &camera->x, &camera->y);
    camera->viewport.x = (int)camera->x;
    camera->viewport.y = (int)camera->y;
}

void UpdateCameraPos(t_Player p, t_Camera* camera, float deltaTime) {
    float targetX, targetY;

    // ease towards the player instead of locking onto every step
    cameraTarget(p, camera, &targetX, &targetY);
    camera->x += (targetX - camera->x) * SDL_min(1.0f, deltaTime * 7.0f);
    camera->y += (targetY - camera->y) * SDL_min(1.0f, deltaTime * 7.0f);
    camera->viewport.x = (int)SDL_floorf(camera->x + 0.5f);
    camera->viewport.y = (int)SDL_floorf(camera->y + 0.5f);

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
    // they only use their walk cycle and idle rows, the third row holds other colors
    ENTITY_STATES row = e->entity.state == IDLE ? IDLE : WALK;
    animationFunctions[row](&(e->entity), &srcrect);

    SDL_Rect destRect = { e->entity.rect.x - camera.viewport.x, e->entity.rect.y - camera.viewport.y, e->entity.rect.w, e->entity.rect.h };
    SDL_RendererFlip flip = e->flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, e->texture, &srcrect, &destRect, 0, NULL, flip);
    if (isFlashing(&e->entity, 240))
    {
        // second additive pass makes the leaf flash bright when hit
        SDL_SetTextureBlendMode(e->texture, SDL_BLENDMODE_ADD);
        SDL_RenderCopyEx(renderer, e->texture, &srcrect, &destRect, 0, NULL, flip);
        SDL_SetTextureBlendMode(e->texture, SDL_BLENDMODE_BLEND);
    }

    // an angry leaf shows it, and its health once it has been hurt
    if (e->mood == MOOD_ANGRY)
    {
        SDL_Rect mark = { destRect.x + 7, destRect.y - 7, 2, 4 };
        SDL_Rect dot = { destRect.x + 7, destRect.y - 2, 2, 1 };
        SDL_SetRenderDrawColor(renderer, 255, 80, 60, 255);
        SDL_RenderFillRect(renderer, &mark);
        SDL_RenderFillRect(renderer, &dot);
    }
    if (e->entity.hp < e->maxHp)
    {
        SDL_Rect back = { destRect.x + 2, destRect.y - 1, 12, 2 };
        SDL_Rect front = back;
        front.w = (int)(12 * (e->entity.hp / e->maxHp));
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 40, 10, 15, 220);
        SDL_RenderFillRect(renderer, &back);
        SDL_SetRenderDrawColor(renderer, 240, 120, 40, 255);
        SDL_RenderFillRect(renderer, &front);
    }
}
