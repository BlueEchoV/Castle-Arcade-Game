#pragma once
#include <SDL.h>
#include <stb_image.h>

#include "Utility.h"
#include "Globals.h"

enum Color_Index {
	CI_BLACK,
	CI_RED,
	CI_GREEN,
	CI_BLUE
};

enum Image_Type {
	IT_BASIC_PARTICLE,

	IT_TOTAL
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
	const char* file_Path;
	SDL_Texture* texture;
	unsigned char* pixel_Data;
};

Image create_Image(const char* file_name);
