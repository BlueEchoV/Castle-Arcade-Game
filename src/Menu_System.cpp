#include "Menu_System.h"
#include "stack"


Font font_1 = {};
Game_State current_Game_State = GS_GAMELOOP;
Game_Data game_Data_New_Game = {};
Game_Data game_Data = {};

std::unordered_map<SDL_Keycode, Key_State> key_States;

bool mouse_Down_This_Frame = false;
std::string current_frame_Hot_Name = "";
std::string next_Frame_Hot_Name = "";

void reset_Pressed_This_Frame() {
	for (auto& key_State : key_States) {
		key_States[key_State.first].pressed_This_Frame = false;
	}
}

Font load_Font_Bitmap(const char* font_File_Path) {
	Font result = {};

	int width, height, channels;
	unsigned char* data = stbi_load(font_File_Path, &width, &height, &channels, 4);

	DEFER{
		stbi_image_free(data);
	};

	result.width = width;
	result.height = height;

	for (int y = 0; y < result.height; y++) {
		for (int x = 0; x < result.width; x++) {
			int index = 0;
			index = (4 * ((y * width) + x));
			// Check if the color is black
			if (data[index] == 0 && data[index + 1] == 0 && data[index + 2] == 0) {
				// Set alpha to 0 (Transparent)
				data[index + 3] = 0;
			}
		}
	}

	// Hard coded values (Potentially bad)
	// This function is for bitmap fonts so 
	// this could be okay.
	result.char_Width = result.width / 18;
	result.char_Height = result.height / 7;

	SDL_Texture* temp = SDL_CreateTexture(Globals::renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

	if (temp == NULL) {
		SDL_Log("ERROR: SDL_CreateTexture returned NULL: %s", SDL_GetError());
		return result;
	}

	if (SDL_SetTextureBlendMode(temp, SDL_BLENDMODE_BLEND) != 0) {
		SDL_Log("ERROR: SDL_SsetTextureBlendMode did not succeed: %s", SDL_GetError());
	}

	result.texture = temp;

	void* pixels;
	int pitch;
	SDL_LockTexture(temp, NULL, &pixels, &pitch);

	// Copy what's in my data into the pixels
	// my_Memory_Copy();
	// memcpy(pixels, data, (width * height) * 4);
	my_Memory_Copy(pixels, data, (width * height) * 4);

	SDL_UnlockTexture(temp);

	return result;
}

void draw_Character(char character, int position_X, int position_Y, int size) {
    int ascii_Dec = (int)character - (int)' ';
    int chars_Per_Row = (font_1.width / font_1.char_Width);

    SDL_Rect src_Rect = {};
    // Position in the row
    src_Rect.x = (ascii_Dec % chars_Per_Row) * font_1.char_Width;
    // Which row the char is in
    src_Rect.y = (ascii_Dec / chars_Per_Row) * font_1.char_Height;
    src_Rect.w = font_1.char_Width;
    src_Rect.h = font_1.char_Height;

    SDL_Rect dest_Rect = {};
    // Position of the character on the screen
    dest_Rect.x = position_X;
    dest_Rect.y = position_Y;
    dest_Rect.w = (int)(font_1.char_Width * size);
    dest_Rect.h = (int)(font_1.char_Height * size);

    SDL_RenderCopyEx(Globals::renderer, font_1.texture, &src_Rect, &dest_Rect, 0, NULL, SDL_FLIP_NONE);
}

void draw_String(const char* string, int position_X, int position_Y, int size, bool center) {
    int offset_X = 0;
    int index = 0;
    char iterator = string[index];
    size_t length_Pixels = strlen(string);
    length_Pixels *= font_1.char_Width * size;

    int char_Position_X = 0;
    int char_Position_Y = 0;
    if (center) {
        char_Position_X = (position_X - (int)(length_Pixels / 2));
        char_Position_Y = (position_Y - ((font_1.char_Height * size) / 2));
    }
    else {
        char_Position_X = position_X;
        char_Position_Y = position_Y;
    }

    while (iterator != '\0') {
        char_Position_X += offset_X;
        draw_Character(iterator, char_Position_X, char_Position_Y, size);
        offset_X = font_1.char_Width * size;
        index++;
        iterator = string[index];
    }
}

void draw_String_With_Background(const char* string, int position_X, int position_Y, int size, bool center, Color_Index color, int outline_Padding) {
    size_t length_Pixels = strlen(string);
    length_Pixels *= font_1.char_Width;
    length_Pixels *= size;
    size_t height_Pixels = font_1.char_Height;
    height_Pixels *= size;

    SDL_Rect canvas_Area = {};
    // Centers the button
    canvas_Area.x = (int)((position_X - (length_Pixels / 2)) - (outline_Padding / 2));
    canvas_Area.y = (int)((position_Y - (height_Pixels / 2)) - (outline_Padding / 2));
    canvas_Area.w = (int)length_Pixels + outline_Padding;
    canvas_Area.h = (int)height_Pixels + outline_Padding;

    // Set background as black
    if (color == CI_BLACK) {
        SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    }
    else if (color == CI_RED) {
        SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    }
    else if (color == CI_GREEN) {
        SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
    }
    else if (color == CI_BLUE) {
        SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    }
    else {
        SDL_SetRenderDrawColor(Globals::renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderFillRect(Globals::renderer, &canvas_Area);

    draw_String(string, position_X, position_Y, size, center);
}

bool button_Text(const char* string, V2 pos, int w, int h, int string_Size) {
	SDL_Rect button_Area = {};
	int outline_Thickness = 5;

	// Centers the button
	button_Area.x = ((int)pos.x - (w / 2));
	button_Area.y = ((int)pos.y - (h / 2));
	button_Area.w = w;
	button_Area.h = h;

	// Set background as black
	SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &button_Area);

	draw_String(string, (int)pos.x, (int)pos.y, string_Size, true);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

	int x, y;
	Uint32 mouse = SDL_GetMouseState(&x, &y);
	bool button_Pressed = false;

	bool was_Hot = (current_frame_Hot_Name == string);

	// Debugging information
	//printf("Button '%s' position: (%d, %d) size: (%d, %d)\n", string, button_Area.x, button_Area.y, button_Area.w, button_Area.h);
	//printf("Mouse position: (%d, %d) was_Hot: %d\n", x, y, was_Hot);

	if (x >= button_Area.x && x <= (button_Area.x + button_Area.w)
		&& y >= button_Area.y && y <= (button_Area.y + button_Area.h)) {
		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		if (mouse_Down_This_Frame) {
			next_Frame_Hot_Name = string;
		}
		else if (was_Hot && !(mouse & SDL_BUTTON_LMASK)) {
			button_Pressed = true;
		}
		else
		{
			SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
		}
	}
	if (was_Hot && (mouse & SDL_BUTTON_LMASK)) {
		next_Frame_Hot_Name = string;
		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	}

	outline_Rect(&button_Area, outline_Thickness);

	return button_Pressed;
}

// Allocated in global/static memory area. This is different from the heap but it does solve the problem of stack overflow.
Game_Data selected_Game_Data;

// Have this display an image?
void display_Save_Game_Info(Saved_Games save_Game, Cache_Data& cache_Data, V2 pos, int w, int h) {
	std::string save_Game_String = create_Save_Game_File_Name(save_Game);
	selected_Game_Data = cache_Data.cache[save_Game_String];

	SDL_Rect button_Area = {};
	//int outline_Thickness = 5;

	// Centers the button
	button_Area.x = ((int)(pos.x - w) - (w / 2));
	button_Area.y = ((int)pos.y - (h / 2));
	button_Area.w = w;
	button_Area.h = h * 2;

	SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &button_Area);
	
	// Stats
	std::string title_String = "Info";
	const char* title_ptr = title_String.c_str();

	int total_Arrows = (int)selected_Game_Data.player_Castle.arrow_Ammo;
	std::string total_Arrows_String = "Player Ammo: " + std::to_string(total_Arrows);
	const char* total_Arrows_Ptr = total_Arrows_String.c_str();

	//int total_Player_Units = count_Active_Handles(selected_Game_Data.player_Unit_Generations, Globals::MAX_UNITS);
	//std::string total_Player_Units_String = "Player Units: " + std::to_string(total_Player_Units);
	//const char* total_Player_Units_Ptr = total_Player_Units_String.c_str();

	//int total_Enemies = (int)selected_Game_Data.enemy_Units.size();
	//std::string total_Enemies_String = "Enemy Units: " + std::to_string(total_Enemies);
	//const char* total_Enemies_Ptr = total_Enemies_String.c_str();

	int x_Offset = button_Area.x + 5;
	int y_Offset = button_Area.y + 5;
	int string_Size = 2;

	draw_String(title_ptr, x_Offset, y_Offset, string_Size, false);
	y_Offset += font_1.char_Height * string_Size;
	draw_String(total_Arrows_Ptr, x_Offset, y_Offset, string_Size, false);
	y_Offset += font_1.char_Height * string_Size;
	//draw_String(font, total_units_Ptr, x_Offset, y_Offset, string_Size, false);
	//y_Offset += font_1.char_Height * string_Size;
	//draw_String(font, total_Enemies_Ptr, x_Offset, y_Offset, string_Size, false);
	//y_Offset += font_1.char_Height * string_Size;

}

void draw_Timer(Game_Data& game_Data_1, V2 position, int timer_Size, Color_Index color, int outline_Padding) {
	SDL_Rect temp = {};

	std::string str = std::to_string((int)game_Data_1.timer);
	const char* ptr = str.c_str();
	draw_String_With_Background(ptr, (int)(position.x - (font_1.char_Width / 2)), (int)(position.y - (font_1.char_Height / 2)), timer_Size, true, color, outline_Padding);
}

bool button_Text_Load_Game_Info(Saved_Games save_Game, Cache_Data& cache_Data, const char* string, V2 pos, int w, int h, int string_Size) {
	SDL_Rect button_Area = {};
	int outline_Thickness = 5;

	// Centers the button
	button_Area.x = ((int)pos.x - (w / 2));
	button_Area.y = ((int)pos.y - (h / 2));
	button_Area.w = w;
	button_Area.h = h;

	// Set background as black
	SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &button_Area);

	draw_String(string, (int)pos.x, (int)pos.y, string_Size, true);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

	int x, y;
	Uint32 mouse = SDL_GetMouseState(&x, &y);
	bool button_Pressed = false;

	bool was_Hot = (current_frame_Hot_Name == string);

	if (x >= button_Area.x && x <= (button_Area.x + button_Area.w)
		&& y >= button_Area.y && y <= (button_Area.y + button_Area.h)) {
		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		// Call the window that displays the info
		display_Save_Game_Info(save_Game, cache_Data, pos, w, h);
		if (mouse_Down_This_Frame) {
			next_Frame_Hot_Name = string;
		}
		else if (was_Hot && !(mouse & SDL_BUTTON_LMASK)) {
			button_Pressed = true;
		}
		else
		{
			SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
		}
	}
	if (was_Hot && (mouse & SDL_BUTTON_LMASK)) {
		next_Frame_Hot_Name = string;
		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	}

	outline_Rect(&button_Area, outline_Thickness);

	return button_Pressed;
}

