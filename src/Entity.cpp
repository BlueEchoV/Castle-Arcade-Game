#include "Entity.h"
#include <assert.h>

static std::unordered_map<std::string, Unit_Data> unit_Data_Map = {};

const Unit_Data bad_Unit_Data = {
	//	Type		food_Cost	sprite_Sheet	proj_Type	spell_Type, max_HP	hp_Multi	damage	damage_Mult speed	attack_Cooldown	ac_Multi	attack_Range
	   "warrior",	25,			"warrior_Stop", "",			"",			100,	1.0f,		25,		1.0f,		50,		1.0f,			1.0f,		150
};

const Unit_Data& get_Unit_Data(std::string key) {
	auto it = unit_Data_Map.find(key);
	if (it != unit_Data_Map.end()) {
		// Key found
		return it->second;
	} 

	assert(false);
	// Return garbage values
	return bad_Unit_Data;
}

std::vector<std::string> get_Unit_Types() {
	std::vector<std::string> result = {};
	for (const auto& iterator : unit_Data_Map) {
		result.push_back(iterator.first);
	}
	return result;
}

static std::unordered_map<std::string, Projectile_Data> projectile_Data_Map = {};

const Projectile_Data bad_Projectile_Data = {
	//	Type			sprite_Sheet	collider	can_Attach		gravity_Multiplier	speed	life_Time
	   "arrow_Short",	"arrow_Short",	"arrow",	1,				1,					500,	10,       
};

const Projectile_Data& get_Projectile_Data(std::string key) {
	auto it = projectile_Data_Map.find(key);
	if (it != projectile_Data_Map.end()) {
		// Key found
		return it->second;
	}

	assert(false);
	// Return garbage values
	return bad_Projectile_Data;
}

static std::unordered_map<std::string, Spell_Data> spell_Data_Map = {};

const Spell_Data bad_Spell_Data = {
	//	Type			sprite_Sheet
	   "raise_Dead",	"warrior"
};

const Spell_Data& get_Spell_Data(std::string key) {
	auto it = spell_Data_Map.find(key);
	if (it != spell_Data_Map.end()) {
		// Key found
		return it->second;
	}

	assert(false);
	// Return garbage values
	return bad_Spell_Data;
}

static std::unordered_map<std::string, Castle_Data> castle_Data_Map = {};

const Castle_Data bad_Castle_Data = {
	// type			sprite_Sheet_Name	projectile	  enhancement	 base_HP	 base_HP_Regen_Per_Sec	base_Food_Points	base_Food_Points_Per_Sec FP_Multi
	  "infernal",  "infernal_Castle",	"arrow_Short", "",			 100,		 5,						100,				5,					     1.1f
};

const Castle_Data& get_Castle_Data(std::string key) {
	auto it = castle_Data_Map.find(key);
	if (it != castle_Data_Map.end()) {
		// Key found
		return it->second;
	}

	assert(false);
	// Return garbage values
	return bad_Castle_Data;
}

int get_Castle_Data_Size() {
	return (int)castle_Data_Map.size();
}


Unit* get_Unit(Storage<Unit>& storage, Handle handle) {
	return get_Entity(storage, handle);
}

Spell* get_Spell(Storage<Spell>& storage, Handle handle) {
	return get_Entity(storage, handle);
}

Projectile* get_Projectile(Storage<Projectile>& storage, Handle handle) {
	return get_Entity(storage, handle);
}

Particle_System* get_Particle_System(Storage<Particle_System>& storage, Handle handle) {
	return get_Entity(storage, handle);
}

bool compare_Handles(Handle handle_1, Handle handle_2) {
	if (handle_1.index == handle_2.index &&
		handle_1.generation == handle_2.generation &&
		handle_1.storage_Type == handle_2.storage_Type &&
		handle_1.generation != 0) {
		return true;
	}
	return false;
}

void delete_Expired_Entity_Handles(Game_Data& game_Data) {
	std::erase_if(game_Data.player_Unit_IDS, [&game_Data](const Handle& player_Unit_ID) {
		Unit* unit = get_Unit(game_Data.units, player_Unit_ID);
		if (unit == nullptr) {
			return true;
		}
		if (unit->destroyed || unit->health_Bar.current_Resource <= 0) {
			delete_Handle(game_Data.units, unit->handle);
			*unit = {};
			// Remove the handle from player_Unit_IDS
			return true;
		}
		// Keep the handle in player_Unit_IDS
		return false;
		});
	std::erase_if(game_Data.player_Proj_IDS, [&game_Data](const Handle& player_Proj_ID) {
		Projectile* projectile = get_Projectile(game_Data.projectiles, player_Proj_ID);
		if (projectile == nullptr) {
			return true;
		}
		if (projectile->destroyed || projectile->life_Time <= 0) {
			delete_Handle(game_Data.projectiles, projectile->handle);
			*projectile = {};
			return true;
		}
		return false;
		});
	std::erase_if(game_Data.enemy_Unit_IDS, [&game_Data](const Handle& enemy_Unit_ID) {
		Unit* unit = get_Unit(game_Data.units, enemy_Unit_ID);
		if (unit == nullptr) {
			return true;
		}
		if (unit->destroyed || unit->health_Bar.current_Resource <= 0) {
			delete_Handle(game_Data.units, unit->handle);
			*unit = {};
			return true;
		}
		return false;
		});
	std::erase_if(game_Data.enemy_Proj_IDS, [&game_Data](const Handle& enemy_Proj_ID) {
		Projectile* projectile = get_Projectile(game_Data.projectiles, enemy_Proj_ID);
		if (projectile == nullptr) {
			return true;
		}
		if (projectile->destroyed || projectile->life_Time <= 0) {
			delete_Handle(game_Data.projectiles, projectile->handle);
			*projectile = {};
			return true;
		}
		return false;
		});
	std::erase_if(game_Data.particle_System_IDS, [&game_Data](const Handle& particle_System_ID) {
		Particle_System* particle_System = get_Particle_System(game_Data.particle_Systems, particle_System_ID);
		if (particle_System == nullptr) {
			return true;
		}
		if (particle_System->destroyed && particle_System->particles.size() == 0) {
			delete_Handle(game_Data.particle_Systems, particle_System->handle);
			*particle_System = {};
			return true;
		}
		return false;
		});
}

void add_Collider(Rigid_Body* rigid_Body, V2 position_LS, float radius) {	
	// assert(rigid_Body->num_Colliders < Globals::MAX_COLLIDERS);
	Collider* collider = &rigid_Body->colliders[rigid_Body->num_Colliders++];
	collider->position_LS = position_LS;
	collider->radius = radius;
}

void progress_CD(Cooldown& cd, float delta_Time) {
	cd.remaining -= delta_Time;
	if (cd.remaining <= 0.0f) {
		cd.remaining = cd.duration;
	}
}

void draw_Castle(Castle* castle, bool flip) {
	REF(flip);
	draw_Resource_Bar(castle->health_Bar, castle->rigid_Body.position_WS);
	draw_Resource_Bar(castle->food_Bar, castle->rigid_Body.position_WS);
	SDL_Rect temp = {};
	Sprite_Sheet* sprite_Sheet = &get_Sprite_Sheet(castle->sprite_Sheet_Tracker.sprite_Sheet_Name);
	Sprite* sprite = &sprite_Sheet->sprites[0];
	SDL_Rect* src_Rect = &sprite->source_Rect;
	V2 sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
	sprite_Half_Size = sprite_Half_Size / 2;
	temp = {
		((int)castle->rigid_Body.position_WS.x - (int)sprite_Half_Size.x),
		((int)castle->rigid_Body.position_WS.y - (int)sprite_Half_Size.y),
		sprite->source_Rect.w,
		sprite->source_Rect.h
	};
	MP_RenderCopy(Globals::renderer, sprite->image.texture, NULL, &temp);
}

