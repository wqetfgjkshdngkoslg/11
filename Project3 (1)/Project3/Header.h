#ifndef HEADER_H
#define HEADER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>


#define NUM 6
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FISHTANK_WIDTH 100
#define FISHTANK_HEIGHT 200

#define MAX_WATER 100
#define MAX_FISH 100      
#define MAX_HEALTH 100

#define WATER_WARNING 20  
#define HEALTH_DANGER 30  

#define GROWTH_PER_LEVEL 40 
#define MAX_FISH_LEVEL 3   


typedef enum {
    FISH_NORMAL = 0,  
    FISH_FAST = 1,    
    FISH_BIG = 2     
} FishType;


typedef struct
{
    int fish;          
    int water;         
    int health;        
    int fishLevel;    
    int isAlive;      
    FishType fishType; 
} FishTank;


extern FishTank fishTanks[NUM];
extern int level;         
extern int position;       
extern bool running;
extern bool gameOver;
extern bool gameWin;
extern long startTime;
extern long lastUpdateTime;

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern TTF_Font* font;
extern SDL_Texture* fishTexture;

extern SDL_AudioDeviceID audioDevice;
extern SDL_AudioSpec wavSpec;
extern Uint8* wavBuffer;
extern Uint32 wavLength;


bool engine_init();
bool initAudio();
void playWaterSound();
SDL_Texture* loadTexture(const char* path);
void renderText(const char* text, int x, int y);
void renderTextColor(const char* text, int x, int y, SDL_Color color);
void cleanupGame();


void initGame();
void updateGame();
void renderGame();
void renderFishTanks();
void handleInput(SDL_Event* e);

// 물고기 종류별 규칙
float getWaterConsumptionRate(FishType type);
const char* getFishTypeName(FishType type);

#endif // HEADER_H