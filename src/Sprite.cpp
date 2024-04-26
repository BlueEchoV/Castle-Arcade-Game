#include "Sprite.h"

Sprite_Sheet sprite_Sheet_Array[SSS_TOTAL_SPRITE_SHEETS] = {};

namespace Globals {
	std::unordered_map<std::string, Sprite_Sheet> sprite_Sheet_Data_Map = {};
}

Sprite_Sheet_Tracker create_Sprite_Sheet_Tracker(Sprite_Sheet_Selector selected) {
	Sprite_Sheet_Tracker result;

	result.selected = selected;
	result.animation_Time = 0.0f;
	result.current_Frame = 0;

	return result;
}

float return_Sprite_Radius(Sprite sprite) {
	float max_Distance = 0;
	for (int y = sprite.source_Rect.y; y < (sprite.source_Rect.h + sprite.source_Rect.y); y++) {
		for (int x = sprite.source_Rect.x; x < (sprite.source_Rect.w + sprite.source_Rect.x); x++) {
			int index = 0;
			index = (4 * ((y * sprite.image.width) + x)) + 3;
			if (sprite.image.pixel_Data[index] != 0) {
				float distance =
					calculate_Distance(
						(float)x,
						(float)y,
						(float)(sprite.source_Rect.x + sprite.source_Rect.w / 2),
						(float)(sprite.source_Rect.y + sprite.source_Rect.h / 2)
					);
				if (distance > max_Distance) {
					max_Distance = distance;
				}
			}
		}
	}
	return max_Distance;
}

// Returns the radius of the first sprite in the sprite sheet
// get_Radius_Of_First_Sprite_In_Selected_Sheet???? Way too long
float get_Sprite_Radius(Sprite_Sheet_Tracker* tracker) {
	const Sprite_Sheet* arr = sprite_Sheet_Array;
	Sprite_Sheet_Selector selected = tracker->selected;
	Sprite sprite = arr[selected].sprites[0];
	float radius = sprite.radius;
	return radius;
}

Sprite create_Sprite(Image image, SDL_Rect source_Rect) {
	Sprite result = {};

	result.image = image;
	// This is the width and height of the individual sprite
	result.source_Rect = source_Rect;
	result.radius = return_Sprite_Radius(result);

	return result;
}

Sprite_Sheet create_Sprite_Sheet(const char* file_Path, int rows, int columns) {
	Image image = create_Image(file_Path);
	Sprite_Sheet result = {};
	for (int c = 0; c < columns; ++c)
	{
		for (int r = 0; r < rows; ++r)
		{
			SDL_Rect source;
			source.x = (c * (image.width / columns));
			source.y = (r * (image.height / rows));
			source.w = (image.width / columns);
			source.h = (image.height / rows);
			result.sprites.push_back(create_Sprite(image, source));
		}
	}
	return result;
}

void add_Sprite_Sheet_To_Array(const char* file_Path, Sprite_Sheet_Selector selected, int rows, int columns) {
	Image image = create_Image(file_Path);
	for (int c = 0; c < columns; ++c)
	{
		for (int r = 0; r < rows; ++r)
		{
			SDL_Rect source;
			source.x = (c * (image.width / columns));
			source.y = (r * (image.height / rows));
			source.w = (image.width / columns);
			source.h = (image.height / rows);
			sprite_Sheet_Array[selected].sprites.push_back(create_Sprite(image, source));
		}
	}
}

void load_Image(const char* file_Path, Sprite_Sheet_Selector selected, int rows, int columns) {
	add_Sprite_Sheet_To_Array(file_Path, selected, rows, columns);
}

void load_Images() {
	load_Image("images/background.jpg", SSS_BKG_GAMELOOP_1, 1, 1);
	load_Image("images/background_2.png", SSS_BKG_MENU_1, 1, 1);
	load_Image("images/collision_Terrain_1.png", SSS_TERRAIN_1, 1, 1);
	load_Image("images/player_Castle.png", SSS_CASTLE_1, 1, 1);
	load_Image("images/game_Over.png", SSS_BKG_GAMEOVER, 1, 1);
	// images/unit_Warrior_Sprite_Sheet.png
	load_Image("images/arrow.png", SSS_ARROW_DEFAULT, 1, 1);
	load_Image("images/unit_Warrior_Short.png", SSS_Warrior_WALKING, 1, 1);
	load_Image("images/unit_Warrior_Short.png", SSS_Warrior_STOP, 1, 1);
	load_Image("images/unit_Archer_Short.png", SSS_ARCHER_WALKING, 1, 1);
	load_Image("images/unit_Archer_Short.png", SSS_ARCHER_STOP, 1, 1);
	load_Image("images/basic_Particle_1.png", SSS_BASIC_PARTICLE, 1, 1);
	load_Image("images/droplet_Particle_1.png", SSS_DROPLET_PARTICLE, 1, 1);
}

void draw_Layer(SDL_Texture* texture) {
	SDL_RenderCopyEx(Globals::renderer, texture, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
}

// Splits a string using a delimiter and returns a vector of strings
std::vector<std::string> split(const std::string& my_String, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	// Input stream class to operate on strings
	std::istringstream my_Stream(my_String);
	while (std::getline(my_Stream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

void load_Image_Data_CSV(const char* file_Path_CSV) {
	std::string my_String = std::string(file_Path_CSV);
	std::ifstream file(my_String);

	if (!file.is_open()) {
		SDL_Log("Error loading .csv file");
	}

	DEFER{
		file.close();
	};

	std::string line;
	// Skip the first line containing the headers and the count
	// NOTE: getline reads characters from an input stream and places them into a string: 
	std::getline(file, line);
	// I could parse out this row to know what each value is
	std::getline(file, line);

	while (std::getline(file, line)) {
		std::vector<std::string> tokens = split(line, ',');
		std::string file_Name = tokens[0];
		// images/basic_Particle_1.png
		std::string file_Path = "images/" + file_Name + ".png";
		if (tokens.size() == 3) {
			int rows = std::stoi(tokens[1]);
			int columns = std::stoi(tokens[2]);
			Sprite_Sheet sprite_Sheet = create_Sprite_Sheet(file_Path.c_str(), rows, columns);
			Globals::sprite_Sheet_Data_Map[file_Name] = sprite_Sheet;
		}
		else {
			SDL_Log("Error: Line does not have enough data");
		}
	}
}