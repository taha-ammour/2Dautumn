#include "headerfile/COLLISION.h"

int areEntitiesColliding(const t_Entity* entity1, const t_Entity* entity2)
{
	return SDL_HasIntersection(&entity1->rect, &entity2->rect);
}

void resolveCollision(t_Entity* entity1, t_Entity* entity2)
{
    SDL_Rect intersection;
    SDL_IntersectRect(&entity1->rect, &entity2->rect, &intersection);

    int xOverlap = intersection.w;
    int yOverlap = intersection.h;

    if (xOverlap < yOverlap) {
        if (entity1->rect.x < entity2->rect.x) {
            entity1->rect.x -= xOverlap / 2;
            entity2->rect.x += xOverlap / 2;
        }
        else {
            entity1->rect.x += xOverlap / 2;
            entity2->rect.x -= xOverlap / 2;
        }
    }
    else {
        if (entity1->rect.y < entity2->rect.y) {
            entity1->rect.y -= yOverlap / 2;
            entity2->rect.y += yOverlap / 2;
        }
        else {
            entity1->rect.y += yOverlap / 2;
            entity2->rect.y -= yOverlap / 2;
        }
    }
}