bool button_Image(SDL_Texture* texture, const char* string, V2 pos, int h) {
	SDL_Rect button_Area = {};
	int outline_Thickness = 5;

	// Centers the button
	button_Area.x = ((int)pos.x - (h / 2));
	button_Area.y = ((int)pos.y - (h / 2));
	button_Area.w = h;
	button_Area.h = h;

	SDL_Rect image_Area = button_Area;
	int image_Size_Based_On_Outline = 6;
	image_Area.w -= outline_Thickness * image_Size_Based_On_Outline;
	image_Area.h -= outline_Thickness * image_Size_Based_On_Outline;
	image_Area.x += outline_Thickness * (image_Size_Based_On_Outline / 2);
	image_Area.y += outline_Thickness * (image_Size_Based_On_Outline / 2);

	// Set background as black
	SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &button_Area);

	SDL_RenderCopyEx(Globals::renderer, texture, NULL, &image_Area, 0, NULL, SDL_FLIP_NONE);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

	int x, y;
	Uint32 mouse = SDL_GetMouseState(&x, &y);
	bool button_Pressed = false;

	bool was_Hot = (current_frame_Hot_Name == string);

	if (x >= button_Area.x && x <= (button_Area.x + button_Area.w)
		&& y >= button_Area.y && y <= (button_Area.y + button_Area.h)) {
		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		if (mouse_Down_This_Frame) {
			next_Frame_Hot_Name = string;
		}
		else if (was_Hot && !(mouse & SDL_BUTTON_LMASK)) {
			button_Pressed = true;
		}
		else
		{
			SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
		}
	}
	if (was_Hot && (mouse & SDL_BUTTON_LMASK)) {
		next_Frame_Hot_Name = string;
		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	}

	outline_Rect(&button_Area, outline_Thickness);

	return button_Pressed;
}

