#pragma once
#include <SDL.h>

#include "Utility.h"
#include "Global_Variables.h"

extern SDL_Renderer* renderer;

enum Color_Index {
	CI_BLACK,
	CI_RED,
	CI_GREEN,
	CI_BLUE
};

struct Color {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
};

// Chris does this instead of enums
const Color BLACK = { 0, 0, 0, 0 };

struct Image {
	int width;
	int height;
	SDL_Texture* texture;
	unsigned char* pixel_Data;
};