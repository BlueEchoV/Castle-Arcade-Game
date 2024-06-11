#pragma once
#include "Menu_System.h"

enum Console_State {
CS_Closed,
CS_Open_Small,
CS_Open_Big
};

struct Console {
	// The console area
	SDL_Rect bkg_Rect;
	SDL_Rect ipt_Rect;
	float ipt_Max_Height;
	Console_State state;
	float max_Openness;
	// How open is the console right now
	float current_Openness;
	// How open does it want to be (Because it's updating slowing frame by frame)
	float target_Openness;
	// The rate at which it's opening
	float rate_Of_Openness_DT;
	Color input_Background_Color;
	Color report_Background_Color;
};

Console create_Console(float max_Openness, float rate_Of_Openness_DT);
void draw_Console(Console& console, float delta_Time);
void update_Openness(Console& console, float delta_Time);
// void get_Console_Bottom();
void open_Close_Console(Console& console);
