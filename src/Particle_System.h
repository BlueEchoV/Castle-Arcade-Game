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
	float max_Fade_In;
	float time_Between_Spawns;
	float lifetime_Min;
	float lifetime_Max;
	V2 velocity_Min;
	V2 velocity_Max;
};

const Particle_Data particle_Data_Array[PT_TOTAL] = {
	//  size  |  max_Fade_In  |  time_Between_Spawns  |  lifetime_Min  |  lifetime_Max  |  velocity_Min  |  velocity_Max
	{	50,		 0.5f,		 0.001f,				 5.0f,            5.0f,		       {50.0f, 50.0f},  {100.0f, 100.0f}}
};

struct Particle {
	int size;
	float life_Time;
	float fade_In;
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

void spawn_Particle_Systems(Game_Data& game_Data, Particle_Type type, V2 pos, int w, int h, Image* image);

void update_Particle_System(Particle_System& particle_System, float delta_Time);

void draw_Particle_Systems(Game_Data& game_Data);