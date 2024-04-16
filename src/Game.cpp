#include "Game.h"
#include <string>

// So I can change the formatting in the future
std::string create_Save_Game_File_Name(Saved_Games save_Game) {
	return "Save Game " + std::to_string(save_Game + 1) + ".dat";
}

// Checks if the file exists, even if the file is locked. And it's faster.
bool check_If_File_Exists(const char* file_Name) {
	struct stat stat_Temp = {};
	int result = stat(file_Name, &stat_Temp);
	if (result == -1) {
		SDL_Log("ERROR: File does not exist");
		// assert(errno == ENOENT);
		return false;
	}
	return true;
};

Archive create_Archive(std::string file_Name, GAME_DATA_OPERATION operation) {
	Archive result = {};

	result.file_Name = file_Name;
	result.file = NULL;
	result.operation = operation;

	return result;
}

void open_Archive(Archive* archive) {
	const char* file_Name_PTR = archive->file_Name.c_str();
	errno_t err = {};
	if (archive->operation == GDO_SAVE) {
		err = fopen_s(&archive->file, file_Name_PTR, "wb");
	}
	else if (archive->operation == GDO_LOAD) {
		err = fopen_s(&archive->file, file_Name_PTR, "rb");
	}
	if (err != 0 || !archive->file) {
		SDL_Log("ERROR: Unable to open file %s in save_Game", file_Name_PTR);
		return;
	}
}

void close_Archive(Archive* archive) {
	fclose(archive->file);
}

// Process the primitive (float, int, double)
void process_Float(float& my_Float, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&my_Float, sizeof(my_Float), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&my_Float, sizeof(my_Float), 1, archive->file);
	}
}

void process_Int(int& my_Int, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&my_Int, sizeof(my_Int), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&my_Int, sizeof(my_Int), 1, archive->file);
	}
}

void process_Castle(Castle& castle, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&castle, sizeof(castle), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&castle, sizeof(castle), 1, archive->file);
	}
}

// void process_Int
// void process_etc..

// function that opens archive
// Close archive function as well

// Save game function creates write archive
// Load game will call read archive (process game_Data)

// Should be given a file handle and not be responsible for opening the file
// I will have a function that opens a archive for reading or writing
//                                                          Archive param
void process_Game_Data(Game_Data* game_Data, Saved_Games save_Game, GAME_DATA_OPERATION operation) {
	std::string file_Name_String = create_Save_Game_File_Name(save_Game).c_str();
	// Create archive
	Archive archive = create_Archive(file_Name_String, operation);
	open_Archive(&archive);

	process_Float(game_Data->timer, &archive);
	process_Castle(game_Data->player_Castle, &archive);
	process_Castle(game_Data->enemy_Castle, &archive);
	process_Vector(game_Data->terrain_Height_Map, &archive);
	process_Vector(game_Data->player_Arrows, &archive);
	process_Vector(game_Data->enemy_Skeletons, &archive);
	process_Vector(game_Data->player_Skeletons, &archive);
	process_Vector(game_Data->player_Archers, &archive);
	process_Int(game_Data->next_Entity_ID, &archive);

	close_Archive(&archive);
}

void reset_Game(Game_Data* game_Data) {
	*game_Data = {};
	game_Data->terrain_Height_Map = create_Height_Map("images/collision_Terrain_1.png");
	spawn_Player_Castle(
		SSS_CASTLE_1,
		game_Data,
		{ (RESOLUTION_WIDTH * 0.05f) , get_Height_Map_Pos_Y(game_Data, (int)((RESOLUTION_WIDTH * 0.05f))) + 25.0f },
		LEVEL_1
	);
	spawn_Enemy_Castle(
		SSS_CASTLE_1,
		game_Data,
		{ (RESOLUTION_WIDTH * 0.95f) , get_Height_Map_Pos_Y(game_Data, (int)((RESOLUTION_WIDTH * 0.95f))) + 25.0f },
		LEVEL_1
	);
}

Cache_Data create_Cache_Data(std::unordered_map<std::string, Game_Data>& cache) {
	//                    Loaded | Cache
	Cache_Data result = { false,   cache };
	return result;
}

void load_Game_Data_Cache(Cache_Data& cache_Data) {
	if (cache_Data.loaded == false) {
		for (int i = 0; i < SG_TOTAL; i++) {
			std::string current_Save_Game = create_Save_Game_File_Name((Saved_Games)(i));
			const char* ptr = current_Save_Game.c_str();
			if (check_If_File_Exists(ptr)) {
				Game_Data game_Data = {};
				// Casting to the enum seems to work like a charm
				process_Game_Data(&game_Data, (Saved_Games)i, GDO_LOAD);
				cache_Data.cache[current_Save_Game] = game_Data;
			}
		}
		cache_Data.loaded = true;
	}
}

void save_Game_To_Cache(Saved_Games save_Game, Game_Data& game_Data, Cache_Data& cache_Data) {
	process_Game_Data(&game_Data, save_Game, GDO_SAVE);
	std::string save_Game_File_Name = create_Save_Game_File_Name(save_Game);
	cache_Data.cache[save_Game_File_Name] = game_Data;
}