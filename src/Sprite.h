#pragma once
#include <vector>

#include "Image.h"

// Has to align with the .csv file
// Specify the type for the serialization
enum Sprite_Sheet_Selector : int {
	// Set the starting value to be the corresponding place 
	// in the .csv file. Also allocates the correct number 
	// of spots in the array
	SSS_Basic_Particle = 1,
	SSS_Arrow,
	SSS_Castle,
	SSS_Warrior_Stop,
	SSS_Archer_Walk,
	SSS_Archer_Stop,
	SSS_Collision_Terrain_1,
	SSS_Bkg_Gameloop,
	SSS_Bkg_Menu,

	SSS_TOTAL
};

struct Sprite {
	SDL_Rect source_Rect;
	float radius;
	Image image;
};

struct Sprite_Sheet {
	std::vector<Sprite> sprites;
};

struct Sprite_Sheet_Tracker {
	Sprite_Sheet_Selector sprite_Sheet_Selector;
	float animation_Time = 0.0f;
	int current_Frame = 0;
};

Sprite_Sheet_Tracker create_Sprite_Sheet_Tracker(Sprite_Sheet_Selector sprite_Sheet_Selector);

float return_Sprite_Radius(Sprite sprite);

float get_Sprite_Radius(Sprite_Sheet_Tracker* tracker);

Sprite create_Sprite(Image image, SDL_Rect source_Rect);

Sprite_Sheet create_Sprite_Sheet(const char* file_Path, int rows, int columns);

void draw_Layer(SDL_Texture* texture);

std::vector<std::string> split(const std::string& my_String, char delimiter);

void load_Sprite_Sheet_Data_CSV(const char* file_Path_CSV);

