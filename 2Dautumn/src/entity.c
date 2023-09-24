#include "headerfile/GAME.h"
#include "headerfile/ENTITY.h"

SDL_Texture* loadTexture(const char* filename) {
    SDL_Texture* texture = NULL;
    SDL_Surface* surface = IMG_Load(filename);
    if (surface != NULL) {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture == NULL) {
            fprintf(stderr, "Unable to create texture from surface: %s\n", SDL_GetError());
        }
        SDL_FreeSurface(surface);
    }
    else {
        fprintf(stderr, "Image not loading: %s, Error: %s\n", filename, IMG_GetError());
    }
    return texture;
}

void initPlayer(t_Player* p, const char* imageFile)
{
    p->entity.hp = 100;
    p->attackdmg = 1.0f;
    p->defence = 0;
    p->entity.state = IDLE;
    p->texture = loadTexture(imageFile);
    p->entity.rect.x = 10;
    p->entity.rect.y = 10;
    if (p->texture)
    {
        SDL_QueryTexture(p->texture, NULL, NULL, &(p->entity.rect.w), &(p->entity.rect.h));
    }
    else
    {
        fprintf(stderr, "texture not loading: %s, Error: %s\n", imageFile, SDL_GetError());

    }
    

}

void renderPlayer(const t_Player* p) {
    SDL_Rect destRect = { p->entity.rect.x, p->entity.rect.y, p->entity.rect.w, p->entity.rect.h };
    SDL_RenderCopy(renderer, p->texture, NULL, &destRect);
}
