#pragma once
#include "Sprite.h"
#include "Particle_System.h"
#include <queue>
#include <stdint.h>
#include <assert.h>

struct Resource_Bar_Color {
	Color left_Rect;
	Color right_Rect;
};

enum Resource_Bar_Color_Selector {
	RBCS_HP_Bar,
	RBCS_Food_Bar,
	RBCS_Spell_Bar,
	RBCS_Total
};

struct Resource_Bar {
	float max_Resource;
	float current_Resource;
	float regen;
	float accumulated_Time = 0.0f;
	int width;
	int height;
	int y_Offset;
	int thickness;
	Resource_Bar_Color_Selector selected_Colors;
};

const Resource_Bar_Color resource_Bar_Colors[RBCS_Total] = {
	// Left rect		 Right rect
	{ {0, 255, 0},		{255, 0, 0} },		// HP Bar
	{ {255, 95, 31},	{255, 255, 255} },  // Food Bar
	{ {171, 32, 253},	{255, 255, 255} }	// Spell Bar
};

enum Nation {
	N_PLAYER,
	N_ENEMY
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

struct Cooldown {
	float duration;
	float remaining;
};

struct Castle_Data {
	std::string type;
	std::string sprite_Sheet_Name;
	std::string enhancement;

	float base_HP;
	float base_HP_Regen;
	float base_Food_Points;
	float base_Food_Points_Regen;
	std::string projectile_Type;
	// Stored_Units stored_Units;
};

struct Projectile_Data {
	std::string type;
	std::string sprite_Sheet_Name;
	std::string collider;

	int max_Penetrations;
	bool can_Attach;

	float gravity_Multiplier;
	float speed;
	float life_Time;

	float collider_Pos_LS_X;
	float collider_Pos_LS_Y;
	float collider_Radius;

	float castle_Base_Ammo_Per_Sec;
	float castle_Ammo_Per_Sec_Multiplier;
	float castle_Max_Ammo_Per_Sec;
};

struct Projectile {
	std::string type;
	Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;

	float damage;
	float speed;
	float life_Time;
	Cooldown attached_Entity_Delay;
	int current_Penetrations;
	int penetrated_Enemy_IDS_Size;
	Handle penetrated_Enemy_IDS[Globals::MAX_PROJECTILE_PENETRATIONS] = {};

	Handle handle;
	Handle parent;

	bool can_Attach;
	float gravity;
	bool stop;
	bool destroyed;
};

struct Spell_Data {
	std::string type;
	std::string summon_Type;
	float base_Cast_Time;
};

struct Spell {
	std::string type;
	bool can_Cast_Spell;
	Cooldown time_To_Cast;
};

// Applies to all units
struct Unit_Data {
	std::string type;
	// Stored in the summonable unit struct
	float food_Cost;
	std::string sprite_Sheet_Name;
	std::string projectile_Type;
	std::string spell_Type;
	float base_HP;
	float hp_Multiplier;
	float base_Damage;
	float damage_Multiplier;
	float speed;
	float base_Attack_Cooldown;
	float attack_Cooldown_Multiplier;
	float attack_Range;
};
 
struct Attached_Entity {
	Sprite_Sheet_Tracker sprite_Sheet_Tracker;
	float angle;
	V2 offset;
};

struct Unit {
	Nation nation;
	int level;
	Sprite_Sheet_Tracker sprite_Sheet_Tracker;
	Rigid_Body rigid_Body;
	Resource_Bar health_Bar;

	Spell spell;
	Resource_Bar spell_Bar;

	float speed;
	float damage;
	float attack_Cooldown;
	float current_Attack_Cooldown;
	float attack_Range;
	std::string projectile_Type;
	bool fires_Projectiles;

	Attached_Entity attached_Entities[Globals::MAX_ATTACHED_ENTITIES];
	int attached_Entities_Size = 0;

	bool destroyed;
	bool stop;

	Handle handle;
};

struct Unit_Level_Tracker {
	int warrior = 1;
	int archer = 1;
	int necromancer = 1;
};

struct Summonable_Unit {
	std::string name;
	int level = 1;
	bool is_Pressed;
	Nation nation;
	float food_Cost;
};

struct Castle {
	Nation nation;
	std::string castle_Type;
	int level;

	Sprite_Sheet_Tracker sprite_Sheet_Tracker;
	Rigid_Body rigid_Body;
	Resource_Bar health_Bar;