void draw_Arrow_Ammo_Tracker(int ammo, V2 pos, int size) {
	std::string str_1 = std::to_string(ammo);
	std::string str_2 = "Arrow ammo:" + str_1;
	const char* ptr = str_2.c_str();
	draw_String_With_Background(ptr, (int)pos.x, (int)pos.y, size, true, CI_BLACK, 5);
}

void draw_Time_Scalar(float time_Scalar, int pos_X, int pos_Y, int size) {
	float converted_Time_Scalar;
	if (time_Scalar > 0) {
		converted_Time_Scalar = time_Scalar * 100;
	}
	else {
		converted_Time_Scalar = 0;
	}
	std::string str_1 = "Time Scalar = %";
	std::string str_2 = std::to_string((int)converted_Time_Scalar);
	std::string str_3 = str_1 + str_2;
	const char* ptr = str_3.c_str();
	draw_String_With_Background(ptr, pos_X, pos_Y, size, true, CI_BLACK, 5);
}

void display_Load_Game_Info() {

}

bool load_Game_Button(Saved_Games save_Game, Cache_Data& cache_Data, V2 pos, int w, int h, int size) {
	bool result = false;
	std::string file_Name_String = create_Save_Game_File_Name(save_Game);
	const char* file_Name = file_Name_String.c_str();
	if (check_If_File_Exists(file_Name)) {
		std::string file_String = file_Name;
		std::string file_String_Trimmed;

		if (file_String.length() >= 4 && file_String.substr(file_String.length() - 4) == ".dat") {
			file_String_Trimmed = file_String.substr(0, file_String.length() - 4);
		}
		else {
			file_String_Trimmed = file_String;
			SDL_Log("ERROR: File_Name doesn't end in .dat");
		}
		const char* file_Name_Trimmed = file_String_Trimmed.c_str();

		if (button_Text_Load_Game_Info(save_Game, cache_Data, file_Name_Trimmed, pos, w, h, size)) {
			result = true;
		}

		// When i switch to menu, cache all of these values. Only update them 
		// when something changes. "Update_Cache_Saved_Game_Values"
		// I could have a vector of game datas

		float timer = cache_Data.cache[file_Name].timer;
		// I need to close the file before I use remove or else it won't delete properly

		std::string game_Time = std::to_string((int)timer);
		std::string final_Str = "Game time: " + game_Time;
		const char* str = final_Str.c_str();
		draw_String(str, (int)pos.x, (int)pos.y + 27, 2, true);

		V2 delete_Button_Pos = pos;
		delete_Button_Pos.x += (w / 2) + (h / 2);
		if (button_Text("X", delete_Button_Pos, h, h, size + 2)) {
			cache_Data.loaded = false;
			cache_Data.cache.erase(file_Name);
			remove(file_Name);
		}

	}

	return result;
}