void draw_Projectile(Projectile* projectile, bool flip) {
	REF(flip);
	Sprite_Sheet* sprite_Sheet = &get_Sprite_Sheet(projectile->sprite_Sheet_Tracker.sprite_Sheet_Name);
	Sprite* sprite = &sprite_Sheet->sprites[0];
	SDL_Rect* src_Rect = &sprite->source_Rect;
	V2 sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
	sprite_Half_Size = sprite_Half_Size / 2;
	SDL_Rect temp = {
		((int)projectile->rigid_Body.position_WS.x - (int)sprite_Half_Size.x),
		((int)projectile->rigid_Body.position_WS.y - (int)sprite_Half_Size.y),
		sprite->source_Rect.w,
		sprite->source_Rect.h
	};
	MP_RenderCopy(Globals::renderer, sprite->image.texture, NULL, &temp);
}

Attached_Entity return_Attached_Entity(std::string sprite_Sheet_Name, float angle, V2 offset) {
	Attached_Entity result = {};

	//                 selected     animation_Time      current_Frame
	result.sprite_Sheet_Tracker = { sprite_Sheet_Name,    0.0f,               0 };
	result.angle = angle;
	result.offset = offset;

	return result;
}

void add_Summonable_Unit_To_Castle(Game_Data& game_Data, Nation nation, std::string unit_Name) {
	Summonable_Unit summonable_Unit;

	summonable_Unit.is_Pressed = false;
	summonable_Unit.level = 1;
	summonable_Unit.name = unit_Name;
	summonable_Unit.nation = nation;
	Unit_Data data = get_Unit_Data(unit_Name);
	summonable_Unit.food_Cost = data.food_Cost;

	if (summonable_Unit.nation == N_PLAYER) {
		game_Data.player_Castle.summonable_Units.push_back(summonable_Unit);
	} else if (summonable_Unit.nation == N_ENEMY) {
		game_Data.enemy_Castle.summonable_Units.push_back(summonable_Unit);
	}
}

void draw_Attached_Entity(Attached_Entity* attached_Entity, V2 position_WS, bool flip) {
	REF(flip);
	SDL_Rect temp = {};
	Sprite_Sheet* sprite_Sheet = &get_Sprite_Sheet(attached_Entity->sprite_Sheet_Tracker.sprite_Sheet_Name);
	Sprite* sprite = &sprite_Sheet->sprites[0];
	SDL_Rect* src_Rect = &sprite->source_Rect;
	V2 sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
	sprite_Half_Size = sprite_Half_Size / 2;
	V2 new_Pos = position_WS + attached_Entity->offset;
	temp = {
		((int)new_Pos.x - (int)sprite_Half_Size.x),
		((int)new_Pos.y - (int)sprite_Half_Size.y),
		sprite->source_Rect.w,
		sprite->source_Rect.h
	};
	MP_RenderCopy(Globals::renderer, sprite->image.texture, NULL, &temp);
}

void update_Animation(Sprite_Sheet_Tracker* tracker, float unit_Speed, float delta_Time) {
	if (unit_Speed > 0) {
		float frames_Per_Second = unit_Speed / 25;
		float conversion_Frames = 1 / frames_Per_Second;
		// It's more precise to record a start time and subtract from
		// the new frames value
		tracker->animation_Time += delta_Time;

		// I can overshoot with delta_Time and it won't be 100% accurate
		// Modulus with floating point captures that overshot value
		if (tracker->animation_Time >= conversion_Frames) {
			tracker->current_Frame++;
			tracker->animation_Time = 0;
			if (tracker->current_Frame >= get_Sprite_Sheet(tracker->sprite_Sheet_Name).sprites.size()) {
				tracker->current_Frame = 0;
			}
		}
	}
}

//void update_Castle_Variables(float delta_Time) {
//
//}

// Flickering when speed changes
// Same animation frame
// NEED MORE VARIABLE TO TRACK
// Current frame time
// Frame duration
// Last frame update time
// is playing
void draw_Unit_Animated(Rigid_Body* rigid_Body, Sprite_Sheet_Tracker* tracker, bool flip) {
	REF(flip);
	Uint32 sprite_Frame = tracker->current_Frame;

	Sprite_Sheet* sprite_Sheet = &get_Sprite_Sheet(tracker->sprite_Sheet_Name);
	SDL_Rect current_Frame_Rect = sprite_Sheet->sprites[sprite_Frame].source_Rect;

	const SDL_Rect* src_Rect = &sprite_Sheet->sprites[0].source_Rect;
	V2 sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
	sprite_Half_Size = sprite_Half_Size / 2;

	SDL_Rect destination_Rect = {
		(int)(rigid_Body->position_WS.x - (int)sprite_Half_Size.x),
		(int)(rigid_Body->position_WS.y - (int)sprite_Half_Size.y),
		current_Frame_Rect.w,
		current_Frame_Rect.h
	};

	// V2 pivot = calculate_Center(
	// 	(float)sprite_Sheet->sprites[sprite_Frame].source_Rect.w,
	// 	(float)sprite_Sheet->sprites[sprite_Frame].source_Rect.h
	// );

	// // Set the center of the rotation
	// SDL_Point pivot_Point = {
	// 	(int)pivot.x,
	// 	(int)pivot.y
	// };

	// Render the current frame of the animation
	MP_RenderCopyEx(
		Globals::renderer,
		sprite_Sheet->sprites[sprite_Frame].image.texture,
		&current_Frame_Rect,
		&destination_Rect,
		rigid_Body->angle,
		NULL,
		flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
	);
}

void change_Animation(Sprite_Sheet_Tracker* tracker, std::string sprite_Sheet_Name) {
	if (tracker->sprite_Sheet_Name != sprite_Sheet_Name) {
		tracker->sprite_Sheet_Name = sprite_Sheet_Name;
		tracker->current_Frame = 0;
	}
}

V2 get_Collider_WS_Position(Rigid_Body* rigid_Body, const Collider* collider) {
	V2 result = rigid_Body->position_WS;

	float angle_In_Radians = rigid_Body->angle * (float)(M_PI / 180.0);
	if (rigid_Body->rigid_Body_Faces_Velocity) {
		angle_In_Radians = (float)atan2(rigid_Body->velocity.y, rigid_Body->velocity.x);
		rigid_Body->angle = angle_In_Radians * (float)(180.0 / M_PI);
	}

	// Rotation in 2D spaces
	float rotated_X = (float)(collider->position_LS.x * cos(angle_In_Radians) - collider->position_LS.y * sin(angle_In_Radians));
	float rotated_Y = (float)(collider->position_LS.x * sin(angle_In_Radians) + collider->position_LS.y * cos(angle_In_Radians));

	result.x += rotated_X;
	result.y += rotated_Y;

	return result;
}

