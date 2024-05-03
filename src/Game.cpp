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

void process(Archive* archive, bool& my_Bool) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&my_Bool, sizeof(my_Bool), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&my_Bool, sizeof(my_Bool), 1, archive->file);
	}
}

void process(Archive* archive, int& my_Int) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&my_Int, sizeof(my_Int), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&my_Int, sizeof(my_Int), 1, archive->file);
	}
}

// Process the primitive (float, int, double)
void process(Archive* archive, float& my_Float) {
	if (archive->operation == GDO_SAVE) {
		fwrite(&my_Float, sizeof(my_Float), 1, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		fread(&my_Float, sizeof(my_Float), 1, archive->file);
	}
}

void process(Archive* archive, SDL_Rect& rect) {
	process(archive, rect.x);
	process(archive, rect.y);
	process(archive, rect.w);
	process(archive, rect.h);
}

void process(Archive* archive, std::string& string) {
	if (archive->operation == GDO_SAVE) {
		size_t length = string.size();
		fwrite(&length, sizeof(length), 1, archive->file);
		fwrite(string.data(), sizeof(char), length, archive->file);
	}
	else if (archive->operation == GDO_LOAD) {
		size_t length;
		fread(&length, sizeof(length), 1, archive->file);

		string.resize(length);
		fread(string.data(), sizeof(char), length, archive->file);
	}
}

void process(Archive* archive, Sprite_Sheet_Tracker& sprite_Sheet_Tracker) {
	process(archive, sprite_Sheet_Tracker.sprite_Sheet_Name);
	process(archive, sprite_Sheet_Tracker.animation_Time);
	process(archive, sprite_Sheet_Tracker.current_Frame);
}

void process(Archive* archive, Particle& particle) {
	process(archive, particle.size);
	process(archive, particle.lifetime_Max);
	process(archive, particle.lifetime);
	process(archive, particle.fade_In);
	process(archive, particle.velocity);
	process(archive, particle.position);
}

void process(Archive* archive, Particle_System& particle_System) {
	process(archive, particle_System.rect);
	process(archive, particle_System.sprite_Sheet_Tracker);
	process(archive, particle_System.particle_Type);
	process(archive, particle_System.time_Between_Spawns);
	process(archive, particle_System.lifetime);
	process(archive, particle_System.destroyed);
	process(archive, particle_System.particles);
}

void process(Archive* archive, std::vector<Particle_System>& particle_Systems) {
	if (archive->operation == GDO_SAVE) {
		size_t length = particle_Systems.size();
		fwrite(&length, sizeof(length), 1, archive->file);
		for (Particle_System& particle_System : particle_Systems) {
			process(archive, particle_System);
		}
	}
	else if (archive->operation == GDO_LOAD) {
		size_t length;
		fread(&length, sizeof(length), 1, archive->file);
		particle_Systems.resize(length);
		for (Particle_System& particle_System : particle_Systems) {
			process(archive, particle_System);
		}
	}
}

void process(Archive* archive, V2& vector) {
	process(archive, vector.x);
	process(archive, vector.y);
}

void process(Archive* archive, Collider& collider) {
	process(archive, collider.position_LS);
	process(archive, collider.radius);
}

void process(Archive* archive, Rigid_Body& rigid_Body) {
	process(archive, rigid_Body.rigid_Body_Faces_Velocity);
	process(archive, rigid_Body.position_WS);
	process(archive, rigid_Body.velocity);
	process(archive, rigid_Body.angle);
	process(archive, rigid_Body.num_Colliders);
	process(archive, rigid_Body.colliders);
}

void process(Archive* archive, Health_Bar& health_Bar) {
	process(archive, health_Bar.max_HP);
	process(archive, health_Bar.current_HP);
	process(archive, health_Bar.width);
	process(archive, health_Bar.height);
	process(archive, health_Bar.y_Offset);
	process(archive, health_Bar.thickness);
}

void process(Archive* archive, Cooldown& cooldown) {
	process(archive, cooldown.duration);
	process(archive, cooldown.remaining);
}

void process(Archive* archive, Castle& castle) {
	process(archive, castle.sprite_Sheet_Tracker);
	process(archive, castle.rigid_Body);
	process(archive, castle.health_Bar);
	process(archive, castle.fire_Cooldown);
	process(archive, castle.spawn_Cooldown);
	process(archive, castle.arrow_Ammo);
	process(archive, castle.arrow_Ammo_Cooldown);
}

void process(Archive* archive, Attached_Entity& attached_Entity) {
	process(archive, attached_Entity.sprite_Sheet_Tracker);
	process(archive, attached_Entity.angle);
	process(archive, attached_Entity.offset);
}

void process(Archive* archive, Warrior& warrior) {
	process(archive, warrior.sprite_Sheet_Tracker);
	process(archive, warrior.rigid_Body);
	process(archive, warrior.health_Bar);
	process(archive, warrior.speed);
	process(archive, warrior.damage);
	process(archive, warrior.attack_Cooldown);
	process(archive, warrior.current_Attack_Cooldown);
	process(archive, warrior.attack_Range);
	process(archive, warrior.attached_Entities);
	process(archive, warrior.attached_Entities_Size);
	process(archive, warrior.destroyed);
	process(archive, warrior.stop);
	process(archive, warrior.ID);
}

void process(Archive* archive, Arrow_Type& arrow_Type) {
	process(archive, (int&)arrow_Type);
}

void process(Archive* archive, Arrow& arrow) {
	process(archive, arrow.type);
	process(archive, arrow.sprite_Sheet_Tracker);
	process(archive, arrow.rigid_Body);
	process(archive, arrow.damage);
	process(archive, arrow.speed);
	process(archive, arrow.life_Time);
	process(archive, arrow.collision_Delay);
	process(archive, arrow.target_ID);
	process(archive, arrow.stop);
	process(archive, arrow.destroyed);
}

void process(Archive* archive, Archer& archer) {
	process(archive, archer.sprite_Sheet_Tracker);
	process(archive, archer.rigid_Body);
	process(archive, archer.health_Bar);
	process(archive, archer.speed);
	process(archive, archer.attack_Cooldown);
	process(archive, archer.current_Attack_Cooldown);
	process(archive, archer.attack_Range);
	process(archive, archer.destroyed);
	process(archive, archer.stop);
}

void process(Archive* archive, Game_Data* game_Data) {
	process(archive, game_Data->timer);
	process(archive, game_Data->player_Castle);
	process(archive, game_Data->enemy_Castle);
	process(archive, game_Data->terrain_Height_Map);
	process(archive, game_Data->player_Arrows);
	process(archive, game_Data->enemy_Warriors);
	process(archive, game_Data->player_Warriors);
	process(archive, game_Data->player_Archers);
	process(archive, game_Data->particle_Systems);
	process(archive, game_Data->next_Entity_ID);
}

// Call load game function and save game function that calls process game data
void load_Game(Game_Data* game_Data, Saved_Games save_Game) {
	std::string file_Name = create_Save_Game_File_Name(save_Game);
	Archive archive = create_Archive(file_Name, GDO_LOAD);
	if (archive.file != NULL) {
		process(&archive, game_Data);
		close_Archive(&archive);
	}
}

void save_Game(Game_Data* game_Data, Saved_Games save_Game) {
	std::string file_Name = create_Save_Game_File_Name(save_Game);
	Archive archive = create_Archive(file_Name, GDO_SAVE);
	if (archive.file != NULL) {
		process(&archive, game_Data);
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