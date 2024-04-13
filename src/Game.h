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
	FILE* file;
	GAME_DATA_OPERATION operation;
};

std::string create_Save_Game_File_Name(Saved_Games save_Game);

bool check_If_File_Exists(const char* file_Name);

void process_Float(float& my_Float, Archive* archive);

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

void process_Game_Data(Game_Data* game_Data, Saved_Games save_Game, GAME_DATA_OPERATION operation);

void reset_Game(Game_Data* game_Data);

Cache_Data create_Cache_Data(std::unordered_map<std::string, Game_Data>& cache);

void load_Game_Data_Cache(Cache_Data& cache_Data);

void save_Game_To_Cache(Saved_Games save_Game, Game_Data& game_Data, Cache_Data& cache_Data);