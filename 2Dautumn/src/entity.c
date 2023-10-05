#include "headerfile/ENTITY.h"
#include "headerfile/MAP.h"
#include "headerfile/ANIMATE.h"
#include "headerfile/COLLISION.h"
#include "headerfile/GAME.h"


void InitPlayer(t_Player* p, const char* imageFile, int nFrames, int speedF)
{
    p->entity.hp = 100;
    p->attackdmg = 1.0f;
    p->defence = 0;
    p->entity.anim.animated = true;
    p->entity.anim.frames = nFrames;
    p->entity.anim.speedframe = speedF;
    p->entity.state = IDLE;
    p->texture = loadTexture(imageFile);
    
    if (!p->texture)
    {
          fprintf(stderr, "texture not loading: %s, Error: %s\n", imageFile, SDL_GetError());
    }
    p->entity.rect.w = TILE_SIZE;
    p->entity.rect.h = TILE_SIZE;
    p->entity.rect.x = WINDOWWIDTH /2 - p->entity.rect.w;
    p->entity.rect.y = WINDOWHEIGHT /2 - p->entity.rect.h + 48;
    
}

void InitEnnemy(t_Ennemy* e, const char* imageFile, int nFrames, int speedF)
{
    e->entity.anim.animated = true;
    e->entity.anim.frames = nFrames;
    e->entity.anim.speedframe = speedF;
    

    e->entity.hp = 25;
    e->drop = true;
    e->num_of_drop = 2;
    e->attackdmg = 2;
    e->defence = 1;

    e->entity.state = IDLE;
    e->texture = loadTexture(imageFile);
    
    e->entity.rect.x = rand() % ((MAP_WIDTH * TILE_SIZE) - 16) + 16;
    e->entity.rect.y = rand() % ((MAP_HEIGHT * TILE_SIZE) - 16) + 16 ;
    e->entity.rect.w = TILE_SIZE;
    e->entity.rect.h = TILE_SIZE;
    
}

void InitCamera(t_Camera* camera) {
    camera->Zoom =3.0f;
    camera->viewport.x = 0;
    camera->viewport.y = 0;
    camera->viewport.w = WINDOWWIDTH - 342 ;
    camera->viewport.h = WINDOWHEIGHT - 210;
}


void UpdateCameraPos(t_Player p, t_Camera* camera) {

    camera->viewport.x =(int) (p.entity.rect.x - WINDOWWIDTH / (2 * camera->Zoom)) + p.entity.rect.w;
    camera->viewport.y =(int) (p.entity.rect.y - WINDOWHEIGHT/ (2 * camera->Zoom)) + p.entity.rect.h;

    if (camera->viewport.x < 0)
        camera->viewport.x = 0;
    if (camera->viewport.y < 0)
        camera->viewport.y = 0;
    if (camera->viewport.x > camera->viewport.w)
        camera->viewport.x = camera->viewport.w;
    if (camera->viewport.y > camera->viewport.h)
        camera->viewport.y = camera->viewport.h;

    SDL_RenderSetScale(renderer, camera->Zoom, camera->Zoom);
}

void UpdatePlayerPos(t_Player* p , float distance, float deltaTime) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    float moveDistance = distance * deltaTime;
    int moved = 0;
    if (keys[SDL_SCANCODE_W])
    {
        p->entity.rect.y -= (int)moveDistance;
        moved = 1;
    }
    if (keys[SDL_SCANCODE_S])
    {
        p->entity.rect.y += (int)moveDistance;
        moved = 1;

    }
    if (keys[SDL_SCANCODE_A])
    {
        p->entity.rect.x -= (int)moveDistance;
        moved = 1;

    }
    if (keys[SDL_SCANCODE_D])
    {
        p->entity.rect.x += (int)moveDistance;
        moved = 1;

    }
    if (!moved)
    {
        p->entity.state = IDLE;
    }
    else {
        p->entity.state = WALK;
    }
    if (p->entity.rect.x < 16)
        p->entity.rect.x = 16;
    if (p->entity.rect.x > MAP_WIDTH * 16 - 32)
        p->entity.rect.x = MAP_WIDTH * 16 - 32;
    if (p->entity.rect.y < 16)
        p->entity.rect.y = 16;
    if (p->entity.rect.y > MAP_HEIGHT * 16 - 32)
        p->entity.rect.y = MAP_HEIGHT * 16 - 32;
}

void updateEnemyPos(t_Entity* ennemy, t_Entity* player, float speed, float deltaTime)
{
    float distance = CalcDistance(player->rect, ennemy->rect);
    float velocitx = 0, velocity = 0;
    if (distance < CHASE_DISTANCE)
    {
        velocitx = (player->rect.x - ennemy->rect.x) / distance * speed;
        velocity = (player->rect.y - ennemy->rect.y) / distance * speed;
        ennemy->state = WALK;
    }
    else {
        ennemy->state = IDLE;
    }
    ennemy->rect.x += (int)(velocitx * deltaTime);
    ennemy->rect.y += (int)(velocity * deltaTime);

}

void renderPlayer(const t_Player* p, t_Camera camera) {
    SDL_Rect srcrect;
    if (p->entity.anim.animated)
    {
        animationFunctions[p->entity.state](&(p->entity), &srcrect);
    }
    SDL_Rect destRect = { p->entity.rect.x - camera.viewport.x, p->entity.rect.y - camera.viewport.y, p->entity.rect.w, p->entity.rect.h};
    SDL_RenderCopy(renderer, p->texture, &srcrect, &destRect);
}

void renderEnnemy(const t_Ennemy* e, t_Camera camera) {
    SDL_Rect srcrect;
    if (e->entity.anim.animated) {

        animationFunctions[e->entity.state](&(e->entity), &srcrect);
    }
    SDL_Rect destRect = { e->entity.rect.x - camera.viewport.x, e->entity.rect.y - camera.viewport.y, e->entity.rect.w, e->entity.rect.h };
    SDL_RenderCopy(renderer, e->texture, &srcrect, &destRect);
}
