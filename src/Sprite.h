#pragma once
#include <vector>

#include "Image.h"

enum Sprite_Sheet_Selector {
	SSS_SKELETON_WALKING,
	SSS_SKELETON_STOP,
	SSS_SKELETON_ATTACKING,
	SSS_SKELETON_DYING,

	SSS_ARCHER_WALKING,
	SSS_ARCHER_STOP,
	SSS_ARCHER_ATTACKING,
	SSS_ARCHER_DYING,

	SSS_ARROW_DEFAULT,

	SSS_CASTLE_1,

	SSS_BKG_GAMELOOP_1,
	SSS_BKG_MENU_1,
	SSS_BKG_GAMEOVER,

	SSS_TERRAIN_1,

	SSS_TOTAL_SPRITE_SHEETS
};

struct Sprite {
	SDL_Rect source_Rect;
	float radius;
	Image* image;
};

struct Sprite_Sheet {
	std::vector<Sprite> sprites;
};

struct Sprite_Sheet_Tracker {
	Sprite_Sheet_Selector selected;
	float animation_Time;
	int current_Frame;
};

// Global variable 
extern Sprite_Sheet sprite_Sheet_Array[SSS_TOTAL_SPRITE_SHEETS];

Sprite_Sheet_Tracker create_Sprite_Sheet_Tracker(Sprite_Sheet_Selector selected);

float return_Sprite_Radius(Sprite sprite);

float get_Sprite_Radius(Sprite_Sheet_Tracker* tracker);

Sprite create_Sprite(Image* image, SDL_Rect* source_Rect);

Sprite_Sheet create_Sprite_Sheet(Image* image, int rows, int columns);

void add_Sprite_Sheet_To_Array(Sprite_Sheet_Selector selected, Image* image, int rows, int columns);