bool save_Game_Button(Saved_Games save_Game, Cache_Data cache_Data, V2 pos, int w, int h, int size) {
	bool result = false;
	std::string file_String = create_Save_Game_File_Name(save_Game);
	std::string file_String_Trimmed;

	if (file_String.length() >= 4 && file_String.substr(file_String.length() - 4) == ".dat") {
		file_String_Trimmed = file_String.substr(0, file_String.length() - 4);
	}
	else {
		file_String_Trimmed = file_String;
		SDL_Log("ERROR: File_Name doesn't end in .dat");
	}
	const char* file_Name_Trimmed = file_String_Trimmed.c_str();

	if (button_Text(file_Name_Trimmed, pos, w, h, size)) {
		result = true;
	}

	float timer = -1;
	// bool valid = false;
	if (cache_Data.cache.find(file_String) != cache_Data.cache.end()) {
		timer = cache_Data.cache[file_String].timer;
	}

	if (timer < 0) {
		std::string final_Str = "Empty";
		const char* str = final_Str.c_str();
		draw_String(str, (int)pos.x, (int)pos.y + 27, 2, true);
	}
	else {
		std::string game_Time = std::to_string((int)timer);
		std::string final_Str = "Game time: " + game_Time;
		const char* str = final_Str.c_str();
		draw_String(str, (int)pos.x, (int)pos.y + 27, 2, true);
	}

	return result;
}


