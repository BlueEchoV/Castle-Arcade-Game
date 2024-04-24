#pragma once
#include "Image.h"
#include <string>
#include <vector>
#include <unordered_map>

struct Game_Data;
struct F_Color {
	float r;
	float g;
	float b;
	float a;
};

struct Particle_Data {
	int size;
	float time_Between_Spawns;

	float max_Fade;
	float lifetime_Min;
	float lifetime_Max;
	V2 velocity_Min;
	V2 velocity_Max;
};

/*
enum Particle_Type {
	PT_BLOOD,
	PT_WATER,
	PT_TOTAL
};

const Particle_Data particle_Data_Array[] = {
	// size  time_Between_Spawns  max_Fade_In lifetime_Min  lifetime_Max  velocity_Min  velocity_Max  
	{ 20,	 0.01f,				  0.5f,		  3.0f,			3.0f,		 {0.0f, 0.0f}, {0.0f, 0.0f}},
	{ 20,	 0.01f,				  0.5f,		  3.0f,			3.0f,		 {0.0f, 0.0f}, {0.0f, 0.0f}}
};

// Good habit to check if the array size matches the enum (Forces compilation error)
static_assert(ARRAY_SIZE(particle_Data_Array) == PT_TOTAL);
*/

struct Particle {
	int size;
	float lifetime_Max;
	float lifetime;
	float fade_In;
	V2 velocity;
	V2 position;
};

struct Particle_System {
	Image* image;
	SDL_Rect rect;
	std::string type;
	float time_Between_Spawns;
	std::vector<Particle> particles;
	// The particle system lifetime
	float lifetime;
	bool destroyed;
	int target_ID = -1;
};

void spawn_Particle_System(Game_Data& game_Data, std::string type, V2 pos, float lifetime, int w, int h, Image* image, int target_ID = -1);

void update_Particle_System(Particle_System& particle_System, float delta_Time);

F_Color HSV_To_RGB(float h, float s, float v);

void draw_Particle_Systems(Game_Data& game_Data);

void load_Particle_Data_CSV(std::string file_Name);