void update_Unit_Position(Rigid_Body* rigid_Body, bool stop_Unit, float delta_Time) {
	if (!stop_Unit) {
		rigid_Body->position_WS.y += Globals::GRAVITY * delta_Time;
		rigid_Body->position_WS.x += (rigid_Body->velocity.x * delta_Time);
		rigid_Body->position_WS.y += (rigid_Body->velocity.y * delta_Time);
	}
}

void update_Units_Positions(Game_Data& game_Data, std::vector<Handle>& units, float delta_Time) {
	for (uint32_t i = 0; i < units.size(); i++) {
		Unit* player_Unit = get_Unit(game_Data.units, units[i]);
		if (player_Unit != nullptr) {
			if (!player_Unit->destroyed) {
				update_Unit_Position(
					&player_Unit->rigid_Body,
					player_Unit->stop,
					delta_Time
				);
			}
		}
	}
}

void update_Projectile_Position(Projectile* projectile, float delta_Time) {
	if (!projectile->stop) {
		projectile->rigid_Body.velocity.y += projectile->gravity * delta_Time;
		projectile->rigid_Body.position_WS.x += projectile->rigid_Body.velocity.x * delta_Time;
		projectile->rigid_Body.position_WS.y += projectile->rigid_Body.velocity.y * delta_Time;
	}
}

void update_Projectiles_Positions(Game_Data& game_Data, std::vector<Handle>& projectiles, float delta_Time) {
	for (uint32_t i = 0; i < projectiles.size(); i++) {
		Projectile* projectile = get_Projectile(game_Data.projectiles, projectiles[i]);
		if (projectile != nullptr) {
			if (!projectile->destroyed) {
				update_Projectile_Position(projectile, delta_Time);
			}
		}
	}
}

void update_Units_Variables(Game_Data& game_Data, std::vector<Handle>& units, float delta_Time) {
	for (uint32_t i = 0; i < units.size(); i++) {
		Unit* unit = get_Unit(game_Data.units, units[i]);
		if (unit != nullptr) {
			unit->stop = false;
			if (unit->attack_CD.remaining > 0.0f) {
				unit->attack_CD.remaining -= delta_Time;
			} else {
				unit->attack_CD.remaining = 0.0f;
			}
		}
	}
}

void update_Resource_Bar(Resource_Bar& bar, float delta_Time) {
	if (bar.current_Resource < bar.max_Resource) {
		bar.accumulated_Time += delta_Time;
		// It seems appropriate to keep this value within the scope of this function for now
		float invervals = 100;
		float inverval_Time = 1 / invervals;
		// Every second (While loop in case delta_Time is large)
		while (bar.accumulated_Time >= inverval_Time) {
			bar.current_Resource += (bar.regen / invervals);
			// Set it equal to the max if it's above. (Clamp it)
			if (bar.current_Resource > bar.max_Resource) {
				bar.current_Resource = bar.max_Resource;
			}
			bar.accumulated_Time -= inverval_Time;
		}
	}
}

void update_Units_Spell_Bars(Game_Data& game_Data, std::vector<Handle>& units, float delta_Time) {
	for (uint32_t i = 0; i < units.size(); i++) {
		Unit* unit = get_Unit(game_Data.units, units[i]);
		if (unit != nullptr) {
			if (unit->spell.can_Cast_Spell) {
				update_Resource_Bar(unit->spell_Bar, delta_Time);
			}
		}
	}
}

void update_Nation_Resource_Bars(/*Game_Data& game_Data, */Castle& castle, /*std::vector<Handle>& units,*/ float delta_Time) {
	update_Resource_Bar(castle.food_Bar, delta_Time);
	// For if I wanted to add unit regen 
	//for (Handle handle : units) {
	//	Unit* unit = get_Unit(game_Data.units, handle);
	//	update_Resource_Bar(unit->health_Bar, delta_Time);
	//}
}

void check_Projectiles_Collisions(Game_Data& game_Data, std::vector<Handle>& projectiles, Castle& target_Castle, std::vector<Handle>& target_Units, float delta_Time) {
	// Projectile Collision
	for (uint32_t i = 0; i < projectiles.size(); i++) 
	{
		Projectile* projectile = get_Projectile(game_Data.projectiles, projectiles[i]);
		if (projectile != nullptr) 
		{
			Castle* castle = &target_Castle;
			if (check_RB_Collision(&projectile->rigid_Body, &castle->rigid_Body)) 
			{
				castle->health_Bar.current_Resource -= projectile->damage;
				projectile->destroyed = true;
			}
			// Collision with map
			if (check_Height_Map_Collision(&projectile->rigid_Body, game_Data.terrain_Height_Map))
			{
				projectile->stop = true;
				if (!projectile->can_Attach) 
				{
					projectile->destroyed = true;
				}
			}
			// Collision with Units
			for (uint32_t j = 0; j < target_Units.size(); j++)
			{
				Unit* target_Unit = get_Unit(game_Data.units, target_Units[j]);
				if (target_Unit != nullptr) 
				{
					if (check_RB_Collision(&projectile->rigid_Body, &target_Unit->rigid_Body)) 
					{
						if (!projectile->stop) 
						{
							if (projectile->attached_Entity_Delay.remaining <= 0.0)
							{
								bool target_Already_Hit = false;
								for (int e = 0; e < projectile->penetrated_Enemy_IDS_Size; e++)
								{
									if (compare_Handles(projectile->penetrated_Enemy_IDS[e], target_Unit->handle))
									{
										target_Already_Hit = true;
									}
								}
								if (!target_Already_Hit && projectile->current_Penetrations >= 0)
								{
									projectile->current_Penetrations--;
									// Guard against the array max size
									if (projectile->penetrated_Enemy_IDS_Size >= ARRAY_SIZE(projectile->penetrated_Enemy_IDS))
									{
										projectile->destroyed = true;
										continue;
									}
									// Store the hit enemy handle
									projectile->penetrated_Enemy_IDS[projectile->penetrated_Enemy_IDS_Size++] = target_Unit->handle;
									//spawn_Particle_System(
									//	game_Data,
									//	"PT_BLOOD",
									//	target_Unit->rigid_Body.position_WS,
									//	0.5,
									//	15,
									//	15,
									//	target_Unit->handle,
									//	false
									//);
									target_Unit->health_Bar.current_Resource -= projectile->damage;
									projectile->parent = target_Unit->handle;
									// Reset it every time we hit an enemy
									projectile->attached_Entity_Delay.remaining = projectile->attached_Entity_Delay.duration;
								}
								Unit* projectile_Parent_Unit = get_Unit(game_Data.units, projectile->parent);
								if (projectile_Parent_Unit != nullptr && 
									projectile->current_Penetrations < 0)
								{
									//float radius = get_Sprite_Radius(&projectile->sprite_Sheet_Tracker);
									// I need to store this value so it doesn't change every frame
									//float rand_Num = ((float)(rand() % 100) - 50.0f);
									//float rand_Enemy_X = enemy_Unit_Second_Check->rigid_Body.position_WS.x + rand_Num;
									if (projectile->can_Attach && projectile_Parent_Unit->attached_Entities_Size < ARRAY_SIZE(projectile_Parent_Unit->attached_Entities))
									{
										//if ((projectile->rigid_Body.position_WS.x) > projectile_Parent_Unit->rigid_Body.position_WS.x) {
										projectile->rigid_Body.position_WS.x = projectile_Parent_Unit->rigid_Body.position_WS.x;
										V2 offset = projectile->rigid_Body.position_WS - target_Unit->rigid_Body.position_WS;
										Attached_Entity attached_Entity = return_Attached_Entity(
											projectile->type,
											projectile->rigid_Body.angle,
											offset
										);
										target_Unit->attached_Entities[target_Unit->attached_Entities_Size++] = attached_Entity;
										projectile->destroyed = true;
										//}
									}
									else {
										projectile->destroyed = true;
									}
								}
								if (projectile->current_Penetrations < 0) {
									projectile->destroyed = true;
								}
							} 
							else 
							{
								projectile->attached_Entity_Delay.remaining -= delta_Time;
							}
						}
					}
				}
			}
		}
	}
}

