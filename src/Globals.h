#pragma once
#include <unordered_map>
#include <string.h>

#define RESOLUTION_WIDTH 1920
#define RESOLUTION_HEIGHT 1080

// Forward declaration
struct SDL_Renderer;

namespace Globals {
	extern SDL_Renderer* renderer;

	const float GRAVITY = 300;
	const float ARCHER_ARROW_GRAVITY = 50;

	const int MAX_ATTACHED_ENTITIES = 100;
	const int MAX_COLLIDERS = 100;

	const int MAX_PARTICLES = 1000;

	const int MAX_ENTITY_ARRAY_LENGTH = 100;
}
