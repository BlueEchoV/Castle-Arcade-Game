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

void close_Archive(Archive* ar) {
	fclose(ar->file);
}

void process(Archive* ar, bool& my_Bool) {
	if (ar->operation == GDO_SAVE) {
		fwrite(&my_Bool, sizeof(my_Bool), 1, ar->file);
	}
	else if (ar->operation == GDO_LOAD) {
		fread(&my_Bool, sizeof(my_Bool), 1, ar->file);
	}
}

void process(Archive* ar, int& my_Int) {
	if (ar->operation == GDO_SAVE) {
		fwrite(&my_Int, sizeof(my_Int), 1, ar->file);
	}
	else if (ar->operation == GDO_LOAD) {
		fread(&my_Int, sizeof(my_Int), 1, ar->file);
	}
}

void process(Archive* ar, uint8_t& my_Int) {
	if (ar->operation == GDO_SAVE) {
		fwrite(&my_Int, sizeof(my_Int), 1, ar->file);
	}
	else if (ar->operation == GDO_LOAD) {
		fread(&my_Int, sizeof(my_Int), 1, ar->file);
	}
}

void process(Archive* ar, uint16_t& my_Int) {
	if (ar->operation == GDO_SAVE) {
		fwrite(&my_Int, sizeof(my_Int), 1, ar->file);
	}
	else if (ar->operation == GDO_LOAD) {
		fread(&my_Int, sizeof(my_Int), 1, ar->file);
	}
}

void process(Archive* ar, uint32_t& my_Int) {
	if (ar->operation == GDO_SAVE) {
		fwrite(&my_Int, sizeof(my_Int), 1, ar->file);
	}
	else if (ar->operation == GDO_LOAD) {
		fread(&my_Int, sizeof(my_Int), 1, ar->file);
	}
}

// Process the primitive (float, int, double)
void process(Archive* ar, float& my_Float) {
	if (ar->operation == GDO_SAVE) {
		fwrite(&my_Float, sizeof(my_Float), 1, ar->file);
	}
	else if (ar->operation == GDO_LOAD) {
		fread(&my_Float, sizeof(my_Float), 1, ar->file);
	}
}

void process(Archive* ar, Handle& handle) {
	process(ar, handle.index);
	process(ar, handle.generation);
}

void process(Archive* ar, SDL_Rect& rect) {
	process(ar, rect.x);
	process(ar, rect.y);
	process(ar, rect.w);
	process(ar, rect.h);
}

void process(Archive* ar, std::string& string) {
	if (ar->operation == GDO_SAVE) {
		size_t length = string.size();
		fwrite(&length, sizeof(length), 1, ar->file);
		fwrite(string.data(), sizeof(char), length, ar->file);
	}
	else if (ar->operation == GDO_LOAD) {
		size_t length;
		fread(&length, sizeof(length), 1, ar->file);

		string.resize(length);
		fread(string.data(), sizeof(char), length, ar->file);
	}
}

void process(Archive* ar, Sprite_Sheet_Tracker& sprite_Sheet_Tracker) {
	process(ar, sprite_Sheet_Tracker.sprite_Sheet_Name);
	process(ar, sprite_Sheet_Tracker.animation_Time);
	process(ar, sprite_Sheet_Tracker.current_Frame);
}

void process(Archive* ar, Particle& particle) {
	process(ar, particle.size);
	process(ar, particle.lifetime_Max);
	process(ar, particle.lifetime);
	process(ar, particle.fade_In);
	process(ar, particle.velocity);
	process(ar, particle.position);
}

void process(Archive* ar, Particle_System& particle_System) {
	process(ar, particle_System.rect);
	process(ar, particle_System.sprite_Sheet_Tracker);
	process(ar, particle_System.particle_Type);
	process(ar, particle_System.time_Between_Spawns);
	process(ar, particle_System.lifetime);
	process(ar, particle_System.destroyed);
	process(ar, particle_System.handle);
	process(ar, particle_System.parent);
	process(ar, particle_System.flip_Horizontally);
	process(ar, particle_System.particles);

}