void check_Units_Collisions_With_Terrain(Game_Data& game_Data, std::vector<Handle>& units) {
	for (uint32_t i = 0; i < units.size(); i++) {
		Unit* unit = get_Unit(game_Data.units, units[i]);
		if (unit != nullptr) {
			if (check_Height_Map_Collision(&unit->rigid_Body, game_Data.terrain_Height_Map)) {
				float radius = get_Sprite_Radius(&unit->sprite_Sheet_Tracker);
				float pos_Y_HM = (float)game_Data.terrain_Height_Map[(int)unit->rigid_Body.position_WS.x];

				unit->rigid_Body.position_WS.y = ((RESOLUTION_HEIGHT - pos_Y_HM) - radius);
			}
		}
	}
}

void check_Units_Collisions_With_Castle(Game_Data& game_Data, std::vector<Handle> units, Castle& target_Castle) {
	for (uint32_t i = 0; i < units.size(); i++) {
		Unit* unit = get_Unit(game_Data.units, units[i]);
		if (unit != nullptr) {
			Castle* castle = &target_Castle;
			//if (check_RB_Collision(&unit->rigid_Body, &castle->rigid_Body)) {
			if (check_Attack_Range_Collision(unit->attack_Range, &unit->rigid_Body, &castle->rigid_Body)) {
				unit->stop = true;
				if (!unit->fires_Projectiles) {
					if (unit->attack_CD.remaining < 0) {
						unit->attack_CD.remaining = unit->attack_CD.duration;
						castle->health_Bar.current_Resource -= unit->damage;
					}
				} else {
					if (unit->attack_CD.remaining <= 0) {
						unit->attack_CD.remaining = unit->attack_CD.duration;
						V2 aim_Head = castle->rigid_Body.position_WS;
						aim_Head.x += get_Sprite_Radius(&castle->sprite_Sheet_Tracker);
						V2 arrow_Spawn_Location = unit->rigid_Body.position_WS;
						arrow_Spawn_Location.y -= get_Sprite_Radius(&castle->sprite_Sheet_Tracker) / 2;
						if (unit->handle.storage_Type)
						spawn_Projectile(game_Data, unit->nation, unit->projectile_Type, unit->damage, arrow_Spawn_Location, aim_Head);
					}
				}
			}
		}
	}
}

void check_Units_Collisions_With_Units(Game_Data& game_Data, std::vector<Handle> origin_Units, std::vector<Handle> target_Units) {
	for (uint32_t i = 0; i < origin_Units.size(); i++) {
		Unit* origin_Unit = get_Unit(game_Data.units, origin_Units[i]);
		// Check if the unit is a ranged unit or not
		if (origin_Unit != nullptr) {
			for (uint32_t j = 0; j < target_Units.size(); j++) {
				Unit* target_Unit = get_Unit(game_Data.units, target_Units[j]);
				if (target_Unit != nullptr) {
					if (check_Attack_Range_Collision(origin_Unit->attack_Range, &origin_Unit->rigid_Body, &target_Unit->rigid_Body)) {
						origin_Unit->stop = true;
						if (!origin_Unit->fires_Projectiles) {
							if (origin_Unit->attack_CD.remaining <= 0) {
								origin_Unit->attack_CD.remaining = origin_Unit->attack_CD.duration;
								target_Unit->health_Bar.current_Resource -= origin_Unit->damage;
							}
						}
						else {
							// change_Animation(&player_Unit->sprite_Sheet_Tracker, "archer_Stop");
							if (origin_Unit->attack_CD.remaining <= 0) {
								origin_Unit->attack_CD.remaining = origin_Unit->attack_CD.duration;
								V2 aim_Head = target_Unit->rigid_Body.position_WS;
								aim_Head.x += get_Sprite_Radius(&target_Unit->sprite_Sheet_Tracker);
								V2 arrow_Spawn_Location = origin_Unit->rigid_Body.position_WS;
								arrow_Spawn_Location.y -= get_Sprite_Radius(&target_Unit->sprite_Sheet_Tracker) / 2;
								spawn_Projectile(game_Data, origin_Unit->nation, origin_Unit->projectile_Type, origin_Unit->damage, arrow_Spawn_Location, aim_Head);
							}
							// change_Animation(&player_Unit->sprite_Sheet_Tracker, "archer_Walk");
						}
					}
					//if (target_Unit->current_Attack_Cooldown <= 0) {
					//	target_Unit->current_Attack_Cooldown = target_Unit->attack_Cooldown;
					//	origin_Unit->health_Bar.current_Resource -= target_Unit->damage;
					//}
				}
			}
		}
	}
}

Resource_Bar create_Resource_Bar(int width, int height, int y_Offset, int thickness, float resource, float regen, Resource_Bar_Color_Selector colors) {
	Resource_Bar result;

	result.width = width;
	result.height = height;
	result.y_Offset = y_Offset;
	result.thickness = thickness;
	result.max_Resource = resource;
	result.regen = regen;
	result.current_Resource = result.max_Resource;
	result.selected_Colors = colors;

	return result;
}

Rigid_Body create_Rigid_Body(V2 position_WS, bool rigid_Body_Faces_Velocity) {
	Rigid_Body result = {};

	result.rigid_Body_Faces_Velocity = rigid_Body_Faces_Velocity;
	result.position_WS = { position_WS.x , position_WS.y };

	return result;
}

std::string create_Unit_Data_Map_Key(std::string sprite_Sheet_Name) {
	std::vector<std::string> tokens = split(sprite_Sheet_Name, '_');
	return tokens[0];
}

float scale_Value(float base_Value, float multiplier, int level) {
	return base_Value + ((base_Value * (multiplier - 1)) * (level - 1));
}

float scale_Castle_Food_Points(const Castle_Data castle_Data, int level) {
	return scale_Value(castle_Data.base_Food_Points, castle_Data.food_Points_Multiplier, level);
}

// This is for values that are firing at a certain amount of times per second
Cooldown scale_Times_Per_Second_Cooldown(float max_Value, float base_Value, float multiplier, int level) {
	float times_Per_Sec = scale_Value(base_Value, multiplier, level);
	// Flip it to get the time for the associated ammo per second
	float scaled_Duration;
	if (times_Per_Sec >= max_Value) {
		scaled_Duration = 1.0f / max_Value;
	}
	else {
		scaled_Duration = 1.0f / times_Per_Sec;
	}
	Cooldown result;
	result.duration = scaled_Duration;
	result.remaining = 0.0f;
	return result;
}

Cooldown scale_Castle_Ammo_CD(const Castle_Data castle_Data, int level) {
	const Projectile_Data projectile_Data = get_Projectile_Data(castle_Data.projectile_Type);
	return scale_Times_Per_Second_Cooldown(
		projectile_Data.castle_Max_Ammo_Per_Sec,
		projectile_Data.castle_Base_Ammo_Per_Sec,
		projectile_Data.castle_Max_Ammo_Per_Sec, 
		level
	);
}

