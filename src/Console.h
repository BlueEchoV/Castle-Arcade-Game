#pragma once
#include "Menu_System.h"

enum Console_State { 
	CS_Closed, 
	CS_Open_Small,
	CS_Open_Big
};

const int max_Console_Array_Size = 50;
struct Command {
	char command[max_Console_Array_Size];
	std::string error_Message;
	bool is_Valid;
};

struct Console {
	Console_State state;
	int text_Size_Multiplier;
	int text_Height;
	Font* font;
	int text_Padding;
	Cooldown cursor_Indicator;
	int cursor_Offset;
	
	float max_Openness;
	// How open is the console right now
	float current_Openness;
	// How open does it want to be (Because it's updating slowing frame by frame)
	float target_Openness;
	// The rate at which it's opening
	float rate_Of_Openness_DT;

	int current_History_Size;
	Command history[max_Console_Array_Size] = {};
	char user_Input[max_Console_Array_Size] = {};
	bool is_Valid_Input;
	std::string error_Output;
	int history_Selector_Index;
	
	// The console 
	SDL_Rect bkg_Rect;
	Color report_Background_Color;

	SDL_Rect ipt_Rect;
	Color input_Background_Color;
};

Console init_Console(Font* font, int text_Size, float max_Openness, float rate_Of_Openness_DT);
void add_Input_To_History(Console& console);
void process_Console_Input(Console& console);
void draw_Console(Console& console, float delta_Time);
void update_Openness(Console& console, float delta_Time);
// void get_Console_Bottom();
void open_Close_Console(Console& console);
bool is_Console_Open(Console& console);
void get_Console_Input(Console& console);
