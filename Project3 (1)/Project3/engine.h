#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>

// 화면 크기
#define WIDTH 800
#define HEIGHT 600

// 함수
bool engine_init();
void engine_update();
void engine_render();
void engine_handle_event(SDL_Event* e);
void engine_cleanup();

void set_background_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

#endif