	Cooldown fire_Cooldown;
	Cooldown spawn_Cooldown;

	std::string projectile_Type;
	int projectile_Ammo;
	Cooldown projectile_Ammo_Cooldown;

	std::vector<Summonable_Unit> summonable_Units;
	Resource_Bar food_Bar;

	// Stored_Units stored_Units;
};

template <typename T>
struct Storage {
	uint32_t index_One_Past_Last = 0;
	Generation generations[Globals::MAX_ENTITY_ARRAY_LENGTH] = {};
	T arr[Globals::MAX_ENTITY_ARRAY_LENGTH] = {};
	Storage_Type st;
};

template <typename T>
Handle create_Handle(Storage<T>& storage) {
	Handle result = {};
	uint32_t length = ARRAY_SIZE(storage.generations);
	for (uint32_t i = 0; i < length; i++) {
		if (!storage.generations[i].slot_Taken) {
			storage.generations[i].slot_Taken = true;
			if (storage.index_One_Past_Last < (i + 1)) {
				storage.index_One_Past_Last = i + 1;
			}
			result.generation = storage.generations[i].generation;
			result.index = i;
			break;
		}
	}
	result.storage_Type = storage.st;
	assert(result.index < length);
	return result;
}

template <typename T>
void delete_Handle(Storage<T>& storage, const Handle handle) {
	uint32_t index = handle.index;
	if (index < ARRAY_SIZE(storage.generations) && handle.generation == storage.generations[index].generation) {
		// This wraps
		storage.generations[index].generation++;
		storage.generations[index].slot_Taken = false;
	}
}

// Setting a max number of units could be the best approach. (Non dynamic arrays)
// I could use a C array (Chris would use this) or a C++ array
// If I started with a vector, it would just be for allocation and NO deleting
struct Game_Data {
	Storage<Unit>							units = { .st = ST_Units };
	Storage<Projectile>						projectiles = { .st = ST_Projectile };
	Storage<Spell>							spells = { .st = ST_Spell };

	Castle									player_Castle;
	std::vector<Handle>						player_Unit_IDS;
	std::vector<Handle>						player_Proj_IDS;
	std::vector<Handle>						player_Spell_IDS;
	
	Castle									enemy_Castle;
	std::vector<Handle>						enemy_Unit_IDS;
	std::vector<Handle>						enemy_Proj_IDS;
	std::vector<Handle>						enemy_Spell_IDS;

	Storage<Particle_System>				particle_Systems = { .st = ST_Particle_System };
	std::vector<Handle>						particle_System_IDS;

	std::vector<int>						terrain_Height_Map;
	float									timer;