void outline_Rect(SDL_Rect* rect, int outline_Thickness) {
	SDL_Rect top_Rect = {};
	top_Rect.x = rect->x;
	top_Rect.y = rect->y;
	top_Rect.w = rect->w;
	top_Rect.h = outline_Thickness;
	SDL_RenderFillRect(Globals::renderer, &top_Rect);

	SDL_Rect bottom_Rect = {};
	bottom_Rect.x = rect->x;
	bottom_Rect.y = ((rect->y + rect->h) - outline_Thickness);
	bottom_Rect.w = rect->w;
	bottom_Rect.h = outline_Thickness;
	SDL_RenderFillRect(Globals::renderer, &bottom_Rect);

	SDL_Rect left_Rect = {};
	left_Rect.x = rect->x;
	left_Rect.y = rect->y;
	left_Rect.w = outline_Thickness;
	left_Rect.h = rect->h;
	SDL_RenderFillRect(Globals::renderer, &left_Rect);

	SDL_Rect right_Rect = {};
	right_Rect.x = ((rect->x + rect->w) - outline_Thickness);
	right_Rect.y = rect->y;
	right_Rect.w = outline_Thickness;
	right_Rect.h = rect->h;
	SDL_RenderFillRect(Globals::renderer, &right_Rect);

	SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(Globals::renderer, rect);
}

void draw_HP_Bar(Health_Bar& health_Bar, V2 pos) {
	float remaining_HP_Percent = (health_Bar.current_HP / health_Bar.max_HP);
	if (remaining_HP_Percent < 0) {
		remaining_HP_Percent = 0;
	}

	// Lerp of T = A * (1 - T) + B * T
	// A is the left side, B is the right side, T is the health %
	float lerp = linear_Interpolation(0, (float)health_Bar.width, remaining_HP_Percent);

	SDL_Rect rect_Green = {};
	rect_Green.w = (int)lerp;
	rect_Green.h = (int)health_Bar.height;
	rect_Green.x = (int)((pos.x) - health_Bar.width / 2);
	rect_Green.y = (int)((pos.y) - health_Bar.y_Offset);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &rect_Green);

	SDL_Rect rect_Red = rect_Green;
	rect_Red.w = health_Bar.width - rect_Green.w;
	rect_Red.x = (int)(rect_Green.x + rect_Green.w);
	SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &rect_Red);

	// Outline HP bars
	SDL_Rect outline = {};
	outline.w = (int)health_Bar.width;
	outline.h = (int)health_Bar.height;
	outline.x = (int)((pos.x) - health_Bar.width / 2);
	outline.y = (int)((pos.y) - health_Bar.y_Offset);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	outline_Rect(&outline, health_Bar.thickness);
}

void draw_Unit_Data(Unit& unit, V2 pos) {
	// HP Bar
	draw_HP_Bar(unit.health_Bar, pos);

	// HP Text
	// Set the color mod for the font
	SDL_SetTextureColorMod(font_1.texture, 0, 0, 0);
	std::string hp_String = std::to_string((int)unit.health_Bar.current_HP);
	draw_String(hp_String.c_str(), (int)pos.x, ((int)pos.y - unit.health_Bar.y_Offset) + (unit.health_Bar.height / 2), 1, true);
	// Reset the color mod for the font
	SDL_SetTextureColorMod(font_1.texture, 255, 255, 255);
	
	// Damage text
	std::string damage_String = std::to_string((int)unit.damage);
	draw_String_With_Background(
		damage_String.c_str(), 
		(int)pos.x, 
		(((int)pos.y - unit.health_Bar.y_Offset) - (unit.health_Bar.height / 2)), 
		1, 
		true,
		CI_BLACK,
		2
	);

}