void process(Archive* ar, std::vector<Particle_System>& particle_Systems) {
	if (ar->operation == GDO_SAVE) {
		size_t length = particle_Systems.size();
		fwrite(&length, sizeof(length), 1, ar->file);
		for (Particle_System& particle_System : particle_Systems) {
			process(ar, particle_System);
		}
	}
	else if (ar->operation == GDO_LOAD) {
		size_t length;
		fread(&length, sizeof(length), 1, ar->file);
		particle_Systems.resize(length);
		for (Particle_System& particle_System : particle_Systems) {
			process(ar, particle_System);
		}
	}
}

void process(Archive* ar, V2& vector) {
	process(ar, vector.x);
	process(ar, vector.y);
}

void process(Archive* ar, Collider& collider) {
	process(ar, collider.position_LS);
	process(ar, collider.radius);
}

void process(Archive* ar, Rigid_Body& rigid_Body) {
	process(ar, rigid_Body.rigid_Body_Faces_Velocity);
	process(ar, rigid_Body.position_WS);
	process(ar, rigid_Body.velocity);
	process(ar, rigid_Body.angle);
	process(ar, rigid_Body.num_Colliders);
	process(ar, rigid_Body.colliders);
}

void process(Archive* ar, Health_Bar& health_Bar) {
	process(ar, health_Bar.max_HP);
	process(ar, health_Bar.current_HP);
	process(ar, health_Bar.width);
	process(ar, health_Bar.height);
	process(ar, health_Bar.y_Offset);
	process(ar, health_Bar.thickness);
}

void process(Archive* ar, Cooldown& cooldown) {
	process(ar, cooldown.duration);
	process(ar, cooldown.remaining);
}

void process(Archive* ar, Unit_Level_Tracker& storage_Type) {
	process(ar, storage_Type.warrior);
	process(ar, storage_Type.archer);
	process(ar, storage_Type.necromancer);
}

void process(Archive* ar, Castle& castle) {
	process(ar, castle.sprite_Sheet_Tracker);
	process(ar, castle.rigid_Body);
	process(ar, castle.health_Bar);

	process(ar, castle.fire_Cooldown);
	process(ar, castle.spawn_Cooldown);

	process(ar, castle.arrow_Ammo);
	process(ar, castle.arrow_Ammo_Cooldown);

	process(ar, castle.unit_Level_Tracker);
}

void process(Archive* ar, Attached_Entity& attached_Entity) {
	process(ar, attached_Entity.sprite_Sheet_Tracker);
	process(ar, attached_Entity.angle);
	process(ar, attached_Entity.offset);
}

void process(Archive* ar, Unit& unit) {
	process(ar, unit.sprite_Sheet_Tracker);
	process(ar, unit.rigid_Body);
	process(ar, unit.health_Bar);

	process(ar, unit.speed);
	process(ar, unit.damage);
	process(ar, unit.attack_Cooldown);
	process(ar, unit.current_Attack_Cooldown);
	process(ar, unit.attack_Range);
	process(ar, unit.projectile_Type);
	process(ar, unit.fire_Projectiles);

	process(ar, unit.attached_Entities);
	process(ar, unit.attached_Entities_Size);

	process(ar, unit.destroyed);
	process(ar, unit.stop);

	process(ar, unit.handle);
}

void process(Archive* ar, Projectile& projectile) {
	process(ar, projectile.type);
	process(ar, projectile.sprite_Sheet_Tracker);

	process(ar, projectile.rigid_Body);

	process(ar, projectile.damage);
	process(ar, projectile.speed);
	process(ar, projectile.life_Time);
	process(ar, projectile.current_Penetrations);
	process(ar, projectile.penetrated_Enemy_IDS_Size);
	process(ar, projectile.penetrated_Enemy_IDS);

	process(ar, projectile.handle);
	process(ar, projectile.parent);

	process(ar, projectile.can_Attach);
	process(ar, projectile.gravity);
	process(ar, projectile.stop);
	process(ar, projectile.destroyed);
}

