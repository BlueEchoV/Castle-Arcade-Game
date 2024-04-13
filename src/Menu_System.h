#pragma once
#include <string>
#include "Game.h"

static bool mouse_Down_This_Frame = false;
static std::string current_frame_Hot_Name;
static std::string next_Frame_Hot_Name;
static bool draw_Rect = false;

enum Button_State {
	BS_NORMAL,
	BS_HOVERED,
	BS_IS_HOT,
	BS_PRESSED
};

struct Key_State {
	bool pressed_This_Frame;
	bool held_Down;
};

extern std::unordered_map<SDL_Keycode, Key_State> key_States;

struct Font {
	int width;
	int height;
	SDL_Texture* texture;
	int char_Height;
	int char_Width;
};

struct Button {
	SDL_Rect rect;
	Font* font;
	Button_State state;
	const char* string;
};

void reset_Pressed_This_Frame();

Font load_Font_Bitmap(const char* font_File_Path);

void draw_Character(Font* font, char character, int position_X, int position_Y, int size);

void draw_String(Font* font, const char* string, int position_X, int position_Y, int size, bool center);

void draw_String_With_Background(Font* font, const char* string, int position_X, int position_Y, int size, bool center, Color_Index color, int outline_Padding);

bool button_Text(Font* font, const char* string, V2 pos, int w, int h, int string_Size);

void display_Save_Game_Info(Saved_Games save_Game, Cache_Data& cache_Data, V2 pos, int w, int h);

void draw_Timer(Game_Data* game_Data, Font* font, V2 position, int timer_Size, Color_Index color, int outline_Padding);

bool button_Text_Load_Game_Info(Saved_Games save_Game, Cache_Data& cache_Data, Font* font, const char* string, V2 pos, int w, int h, int string_Size);

bool button_Image(SDL_Texture* texture, const char* string, V2 pos, int h);

void draw_Arrow_Ammo_Tracker(Font* font, int ammo, V2 pos, int size);

void draw_Time_Scalar(Font* font, float time_Scalar, int pos_X, int pos_Y, int size);

void display_Load_Game_Info();

bool load_Game_Button(Saved_Games save_Game, Cache_Data& cache_Data, Font* font, V2 pos, int w, int h, int size);

bool save_Game_Button(Saved_Games save_Game, Cache_Data cache_Data, Font* font, V2 pos, int w, int h, int size);
