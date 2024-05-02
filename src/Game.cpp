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

void process(bool& my_Bool, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&my_Bool, sizeof(my_Bool), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&my_Bool, sizeof(my_Bool), 1, archive->file);
	}
}

void process(int& my_Int, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&my_Int, sizeof(my_Int), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&my_Int, sizeof(my_Int), 1, archive->file);
	}
}

// Process the primitive (float, int, double)
void process(float& my_Float, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&my_Float, sizeof(my_Float), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&my_Float, sizeof(my_Float), 1, archive->file);
	}
}

void process(SDL_Rect& rect, Archive* archive) {
	process(rect.x, archive);
	process(rect.y, archive);
	process(rect.w, archive);
	process(rect.h, archive);
}

void process(std::string& string, Archive* archive) {
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

void process(Sprite_Sheet_Tracker& sprite_Sheet_Tracker, Archive* archive) {
	process(sprite_Sheet_Tracker.sprite_Sheet_Name, archive);
	process(sprite_Sheet_Tracker.animation_Time, archive);
	process(sprite_Sheet_Tracker.current_Frame, archive);
}

void process(Particle& particle, Archive* archive) {
	process(particle.size, archive);
	process(particle.lifetime_Max, archive);
	process(particle.lifetime, archive);
	process(particle.fade_In, archive);
	process(particle.velocity, archive);
	process(particle.position, archive);
}

void process(Particle_System& particle_System, Archive* archive) {
	process(particle_System.rect, archive);
	process(particle_System.sprite_Sheet_Tracker, archive);
	process(particle_System.particle_Type, archive);
	process(particle_System.time_Between_Spawns, archive);
	process(particle_System.lifetime, archive);
	process(particle_System.destroyed, archive);
	process(particle_System.particles, archive);
}

void process(std::vector<Particle_System>& particle_Systems, Archive* archive) {
	if (archive->operation == GDO_SAVE) {
		size_t length = particle_Systems.size();
		fwrite(&length, sizeof(length), 1, archive->file);
		for (Particle_System& particle_System : particle_Systems) {
			process(particle_System, archive);
		}
	}
	else if (archive->operation == GDO_LOAD) {
		size_t length;
		fread(&length, sizeof(length), 1, archive->file);
		particle_Systems.resize(length);
		for (Particle_System& particle_System : particle_Systems) {
			process(particle_System, archive);
		}
	}
}

void process(V2& vector, Archive* archive) {
	process(vector.x, archive);
	process(vector.y, archive);
}

void process(Collider& collider, Archive* archive) {
	process(collider.position_LS, archive);
	process(collider.radius, archive);
}

void process(Rigid_Body& rigid_Body, Archive* archive) {
	process(rigid_Body.rigid_Body_Faces_Velocity, archive);
	process(rigid_Body.position_WS, archive);
	process(rigid_Body.velocity, archive);
	process(rigid_Body.angle, archive);
	process(rigid_Body.num_Colliders, archive);
	process(rigid_Body.colliders, archive);
}

void process(Health_Bar& health_Bar, Archive* archive) {
	process(health_Bar.max_HP, archive);
	process(health_Bar.current_HP, archive);
	process(health_Bar.width, archive);
	process(health_Bar.height, archive);
	process(health_Bar.y_Offset, archive);
	process(health_Bar.thickness, archive);
}

void process(Cooldown& cooldown, Archive* archive) {
	process(cooldown.duration, archive);
	process(cooldown.remaining, archive);
}

void process(Castle& castle, Archive* archive) {
	process(castle.sprite_Sheet_Tracker, archive);
	process(castle.rigid_Body, archive);
	process(castle.health_Bar, archive);
	process(castle.fire_Cooldown, archive);
	process(castle.spawn_Cooldown, archive);
	process(castle.arrow_Ammo, archive);
	process(castle.arrow_Ammo_Cooldown, archive);
}

void process(Attached_Entity& attached_Entity, Archive* archive) {
	process(attached_Entity.sprite_Sheet_Tracker, archive);
	process(attached_Entity.angle, archive);
	process(attached_Entity.offset, archive);
}

void process(Warrior& warrior, Archive* archive) {
	process(warrior.sprite_Sheet_Tracker, archive);
	process(warrior.rigid_Body, archive);
	process(warrior.health_Bar, archive);
	process(warrior.speed, archive);
	process(warrior.damage, archive);
	process(warrior.attack_Cooldown, archive);
	process(warrior.current_Attack_Cooldown, archive);
	process(warrior.attack_Range, archive);
	process(warrior.attached_Entities, archive);
	process(warrior.attached_Entities_Size, archive);
	process(warrior.destroyed, archive);
	process(warrior.stop, archive);
	process(warrior.ID, archive);
}

void process(Arrow_Type& arrow_Type, Archive* archive) {
	process((int&)arrow_Type, archive);
}

void process(Arrow& arrow, Archive* archive) {
	process(arrow.type, archive);
	process(arrow.sprite_Sheet_Tracker, archive);
	process(arrow.rigid_Body, archive);
	process(arrow.damage, archive);
	process(arrow.speed, archive);
	process(arrow.life_Time, archive);
	process(arrow.collision_Delay, archive);
	process(arrow.target_ID, archive);
	process(arrow.stop, archive);
	process(arrow.destroyed, archive);
}

void process(Archer& archer, Archive* archive) {
	process(archer.sprite_Sheet_Tracker, archive);
	process(archer.rigid_Body, archive);
	process(archer.health_Bar, archive);
	process(archer.speed, archive);
	process(archer.attack_Cooldown, archive);
	process(archer.current_Attack_Cooldown, archive);
	process(archer.attack_Range, archive);
	process(archer.destroyed, archive);
	process(archer.stop, archive);
}

void process(Game_Data* game_Data, Archive* archive) {
	process(game_Data->timer, archive);
	process(game_Data->player_Castle, archive);
	process(game_Data->enemy_Castle, archive);
	process(game_Data->terrain_Height_Map, archive);
	process(game_Data->player_Arrows, archive);
	process(game_Data->enemy_Warriors, archive);
	process(game_Data->player_Warriors, archive);
	process(game_Data->player_Archers, archive);
	process(game_Data->particle_Systems, archive);
	process(game_Data->next_Entity_ID, archive);
}

// Call load game function and save game function that calls process game data
void load_Game(Game_Data* game_Data, Saved_Games save_Game) {
	std::string file_Name = create_Save_Game_File_Name(save_Game);
	Archive archive = create_Archive(file_Name, GDO_LOAD);
	if (archive.file != NULL) {
		process(game_Data, &archive);
		close_Archive(&archive);
	}
}

void save_Game(Game_Data* game_Data, Saved_Games save_Game) {
	std::string file_Name = create_Save_Game_File_Name(save_Game);
	Archive archive = create_Archive(file_Name, GDO_SAVE);
	if (archive.file != NULL) {
		process(game_Data, &archive);
		close_Archive(&archive);
	}
}

void start_Game(Game_Data* game_Data) {
	*game_Data = {};
	game_Data->terrain_Height_Map = create_Height_Map("images/collision_Terrain_1.png");
	spawn_Player_Castle(
		"castle",
		game_Data,
		{ (RESOLUTION_WIDTH * 0.05f) , get_Height_Map_Pos_Y(game_Data, (int)((RESOLUTION_WIDTH * 0.05f))) + 25.0f },
		LEVEL_1
	);
	spawn_Enemy_Castle(
		"castle",
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