	Handle									particle_System_Rain_ID;
};

template <typename T>
T* get_Entity(Storage<T>& storage, Handle handle) {
	if (handle.index < ARRAY_SIZE(storage.generations) &&
		handle.generation == storage.generations[handle.index].generation &&
		handle.storage_Type == storage.st &&
		handle.generation != 0) {
		return &storage.arr[handle.index];
	}
	return nullptr;
}

Unit* get_Unit(Storage<Unit>& storage, Handle handle);
Spell* get_Spell(Storage<Spell>& storage, Handle handle);
Projectile* get_Projectile(Storage<Projectile>& storage, Handle handle);
Particle_System* get_Particle_System(Storage<Particle_System>& storage, Handle handle);
bool compare_Handles(Handle handle_1, Handle handle_2);
void delete_Expired_Entity_Handles(Game_Data& game_Data);

void clear_Game_Data(Game_Data* game_Data);

void add_Collider(Rigid_Body* rigid_Body, V2 position_LS, float radius);

void update_Units_Variables(Game_Data& game_Data, std::vector<Handle>& units, float delta_Time);
void update_Resource_Bar(Resource_Bar& bar, float delta_Time);
void update_Units_Spell_Bars(Game_Data& game_Data, std::vector<Handle>& units, float delta_Time);
// Not liking this function atm but I'm going to keep it temporarily until I finish this api
void update_Nation_Resource_Bars(/*Game_Data& game_Data, */Castle& castle, /*std::vector<Handle>& units,*/ float delta_Time);

bool check_Height_Map_Collision(Rigid_Body* rigid_Body, std::vector<int>& height_Map);
bool check_RB_Collision(Rigid_Body* rigid_Body_1, Rigid_Body* rigid_Body_2);
bool check_Attack_Range_Collision(float origin_Attack_Range, Rigid_Body* origin_RB, Rigid_Body* target_RB);
void check_Player_Unit_Castle_Collision(Game_Data& game_Data);
void check_Units_Collisions_With_Terrain(Game_Data& game_Data, std::vector<Handle>& units);
void check_Units_Collisions_With_Castle(Game_Data& game_Data, std::vector<Handle> units, Castle& target_Castle);
void check_Units_Collisions_With_Units(Game_Data& game_Data, std::vector<Handle> origin_Units, std::vector<Handle> target_Units);
void check_Projectiles_Collisions(Game_Data& game_Data, std::vector<Handle>& projectiles, Castle& target_Castle, std::vector<Handle>& target_Units, float delta_Time);

V2 get_Collider_WS_Position(Rigid_Body* rigid_Body, const Collider* collider);
float get_Height_Map_Pos_Y(Game_Data& game_Data, int x_Pos);

const Unit_Data& get_Unit_Data(std::string key);
const Projectile_Data& get_Projectile_Data(std::string key);
const Castle_Data& get_Castle_Data(std::string key);
int get_Castle_Data_Size();

Attached_Entity return_Attached_Entity(std::string sprite_Sheet_Name, float angle, V2 offset);

void add_Summonable_Unit_To_Castle(Game_Data& game_Data, Nation nation, std::string unit_Name);

void spawn_Castle(Game_Data& game_Data, Nation nation, std::string castle_Type, int castle_Level);
void spawn_Projectile(Game_Data& game_Data, Nation unit_Side, std::string projectile_Type, float damage, V2 origin_Pos, V2 target_Pos);
// spawn_Unit("type", level (scalar));
// Anytime I need a 'if' statement, add it in the csv.
// Anytime something is different in the spawn functions, add it to the .csv file. All units should 
// be treated the exact same.
void spawn_Unit(Game_Data& game_Data, Nation unit_Side, std::string unit_Type, int level, V2 spawn_Position, V2 target_Position);
void spawn_Unit_At_Castle(Game_Data& game_Data, Summonable_Unit& summonable_Unit);
void cast_Raise_Dead(Game_Data& game_Data, Handle casting_Unit_ID);
void cast_Spell(Game_Data& game_Data, Handle casting_Unit_ID);
void cast_Units_Spells(Game_Data& game_Data, std::vector<Handle> units, float delta_Time);

void update_Animation(Sprite_Sheet_Tracker* tracker, float unit_Speed, float delta_Time);
void update_Unit_Position(Rigid_Body* rigid_Body, bool stop_Unit, float delta_Time);
void update_Units_Positions(Game_Data& game_Data, std::vector<Handle>& units, float delta_Time);
void update_Projectile_Position(Projectile* projectile, float delta_Time);
void update_Projectiles_Positions(Game_Data& game_Data, std::vector<Handle>& projectiles, float delta_Time);

void draw_Castle(Castle* castle, bool flip);
void draw_Projectile(Projectile* projectile, bool flip);
void draw_Attached_Entity(Attached_Entity* attached_Entity, V2 position_WS, bool flip);
void draw_Unit_Animated(Rigid_Body* rigid_Body, Sprite_Sheet_Tracker* tracker, bool flip);
void draw_Circle(float center_X, float center_Y, float radius, Color_Index color);
void draw_RigidBody_Colliders(Rigid_Body* rigid_Body, Color_Index color);
void outline_Rect(SDL_Rect* rect, int outline_Thickness);
void draw_Resource_Bar(Resource_Bar& health_Bar, V2 pos);

void change_Animation(Sprite_Sheet_Tracker* tracker, std::string sprite_Sheet_Name);

Resource_Bar create_Resource_Bar(int width, int height, int y_Offset, int thickness, float resource, float regen, Resource_Bar_Color_Selector colors);
Rigid_Body create_Rigid_Body(V2 position_WS, bool rigid_Body_Faces_Velocity);
std::string create_Unit_Data_Map_Key(std::string sprite_Sheet_Name);
std::vector<int> create_Height_Map(const char* filename);

void load_Unit_Data_CSV(CSV_Data* csv_Data);
void load_Projectile_Data_CSV(CSV_Data* csv_Data);
void load_Spell_Data_CSV(CSV_Data* csv_Data);
void load_Castle_Data_CSV(CSV_Data* csv_Data);