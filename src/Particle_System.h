#pragma once
#include "Image.h"
#include <vector>

struct Game_Data;

enum Particle_Type {
	PT_BLOOD,
	PT_WATER,
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

	float max_Fade_In;
	float lifetime_Min;
	float lifetime_Max;
	V2 velocity_Min;
	V2 velocity_Max;
};

const Particle_Data particle_Data_Array[] = {
	//  
	{ 20, 0.01f, 0.5f, 2.0f, 3.0f, {0.0f, 0.0f}, {0.0f, 0.0f}},
	{ 20, 0.01f, 0.5f, 2.0f, 3.0f, {0.0f, 0.0f}, {0.0f, 0.0f}}
};

// Good habit to check if the array size matches the enum (Forces compilation error)
static_assert(ARRAY_SIZE(particle_Data_Array) == PT_TOTAL);

struct Particle {
	int size;
	float lifetime;
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
	// The particle system lifetime
	float lifetime;
	bool destroyed;
	int id;
};

void spawn_Particle_Systems(Game_Data& game_Data, Particle_Type type, float lifetime, V2 pos, int w, int h, Image* image);

void update_Particle_System(Particle_System& particle_System, V2 target_Position, float delta_Time);

void draw_Particle_Systems(Game_Data& game_Data);