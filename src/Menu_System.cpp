#include "Menu_System.h"

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


void draw_Character(Font* font, char character, int position_X, int position_Y, int size) {
    int ascii_Dec = (int)character - (int)' ';
    int chars_Per_Row = (font->width / font->char_Width);

    SDL_Rect src_Rect = {};
    // Position in the row
    src_Rect.x = (ascii_Dec % chars_Per_Row) * font->char_Width;
    // Which row the char is in
    src_Rect.y = (ascii_Dec / chars_Per_Row) * font->char_Height;
    src_Rect.w = font->char_Width;
    src_Rect.h = font->char_Height;

    SDL_Rect dest_Rect = {};
    // Position of the character on the screen
    dest_Rect.x = position_X;
    dest_Rect.y = position_Y;
    dest_Rect.w = (int)(font->char_Width * size);
    dest_Rect.h = (int)(font->char_Height * size);

    SDL_RenderCopyEx(Globals::renderer, font->texture, &src_Rect, &dest_Rect, 0, NULL, SDL_FLIP_NONE);
}

void draw_String(Font* font, const char* string, int position_X, int position_Y, int size, bool center) {
    int offset_X = 0;
    int index = 0;
    char iterator = string[index];
    size_t length_Pixels = strlen(string);
    length_Pixels *= font->char_Width * size;

    int char_Position_X = 0;
    int char_Position_Y = 0;
    if (center) {
        char_Position_X = (position_X - (int)(length_Pixels / 2));
        char_Position_Y = (position_Y - ((font->char_Height * size) / 2));
    }
    else {
        char_Position_X = position_X;
        char_Position_Y = position_Y;
    }

    while (iterator != '\0') {
        char_Position_X += offset_X;
        draw_Character(font, iterator, char_Position_X, char_Position_Y, size);
        offset_X = font->char_Width * size;
        index++;
        iterator = string[index];
    }
}

void draw_String_With_Background(Font* font, const char* string, int position_X, int position_Y, int size, bool center, Color_Index color, int outline_Padding) {
    size_t length_Pixels = strlen(string);
    length_Pixels *= font->char_Width;
    length_Pixels *= size;
    size_t height_Pixels = font->char_Height;
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

    draw_String(font, string, position_X, position_Y, size, center);
}

