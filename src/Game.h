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
void write_Vector(std::vector<T>& vector, FILE* file) {
	size_t vector_Size = vector.size();
	fwrite(&vector_Size, sizeof(vector_Size), 1, file);
	fwrite(vector.data(), sizeof(vector[0]), vector_Size, file);
}

template <typename T>
void read_Vector(std::vector<T>& vector, FILE* file) {
	size_t vector_Size = 0;
	fread(&vector_Size, sizeof(vector_Size), 1, file);
	vector.clear();
	vector.resize(vector_Size);
	fread(vector.data(), sizeof(vector[0]), vector_Size, file);
}

template <typename T>
void process(std::vector<T>& vector, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		size_t vector_Size = vector.size();
		fwrite(&vector_Size, sizeof(vector_Size), 1, archive->file);
		for (T& value : vector) {
			process(value, archive);
		}
	}
	else if (archive->operation == GDO_LOAD) {
		size_t vector_Size = 0;
		fread(&vector_Size, sizeof(vector_Size), 1, archive->file);
		vector.clear();
		vector.resize(vector_Size);
		for (T& value : vector) {
			process(value, archive);
		}
	}
}

template <typename T, int size>
void process(T (&my_Array)[size], Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		for (int i = 0; i < size; i++) {
			process(my_Array[i], archive);
		}
	}
	else if (archive->operation == GDO_LOAD) {
		for (int i = 0; i < size; i++) {
			process(my_Array[i], archive);
		}
	}
}

void process(bool& my_Bool, Archive* archive);

void process(int& my_Int, Archive* archive);

void process(float& my_Float, Archive* archive);

void process(SDL_Rect& rect, Archive* archive);

void process(std::string& string, Archive* archive);

void process(Sprite_Sheet_Tracker& sprite_Sheet_Tracker, Archive* archive);

void process(Particle& particle, Archive* archive);

void process(Particle_System& particle_System, Archive* archive);

void process(std::vector<Particle_System>& particle_Systems, Archive* archive);

void process(V2& vector, Archive* archive);

void process(Rigid_Body& rigid_Body, Archive* archive);

void process(Health_Bar& health_Bar, Archive* archive);

void process(Cooldown& cooldown, Archive* archive);

void process(Castle& castle, Archive* archive);

void process(Warrior& warrior, Archive* archive);

void process(Arrow_Type& arrow_Type, Archive* archive);

void process(Arrow& arrow, Archive* archive);

void process(Archer& archer, Archive* archive);

void process(Game_Data* game_Data, Archive* archive);

void load_Game(Game_Data* game_Data, Saved_Games save_Game);

void save_Game(Game_Data* game_Data, Saved_Games save_Game);

void start_Game(Game_Data* game_Data);

Cache_Data create_Cache_Data(std::unordered_map<std::string, Game_Data>& cache);

void load_Game_Data_Cache(Cache_Data& cache_Data);

void save_Game_To_Cache(Saved_Games save_Game, Game_Data& game_Data, Cache_Data& cache_Data);