void draw_Mana_Bar() {
	float remaining_HP_Percent = (health_Bar.current_HP / health_Bar.max_HP);
	if (remaining_HP_Percent < 0) {
		remaining_HP_Percent = 0;
	}

	// Lerp of T = A * (1 - T) + B * T
	// A is the left side, B is the right side, T is the health %
	float lerp = linear_Interpolation(0, (float)health_Bar.width, remaining_HP_Percent);

	SDL_Rect rect_Green = {};
	rect_Green.w = (int)lerp;
	rect_Green.h = (int)health_Bar.height;
	rect_Green.x = (int)((pos.x) - health_Bar.width / 2);
	rect_Green.y = (int)((pos.y) - health_Bar.y_Offset);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &rect_Green);

	SDL_Rect rect_Red = rect_Green;
	rect_Red.w = health_Bar.width - rect_Green.w;
	rect_Red.x = (int)(rect_Green.x + rect_Green.w);
	SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &rect_Red);

	// Outline HP bars
	SDL_Rect outline = {};
	outline.w = (int)health_Bar.width;
	outline.h = (int)health_Bar.height;
	outline.x = (int)((pos.x) - health_Bar.width / 2);
	outline.y = (int)((pos.y) - health_Bar.y_Offset);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	outline_Rect(&outline, health_Bar.thickness);
}

void draw_Summonable_Player_Units_Buttons() {
	Castle* player_Castle = &game_Data.player_Castle;

	V2 button_Pos = { (RESOLUTION_WIDTH / 16), ((RESOLUTION_HEIGHT / 9) * 8) };
	int spawn_Unit_Button_W = 150;

	int level_Up_Button_H = 30;
	V2 level_Up_Button_Pos = { button_Pos.x, (button_Pos.y - (spawn_Unit_Button_W / 2)) - level_Up_Button_H / 2 };

	V2 level_Text_Pos = { level_Up_Button_Pos.x , level_Up_Button_Pos.y - level_Up_Button_H };

	int unit_Counter = 1;
	for (Summonable_Unit& summonable_Unit : player_Castle->summonable_Units) {
		std::string button_Name = "Spawn " + summonable_Unit.name;
		std::string displayed_Image = summonable_Unit.name + "_Stop";
		if (button_Image(get_Sprite_Sheet_Texture(displayed_Image), button_Name.c_str(), button_Pos, spawn_Unit_Button_W)) {
			summonable_Unit.is_Pressed = true;
		}
		button_Pos.x += spawn_Unit_Button_W;
		
		// *** Debugging purposes ***
		std::string level_Up_String = std::to_string(unit_Counter) + ": Level Up+";
		unit_Counter++;
		if (button_Text(level_Up_String.c_str(), level_Up_Button_Pos, spawn_Unit_Button_W, level_Up_Button_H, 1)) {
			summonable_Unit.level++;
		}
		level_Up_Button_Pos.x += spawn_Unit_Button_W;

		std::string debug_String = std::to_string(summonable_Unit.level);
		draw_String(debug_String.c_str(), (int)level_Text_Pos.x, (int)level_Text_Pos.y, 2, true);
		level_Text_Pos.x += spawn_Unit_Button_W;
		// *************************
	}
}

void draw_Player_Hud() {
	draw_HP_Bar(game_Data.player_Castle.health_Bar, game_Data.player_Castle.rigid_Body.position_WS);
	draw_HP_Bar(game_Data.enemy_Castle.health_Bar, game_Data.enemy_Castle.rigid_Body.position_WS);
	draw_Timer(
		game_Data,
		{ RESOLUTION_WIDTH / 2, (RESOLUTION_HEIGHT / 9) * 0.5 },
		6,
		CI_BLACK,
		3
	);
	draw_Arrow_Ammo_Tracker(
		game_Data.player_Castle.arrow_Ammo,
		{ ((RESOLUTION_WIDTH / 16) * 2), ((RESOLUTION_HEIGHT / 9) * 0.5) },
		3
	);
	draw_Summonable_Player_Units_Buttons();
}

static std::stack<Menu_Mode> menu_Stack;

size_t get_Menu_Stack_Size() {
	return menu_Stack.size();
}

void empty_Menu_Stack() {
	while (menu_Stack.size() > 0) {
		menu_Stack.pop();
	}
}

void push_To_Menu_Stack(Menu_Mode menu_Mode) {
	if (menu_Mode == MM_Main_Menu) {
		empty_Menu_Stack();
	}
	menu_Stack.push(menu_Mode);
}

void pop_Menu_From_Stack() {
	if (menu_Stack.size() > 0) {
		menu_Stack.pop();
	}
}

