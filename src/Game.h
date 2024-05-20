#pragma once
#include <unordered_map>
#include "Entity.h"

enum Game_State {
	GS_MENU,
	GS_GAMELOOP,
	GS_PAUSED,
	GS_GAMEOVER,
	GS_VICTORY,
	GS_LOADGAME,
	GS_SAVEGAME
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
void process(Archive* ar, T (&my_Array)[size]) {
	for (int i = 0; i < size; i++) {
		process(ar, my_Array[i]);
	}
}

void process(Archive* ar, bool& my_Bool);
void process(Archive* ar, int& my_Int);
void process(Archive* ar, float& my_Float);
void process(Archive* ar, SDL_Rect& rect);
void process(Archive* ar, std::string& string);
void process(Archive* ar, Sprite_Sheet_Tracker& sprite_Sheet_Tracker);
void process(Archive* ar, Particle& particle);
void process(Archive* ar, Particle_System& particle_System);
void process(Archive* ar, std::vector<Particle_System>& particle_Systems);
void process(Archive* ar, V2& vector);
void process(Archive* ar, Rigid_Body& rigid_Body);
void process(Archive* ar, Health_Bar& health_Bar);
void process(Archive* ar, Cooldown& cooldown);
void process(Archive* ar, Castle& castle);
void process(Archive* ar, Projectile& projectile);
void process(Archive* ar, Game_Data* game_Data);

void load_Game(Game_Data& game_Data, Saved_Games save_Game);
void save_Game(Game_Data& game_Data, Saved_Games save_Game);
void start_Game(Game_Data* game_Data);

Cache_Data create_Cache_Data(std::unordered_map<std::string, Game_Data>& cache);
void load_Game_Data_Cache(Cache_Data& cache_Data);
void save_Game_To_Cache(Saved_Games save_Game, Game_Data& game_Data, Cache_Data& cache_Data);