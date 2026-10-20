#include "headerfile/COLLISION.h"

int areEntitiesColliding(const t_Entity* entity1, const t_Entity* entity2)
{
	return SDL_HasIntersection(&entity1->rect, &entity2->rect);
}

void resolveCollision(t_Entity* entity1, t_Entity* entity2)
{
    if (entity1->hp <=0 || entity2->hp <= 0)
    {
        return;
    }
    SDL_Rect intersection;
    if (!SDL_IntersectRect(&entity1->rect, &entity2->rect, &intersection))
    {
        return;
    }

    float xOverlap = (float)intersection.w;
    float yOverlap = (float)intersection.h;

    // push entity1 fully out of entity2 along the smallest overlap
    if (xOverlap < yOverlap) {
        if (entity1->rect.x < entity2->rect.x) {
            entity1->posX -= xOverlap;
        }
        else {
            entity1->posX += xOverlap;
        }
    }
    else {
        if (entity1->rect.y < entity2->rect.y) {
            entity1->posY -= yOverlap;
        }
        else {
            entity1->posY += yOverlap;
        }
    }
    SyncEntityRect(entity1);
}

void separateEntities(t_Entity* entity1, t_Entity* entity2)
{
    float dx = entity1->posX - entity2->posX;
    float dy = entity1->posY - entity2->posY;
    float distSq = dx * dx + dy * dy;
    float minDist = 10.0f;

    if (distSq >= minDist * minDist)
        return;
    if (distSq < 0.01f)
    {
        dx = (float)(rand() % 3 - 1);
        dy = (float)(rand() % 3 - 1);
        distSq = dx * dx + dy * dy + 0.01f;
    }
    float dist = sqrtf(distSq);
    float push = (minDist - dist) * 0.5f;
    entity1->posX += dx / dist * push;
    entity1->posY += dy / dist * push;
    entity2->posX -= dx / dist * push;
    entity2->posY -= dy / dist * push;
    SyncEntityRect(entity1);
    SyncEntityRect(entity2);
}
