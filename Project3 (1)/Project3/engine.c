#include "Header.h"
#include <stdio.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
TTF_Font* smallFont = NULL;
SDL_Texture* fishTexture = NULL;

SDL_AudioDeviceID audioDevice = 0;
SDL_AudioSpec wavSpec;
Uint8* wavBuffer = NULL;
Uint32 wavLength = 0;

bool engine_init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        return 0;

    window = SDL_CreateWindow("Raising Fishes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window)
        return 0;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        return 0;

    if (TTF_Init() != 0)
        return 0;

    font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 20);
    if (!font) {
        printf("폰트 로드 실패: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }

    smallFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 13);
    if (!smallFont) {
        printf("작은 폰트 로드 실패: %s - 기본 폰트로 대체합니다.\n", TTF_GetError());
        smallFont = font;
    }

    fishTexture = loadTexture("C:\\2060033\\1\\Project3 (1)\\x64\\Debug\\fish.bmp");
    if (!fishTexture) {
        printf("물고기 이미지 로드 실패 - 사각형으로 대체합니다.\n");
    }

    if (!initAudio()) {
        printf("오디오 초기화 실패 - 소리 없이 진행합니다.\n");
    }

    return 1;
}

bool initAudio()
{
    if (SDL_LoadWAV("C:\\2060033\\1\\Project3 (1)\\x64\\Debug\\water.wav", &wavSpec, &wavBuffer, &wavLength) == NULL) {
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

void renderTextColor(const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderText(const char* text, int x, int y) {
    SDL_Color color = { 255, 255, 255 };
    renderTextColor(text, x, y, color);
}

void renderTextColorSmall(const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(smallFont, text, color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderTextSmall(const char* text, int x, int y) {
    SDL_Color color = { 255, 255, 255 };
    renderTextColorSmall(text, x, y, color);
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

    if (fishTexture) SDL_DestroyTexture(fishTexture);
    if (audioDevice != 0) SDL_CloseAudioDevice(audioDevice);
    if (wavBuffer != NULL) SDL_FreeWAV(wavBuffer);
    if (smallFont && smallFont != font) TTF_CloseFont(smallFont);
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();
}