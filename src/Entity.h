#pragma once
#include "Sprite.h"
#include "Particle_System.h"

struct Health_Bar {
	float max_HP;
	float current_HP;
	int width;
	int height;
	int y_Offset;
	int thickness;
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

enum Arrow_Type {
	AT_PLAYER_ARROW,
	AT_ARCHER_ARROW,
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
	{  100.0f,    {1.0f, 0.0f},      {1.0f, 0.0f},       0,                {0.25f, 0.0f},          {0, 3}     }
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

struct Arrow_Stats {
	float speed;
	float damage;
	float life_Time;
};

const Arrow_Stats arrow_Stats_Array[TOTAL_LEVELS] = {
	// speed    |   damage  |   life_Time
	{  800,         25,          100     },
	{  800,         100,         100     }
};

struct Arrow {
	Arrow_Type type;

	Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;

	float damage;
	float speed;
	float life_Time;
	Cooldown collision_Delay;
	int target_ID;

	bool stop;
	bool destroyed;
};

enum Unit_Side {
	PLAYER,
	ENEMY
};

// Applies to all units
struct Unit_Data {
	std::string type;
	std::string sprite_Sheet_Name;
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

	Attached_Entity attached_Entities[Globals::MAX_ATTACHED_ENTITIES];
	int attached_Entities_Size = 0;

	bool destroyed;
	bool stop;

	int ID;
};

struct Game_Data {
	float									timer;
	Castle									player_Castle;
	Castle									enemy_Castle;
	std::vector<int>						terrain_Height_Map;
	std::vector<Arrow>						player_Arrows;
	std::vector<Unit>						player_Units;
	std::vector<Unit>						enemy_Units;
	std::vector<Particle_System>			particle_Systems;
	int										next_Entity_ID;
};

void add_Collider(Rigid_Body* rigid_Body, V2 position_LS, float radius);

bool check_Height_Map_Collision(Rigid_Body* rigid_Body, std::vector<int>& height_Map);
bool check_RB_Collision(Rigid_Body* rigid_Body_1, Rigid_Body* rigid_Body_2);

V2 get_WS_Position(Rigid_Body* rigid_Body, const Collider* collider);
float get_Height_Map_Pos_Y(Game_Data* game_Data, int x_Pos);

Attached_Entity return_Attached_Entity(std::string sprite_Sheet_Name, float angle, V2 offset);

void spawn_Player_Castle(Game_Data* game_Data, V2 position_WS, Level level);
void spawn_Enemy_Castle(Game_Data* game_Data, V2 position_WS, Level level);
void spawn_Arrow(Game_Data* game_Data, Arrow_Type type, V2 spawn_Position, V2 target_Position, Level level);
// spawn_Unit("type", level (scalar));
// Anytime I need a 'if' statement, add it in the csv.
// Anytime something is different in the spawn functions, add it to the .csv file. All units should 
// be treated the exact same.
void spawn_Unit(Game_Data* game_Data, Unit_Side unit_Side, std::string unit_Type, int level, V2 spawn_Position, V2 target_Position);

void update_Animation(Sprite_Sheet_Tracker* tracker, float unit_Speed, float delta_Time);
void update_Arrow_Position(Arrow* arrow, float delta_Time);
void update_Unit_Position(Rigid_Body* rigid_Body, bool stop_Unit, float delta_Time);

void draw_Castle(Castle* castle, bool flip);
void draw_Arrow(Arrow* arrow, bool flip);
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

void load_Unit_Data_CSV(std::string file_Name);