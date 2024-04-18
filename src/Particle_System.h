#pragma once
#include "Image.h"
#include <vector>

struct Game_Data;

enum Particle_Type {
	PT_BLOOD,
	PT_TOTAL
};

struct F_Color {
	float r;
	float g;
	float b;
	float a;
};

struct Particle_Data {
	int size;
	float time_Between_Spawns;
	float lifetime_Min;
	float lifetime_Max;
	V2 velocity_Min;
	V2 velocity_Max;
};

const Particle_Data particle_Data_Array[PT_TOTAL] = {
	//  size  |  time_Between_Spawns  |  lifetime_Min  |  lifetime_Max  |  velocity_Min  |  velocity_Max
	{	100,	 0.01f,					 20.0f,            30.0f,		   {0.0f, 0.0f},  {0.0f, 0.0f}}
};

struct Particle {
	int size;
	float life_Time;
	V2 velocity;
	V2 position;
};

struct Particle_System {
	Image* image;
	SDL_Rect rect;
	Particle_Type type;
	float time_Between_Spawns;
	std::vector<Particle> particles;
};

void swap_Floats(float& a, float& b);

float random_Float_In_Range(float min, float max);

V2 random_Vector_In_Range(V2 min, V2 max);

void spawn_Particle_Systems(Game_Data& game_Data, Particle_Type type, V2 pos, int w, int h, Image* image);

void update_Particle_System(Particle_System& particle_System, float delta_Time);

void draw_Particle_Systems(Game_Data& game_Data);