void spawn_Castle(Game_Data& game_Data, Nation nation, std::string castle_Type, int map_Power_Level) {
	const Castle_Data castle_Data = get_Castle_Data(castle_Type);
	
	Castle castle = {};
	castle.nation = nation;
	castle.level = map_Power_Level;
	castle.castle_Type = castle_Type;
	castle.projectile_Type = castle_Data.projectile_Type;
	castle.projectile_Ammo = 0;
	castle.projectile_Ammo_Cooldown = scale_Castle_Ammo_CD(castle_Data, map_Power_Level);
	castle.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(castle_Data.sprite_Sheet_Name);
	// Place the castles onto the map
	V2 position_WS = {};
	if (nation == N_PLAYER) {
		position_WS = { (RESOLUTION_WIDTH * 0.05f) , get_Height_Map_Pos_Y(game_Data, (int)((RESOLUTION_WIDTH * 0.05f))) };
	} else if (nation == N_ENEMY) {
		position_WS = { (RESOLUTION_WIDTH * 0.95f), get_Height_Map_Pos_Y(game_Data, (int)((RESOLUTION_WIDTH * 0.95f))) };
	} else {
		assert(false);
	}
	// Lower the castle onto the map by 25% of the size of the castle sprite
	float image_Radius = get_Sprite_Radius(&castle.sprite_Sheet_Tracker);
	position_WS.y -= image_Radius / 1.5f;
	castle.rigid_Body = create_Rigid_Body(position_WS, false);
	// *********************
	float food_Points_Regen;
	if (castle.nation == N_ENEMY) {
		food_Points_Regen = scale_Castle_Food_Points(castle_Data, map_Power_Level);
	} else {
		// Don't scale the player right now
		food_Points_Regen = castle_Data.base_Food_Points_Regen;
	}
	castle.health_Bar = create_Resource_Bar(90, 20, 115, 3, castle_Data.base_HP, castle_Data.base_HP_Regen, RBCS_HP_Bar);
	castle.food_Bar = create_Resource_Bar(90, 10, (115 - 20), 3, castle_Data.base_Food_Points, food_Points_Regen, RBCS_Food_Bar);
	// NOTE: This value is just the interval 
	// at which a projectiles are fired if 
	// the fire button is held down.
	castle.fire_Cooldown.duration = 0.05f;
	castle.fire_Cooldown.remaining = 0.0f;
	//  ***************************
	// NOTE: These values may get removed
	castle.spawn_Cooldown.duration = 2.0f;
	castle.spawn_Cooldown.remaining = 0.0f;
	// ****************************
	float radius = get_Sprite_Radius(&castle.sprite_Sheet_Tracker);
	add_Collider(&castle.rigid_Body, { 0.0f, -(radius / 2) }, (radius / 2));
	add_Collider(&castle.rigid_Body, { 0.0f, 0.0f }, (radius / 2));
	add_Collider(&castle.rigid_Body, { 0.0f, (radius / 2) }, (radius / 2));

	//add_Collider(&castle.rigid_Body, { 0.0f, 0.0f }, get_Sprite_Radius(&castle.sprite_Sheet_Tracker));
	if (castle.nation == N_PLAYER) {
		game_Data.player_Castle = castle;
	} else if (castle.nation == N_ENEMY) {
		game_Data.enemy_Castle = castle;
	}
}

// The damage of the projectile is based off the unit's damage
void spawn_Projectile(Game_Data& game_Data, Nation unit_Side, std::string projectile_Type, float damage, V2 origin_Pos, V2 target_Pos) {
	Projectile projectile = {};
	projectile.attached_Entity_Delay.duration = 0.1f;
	projectile.attached_Entity_Delay.remaining = 0.0f;

	Projectile_Data projectile_Data = get_Projectile_Data(projectile_Type);
	projectile.type = projectile_Data.type;

	projectile.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(projectile_Data.type);
	projectile.rigid_Body = create_Rigid_Body(origin_Pos, true);

	projectile.damage = damage;
	projectile.speed = projectile_Data.speed;
	projectile.life_Time = projectile_Data.life_Time;
	// 90000 is a pretty garbage number. I may want to come up with a better means of measuring speed
	//projectile.collision_Delay.duration = 0.02f;
	//projectile.collision_Delay.remaining = projectile.collision_Delay.duration;
	projectile.current_Penetrations = projectile_Data.max_Penetrations;

	projectile.stop = false;
	projectile.destroyed = false;

	V2 direction_V2 = calculate_Direction_V2(target_Pos, origin_Pos);

	projectile.rigid_Body.velocity.x = direction_V2.x * projectile.speed;
	projectile.rigid_Body.velocity.y = direction_V2.y * projectile.speed;
	projectile.gravity = Globals::GRAVITY * projectile_Data.gravity_Multiplier;
	projectile.can_Attach = projectile_Data.can_Attach;

	add_Collider(&projectile.rigid_Body, { projectile_Data.collider_Pos_LS_X, projectile_Data.collider_Pos_LS_Y }, projectile_Data.collider_Radius);
	// add_Collider(&unit.rigid_Body, { 0.0f, -(radius / 2) }, (radius / 2));
	if (unit_Side == N_PLAYER) {
		projectile.handle = create_Handle(game_Data.projectiles);
		game_Data.projectiles.arr[projectile.handle.index] = projectile;

		game_Data.player_Proj_IDS.push_back(projectile.handle);
	}
	else if (unit_Side == N_ENEMY) {
		projectile.handle = create_Handle(game_Data.projectiles);
		game_Data.projectiles.arr[projectile.handle.index] = projectile;

		game_Data.enemy_Proj_IDS.push_back(projectile.handle);
	}
}

float scale_Unit_HP(const Unit_Data unit_Data, int unit_Level) {
	return scale_Value(unit_Data.base_HP, unit_Data.hp_Multiplier, unit_Level);
}

float scale_Unit_Damage(const Unit_Data unit_Data, int unit_Level) {
	return scale_Value(unit_Data.base_Damage, unit_Data.damage_Multiplier, unit_Level);
}

Cooldown scale_Unit_Attack_CD(const Unit_Data unit_Data, int unit_Level) {
	return scale_Times_Per_Second_Cooldown(
		unit_Data.max_Attacks_Per_Second,
		unit_Data.base_Attacks_Per_Second,
		unit_Data.attacks_Per_Second_Multiplier,
		unit_Level
	);
}

