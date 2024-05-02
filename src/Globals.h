#pragma once
#include <unordered_map>
#include <string.h>

#define RESOLUTION_WIDTH 1920
#define RESOLUTION_HEIGHT 1080

// Forward declaration
struct SDL_Renderer;
struct Particle_Data;
struct Sprite_Sheet;
struct Unit_Data;

namespace Globals {
	extern SDL_Renderer* renderer;
	extern std::unordered_map<std::string, Particle_Data> particle_Data_Map;
	extern std::unordered_map<std::string, Sprite_Sheet> sprite_Sheet_Map;
	extern std::unordered_map<std::string, Unit_Data> unit_Data_Map;

	const float GRAVITY = 300;
	const float ARCHER_ARROW_GRAVITY = 50;

	const int MAX_ATTACHED_ENTITIES = 100;
	const int MAX_COLLIDERS = 100;

	const int MAX_PARTICLES = 1000;
}
