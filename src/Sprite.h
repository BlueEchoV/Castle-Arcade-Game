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

std::vector<std::string> split(const std::string& my_String, char delimiter);

void load_Sprite_Sheet_Data_CSV(std::string file_Path);

Sprite_Sheet& get_Sprite_Sheet(std::string key);

SDL_Texture* get_Sprite_Sheet_Texture(std::string key);

void free_Pixel_Data_In_Sprite_Sheet_Map();

enum Data_Type {
	MT_BOOL,
	MT_INT,
	MT_FLOAT,
	MT_STRING
};

struct Type_Descriptor {
	Data_Type variable_Type;
	int variable_Offset;
	std::string column_Name;
};

// Macro named FIELD
// data_Type: This is the type of the field, passed as an argument to the macro.
// offsetof(struct_Type, name): This macro is used to determine the offset of a member within a struct.
//					  It returns the byte offset of name within the struct_Type. This assumes 
//					  that name is a member of the struct_Type.
// #name: This is a preprocessor operator that turns the name into a string literal.
#define FIELD(struct_Type, data_Type, name) { data_Type, offsetof(struct_Type, name), #name }

int count_CSV_Rows(std::string file_Name);
int get_Column_Index(std::vector<std::string> column_Names, std::string current_Column_Name);
void load_CSV(std::string file_Name, char* destination, size_t stride, Type_Descriptor* type_Descriptors, int total_Descriptors);