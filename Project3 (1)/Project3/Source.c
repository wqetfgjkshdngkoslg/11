#include "Header.h"
#include <stdio.h>

FishTank fishTanks[NUM];
int level = 1;
int position = 0;
bool running = true;
bool gameOver = false;
bool gameWin = false;
long startTime = 0;
long lastUpdateTime = 0;

int main(int argc, char* argv[]) {
    if (!engine_init()) {
        printf("Error initializing engine: %s\n", SDL_GetError());
        return 1;
    }

    initGame();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
            handleInput(&event);
        }
        updateGame();
        renderGame();
        SDL_Delay(100);
    }
    cleanupGame();
    return 0;
}


float getWaterConsumptionRate(FishType type) {
    switch (type) {
    case FISH_NORMAL: return 1.0f;
    case FISH_FAST:   return 1.3f;
    case FISH_BIG:    return 2.0f;
    default:          return 1.0f;
    }
}

const char* getFishTypeName(FishType type) {
    switch (type) {
    case FISH_NORMAL: return "Normal";
    case FISH_FAST:   return "Fast";
    case FISH_BIG:    return "Big";
    default:          return "???";
    }
}

void initGame() {
    for (int i = 0; i < NUM; i++) {
        fishTanks[i].fish = 10;
        fishTanks[i].water = 100;
        fishTanks[i].health = MAX_HEALTH;
        fishTanks[i].fishLevel = 1;
        fishTanks[i].isAlive = 1;
        fishTanks[i].fishType = (FishType)(i % 3);
    }
    startTime = SDL_GetTicks();
    lastUpdateTime = startTime;
}

void updateGame() {
    long currentTime = SDL_GetTicks();
    long elapsed = (currentTime - lastUpdateTime) / 1000;
    if (elapsed > 0) {
        int aliveCount = 0;

        for (int i = 0; i < NUM; i++) {
            if (fishTanks[i].isAlive == 1) {

                float rate = getWaterConsumptionRate(fishTanks[i].fishType);

                int consumption = (int)(level * (fishTanks[i].fish / 20 + 1) * elapsed * rate);
                fishTanks[i].water -= consumption;
                if (fishTanks[i].water < 0) fishTanks[i].water = 0;

                if (fishTanks[i].water > 0) {
                    int growthBonus = (fishTanks[i].fishType == FISH_FAST) ? 2 : 1;
                    fishTanks[i].fish += ((fishTanks[i].water / 100 + 1) * elapsed * growthBonus);
                    if (fishTanks[i].fish > MAX_FISH) fishTanks[i].fish = MAX_FISH;

                    if (fishTanks[i].water >= 50) {
                        fishTanks[i].health += (int)elapsed;
                        if (fishTanks[i].health > MAX_HEALTH) fishTanks[i].health = MAX_HEALTH;
                    }
                }
                else {
                    fishTanks[i].health -= (int)(2 * elapsed);
                }

                if (fishTanks[i].health <= 0) {
                    fishTanks[i].health = 0;
                    fishTanks[i].isAlive = 0;
                }
                else {
                    aliveCount++;
                }

                if (fishTanks[i].fishLevel < MAX_FISH_LEVEL &&
                    fishTanks[i].fish >= fishTanks[i].fishLevel * GROWTH_PER_LEVEL) {
                    fishTanks[i].fishLevel++;
                }
            }
        }

        if (aliveCount == 0) {
            gameOver = true;
            running = false;
        }

        long totalElapsed = (currentTime - startTime) / 1000;
        if (totalElapsed / 20 > level - 1) {
            level++;
            if (level > 5) {
                level = 5;
                gameWin = true;
                running = false;
            }
        }

        lastUpdateTime = currentTime;
    }
}

void handleInput(SDL_Event* e) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
        case SDLK_j:
            if (position > 0) position--;
            break;
        case SDLK_l:
            if (position < NUM - 1) position++;
            break;
        case SDLK_k:
            if (fishTanks[position].water >= 0 && fishTanks[position].water < 100)
                fishTanks[position].water += 5;
            playWaterSound();
            if (fishTanks[position].water > 100) fishTanks[position].water = 100;
            break;
        case SDLK_ESCAPE:
            running = false;
            break;
        }
    }
}

void renderFishTanks() {
    for (int i = 0; i < NUM; i++) {
        int x = 50 + i * (FISHTANK_WIDTH + 10);

        SDL_Rect bowl = { x, 300, FISHTANK_WIDTH, FISHTANK_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderDrawRect(renderer, &bowl);

        int waterHeight = fishTanks[i].water * FISHTANK_HEIGHT / 100;
        SDL_Rect water = { x, 300 + FISHTANK_HEIGHT - waterHeight, FISHTANK_WIDTH, waterHeight };
        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
        SDL_RenderFillRect(renderer, &water);

        int fishW = 40 + (fishTanks[i].fishLevel - 1) * 10;
        int fishH = 30 + (fishTanks[i].fishLevel - 1) * 6;
        SDL_Rect fishRect = {
            x + (FISHTANK_WIDTH - fishW) / 2,
            300 + FISHTANK_HEIGHT - waterHeight - fishH,
            fishW, fishH
        };

        if (fishTanks[i].isAlive) {
            if (fishTexture) {
                SDL_RenderCopy(renderer, fishTexture, NULL, &fishRect);
            }
            else {
                switch (fishTanks[i].fishType) {
                case FISH_NORMAL: SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255); break;
                case FISH_FAST:   SDL_SetRenderDrawColor(renderer, 0, 230, 120, 255); break;
                case FISH_BIG:    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255); break;
                }
                SDL_RenderFillRect(renderer, &fishRect);
            }
        }
        else {
            SDL_SetRenderDrawColor(renderer, 110, 110, 110, 255);
            SDL_RenderFillRect(renderer, &fishRect);
        }

        char typeLabel[4];
        switch (fishTanks[i].fishType) {
        case FISH_NORMAL: sprintf_s(typeLabel, sizeof(typeLabel), "N"); break;
        case FISH_FAST:   sprintf_s(typeLabel, sizeof(typeLabel), "F"); break;
        case FISH_BIG:    sprintf_s(typeLabel, sizeof(typeLabel), "B"); break;
        }
        renderText(typeLabel, x + 40, 275);

        if (fishTanks[i].isAlive) {
            char line1[32], line2[32];
            sprintf_s(line1, sizeof(line1), "F:%d W:%d", fishTanks[i].fish, fishTanks[i].water);
            sprintf_s(line2, sizeof(line2), "H:%d Lv%d", fishTanks[i].health, fishTanks[i].fishLevel);
            renderTextSmall(line1, x + 2, 505);
            renderTextSmall(line2, x + 2, 522);
        }
        else {
            renderTextSmall("DEAD", x + 2, 505);
        }

        SDL_Color red = { 255, 70, 70, 255 };
        if (fishTanks[i].isAlive && fishTanks[i].water < WATER_WARNING)
            renderTextColorSmall("Low water!", x + 2, 539, red);
        if (fishTanks[i].isAlive && fishTanks[i].health < HEALTH_DANGER)
            renderTextColorSmall("DANGER!", x + 2, 556, red);

        if (i == position) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &bowl);
        }
    }
}

void renderGame() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    renderFishTanks();

    char levelText[64];
    sprintf_s(levelText, sizeof(levelText), "Level %d", level);
    renderText(levelText, 10, 10);

    renderText("N=Normal  F=Fast  B=Big", 10, 40);

    SDL_RenderPresent(renderer);
}