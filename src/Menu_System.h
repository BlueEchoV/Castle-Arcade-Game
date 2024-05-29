#pragma once
#include <string>
#include "Game.h"

struct Font {
	int width;
	int height;
	SDL_Texture* texture;
	int char_Height;
	int char_Width;
};

extern bool running;
extern Font font_1;
extern Game_State current_Game_State;
extern Game_Data game_Data_New_Game;
extern Game_Data game_Data;

extern bool mouse_Down_This_Frame;
extern std::string current_frame_Hot_Name;
extern std::string next_Frame_Hot_Name;
static bool draw_Rect = false;

enum Button_State {
	BS_NORMAL,
	BS_HOVERED,
	BS_IS_HOT,
	BS_PRESSED
};

enum Menu_Mode {
	MM_Main_Menu,
	MM_Game_Loop_UI,
	MM_Sub_Menu_Paused,
	MM_Sub_Menu_Save_Game
};

struct Key_State {
	bool pressed_This_Frame;
	bool held_Down;
};

extern std::unordered_map<SDL_Keycode, Key_State> key_States;

struct Button {
	SDL_Rect rect;
	Font* font;
	Button_State state;
	const char* string;
};

void reset_Pressed_This_Frame();

Font load_Font_Bitmap(const char* font_File_Path);

void draw_Character(char character, int position_X, int position_Y, int size);
void draw_String(const char* string, int position_X, int position_Y, int size, bool center);
void draw_String_With_Background(const char* string, int position_X, int position_Y, int size, bool center, Color_Index color, int outline_Padding);
void draw_Arrow_Ammo_Tracker(int ammo, V2 pos, int size);
void draw_Time_Scalar(float time_Scalar, int pos_X, int pos_Y, int size);
void draw_Timer(Game_Data& game_Data, V2 position, int timer_Size, Color_Index color, int outline_Padding);

bool button_Text(const char* string, V2 pos, int w, int h, int string_Size);
bool button_Text_Load_Game_Info(Saved_Games save_Game, Cache_Data& cache_Data, const char* string, V2 pos, int w, int h, int string_Size);
bool button_Image(SDL_Texture* texture, const char* string, V2 pos, int h);
 
void display_Save_Game_Info(Saved_Games save_Game, Cache_Data& cache_Data, V2 pos, int w, int h);
void display_Load_Game_Info();

bool load_Game_Button(Saved_Games save_Game, Cache_Data& cache_Data, V2 pos, int w, int h, int size);
bool save_Game_Button(Saved_Games save_Game, Cache_Data cache_Data, V2 pos, int w, int h, int size);

void outline_Rect(SDL_Rect* rect, int outline_Thickness);
void draw_HP_Bar(V2* position, Health_Bar* health_Bar);
void draw_HP_Bar_With_String(V2* position, Health_Bar* health_Bar);

void draw_Summonable_Player_Units_Buttons(Game_Data& game_Data);

void empty_Stack();
void push_To_Menu_Stack(Menu_Mode menu_Mode);
void pop_Menu_From_Stack();

void draw_Menu();
