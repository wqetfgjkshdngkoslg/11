// SDL2를 이용한 GUI 기반 물고기 키우기 게임 샘플 코드 (기초 프레임 + 게임 로직 + 승리/패배 조건 + 이미지 추가)

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define NUM 6
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FISHTANK_WIDTH 100
#define FISHTANK_HEIGHT 200


typedef enum {
    FISH_NORMAL = 0,  // 기본 물고기
    FISH_FAST = 1,  // 빠르게 자라는 물고기
    FISH_BIG = 2   // 물을 많이 소비하는 물고기
}

FishType;

// 게임 상태 구조체 정의
typedef struct
{
    int fish;
    int water;
    int isAlive;
    FishType fishType;
}

FishTank;

// 전역 변수 정의
FishTank fishTanks[NUM];
int level = 1;
int position = 0;
bool running = true;
bool gameOver = false;
bool gameWin = false;
long startTime = 0;
long lastUpdateTime = 0;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
SDL_Texture* fishTexture = NULL;
SDL_AudioDeviceID audioDevice = 0;
SDL_AudioSpec wavSpec;
Uint8* wavBuffer = NULL;
Uint32 wavLength = 0;

// 함수 프로토타입 선언
bool engine_init();
void initGame();
void renderText(const char* text, int x, int y);
void renderFishTanks();
void updateGame();
void renderGame();
void cleanupGame();
void handleInput(SDL_Event* e);
SDL_Texture* loadTexture(const char* path);
bool initAudio();
void playWaterSound();


float getWaterConsumptionRate(FishType type) {
    switch (type) {
    case FISH_NORMAL: return 1.0f;  // 기본 소비량
    case FISH_FAST:   return 1.3f;  // 30% 더 소비
    case FISH_BIG:    return 2.0f;  // 2배 소비
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

// 메인 함수
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

bool engine_init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        return 0;
    window = SDL_CreateWindow("Raising Fishes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!window)
        return 0;
    if (TTF_Init() != 0)
        return 0;
    font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 20);
    if (!font) {
        printf("폰트 로드 실패: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }
    fishTexture = loadTexture("C:\\2060033\\1\\Project3 (1)\\x64\\Debug\\fish.bmp");
    if (!fishTexture) {
        SDL_Quit();
        return 0;
    }
    if (!initAudio()) {
        SDL_Quit();
        return 0;
    }
    return 1;
}

bool initAudio()
{
    if (SDL_LoadWAV("C:\\060033\\1\\Project3 (1)\\x64\\Debug\\water.wav", &wavSpec, &wavBuffer, &wavLength) == NULL) {
        printf("WAV 파일 로드 실패: %s\n", SDL_GetError());
        return false;
    }
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
    if (audioDevice == 0) {
        printf("오디오 장치 열기 실패: %s\n", SDL_GetError());
        SDL_FreeWAV(wavBuffer);
        wavBuffer = NULL;
        return false;
    }
    SDL_PauseAudioDevice(audioDevice, 0);
    return true;
}

void playWaterSound()
{
    if (audioDevice != 0 && wavBuffer != NULL) {
        SDL_ClearQueuedAudio(audioDevice);
        SDL_QueueAudio(audioDevice, wavBuffer, wavLength);
    }
}

void initGame() {
    for (int i = 0; i < NUM; i++) {
        fishTanks[i].fish = 10;
        fishTanks[i].water = 100;
        fishTanks[i].isAlive = 1;
        fishTanks[i].fishType = (FishType)(i % 3);
    }
    startTime = SDL_GetTicks();
    lastUpdateTime = startTime;
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

void cleanupGame() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    if (gameWin)
        renderText("You Win! All levels completed!", 200, 200);
    else if (gameOver)
        renderText("Game Over! All fish are dead!", 200, 200);
    else
        renderText("Game Over", 200, 200);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);

    SDL_DestroyTexture(fishTexture);
    if (audioDevice != 0)
        SDL_CloseAudioDevice(audioDevice);
    if (wavBuffer != NULL)
        SDL_FreeWAV(wavBuffer);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void renderText(const char* text, int x, int y) {
    SDL_Color color = { 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
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

        if (fishTanks[i].isAlive) {
            SDL_Rect fishRect = { x + 20, 300 + FISHTANK_HEIGHT - waterHeight - 30, 60, 30 };
            SDL_RenderCopy(renderer, fishTexture, NULL, &fishRect);
        }


        char typeLabel[4];
        switch (fishTanks[i].fishType) {
        case FISH_NORMAL: sprintf_s(typeLabel, sizeof(typeLabel), "N"); break;
        case FISH_FAST:   sprintf_s(typeLabel, sizeof(typeLabel), "F"); break;
        case FISH_BIG:    sprintf_s(typeLabel, sizeof(typeLabel), "B"); break;
        }
        renderText(typeLabel, x + 40, 275);

        // 물고기 수치 텍스트
        char status[64];
        if (fishTanks[i].isAlive)
            sprintf_s(status, sizeof(status), "F:%d W:%d", fishTanks[i].fish, fishTanks[i].water);
        else
            sprintf_s(status, sizeof(status), "DEAD");
        renderText(status, x + 10, 520);

        // 커서 표시
        if (i == position) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &bowl);
        }
    }
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

                if (fishTanks[i].water < 0) {
                    fishTanks[i].water = 0;
                    fishTanks[i].isAlive = 0;
                }


                if (fishTanks[i].water > 0) {
                    int growthBonus = (fishTanks[i].fishType == FISH_FAST) ? 2 : 1;
                    fishTanks[i].fish += ((fishTanks[i].water / 100 + 1) * elapsed * growthBonus);
                }
                if (fishTanks[i].fish > 100) fishTanks[i].fish = 100;

                aliveCount++;
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

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        printf("이미지 로드 실패: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}