bool button_Text(Font* font, const char* string, V2 pos, int w, int h, int string_Size) {
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

	draw_String(font, string, (int)pos.x, (int)pos.y, string_Size, true);
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

// Have this display an image?
void display_Save_Game_Info(Saved_Games save_Game, Cache_Data& cache_Data, Font* font, V2 pos, int w, int h) {
	std::string save_Game_String = create_Save_Game_File_Name(save_Game);
	Game_Data selected_Game_Data = cache_Data.cache[save_Game_String];

	SDL_Rect button_Area = {};
	int outline_Thickness = 5;
	REF(outline_Thickness);
	REF(cache_Data);

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

	int total_Player_Units = (int)selected_Game_Data.player_Units.size();
	std::string total_Player_Units_String = "Player Units: " + std::to_string(total_Player_Units);
	const char* total_Player_Units_Ptr = total_Player_Units_String.c_str();

	int total_Enemies = (int)selected_Game_Data.enemy_Units.size();
	std::string total_Enemies_String = "Enemy Units: " + std::to_string(total_Enemies);
	const char* total_Enemies_Ptr = total_Enemies_String.c_str();

	int x_Offset = button_Area.x + 5;
	int y_Offset = button_Area.y + 5;
	int string_Size = 2;

	draw_String(font, title_ptr, x_Offset, y_Offset, string_Size, false);
	y_Offset += font->char_Height * string_Size;
	draw_String(font, total_Arrows_Ptr, x_Offset, y_Offset, string_Size, false);
	y_Offset += font->char_Height * string_Size;
	draw_String(font, total_Player_Units_Ptr, x_Offset, y_Offset, string_Size, false);
	y_Offset += font->char_Height * string_Size;
	draw_String(font, total_Enemies_Ptr, x_Offset, y_Offset, string_Size, false);
	y_Offset += font->char_Height * string_Size;

}

void draw_Timer(Game_Data* game_Data, Font* font, V2 position, int timer_Size, Color_Index color, int outline_Padding) {
	SDL_Rect temp = {};

	std::string str = std::to_string((int)game_Data->timer);
	const char* ptr = str.c_str();
	draw_String_With_Background(font, ptr, (int)(position.x - (font->char_Width / 2)), (int)(position.y - (font->char_Height / 2)), timer_Size, true, color, outline_Padding);
}

bool button_Text_Load_Game_Info(Saved_Games save_Game, Cache_Data& cache_Data, Font* font, const char* string, V2 pos, int w, int h, int string_Size) {
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

	draw_String(font, string, (int)pos.x, (int)pos.y, string_Size, true);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

	int x, y;
	Uint32 mouse = SDL_GetMouseState(&x, &y);
	bool button_Pressed = false;

	bool was_Hot = (current_frame_Hot_Name == string);

	if (x >= button_Area.x && x <= (button_Area.x + button_Area.w)
		&& y >= button_Area.y && y <= (button_Area.y + button_Area.h)) {
		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		// Call the window that displays the info
		display_Save_Game_Info(save_Game, cache_Data, font, pos, w, h);
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

void draw_Arrow_Ammo_Tracker(Font* font, int ammo, V2 pos, int size) {
	std::string str_1 = std::to_string(ammo);
	std::string str_2 = "Arrow ammo:" + str_1;
	const char* ptr = str_2.c_str();
	draw_String_With_Background(font, ptr, (int)pos.x, (int)pos.y, size, true, CI_BLACK, 5);
}

void draw_Time_Scalar(Font* font, float time_Scalar, int pos_X, int pos_Y, int size) {
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
	draw_String_With_Background(font, ptr, pos_X, pos_Y, size, true, CI_BLACK, 5);
}

void display_Load_Game_Info() {

}


bool load_Game_Button(Saved_Games save_Game, Cache_Data& cache_Data, Font* font, V2 pos, int w, int h, int size) {
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

		if (button_Text_Load_Game_Info(save_Game, cache_Data, font, file_Name_Trimmed, pos, w, h, size)) {
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
		draw_String(font, str, (int)pos.x, (int)pos.y + 27, 2, true);

		V2 delete_Button_Pos = pos;
		delete_Button_Pos.x += (w / 2) + (h / 2);
		if (button_Text(font, "X", delete_Button_Pos, h, h, size + 2)) {
			cache_Data.loaded = false;
			cache_Data.cache.erase(file_Name);
			remove(file_Name);
		}

	}

	return result;
}

bool save_Game_Button(Saved_Games save_Game, Cache_Data cache_Data, Font* font, V2 pos, int w, int h, int size) {
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

	if (button_Text(font, file_Name_Trimmed, pos, w, h, size)) {
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
		draw_String(font, str, (int)pos.x, (int)pos.y + 27, 2, true);
	}
	else {
		std::string game_Time = std::to_string((int)timer);
		std::string final_Str = "Game time: " + game_Time;
		const char* str = final_Str.c_str();
		draw_String(font, str, (int)pos.x, (int)pos.y + 27, 2, true);
	}

	return result;
}


void draw_Game_Loop_UI() {
	// Draw game loop UI here
	// Check the current units the player has selected and/or equipped
	// and draw the associated buttons
}

void draw_Main_Menu() {
	// Play, Load, Quit buttons
}

void draw_Sub_Menu_Paused() {
	// Save, quit, return to main menu buttons
}

// Drawing the hud should be separate from drawing the menu

// I could have a stack of menus and then I have a hierarchy of menus
// When I press escape, I pop the top of the stack
void draw_Menu(/*Menu mode enum*/) {
	// ****************************
	/*
	if (mm == Menu) {
		draw_Main_Menu();
	} else if (mm == Gameloop) {
		if (gs == paused) {
			draw_Sub_Menu_Paused();
		} else if (gs == Victory) {
			draw_Victory_Screen();
		} else if (gs == Game Over) {
			draw_Game_Over_Screen();
		}
	}
	*/
}
