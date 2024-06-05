#pragma once
#include <unordered_map>
#include "Entity.h"

enum Game_State {
	GS_MAIN_MENU,
	GS_GAMELOOP,
	GS_PAUSED,
	GS_GAMEOVER,
	GS_VICTORY
};

enum GAME_DATA_OPERATION {
	GDO_SAVE,
	GDO_LOAD
};

enum Saved_Games {
	SG_SAVE_GAME_1,
	SG_SAVE_GAME_2,
	SG_SAVE_GAME_3,
	SG_TOTAL,
};

struct Cache_Data {
	bool loaded;
	std::unordered_map<std::string, Game_Data> cache;
};

struct Archive {
	std::string file_Name;
	FILE* file;
	GAME_DATA_OPERATION operation;
};

std::string create_Save_Game_File_Name(Saved_Games save_Game);

bool check_If_File_Exists(const char* file_Name);

// NOTE: Templates should be defined in a header file because 
// the compiler needs to have access to the full definition
template <typename T>
void write_Vector(FILE* file, std::vector<T>& vector) {
	size_t vector_Size = vector.size();
	fwrite(&vector_Size, sizeof(vector_Size), 1, file);
	fwrite(vector.data(), sizeof(vector[0]), vector_Size, file);
}

template <typename T>
void read_Vector(FILE* file, std::vector<T>& vector) {
	size_t vector_Size = 0;
	fread(&vector_Size, sizeof(vector_Size), 1, file);
	vector.clear();
	vector.resize(vector_Size);
	fread(vector.data(), sizeof(vector[0]), vector_Size, file);
}

template <typename T>
void process(Archive* ar, std::vector<T>& vector) {
	if (ar->operation == GDO_SAVE) {
		size_t vector_Size = vector.size();
		fwrite(&vector_Size, sizeof(vector_Size), 1, ar->file);
		for (T& value : vector) {
			process(ar, value);
		}
	}
	else if (ar->operation == GDO_LOAD) {
		size_t vector_Size = 0;
		fread(&vector_Size, sizeof(vector_Size), 1, ar->file);
		vector.clear();
		vector.resize(vector_Size);
		for (T& value : vector) {
			process(ar, value);
		}
	}
}

// T my_Array[] is the same as T* my_Array
// T my_Array[3] is also the same as T* my_Array. This syntax could be 
// good for documenting the number I expect, but it's STILL a pointer
// int size is creating a new function for different sizes (more code)
template <typename T, int size>
//										Deduces the size of the array at compile time
void process(Archive* ar, T (&my_Array)[size]) {
	for (int i = 0; i < size; i++) {
		process(ar, my_Array[i]);
	}
}

// Only loop through the active indices
template <typename T, int size>
void process(Archive* ar, T (&my_Array)[size], uint32_t active_Indicies) {
	for (uint32_t i = 0; i < active_Indicies; i++) {
		process(ar, my_Array[i]);
	}
}

template <typename T>
void process(Archive* ar, Storage<T>& storage) {
	process(ar, storage.index_One_Past_Last);
	for (uint32_t i = 0; i < storage.index_One_Past_Last; i++) {
		process(ar, storage.generations[i]);
	}
	for (uint32_t i = 0; i < storage.index_One_Past_Last; i++) {
		process(ar, storage.arr[i]);
	}
	process(ar, storage.st);
}

void process(Archive* ar, bool& my_Bool);
void process(Archive* ar, int& my_Int);
void process(Archive* ar, float& my_Float);
void process(Archive* ar, uint8_t& my_Int);
void process(Archive* ar, uint16_t& my_Int);
void process(Archive* ar, uint32_t& my_Int);
void process(Archive* ar, SDL_Rect& rect);
void process(Archive* ar, Storage_Type& storage_Type);
void process(Archive* ar, Generation& gen);
void process(Archive* ar, Handle& handle);
void process(Archive* ar, Unit_Level_Tracker& storage_Type);
void process(Archive* ar, std::string& string);
void process(Archive* ar, Sprite_Sheet_Tracker& sprite_Sheet_Tracker);
void process(Archive* ar, Particle& particle);
void process(Archive* ar, Particle_System& particle_System);
void process(Archive* ar, std::vector<Particle_System>& particle_Systems);
void process(Archive* ar, V2& vector);
void process(Archive* ar, Rigid_Body& rigid_Body);
void process(Archive* ar, Resource_Bar& health_Bar);
void process(Archive* ar, Cooldown& cooldown);
void process(Archive* ar, Castle& castle);
void process(Archive* ar, Projectile& projectile);
void process(Archive* ar, Game_Data* game_Data);

extern std::unordered_map<std::string, Game_Data> saved_Games_Cache;
extern Cache_Data save_Game_Cache_Data;

void load_Game(Game_Data& game_Data, Saved_Games save_Game);
void save_Game(Game_Data& game_Data, Saved_Games save_Game);
void start_Game(Game_Data& game_Data);

Cache_Data create_Cache_Data(std::unordered_map<std::string, Game_Data>& cache);
void load_Game_Data_Cache(Cache_Data& cache_Data);
void save_Game_To_Cache(Saved_Games save_Game, Game_Data& game_Data, Cache_Data& cache_Data);

struct Castle_Info {
	Nation nation;
	std::string castle_Type;
	V2 position_WS;
	int castle_Level;
};

struct Game_Level {
	Castle_Info enemy_Castle;
	std::string background;
	std::string terrain;
	// Unit stats for the enemy that are independent for the player
	// Unit AI
	// Rewards for the player
};

struct Game_Level_Map {
	std::vector<Game_Level> game_Levels;
};

// As the map number increases, so does the difficulty and rewards
Game_Level create_Game_Level(std::string background, std::string terrain);
void create_Game_Level_Storage(int current_Map_Number);
// Override the current game_Data and init the character castle values to zero that 
// need to be initialized to zero
void load_Game_Level(Game_Data& game_Data, Game_Level_Map level_Map, Game_Level game_Level);
void update_Game_Level_Map();
void draw_Game_Level_Map();