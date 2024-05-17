#pragma once
#include <vector>
#include "Image.h"

struct Sprite {
	SDL_Rect source_Rect;
	float radius;
	Image image;
};

struct Sprite_Sheet_Data {
	std::string sprite_Sheet_Name;
	int rows;
	int columns;
};

struct Sprite_Sheet {
	std::vector<Sprite> sprites;
};

struct Sprite_Sheet_Tracker {
	std::string sprite_Sheet_Name;
	float animation_Time = 0.0f;
	int current_Frame = 0;
};

Sprite_Sheet_Tracker create_Sprite_Sheet_Tracker(std::string sprite_Sheet_Name);

float return_Sprite_Radius(Sprite sprite);

float get_Sprite_Radius(Sprite_Sheet_Tracker* tracker);

Sprite create_Sprite(Image image, SDL_Rect source_Rect);

Sprite_Sheet create_Sprite_Sheet(const char* file_Path, int rows, int columns);

void draw_Layer(SDL_Texture* texture);

void load_Sprite_Sheet_Data_CSV(CSV_Data* csv_Data);

Sprite_Sheet& get_Sprite_Sheet(std::string key);

SDL_Texture* get_Sprite_Sheet_Texture(std::string key);

void free_Pixel_Data_In_Sprite_Sheet_Map();
