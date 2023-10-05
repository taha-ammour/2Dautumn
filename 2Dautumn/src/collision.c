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
    SDL_IntersectRect(&entity1->rect, &entity2->rect, &intersection);

    int xOverlap = intersection.w;
    int yOverlap = intersection.h;

    if (xOverlap < yOverlap) {
        if (entity1->rect.x < entity2->rect.x) {
            entity1->rect.x -= (int)(xOverlap / 1.5);
        }
        else {
            entity1->rect.x += (int)(xOverlap / 1.5);
        }
    }
    else {
        if (entity1->rect.y < entity2->rect.y) {
            entity1->rect.y -= (int)(yOverlap / 1.5);
        }
        else {
            entity1->rect.y += (int)(yOverlap / 1.5);
        }
    }
}