void process(Archive* ar, Generation& gen) {
	process(ar, gen.slot_Taken);
	process(ar, gen.generation);
}

void process(Archive* ar, Storage_Type& storage_Type) {
	int temp;
	if (ar->operation == GDO_SAVE) {
		temp = (int)storage_Type;
		fwrite(&temp, sizeof(temp), 1, ar->file);
	}
	else if (ar->operation == GDO_LOAD) {
		fread(&temp, sizeof(temp), 1, ar->file);
		storage_Type = (Storage_Type)temp;
	}
}

void process(Archive* ar, Game_Data* game_Data) {
	process(ar, game_Data->units);
	process(ar, game_Data->projectiles);

	process(ar, game_Data->player_Castle);
	process(ar, game_Data->player_Unit_IDS);
	process(ar, game_Data->player_Proj_IDS);

	process(ar, game_Data->enemy_Castle);
	process(ar, game_Data->enemy_Unit_IDS);
	process(ar, game_Data->enemy_Proj_IDS);

	process(ar, game_Data->particle_Systems);
	process(ar, game_Data->particle_System_IDS);

	process(ar, game_Data->terrain_Height_Map);
	process(ar, game_Data->timer);
}

// Call load game function and save game function that calls process game data
void load_Game(Game_Data& game_Data, Saved_Games save_Game) {
	std::string file_Name = create_Save_Game_File_Name(save_Game);
	Archive ar = create_Archive(file_Name, GDO_LOAD);
	if (ar.file != NULL) {
		process(&ar, &game_Data);
		close_Archive(&ar);
	}
}

void save_Game(Game_Data& game_Data, Saved_Games save_Game) {
	std::string file_Name = create_Save_Game_File_Name(save_Game);
	Archive ar = create_Archive(file_Name, GDO_SAVE);
	if (ar.file != NULL) {
		process(&ar, &game_Data);
		close_Archive(&ar);
	}
}

void start_Game(Game_Data& game_Data) {
	game_Data.terrain_Height_Map = create_Height_Map("images/collision_Terrain_1.png");
	spawn_Player_Castle(
		game_Data,
		{ (RESOLUTION_WIDTH * 0.05f) , get_Height_Map_Pos_Y(game_Data, (int)((RESOLUTION_WIDTH * 0.05f))) + 25.0f },
		LEVEL_1
	);
	add_Summonable_Unit_To_Castle(game_Data, N_PLAYER, "warrior");
	add_Summonable_Unit_To_Castle(game_Data, N_PLAYER, "archer");
	add_Summonable_Unit_To_Castle(game_Data, N_PLAYER, "necromancer");

	spawn_Enemy_Castle(
		game_Data,
		{ (RESOLUTION_WIDTH * 0.95f) , get_Height_Map_Pos_Y(game_Data, (int)((RESOLUTION_WIDTH * 0.95f))) + 25.0f },
		LEVEL_1
	);
	Handle test_Handle = {};
	spawn_Particle_System(
		game_Data,
		"PT_RAIN",
		{ RESOLUTION_WIDTH / 2, -50 },
		1000,
		RESOLUTION_WIDTH,
		50,
		test_Handle,
		false
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
				load_Game(game_Data, (Saved_Games)(i));
				cache_Data.cache[current_Save_Game] = game_Data;
			}
		}
		cache_Data.loaded = true;
	}
}

void save_Game_To_Cache(Saved_Games save_Game_enum, Game_Data& game_Data, Cache_Data& cache_Data) {
	save_Game(game_Data, save_Game_enum);
	std::string save_Game_File_Name = create_Save_Game_File_Name(save_Game_enum);
	cache_Data.cache[save_Game_File_Name] = game_Data;
}