// So we don't pop past the base menu state (If applicable)
void pop_Menu_From_Stack_Keep_First() {
	if (menu_Stack.size() > 1) {
		menu_Stack.pop();
	}
}

bool running = true;

void draw_Main_Menu() {
	// No game logic
	SDL_RenderCopy(Globals::renderer, get_Sprite_Sheet_Texture("bkg_Menu"), NULL, NULL);

	draw_String_With_Background(
		"Castle Defense",
		RESOLUTION_WIDTH / 2,
		RESOLUTION_HEIGHT / 4,
		8,
		true,
		CI_BLACK,
		20
	);

	V2 button_Pos = { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 };
	int button_Width = 300;
	int button_Height = 100;
	int string_Size = 4;

	if (button_Text("Play", button_Pos, button_Width, button_Height, string_Size)) {
		current_Game_State = GS_GAMELOOP;
		game_Data = game_Data_New_Game;
		empty_Menu_Stack();
		start_Game(game_Data);
	}
	button_Pos.y += 100;
	if (button_Text("Load Game", button_Pos, button_Width, button_Height, string_Size)) {
		push_To_Menu_Stack(MM_Sub_Menu_Load_Game);
	}
	button_Pos.y += 100;
	if (button_Text("Options", button_Pos, button_Width, button_Height, string_Size)) {

	}
	button_Pos.y += 100;
	if (button_Text("Quit", button_Pos, button_Width, button_Height, string_Size)) {
		running = false;
	}
	button_Pos.y += 100;
}

void draw_Sub_Menu_Paused() {
	int button_Width_Paused = 325;
	int button_Height_Paused = 90;
	int string_Size_Paused = 3;
	V2 button_Pos_Paused = { RESOLUTION_WIDTH / 2 , RESOLUTION_HEIGHT / 2 };
	draw_String_With_Background(
		"Game Paused",
		RESOLUTION_WIDTH / 2,
		RESOLUTION_HEIGHT / 2,
		5,
		true,
		CI_BLACK,
		5
	);
	button_Pos_Paused.y += button_Height_Paused;
	if (button_Text("Return to Menu", button_Pos_Paused, button_Width_Paused, button_Height_Paused, string_Size_Paused)) {
		push_To_Menu_Stack(MM_Main_Menu);
		current_Game_State = GS_MAIN_MENU;
	}
	button_Pos_Paused.y += button_Height_Paused;
	if (button_Text("Save Game", button_Pos_Paused, button_Width_Paused, button_Height_Paused, string_Size_Paused)) {
		push_To_Menu_Stack(MM_Sub_Menu_Save_Game);
	}
	button_Pos_Paused.y += button_Height_Paused;
}

void draw_Sub_Menu_Save_Game() {
	if (key_States[SDLK_ESCAPE].pressed_This_Frame) {
		current_Game_State = GS_GAMELOOP;
	}
	int button_Width_Saved = 325;
	int button_Height_Saved = 90;
	int offset = button_Height_Saved;
	V2 button_Pos_Saved = { RESOLUTION_WIDTH / 2 , RESOLUTION_HEIGHT / 10 * 3 };
	int size = 3;

	draw_String_With_Background("Saved Games", (int)button_Pos_Saved.x, (int)button_Pos_Saved.y, size, true, CI_BLACK, 3);

	// This is a loop
	if (save_Game_Button(SG_SAVE_GAME_1, save_Game_Cache_Data, button_Pos_Saved, button_Width_Saved, button_Height_Saved, size)) {
		// Put this in the save_Game_Button
		save_Game_To_Cache(SG_SAVE_GAME_1, game_Data, save_Game_Cache_Data);
	}
	button_Pos_Saved.y += offset;
	if (save_Game_Button(SG_SAVE_GAME_2, save_Game_Cache_Data, button_Pos_Saved, button_Width_Saved, button_Height_Saved, size)) {
		save_Game_To_Cache(SG_SAVE_GAME_2, game_Data, save_Game_Cache_Data);
	}
	button_Pos_Saved.y += offset;
	if (save_Game_Button(SG_SAVE_GAME_3, save_Game_Cache_Data, button_Pos_Saved, button_Width_Saved, button_Height_Saved, size)) {
		save_Game_To_Cache(SG_SAVE_GAME_3, game_Data, save_Game_Cache_Data);
	}
	button_Pos_Saved.y += offset;
	/*
	if (button_Text(&font_1, "Return to Menu", button_Pos_Saved, button_Width_Saved, button_Height_Saved, size)) {
		current_Game_State = GS_MAIN_MENU;
	}
	*/
}