void spawn_Unit(Game_Data& game_Data, Nation unit_Side, std::string unit_Type, int level, V2 spawn_Position, V2 target_Position) {
	Unit unit = {};
	unit.level = level;
	unit.nation = unit_Side;

	Unit_Data unit_Data = get_Unit_Data(unit_Type);
	unit.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(unit_Data.sprite_Sheet_Name);
	unit.rigid_Body = create_Rigid_Body(spawn_Position, false);
	
	unit.speed = unit_Data.speed;

	// ***Level based ***
	float scaled_HP = scale_Unit_HP(unit_Data, level);
	unit.health_Bar = create_Resource_Bar(50, 13, 60, 2, scaled_HP, 0, RBCS_HP_Bar);

	unit.spell.type= unit_Data.spell_Type;
	if (unit.spell.type != "") {
		unit.spell.can_Cast_Spell = true;
		Spell_Data spell_Data = get_Spell_Data(unit.spell.type);
		// So the spell doesn't cast instantly
		unit.spell.time_To_Cast.remaining = spell_Data.base_Cast_Time;
		unit.spell.time_To_Cast.duration = spell_Data.base_Cast_Time;
		// This bar is a visual indicator that just shows a display cd until the spell is cast again
		// The bar needs to regen based off the duration / cd of a spell
		float bar_Size = 100;
		unit.spell_Bar = create_Resource_Bar(50, 7, 47, 2, bar_Size, (bar_Size / unit.spell.time_To_Cast.duration), RBCS_Spell_Bar);
		unit.spell_Bar.current_Resource = 0.0f;
	} else {
		unit.spell.can_Cast_Spell = false;
	}

	unit.damage = scale_Unit_Damage(unit_Data, level);
	unit.attack_CD = scale_Unit_Attack_CD(unit_Data, level);

	// *****************
	unit.attack_Range = unit_Data.attack_Range;
	unit.destroyed = false;
	unit.stop = false;

	V2 direction_V2 = calculate_Direction_V2(target_Position, spawn_Position);
	unit.rigid_Body.velocity.x = direction_V2.x * unit_Data.speed;
	unit.rigid_Body.velocity.y = direction_V2.y * unit_Data.speed;

	float radius = get_Sprite_Radius(&unit.sprite_Sheet_Tracker);

	if (unit_Data.projectile_Type != "") {
		Projectile_Data projectile_Data = get_Projectile_Data(unit_Data.projectile_Type);
		unit.projectile_Type = projectile_Data.type;
		unit.fires_Projectiles = true;
	} else {
		unit.fires_Projectiles = false;
	}

	add_Collider(&unit.rigid_Body, { 0.0f, -(radius / 2) }, (radius / 2));
	add_Collider(&unit.rigid_Body, { 0.0f, 0.0f }, (radius / 2));
	add_Collider(&unit.rigid_Body, { 0.0f, (radius / 2) }, (radius / 2));

	// unit.ID = allocate_Entity_ID(*game_Data);
	if (unit.nation == N_PLAYER) {
		unit.handle = create_Handle(game_Data.units);
		game_Data.units.arr[unit.handle.index] = unit;

		game_Data.player_Unit_IDS.push_back(unit.handle);
	} else if (unit.nation == N_ENEMY) {
		unit.handle = create_Handle(game_Data.units);
		game_Data.units.arr[unit.handle.index] = unit;

		game_Data.enemy_Unit_IDS.push_back(unit.handle);
	}
	// Could push onto the active_Entites vector here as well
}

void spawn_Unit_At_Castle(Game_Data& game_Data, Summonable_Unit& summonable_Unit) {
	Castle* origin_Castle;
	Castle* target_Castle;
	if (summonable_Unit.nation == N_PLAYER) {
		origin_Castle = &game_Data.player_Castle;
		target_Castle = &game_Data.enemy_Castle;
	} else if (summonable_Unit.nation == N_ENEMY) {
		origin_Castle = &game_Data.enemy_Castle;
		target_Castle = &game_Data.player_Castle;
	} else {
		assert(false);
		return;
	}
	spawn_Unit(
		game_Data,
		summonable_Unit.nation,
		summonable_Unit.name,
		summonable_Unit.level,
		{
			(float)origin_Castle->rigid_Body.position_WS.x,
			((float)game_Data.terrain_Height_Map[(int)origin_Castle->rigid_Body.position_WS.x] + get_Sprite_Radius(&origin_Castle->sprite_Sheet_Tracker))
		},
		target_Castle->rigid_Body.position_WS
	);
	summonable_Unit.is_Pressed = false;
}

void cast_Raise_Dead(Game_Data& game_Data, Handle casting_Unit_ID) {
	Unit* unit = get_Unit(game_Data.units, casting_Unit_ID);
	if (unit != nullptr) {
		Spell_Data spell_Data = get_Spell_Data(unit->spell.type);
		int range_Offset_From_Unit = 25;
		int spawn_Range = 100;
		int random_X_To_Summon = rand() % spawn_Range;
		random_X_To_Summon += range_Offset_From_Unit;
		if (unit->nation == N_ENEMY) {
			// Invert the range
			random_X_To_Summon *= -1;
		}

		V2 new_Pos;
		new_Pos.x = (float)random_X_To_Summon + unit->rigid_Body.position_WS.x;
		new_Pos.y = get_Height_Map_Pos_Y(game_Data, (int)new_Pos.x);
		new_Pos.y += (RESOLUTION_HEIGHT - new_Pos.y);

		if (unit->nation == N_PLAYER) {
			spawn_Unit(game_Data, unit->nation, spell_Data.summon_Type, unit->level, new_Pos, game_Data.enemy_Castle.rigid_Body.position_WS);
		}
		else if (unit->nation == N_ENEMY) {
			spawn_Unit(game_Data, unit->nation, spell_Data.summon_Type, unit->level, new_Pos, game_Data.player_Castle.rigid_Body.position_WS);
		}
	}
}

void cast_Spell(Game_Data& game_Data, Handle casting_Unit_ID) {
	Unit* unit = get_Unit(game_Data.units, casting_Unit_ID);
	if (unit != nullptr) {
		if (unit->spell.type == "raise_Dead") {
			unit->spell_Bar.current_Resource -= unit->spell_Bar.max_Resource;
			cast_Raise_Dead(game_Data, casting_Unit_ID);
		}
	}
}

void cast_Units_Spells(Game_Data& game_Data, std::vector<Handle> units, float delta_Time) {
	for (Handle handle : units) {
		Unit* unit = get_Unit(game_Data.units, handle);
		if (unit != nullptr) {
			if (unit->spell.can_Cast_Spell) {
				while (unit->spell.time_To_Cast.remaining <= 0) {
					cast_Spell(game_Data, unit->handle);
					unit->spell.time_To_Cast.remaining += unit->spell.time_To_Cast.duration;
				}
				unit->spell.time_To_Cast.remaining -= delta_Time;
			}
		}
	}
}


