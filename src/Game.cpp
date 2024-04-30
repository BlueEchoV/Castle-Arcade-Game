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

	const char* file_Name_PTR = file_Name.c_str();
	errno_t err = {};
	if (operation == GDO_SAVE) {
		err = fopen_s(&result.file, file_Name_PTR, "wb");
	}
	else if (operation == GDO_LOAD) {
		err = fopen_s(&result.file, file_Name_PTR, "rb");
	}
	if (err != 0 || !result.file) {
		SDL_Log("ERROR: Unable to open file %s in save_Game", file_Name_PTR);
	}

	return result;
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

void process_SDL_Rect(SDL_Rect& rect, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&rect, sizeof(rect), 1, archive->file);
	} else if (archive->operation == GDO_LOAD) {
		fread(&rect, sizeof(rect), 1, archive->file);
	}
}

void process_Bool(bool& my_Bool, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&my_Bool, sizeof(my_Bool), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&my_Bool, sizeof(my_Bool), 1, archive->file);
	}
}

void process_String(std::string& string, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		size_t length = string.size();
		fwrite(&length, sizeof(length), 1, archive->file);
		fwrite(string.data(), sizeof(char), length, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		size_t length;
		fread(&length, sizeof(length), 1, archive->file);

		std::vector<char> buffer(length);
		fread(buffer.data(), sizeof(char), length, archive->file);

		// The whole vector
		string.assign(buffer.begin(), buffer.end());
	}
}

void process_Particle_System(Particle_System& particle_System, Archive* archive) {
	process_SDL_Rect(particle_System.rect, archive);
	process_Struct(particle_System.sprite_Sheet_Tracker, archive);
	process_String(particle_System.particle_Type, archive);
	process_Float(particle_System.time_Between_Spawns, archive);
	process_Float(particle_System.lifetime, archive);
	process_Bool(particle_System.destroyed, archive);
	process_Int(particle_System.target_ID, archive);
	process_Vector(particle_System.particles, archive);
}

void process_Particle_Systems_Vector(std::vector<Particle_System>& particle_Systems, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		size_t length = particle_Systems.size();
		fwrite(&length, sizeof(length), 1, archive->file);
		for (Particle_System& particle_System : particle_Systems) {
			process_Particle_System(particle_System, archive);
		}
	}
	else if (archive->operation == GDO_LOAD) {
		size_t length;
		fread(&length, sizeof(length), 1, archive->file);
		particle_Systems.resize(length);
		for (Particle_System& particle_System : particle_Systems) {
			process_Particle_System(particle_System, archive);
		}
	}
}

void process_Game_Data(Game_Data* game_Data, Archive* archive) {
	process_Float(game_Data->timer, archive);
	process_Castle(game_Data->player_Castle, archive);
	process_Castle(game_Data->enemy_Castle, archive);
	process_Vector(game_Data->terrain_Height_Map, archive);
	process_Vector(game_Data->player_Arrows, archive);
	process_Vector(game_Data->enemy_Warriors, archive);
	process_Vector(game_Data->player_Warriors, archive);
	process_Vector(game_Data->player_Archers, archive);
	process_Particle_Systems_Vector(game_Data->particle_Systems, archive);
	process_Int(game_Data->next_Entity_ID, archive);
}

// Call load game function and save game function that calls process game data
void load_Game(Game_Data* game_Data, Saved_Games save_Game) {
	std::string file_Name = create_Save_Game_File_Name(save_Game);
	Archive archive = create_Archive(file_Name, GDO_LOAD);
	if (archive.file != NULL) {
		process_Game_Data(game_Data, &archive);
		close_Archive(&archive);
	}
}

void save_Game(Game_Data* game_Data, Saved_Games save_Game) {
	std::string file_Name = create_Save_Game_File_Name(save_Game);
	Archive archive = create_Archive(file_Name, GDO_SAVE);
	if (archive.file != NULL) {
		process_Game_Data(game_Data, &archive);
		close_Archive(&archive);
	}
}

void start_Game(Game_Data* game_Data) {
	*game_Data = {};
	game_Data->terrain_Height_Map = create_Height_Map("images/collision_Terrain_1.png");
	spawn_Player_Castle(
		SSS_Castle,
		game_Data,
		{ (RESOLUTION_WIDTH * 0.05f) , get_Height_Map_Pos_Y(game_Data, (int)((RESOLUTION_WIDTH * 0.05f))) + 25.0f },
		LEVEL_1
	);
	spawn_Enemy_Castle(
		SSS_Castle,
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
	Game_Data game_Data;
	if (cache_Data.loaded == false) {
		for (int i = 0; i < SG_TOTAL; i++) {
			std::string current_Save_Game = create_Save_Game_File_Name((Saved_Games)(i));
			const char* ptr = current_Save_Game.c_str();
			if (check_If_File_Exists(ptr)) {
				// Casting to the enum seems to work like a charm
				load_Game(&game_Data, (Saved_Games)(i));
				cache_Data.cache[current_Save_Game] = game_Data;
				int j = 0;
				j++;
			}
		}
		cache_Data.loaded = true;
	}
}

void save_Game_To_Cache(Saved_Games save_Game_enum, Game_Data& game_Data, Cache_Data& cache_Data) {
	save_Game(&game_Data, save_Game_enum);
	std::string save_Game_File_Name = create_Save_Game_File_Name(save_Game_enum);
	cache_Data.cache[save_Game_File_Name] = game_Data;
}