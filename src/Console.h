#pragma once
#include "Menu_System.h"
extern std::unordered_map<SDL_Keycode, Key_State> console_Key_States;
enum Console_State { 
	CS_Closed, 
	CS_Open_Small,
	CS_Open_Big
};

const int max_History_String = 100;
struct Console {
	Console_State state;
	int text_Size_Multiplier;
	int text_Height;
	Font* font;
	Cooldown cursor_Indicator;
	int cursor_Offset;
	
	float max_Openness;
	// How open is the console right now
	float current_Openness;
	// How open does it want to be (Because it's updating slowing frame by frame)
	float target_Openness;
	// The rate at which it's opening
	float rate_Of_Openness_DT;

	int history_Size;
	std::string history[max_History_String];
	char user_Input[100] = {};
	
	// The console 
	SDL_Rect bkg_Rect;
	Color report_Background_Color;

	SDL_Rect ipt_Rect;
	Color input_Background_Color;
};

Console create_Console(Font* font, int text_Size, float max_Openness, float rate_Of_Openness_DT);
void add_Input_To_History(Console& console);
void process_Console_Input(Console& console);
void draw_Console(Console& console, float delta_Time);
void update_Openness(Console& console, float delta_Time);
// void get_Console_Bottom();
void open_Close_Console(Console& console);
bool is_Console_Open(Console& console);
