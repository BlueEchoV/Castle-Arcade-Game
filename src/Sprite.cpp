#include "Sprite.h"
#include <assert.h>

static std::unordered_map<std::string, Sprite_Sheet> sprite_Sheet_Map = {};

Sprite_Sheet bad_Sprite_Sheet;

void init_Sprites() {
	bad_Sprite_Sheet = create_Sprite_Sheet("images/unit_Skeleton.png", 1, 1);
}

Sprite_Sheet& get_Sprite_Sheet(std::string key) {
	auto it = sprite_Sheet_Map.find(key);
	if (it != sprite_Sheet_Map.end()) {
		return it->second;
	}

	assert(false);
	return bad_Sprite_Sheet;
}

SDL_Texture* get_Sprite_Sheet_Texture(std::string key) {
	Sprite_Sheet& sprite_Sheet = get_Sprite_Sheet(key);
	return sprite_Sheet.sprites[0].image.texture;
}

Sprite_Sheet_Tracker create_Sprite_Sheet_Tracker(std::string sprite_Sheet_Name) {
	Sprite_Sheet_Tracker result;

	result.sprite_Sheet_Name = sprite_Sheet_Name;
	result.animation_Time = 0.0f;
	result.current_Frame = 0;

	return result;
}

void free_Pixel_Data_In_Sprite_Sheet_Map() {
	for (const auto& value : sprite_Sheet_Map) {
		stbi_image_free(value.second.sprites[0].image.pixel_Data);
	}
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
	Sprite_Sheet sprite_Sheet = get_Sprite_Sheet(tracker->sprite_Sheet_Name);
	Sprite sprite = sprite_Sheet.sprites[0];
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

void draw_Layer(SDL_Texture* texture) {
	MP_RenderCopy(Globals::renderer, texture, NULL, NULL);
}

Type_Descriptor sprite_Sheet_Type_Descriptor[] = {
	FIELD(Sprite_Sheet_Data, DT_STRING, sprite_Sheet_Name),
	FIELD(Sprite_Sheet_Data, DT_INT, rows),
	FIELD(Sprite_Sheet_Data, DT_INT, columns),
};

void load_Sprite_Sheet_Data_CSV(CSV_Data* csv_Data) {
	csv_Data->file.open(csv_Data->file_Path);
	if (!csv_Data->file.is_open()) {
		log("ERROR: Unable to open CSV file");
		return;
	}
	DEFER{
		csv_Data->file.close();
	};

	int rows = count_CSV_Rows(csv_Data);
	if (rows <= 0) {
		return;
	}

	std::vector<Sprite_Sheet_Data> sprite_Sheets;
	sprite_Sheets.resize(rows);

	std::span<Type_Descriptor> sprite_Sheet_Descriptors(sprite_Sheet_Type_Descriptor);

	load_CSV_Data(csv_Data, (char*)sprite_Sheets.data(), sizeof(sprite_Sheets[0]), sprite_Sheet_Descriptors);

	for (Sprite_Sheet_Data& iterator : sprite_Sheets) {
		std::string sprite_Sheet_File_Path = "images/" + iterator.sprite_Sheet_Name + ".png";
		Sprite_Sheet sprite_Sheet = create_Sprite_Sheet(sprite_Sheet_File_Path.c_str(), iterator.rows, iterator.columns);
		sprite_Sheet_Map[iterator.sprite_Sheet_Name] = sprite_Sheet;
	}
}