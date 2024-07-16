#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <filesystem>

#define INVLIGS 5
#define INVCOLS 10
#define INVSPCE 40
#define NBSHOOT 5
#define SPEEDSHOOT 20
#define ALIVE 1
#define DEAD 0
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

namespace fs = std::filesystem;

struct Sprite {
    SDL_Surface* frames[2];
};

struct Invader {
    int x;
    int y;
    int status;
    Sprite* sprite;
};

struct Projectile {
    int x;
    int y;
    int status;
};

SDL_Renderer* gRenderer = nullptr;
SDL_Window* gWindow = nullptr;
Sprite sprites[6];
Invader invaders[INVLIGS][INVCOLS];
Projectile shoot[NBSHOOT];

void loadSprites();
void initInvaders();
int drawInvaders(int offX, int offY, int version);
void drawVaisseau();
void drawTir();
void handleShooting();
void analyseTir(int offX, int offY);
SDL_Texture* loadTextureFromSurface(SDL_Surface* surface);
std::string absolutePath = "";

int main(int argc, char* argv[])
{
    absolutePath = fs::absolute(fs::path(argv[0])).parent_path().string();

    int offX = 2;
    int offY = 32;
    int speedX = 1;
    int flag;
    int page = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Unable to load SDL : " << SDL_GetError() << std::endl;
        return 1;
    }

    gWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr)
    {
        std::cerr << "Unable to create window : " << SDL_GetError() << std::endl;
        return 1;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == nullptr)
    {
        std::cerr << "Unable to load renderer : " << SDL_GetError() << std::endl;
        return 1;
    }
    

    loadSprites();
    initInvaders();

    bool running = true;
    SDL_Event e;
    while (running)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
        SDL_RenderClear(gRenderer);

        if ((flag = drawInvaders(offX += speedX, offY, (offX / 4) % 2)))
        {
            speedX *= -1; 
            offY += INVSPCE / 2;
        }

        drawVaisseau();
        handleShooting();
        drawTir();
        analyseTir(offX, offY);

        SDL_RenderPresent(gRenderer);
        SDL_Delay(20);
    }

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    return 0;
}

void loadSprites()
{

    sprites[0].frames[0] = SDL_LoadBMP((absolutePath + "/../../assets/Invader3A.bmp").c_str());
    sprites[0].frames[1] = SDL_LoadBMP((absolutePath + "/../../assets/Invader3B.bmp").c_str());
    sprites[1].frames[0] = SDL_LoadBMP((absolutePath + "/../../assets/Invader2A.bmp").c_str());
    sprites[1].frames[1] = SDL_LoadBMP((absolutePath + "/../../assets/Invader2B.bmp").c_str());
    sprites[2].frames[0] = SDL_LoadBMP((absolutePath + "/../../assets/Invader1A.bmp").c_str());
    sprites[2].frames[1] = SDL_LoadBMP((absolutePath + "/../../assets/Invader1B.bmp").c_str());

    sprites[3].frames[0] = SDL_LoadBMP((absolutePath + "/../../assets/Vaisseau.bmp").c_str());
    sprites[3].frames[1] = SDL_LoadBMP((absolutePath + "/../../assets/Vaisseau.bmp").c_str());
    sprites[4].frames[0] = SDL_LoadBMP((absolutePath + "/../../assets/Tir.bmp").c_str());
    sprites[4].frames[1] = SDL_LoadBMP((absolutePath + "/../../assets/Tir.bmp").c_str());

    sprites[5].frames[0] = SDL_LoadBMP((absolutePath + "/../../assets/Explode.bmp").c_str());
    sprites[5].frames[1] = SDL_LoadBMP((absolutePath + "/../../assets/Explode.bmp").c_str());
}

void initInvaders()
{
    for (int i = 0; i < INVLIGS; i++)
    {
        for (int j = 0; j < INVCOLS; j++)
        {
            invaders[i][j].sprite = &sprites[(i + 1) / 2];
            invaders[i][j].x = INVSPCE * j;
            invaders[i][j].y = INVSPCE * i;
            invaders[i][j].status = ALIVE;
        }
    }

    for (int i = 0; i < NBSHOOT; i++)
    {
        shoot[i].status = DEAD;
    }
}

int drawInvaders(int offX, int offY, int version)
{
    int flag = 0;
    for (int i = 0; i < INVLIGS; i++)
    {
        for (int j = 0; j < INVCOLS; j++)
        {
            if (invaders[i][j].status > 0)
            {
                SDL_Rect dest = { invaders[i][j].x + offX, invaders[i][j].y + offY, INVSPCE, INVSPCE };
                SDL_Texture* texture = loadTextureFromSurface(invaders[i][j].sprite->frames[version]);
                SDL_RenderCopy(gRenderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
                if (invaders[i][j].status != ALIVE)
                {
                    invaders[i][j].status--;
                }
            }
            if ((invaders[i][j].x + offX >= (SCREEN_WIDTH - INVSPCE)) || (invaders[i][j].x + offX <= 0))
            {
                flag = 1;
            }
        }
    }
    return flag;
}

void drawVaisseau()
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    SDL_Rect dest = { x, SCREEN_HEIGHT - 100, INVSPCE, INVSPCE };
    SDL_Texture* texture = loadTextureFromSurface(sprites[3].frames[0]);
    SDL_RenderCopy(gRenderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
}

void handleShooting()
{
    static int delayTir = 0;
    int x, y;
    Uint32 btn;
    SDL_GetMouseState(&x, &y);
    btn = SDL_GetMouseState(NULL, NULL);

    if (!delayTir && (btn & SDL_BUTTON(SDL_BUTTON_LEFT)))
    {
        for (int i = 0; i < NBSHOOT; i++)
        {
            if (shoot[i].status == DEAD)
            {
                shoot[i].x = x + INVSPCE / 2;
                shoot[i].y = SCREEN_HEIGHT - 102;
                shoot[i].status = ALIVE;
                delayTir = 10;
                break;
            }
        }
    }
    else if (delayTir > 0)
    {
        delayTir--;
    }
}

void drawTir()
{
    for (int i = 0; i < NBSHOOT; i++)
    {
        if (shoot[i].status)
        {
            if ((shoot[i].y -= SPEEDSHOOT) >= 0)
            {
                SDL_Rect dest = { shoot[i].x, shoot[i].y, INVSPCE/4, INVSPCE/4 };
                SDL_Texture* texture = loadTextureFromSurface(sprites[4].frames[0]);
                SDL_RenderCopy(gRenderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            else
            {
                shoot[i].status = DEAD;
            }
        }
    }
}

void analyseTir(int offX, int offY)
{
    int caseX = 0;
    int caseY = 0;

    for (int i = 0; i < NBSHOOT; i++)
    {
        if (shoot[i].status)
        {
            caseX = (shoot[i].x - offX) / INVSPCE;
            caseY = (shoot[i].y - offY) / INVSPCE;

            if ((caseX >= 0 && caseX < INVCOLS) && (caseY >= 0 && caseY < INVLIGS))
            {
                if (invaders[caseY][caseX].status == ALIVE)
                {
                    invaders[caseY][caseX].status = DEAD;
                    invaders[caseY][caseX].sprite = &sprites[5];
                    shoot[i].status = DEAD;
                }
            }
        }
    }
}
SDL_Texture* loadTextureFromSurface(SDL_Surface* surface)
{
    SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surface);
    if (texture == nullptr)
    {
        std::cerr << "loading texture failed : " << SDL_GetError() << std::endl;
    }
    return texture;
}