void draw_Circle(float center_X, float center_Y, float radius, Color_Index color) {
	float total_Lines = 30;
	float line_Step = (float)(2 * M_PI) / total_Lines;
	int x1, y1, x2, y2 = 0;

	for (float angle = 0; angle < (2 * M_PI); angle += line_Step) {
		// sin and cos gives us the ratio of the length
		x1 = (int)(center_X + (radius * cos(angle - line_Step)));
		y1 = (int)(center_Y + (radius * sin(angle - line_Step)));
		x2 = (int)(center_X + (radius * cos(angle)));
		y2 = (int)(center_Y + (radius * sin(angle)));
		if (color == CI_RED) {
			MP_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		}
		else if (color == CI_GREEN) {
			MP_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
		}
		else if (color == CI_BLUE) {
			MP_SetRenderDrawColor(Globals::renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
		}
		else {
			// Yellow is default
			MP_SetRenderDrawColor(Globals::renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
		}
		MP_RenderDrawLine(Globals::renderer, x1, y1, x2, y2);
	}
}

void draw_RigidBody_Colliders(Rigid_Body* rigid_Body, Color_Index color) {
	// This is a little weird
	for (int i = 0; i < rigid_Body->num_Colliders; i++) {
		Collider* collider = &rigid_Body->colliders[i];
		V2 world_Position = get_Collider_WS_Position(rigid_Body, collider);
		draw_Circle(world_Position.x, world_Position.y, collider->radius, color);
	}
}

void outline_Rect(SDL_Rect* rect, int outline_Thickness) {
	SDL_Rect top_Rect = {};
	top_Rect.x = rect->x;
	top_Rect.y = rect->y;
	top_Rect.w = rect->w;
	top_Rect.h = outline_Thickness;
	MP_RenderFillRect(Globals::renderer, &top_Rect);

	SDL_Rect bottom_Rect = {};
	bottom_Rect.x = rect->x;
	bottom_Rect.y = ((rect->y + rect->h) - outline_Thickness);
	bottom_Rect.w = rect->w;
	bottom_Rect.h = outline_Thickness;
	MP_RenderFillRect(Globals::renderer, &bottom_Rect);

	SDL_Rect left_Rect = {};
	left_Rect.x = rect->x;
	left_Rect.y = rect->y;
	left_Rect.w = outline_Thickness;
	left_Rect.h = rect->h;
	MP_RenderFillRect(Globals::renderer, &left_Rect);

	SDL_Rect right_Rect = {};
	right_Rect.x = ((rect->x + rect->w) - outline_Thickness);
	right_Rect.y = rect->y;
	right_Rect.w = outline_Thickness;
	right_Rect.h = rect->h;
	MP_RenderFillRect(Globals::renderer, &right_Rect);

	MP_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	MP_RenderDrawRect(Globals::renderer, rect);
}

void draw_Resource_Bar(Resource_Bar& resource_Bar, V2 pos) {
	// Pull the data from the colors 
	Color left_Side = resource_Bar_Colors[resource_Bar.selected_Colors].left_Rect;
	Color right_Side = resource_Bar_Colors[resource_Bar.selected_Colors].right_Rect;

	float remaining_HP_Percent = (resource_Bar.current_Resource / resource_Bar.max_Resource);
	if (remaining_HP_Percent < 0) {
		remaining_HP_Percent = 0;
	}

	// Lerp of T = A * (1 - T) + B * T
	// A is the left side, B is the right side, T is the health %
	float lerp = linear_Interpolation(0, (float)resource_Bar.width, remaining_HP_Percent);

	SDL_Rect left_Rect = {};
	left_Rect.w = (int)lerp;
	left_Rect.h = (int)resource_Bar.height;
	left_Rect.x = (int)((pos.x) - resource_Bar.width / 2);
	left_Rect.y = (int)((pos.y) - resource_Bar.y_Offset);
	MP_SetRenderDrawColor(Globals::renderer, left_Side.r, left_Side.g, left_Side.b, SDL_ALPHA_OPAQUE);
	MP_RenderFillRect(Globals::renderer, &left_Rect);

	SDL_Rect right_Rect = left_Rect;
	right_Rect.w = resource_Bar.width - left_Rect.w;
	right_Rect.x = (int)(left_Rect.x + left_Rect.w);
	MP_SetRenderDrawColor(Globals::renderer, right_Side.r, right_Side.g, right_Side.b, SDL_ALPHA_OPAQUE);
	MP_RenderFillRect(Globals::renderer, &right_Rect);

	// Outline HP bars
	SDL_Rect outline = {};
	outline.w = (int)resource_Bar.width;
	outline.h = (int)resource_Bar.height;
	outline.x = (int)((pos.x) - resource_Bar.width / 2);
	outline.y = (int)((pos.y) - resource_Bar.y_Offset);
	MP_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	outline_Rect(&outline, resource_Bar.thickness);
}


std::vector<int> create_Height_Map(std::string map_Name) {
	std::string file_Path = "images/" + map_Name + ".png";
	int channels = 0;
	int terrain_Width = 0;
	int terrain_Height = 0;
	unsigned char* data = stbi_load(file_Path.c_str(), &terrain_Width, &terrain_Height, &channels, 4);

	if (data == NULL) {
		#ifndef USE_CUSTOM_SDL
			log("ERROR: stbi_load returned NULL");
		#else 
			SDL_Log("ERROR: stbi_load returned NULL");
		#endif
		return std::vector<int>();
	}

	DEFER{
		stbi_image_free(data);
	};

	std::vector<int> height_Map(terrain_Width, -1);

	for (int x = 0; x < terrain_Width; x++) {
		for (int y = 0; y < terrain_Height; y++) {
			int index = (((y * terrain_Width) + x) * channels);
			unsigned char alpha = data[index + 3];
			if (alpha > 0) {
				height_Map[x] = terrain_Height - y;
				break;
			}
		}
	}

	return height_Map;
}

bool check_Height_Map_Collision(Rigid_Body* rigid_Body, std::vector<int>& height_Map) {
	for (int i = 0; i < rigid_Body->num_Colliders; i++) {
		Collider* collider = &rigid_Body->colliders[i];
		V2 world_Position = get_Collider_WS_Position(rigid_Body, collider);

		int collider_X = (int)world_Position.x;
		int collider_Y = (int)world_Position.y + (int)collider->radius;

		if (collider_X < 0 || collider_X >= height_Map.size()) {
			continue;
		}

		int terrain_Position = RESOLUTION_HEIGHT - height_Map[collider_X];

		if (collider_Y >= terrain_Position) {
			return true;
		}
	}
	return false;
}

// Need to have world position by the time I get here
bool check_RB_Collision(Rigid_Body* rigid_Body_1, Rigid_Body* rigid_Body_2) {
	// Apply rotation at this point
	// LOCAL POSITION DOES NOT CHANGE
	// SET THE LOCAL POSITION ONE TIME BUT THAT'S IT. Unless I want to animate the collider.
	for (int i = 0; i < rigid_Body_1->num_Colliders; i++) {
		Collider* collider_1 = &rigid_Body_1->colliders[i];
		V2 world_Pos_1 = get_Collider_WS_Position(rigid_Body_1, collider_1);

		for (int j = 0; j < rigid_Body_2->num_Colliders; j++) {
			Collider* collider_2 = &rigid_Body_2->colliders[j];
			V2 world_Pos_2 = get_Collider_WS_Position(rigid_Body_2, collider_2);

			float distance_Between = calculate_Distance(
				world_Pos_1.x, world_Pos_1.y,
				world_Pos_2.x, world_Pos_2.y);
			float radius_Sum = collider_1->radius + collider_2->radius;

			if (distance_Between <= radius_Sum) {
				return true;
			}
		}
	}
	return false;
}

// Calculates the attack from of the originating unit
bool check_Attack_Range_Collision(float origin_Attack_Range, Rigid_Body* origin_RB, Rigid_Body* target_RB) {
	V2 origin_Pos = origin_RB->position_WS;
	V2 target_Pos = target_RB->position_WS;
	for (int i = 0; i < target_RB->num_Colliders; i++) {
		Collider* target_Collider = &target_RB->colliders[i];
		V2 collider_WS_Pos = get_Collider_WS_Position(target_RB, target_Collider);
		float distance_Between = calculate_Distance(origin_Pos, target_Pos);
		float sum = target_Collider->radius + origin_Attack_Range;
		if (distance_Between <= sum) {
			return true;
		}
	}
	return false;
}

void check_Player_Unit_Castle_Collision(Game_Data& game_Data) {
	for (int i = 0; i < Globals::MAX_ENTITY_ARRAY_LENGTH; i++) {
		Unit* player_Unit = get_Unit(game_Data.units, game_Data.units.arr[i].handle);
		Castle* castle = &game_Data.enemy_Castle;
		if (check_RB_Collision(&player_Unit->rigid_Body, &castle->rigid_Body)) {
			player_Unit->stop = true;
			if (player_Unit->attack_CD.remaining < 0) {
				player_Unit->attack_CD.remaining = player_Unit->attack_CD.duration;
				castle->health_Bar.current_Resource -= player_Unit->damage;
			}
		}
	}
}

// Doesn't account for empty height map
float get_Height_Map_Pos_Y(Game_Data& game_Data, int x_Pos) {
	if (x_Pos < 0) {
		x_Pos = 0;
	}
	if (x_Pos >= game_Data.terrain_Height_Map.size()) {
		x_Pos = (int)(game_Data.terrain_Height_Map.size() - 1);
	}
	// Account for the inversion of the map
	float result = (float)game_Data.terrain_Height_Map[x_Pos];
	result += (RESOLUTION_HEIGHT - ((float)game_Data.terrain_Height_Map[x_Pos] * 2.0f));
	return result;
}

// Array size will be determined based off total number of initializations
Type_Descriptor unit_Type_Descriptors[] = {
	FIELD(Unit_Data, DT_STRING, type),
	FIELD(Unit_Data, DT_FLOAT, food_Cost),
	FIELD(Unit_Data, DT_STRING, sprite_Sheet_Name),
	FIELD(Unit_Data, DT_STRING, projectile_Type),
	FIELD(Unit_Data, DT_STRING, spell_Type),
	FIELD(Unit_Data, DT_FLOAT, base_HP),
	FIELD(Unit_Data, DT_FLOAT, hp_Multiplier),
	FIELD(Unit_Data, DT_FLOAT, base_Damage),
	FIELD(Unit_Data, DT_FLOAT, damage_Multiplier),
	FIELD(Unit_Data, DT_FLOAT, speed),
	FIELD(Unit_Data, DT_FLOAT, base_Attacks_Per_Second),
	FIELD(Unit_Data, DT_FLOAT, attacks_Per_Second_Multiplier),
	FIELD(Unit_Data, DT_FLOAT, max_Attacks_Per_Second),
	FIELD(Unit_Data, DT_FLOAT, attack_Range)
};

void load_Unit_Data_CSV(CSV_Data* csv_Data) {
	csv_Data->last_Modified_Time = file_Last_Modified(csv_Data->file_Path);
	
	std::vector<Unit_Data> unit_Data;
	unit_Data.resize(csv_Data->rows);

	std::span<Type_Descriptor> unit_Descriptors(unit_Type_Descriptors);

	//					 Data destination		 size of one stride	   descriptors above
	//					(char*) ptr math
	load_CSV_Data(csv_Data, (char*)unit_Data.data(), sizeof(unit_Data[0]), unit_Descriptors);

	// Loop through the vector and add the data to the unordered map
	for (Unit_Data& iterator : unit_Data) {
		unit_Data_Map[iterator.type] = iterator;
	}
}

// Array size will be determined based off total number of initializations
Type_Descriptor projectile_Type_Descriptors[] = {
	FIELD(Projectile_Data, DT_STRING, type),
	FIELD(Projectile_Data, DT_STRING, sprite_Sheet_Name),
	FIELD(Projectile_Data, DT_STRING, collider),
	// Treat a bool like an int. 0 is false. 1 is true.
	FIELD(Projectile_Data, DT_INT, max_Penetrations),
	FIELD(Projectile_Data, DT_INT, can_Attach),
	FIELD(Projectile_Data, DT_FLOAT, gravity_Multiplier),
	FIELD(Projectile_Data, DT_FLOAT, speed),
	FIELD(Projectile_Data, DT_FLOAT, life_Time),
	FIELD(Projectile_Data, DT_FLOAT, collider_Pos_LS_X),
	FIELD(Projectile_Data, DT_FLOAT, collider_Pos_LS_Y),
	FIELD(Projectile_Data, DT_FLOAT, collider_Radius),
	FIELD(Projectile_Data, DT_FLOAT, castle_Base_Ammo_Per_Sec),
	FIELD(Projectile_Data, DT_FLOAT, castle_Ammo_Per_Sec_Multiplier),
	FIELD(Projectile_Data, DT_FLOAT, castle_Max_Ammo_Per_Sec),
};

void load_Projectile_Data_CSV(CSV_Data* csv_Data) {
	csv_Data->last_Modified_Time = file_Last_Modified(csv_Data->file_Path);

	std::vector<Projectile_Data> projectile_Data;
	projectile_Data.resize(csv_Data->rows);
	
	std::span<Type_Descriptor> projectile_Descriptors(projectile_Type_Descriptors);

	load_CSV_Data(csv_Data, (char*)projectile_Data.data(), sizeof(projectile_Data[0]), projectile_Descriptors);

	for (Projectile_Data& iterator : projectile_Data) {
		projectile_Data_Map[iterator.type] = iterator;
	}
}

Type_Descriptor spell_Type_Descriptors[] = {
	FIELD(Spell_Data, DT_STRING, type),
	FIELD(Spell_Data, DT_STRING, summon_Type),
	FIELD(Spell_Data, DT_FLOAT, base_Cast_Time)
};

void load_Spell_Data_CSV(CSV_Data* csv_Data) {
	csv_Data->last_Modified_Time = file_Last_Modified(csv_Data->file_Path);

	std::vector<Spell_Data> spell_Data;
	spell_Data.resize(csv_Data->rows);

	std::span<Type_Descriptor> spell_Descriptors(spell_Type_Descriptors);

	load_CSV_Data(csv_Data, (char*)spell_Data.data(), sizeof(spell_Data[0]), spell_Descriptors);

	for (Spell_Data& iterator : spell_Data) {
		spell_Data_Map[iterator.type] = iterator;
	}
}

Type_Descriptor castle_Type_Descriptors[] = {
	FIELD(Castle_Data, DT_STRING, type),
	FIELD(Castle_Data, DT_STRING, sprite_Sheet_Name),
	FIELD(Castle_Data, DT_STRING, projectile_Type),
	FIELD(Castle_Data, DT_STRING, enhancement),
	FIELD(Castle_Data, DT_FLOAT, base_HP),
	FIELD(Castle_Data, DT_FLOAT, base_HP_Regen),
	FIELD(Castle_Data, DT_FLOAT, base_Food_Points),
	FIELD(Castle_Data, DT_FLOAT, base_Food_Points_Regen),
	FIELD(Castle_Data, DT_FLOAT, food_Points_Multiplier)
};

void load_Castle_Data_CSV(CSV_Data* csv_Data) {
	csv_Data->last_Modified_Time = file_Last_Modified(csv_Data->file_Path);

	std::vector<Castle_Data> castle_Data;
	castle_Data.resize(csv_Data->rows);

	std::span<Type_Descriptor> castle_Descriptors(castle_Type_Descriptors);

	load_CSV_Data(csv_Data, (char*)castle_Data.data(), sizeof(castle_Data[0]), castle_Descriptors);

	for (Castle_Data& iterator : castle_Data) {
		castle_Data_Map[iterator.type] = iterator;
	}
}