void draw_Sub_Menu_Load_Game() {
	SDL_RenderCopy(Globals::renderer, get_Sprite_Sheet_Texture("bkg_Menu"), NULL, NULL);

	int button_Width = 325;
	int button_Height = 90;
	int offset = button_Height;
	V2 button_Pos = { RESOLUTION_WIDTH / 2 , RESOLUTION_HEIGHT / 10 * 3 };
	int size = 3;

	draw_String_With_Background("Saved Games", (int)button_Pos.x, (int)button_Pos.y, size, true, CI_BLACK, 3);
	button_Pos.y += offset;

	if (load_Game_Button(SG_SAVE_GAME_1, save_Game_Cache_Data, button_Pos, button_Width, button_Height, size)) {
		load_Game(game_Data, SG_SAVE_GAME_1);
		current_Game_State = GS_GAMELOOP;
	}
	button_Pos.y += offset;
	if (load_Game_Button(SG_SAVE_GAME_2, save_Game_Cache_Data, button_Pos, button_Width, button_Height, size)) {
		load_Game(game_Data, SG_SAVE_GAME_2);
		current_Game_State = GS_GAMELOOP;
	}
	button_Pos.y += offset;
	if (load_Game_Button(SG_SAVE_GAME_3, save_Game_Cache_Data, button_Pos, button_Width, button_Height, size)) {
		load_Game(game_Data, SG_SAVE_GAME_3);
		current_Game_State = GS_GAMELOOP;
	}
	button_Pos.y += offset;
	if (key_States[SDLK_ESCAPE].pressed_This_Frame) {
		current_Game_State = GS_MAIN_MENU;
	}
}

void draw_Menu_Victory_Screen() {
	SDL_RenderCopy(Globals::renderer, get_Sprite_Sheet_Texture("bkg_Gameloop"), NULL, NULL);
	if (current_Game_State == GS_VICTORY) {
		draw_String_With_Background(
			"Victory!!!",
			RESOLUTION_WIDTH / 2,
			RESOLUTION_HEIGHT / 2,
			4,
			true,
			CI_BLACK,
			3
		);
	}
	if (button_Text("Return to Menu", { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 90 }, 325, 90, 3)) {
		push_To_Menu_Stack(MM_Main_Menu);
		current_Game_State = GS_MAIN_MENU;
	}
}
void draw_Menu_Game_Over_Screen() {
	SDL_RenderCopy(Globals::renderer, get_Sprite_Sheet_Texture("bkg_Gameloop"), NULL, NULL);
	if (current_Game_State == GS_GAMEOVER) {
		draw_String_With_Background(
			"Game Over",
			RESOLUTION_WIDTH / 2,
			RESOLUTION_HEIGHT / 2,
			4,
			true,
			CI_BLACK,
			3
		);
	}
	if (button_Text("Return to Menu", { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 90 }, 325, 90, 3)) {
		push_To_Menu_Stack(MM_Main_Menu);
		current_Game_State = GS_MAIN_MENU;
	}
}

// Drawing the hud should be separate from drawing the menu
// I could have a stack of menus and then I have a hierarchy of menus
// When I press escape, I pop the top of the stack
void draw_Menu() {
	if (menu_Stack.empty())
	{
		return;
	};

	switch (menu_Stack.top()) 
	{
	case MM_Main_Menu:
	{
		draw_Main_Menu();
		break;
	}
	case MM_Sub_Menu_Paused:
	{
		draw_Sub_Menu_Paused();
		break;
	}
	case MM_Sub_Menu_Save_Game:
	{
		draw_Sub_Menu_Save_Game();
		break;
	}
	case MM_Sub_Menu_Load_Game:
	{
		draw_Sub_Menu_Load_Game();
		break;
	}
	case MM_Menu_Victory_Screen:
	{
		draw_Menu_Victory_Screen();
		break;
	}
	case MM_Menu_Game_Over_Screen:
	{
		draw_Menu_Game_Over_Screen();
		break;
	}
	}
}
