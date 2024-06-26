#pragma once
#include "Sprite.h"
#include <string>
#include <vector>
#include <unordered_map>

struct Handle;
struct Game_Data;
struct F_Color {
	float r;
	float g;
	float b;
	float a;
};

enum Particle_Type {
	PT_BLOOD,
	PT_WATER,
	PT_RAIN,
	PT_RAINBOW,

	PT_TOTAL
};

struct Particle_Data {
	std::string type;
	std::string sprite_Sheet_Name;
	int size;
	float particles_Per_Second;
	int can_Collide_With_Terrain;
	float max_Fade;
	float lifetime_Min;
	float lifetime_Max;
	float gravity_Multi;
	V2 velocity_Min;
	V2 velocity_Max;
};

struct Particle {
	bool can_Collide_With_Terrain;
	int size;
	float lifetime_Max;
	float lifetime;
	float fade_In;
	V2 velocity;
	V2 position;
};

struct Particle_System {
	SDL_Rect rect;
	Sprite_Sheet_Tracker sprite_Sheet_Tracker;
	std::string particle_Type;
	float time_Between_Spawns;
	float lifetime;
	bool destroyed;
	Handle handle;
	Handle parent;
	bool flip_Horizontally = false;
	std::vector<Particle> particles;
};

Handle spawn_Particle_System(Game_Data& game_Data, std::string particle_Type, V2 pos, float lifetime, int w, int h, Handle parent = { (uint8_t)-1, (uint8_t)0 }, bool flip_Horizontally = false);
void update_Particle_System(Game_Data& game_Data, Particle_System& particle_System, float delta_Time);
void draw_Particle_Systems(Game_Data& game_Data);
void check_Particle_System_Collision_With_Terrain(Game_Data& game_Data, Particle_System& particle_System);
void check_Particle_Collision_With_Terrain(Game_Data& game_Data, Particle& particle);
F_Color HSV_To_RGB(float h, float s, float v);

void load_Particle_Data_CSV(CSV_Data* csv_Data);
void attempt_Reload_Particle_CSV_File(CSV_Data* csv_Data);