#pragma once
#include "Sprite.h"
#include "Particle_System.h"
#include <queue>
#include <stdint.h>
#include <assert.h>

struct Health_Bar {
	float max_HP;
	float current_HP;
	int width;
	int height;
	int y_Offset;
	int thickness;
};

struct Collider_Data {
	std::string type;
	float position_LS_X;
	float position_LS_Y;
	float radius;
};

struct Collider {
	V2 position_LS;
	float radius;
};

struct Rigid_Body {
	bool rigid_Body_Faces_Velocity;
	V2 position_WS;
	V2 velocity;
	float angle;
	int num_Colliders;
	Collider colliders[Globals::MAX_COLLIDERS];
};

struct Stored_Units {
	int current;
	int max;
};

enum Level {
	LEVEL_1,
	LEVEL_2,
	TOTAL_LEVELS
};

struct Cooldown {
	float duration;
	float remaining;
};

struct Castle_Stats {
	float hp;

	Cooldown fire_Cooldown;
	Cooldown spawn_Cooldown;

	int arrow_Ammo;
	Cooldown arrow_Ammo_Cooldown;

	Stored_Units stored_Units;
};

const Castle_Stats castle_Stats_Array[TOTAL_LEVELS] = {
	// hp    |    fire_Cooldown   |  spawn_Cooldown   |  arrow_Ammo    |   arrow_Ammo_Cooldown  |  stored_Units
	{  100.0f,    {0.01f, 0.0f},     {1.0f, 0.0f},       0,                {0.25f, 0.0f},          {0, 3}     },
	{  100.0f,    {1.0f, 0.0f},      {100.0f, 0.0f},       0,                {0.25f, 0.0f},          {0, 3}     }
};

struct Castle {
	Sprite_Sheet_Tracker sprite_Sheet_Tracker;
	Rigid_Body rigid_Body;
	Health_Bar health_Bar;

	Cooldown fire_Cooldown;
	Cooldown spawn_Cooldown;

	int arrow_Ammo;
	Cooldown arrow_Ammo_Cooldown;

	Stored_Units stored_Units;
};

struct Projectile_Data {
	std::string type;
	std::string sprite_Sheet_Name;
	std::string collider;

	bool can_Attach;

	float gravity;
	float speed;
	float life_Time;

	float collider_Pos_LS_X;
	float collider_Pos_LS_Y;
	float collider_Radius;
};

struct Projectile {
	std::string type;
	Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;

	float damage;
	float speed;
	float life_Time;
	Cooldown collision_Delay;

	Handle handle;
	Handle target_Handle;

	bool can_Attach;
	float gravity;
	bool stop;
	bool destroyed;
};

enum Nation {
	N_PLAYER,
	N_ENEMY
};

// Applies to all units
struct Unit_Data {
	std::string type;
	std::string sprite_Sheet_Name;
	std::string projectile_Type;
	float max_HP;
	float damage;
	float speed;
	float attack_Cooldown;
	float attack_Range;
	// std::string spell_Type;
};

struct Attached_Entity {
	Sprite_Sheet_Tracker sprite_Sheet_Tracker;
	float angle;
	V2 offset;
};

struct Unit {
	Sprite_Sheet_Tracker sprite_Sheet_Tracker;
	Rigid_Body rigid_Body;
	Health_Bar health_Bar;

	float speed;
	float damage;
	float attack_Cooldown;
	float current_Attack_Cooldown;
	float attack_Range;
	std::string projectile_Type;
	bool fire_Projectiles;

	Attached_Entity attached_Entities[Globals::MAX_ATTACHED_ENTITIES];
	int attached_Entities_Size = 0;

	bool destroyed;
	bool stop;

	Handle handle;
};

//struct Unit_Storage {
//	Generation generations[Globals::MAX_ENTITY_ARRAY_LENGTH] = {};
//	uint64_t index_One_Past_Last = 0;
//	// Array of units
//	Unit arr[Globals::MAX_ENTITY_ARRAY_LENGTH] = {};
//};

template <typename T>
struct Storage {
	Generation generations[Globals::MAX_ENTITY_ARRAY_LENGTH] = {};
	uint64_t index_One_Past_Last = 0;
	T arr[Globals::MAX_ENTITY_ARRAY_LENGTH] = {};
};

template <typename T>
Handle create_Handle(Storage<T>& storage) {
	Handle result = {};
	uint64_t length = ARRAY_SIZE(storage.generations);
	// For wrapping. We start at the last known open handle position.
	uint64_t start_Index = storage.index_One_Past_Last % length;
	for (uint64_t i = 0; i < length; i++) {
		// Wrapping
		uint64_t index = (start_Index + i) % length;
		if (!storage.generations[index].slot_Taken) {
			storage.generations[index].slot_Taken = true;
			if (storage.index_One_Past_Last < (index + 1)) {
				storage.index_One_Past_Last = index + 1;
			}
			result.generation = storage.generations[index].generation;
			result.index = index;
			break;
		}
	}
	assert(result.index < length);
	return result;
}

