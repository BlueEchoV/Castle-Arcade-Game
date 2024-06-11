#pragma once
#include "Menu_System.h"

struct Console {
	SDL_Rect dimension;
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

enum Console_State {
	CS_Closed,
	CS_Open_Small,
	CS_Open_Big
};

Console create_Console();
void draw_Console();
void update_Openness();
void get_Console_Bottom();