#pragma once
#include "Entity.h"

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
	int spawn_Size;
	float time_Between_Spawns;
	float lifetime_Min;
	float lifetime_Max;
	V2 velocity_Min;
	V2 velocity_Max;
};

const Particle_Data particle_Data_Array[PT_TOTAL] = {
	//  spawn_Size  |  time_Between_Spawns  |  lifetime_Min  |  lifetime_Max  |  velocity_Min  |  velocity_Max
	{	100,		   0.2f,				   3.0f,            10.0f,          {10.0f, 10.0f},  {80.0f, 80.0f}}
};

struct Particle {
	V2 position;
	V2 velocity;
	int size;
	float life_Time;
	Image image;
};

struct Particle_System {
	Particle_Type type;
	Particle particle_Info;
	float time_Between_Spawns;
	std::vector<Particle> particles;
};

void swap_Floats(float& a, float& b);

float random_Float_In_Range(float min, float max);

V2 random_Vector_In_Range(V2 min, V2 max);

void spawn_Particle_Systems(Game_Data& game_Data, Particle_Type type, V2 pos, int w, int h, Image image);

void update_Particle_Systems(Particle_System particle_System, float delta_Time);

void render_Particle_System(Game_Data& game_Data);