template <typename T>
void delete_Handle(Storage<T>& storage, const Handle handle) {
	uint64_t index = handle.index;
	if (index < ARRAY_SIZE(storage.generations) && handle.generation == storage.generations[index].generation) {
		storage.generations[index].generation++;
		storage.generations[index].slot_Taken = false;
	}
}

template <typename T>
T* get_Ptr_From_Handle_In_Storage(Storage<T>& storage, const Handle handle) {
	uint64_t index = handle.index;
	if (index < ARRAY_SIZE(storage.generations) && handle.generation == storage.generations[index].generation) {
		return &storage.arr[index];
	}
	return nullptr;
}

// Setting a max number of units could be the best approach. (Non dynamic arrays)
// I could use a C array (Chris would use this) or a C++ array
// If I started with a vector, it would just be for allocation and NO deleting
struct Game_Data {
	Castle									player_Castle;
	Storage<Unit>							player_Units;
	Storage<Projectile>						player_Projectiles;
	// Storage<Spell>						player_Spells;

	Castle									enemy_Castle;
	Storage<Unit>							enemy_Units;
	Storage<Projectile>						enemy_Projectiles;
	// Storage<Spell>						enemy_Spells;

	std::vector<Handle>						active_Entities;

	Storage<Particle_System>				particle_Systems;

	std::vector<int>						terrain_Height_Map;
	float									timer;
};

void* get_Ptr_From_Handle(Game_Data& game_Data, Handle handle);
void delete_Entity_From_Handle(Game_Data& game_Data, Handle handle);

void add_Collider(Rigid_Body* rigid_Body, V2 position_LS, float radius);

bool check_Height_Map_Collision(Rigid_Body* rigid_Body, std::vector<int>& height_Map);
bool check_RB_Collision(Rigid_Body* rigid_Body_1, Rigid_Body* rigid_Body_2);
bool check_Attack_Range_Collision(float origin_Attack_Range, Rigid_Body* origin_RB, Rigid_Body* target_RB);
void check_Player_Unit_Castle_Collision(Game_Data& game_Data);

V2 get_Collider_WS_Position(Rigid_Body* rigid_Body, const Collider* collider);
float get_Height_Map_Pos_Y(Game_Data& game_Data, int x_Pos);

const Unit_Data& get_Unit_Data(std::string key);
const Projectile_Data& get_Projectile_Data(std::string key);

Attached_Entity return_Attached_Entity(std::string sprite_Sheet_Name, float angle, V2 offset);

void spawn_Player_Castle(Game_Data& game_Data, V2 position_WS, Level level);
void spawn_Enemy_Castle(Game_Data& game_Data, V2 position_WS, Level level);
void spawn_Projectile(Game_Data& game_Data, Nation unit_Side, std::string projectile_Type, float damage, V2 origin_Pos, V2 target_Pos);
// spawn_Unit("type", level (scalar));
// Anytime I need a 'if' statement, add it in the csv.
// Anytime something is different in the spawn functions, add it to the .csv file. All units should 
// be treated the exact same.
void spawn_Unit(Game_Data& game_Data, Nation unit_Side, std::string unit_Type, int level, V2 spawn_Position, V2 target_Position);

void update_Animation(Sprite_Sheet_Tracker* tracker, float unit_Speed, float delta_Time);
void update_Projectile_Position(Projectile* projectile, float delta_Time);
void update_Unit_Position(Rigid_Body* rigid_Body, bool stop_Unit, float delta_Time);

void draw_Castle(Castle* castle, bool flip);
void draw_Projectile(Projectile* projectile, bool flip);
void draw_Attached_Entity(Attached_Entity* attached_Entity, V2 position_WS, bool flip);
void draw_Unit_Animated(Rigid_Body* rigid_Body, Sprite_Sheet_Tracker* tracker, bool flip);
void draw_Circle(float center_X, float center_Y, float radius, Color_Index color);
void draw_RigidBody_Colliders(Rigid_Body* rigid_Body, Color_Index color);

void change_Animation(Sprite_Sheet_Tracker* tracker, std::string sprite_Sheet_Name);

void outline_Rect(SDL_Rect* rect, int outline_Thickness);
void draw_HP_Bar(V2* position, Health_Bar* health_Bar);

Health_Bar create_Health_Bar(int width, int height, int y_Offset, int thickness, float hp);
Rigid_Body create_Rigid_Body(V2 position_WS, bool rigid_Body_Faces_Velocity);
std::string create_Unit_Data_Map_Key(std::string sprite_Sheet_Name);
std::vector<int> create_Height_Map(const char* filename);

void load_Unit_Data_CSV(CSV_Data* csv_Data);
void load_Projectile_Data_CSV(CSV_Data* csv_Data);