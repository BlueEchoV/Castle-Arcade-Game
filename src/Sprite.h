#pragma once
#include <vector>

#include "Image.h"

struct Sprite {
	SDL_Rect source_Rect;
	float radius;
	Image image;
};

struct Sprite_Sheet {
	std::vector<Sprite> sprites;
};

struct Sprite_Sheet_Tracker {
	std::string sprite_Sheet_Name;
	float animation_Time;
	int current_Frame;
};

Sprite_Sheet_Tracker create_Sprite_Sheet_Tracker(std::string sprite_Sheet_Name);

float return_Sprite_Radius(Sprite sprite);

float get_Sprite_Radius(Sprite_Sheet_Tracker* tracker);

Sprite create_Sprite(Image image, SDL_Rect source_Rect);

Sprite_Sheet create_Sprite_Sheet(const char* file_Path, int rows, int columns);

void draw_Layer(SDL_Texture* texture);

std::vector<std::string> split(const std::string& my_String, char delimiter);

void load_Sprite_Sheet_Data_CSV(const char* file_Path_CSV);

