#include "Entity.h"
#include <assert.h>

static std::unordered_map<std::string, Unit_Data> unit_Data_Map = {};

const Unit_Data bad_Unit_Data = {
	//	Type		sprite_Sheet	max_HP	damage	speed	attack_Cooldown		attack_Range	spell_Type;
	   "warrior",	"warrior_Stop", "",		100,	25,		50,		1.0f,				150//,		    ""         
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

Unit* get_Unit(Storage<Unit>& storage, Handle handle) {
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
		if (unit->destroyed || unit->health_Bar.current_HP <= 0) {
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
		if (unit->destroyed || unit->health_Bar.current_HP <= 0) {
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

void draw_Castle(Castle* castle, bool flip) {
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
	SDL_RenderCopyEx(Globals::renderer, sprite->image.texture, NULL, &temp, 0, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

void draw_Projectile(Projectile* projectile, bool flip) {
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
	SDL_RenderCopyEx(Globals::renderer, sprite->image.texture, NULL, &temp, projectile->rigid_Body.angle, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
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

	if (summonable_Unit.nation == N_PLAYER) {
		game_Data.player_Castle.summonable_Units.push_back(summonable_Unit);
	} else if (summonable_Unit.nation == N_ENEMY) {
		game_Data.enemy_Castle.summonable_Units.push_back(summonable_Unit);
	}
}

void draw_Attached_Entity(Attached_Entity* attached_Entity, V2 position_WS, bool flip) {
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
	SDL_RenderCopyEx(Globals::renderer, sprite->image.texture, NULL, &temp, attached_Entity->angle, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
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

// Flickering when speed changes
// Same animation frame
// NEED MORE VARIABLE TO TRACK
// Current frame time
// Frame duration
// Last frame update time
// is playing
void draw_Unit_Animated(Rigid_Body* rigid_Body, Sprite_Sheet_Tracker* tracker, bool flip) {
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

	V2 pivot = calculate_Center(
		(float)sprite_Sheet->sprites[sprite_Frame].source_Rect.w,
		(float)sprite_Sheet->sprites[sprite_Frame].source_Rect.h
	);

	// Set the center of the rotation
	SDL_Point pivot_Point = {
		(int)pivot.x,
		(int)pivot.y
	};

	// Render the current frame of the animation
	SDL_RenderCopyEx(
		Globals::renderer,
		sprite_Sheet->sprites[sprite_Frame].image.texture,
		&current_Frame_Rect,
		&destination_Rect,
		rigid_Body->angle,
		&pivot_Point,
		(flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)
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
		Projectile* projectile = get_Projectile(game_Data.projectiles, game_Data.player_Proj_IDS[i]);
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
			unit->current_Attack_Cooldown -= delta_Time;
		}
	}
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
				castle->health_Bar.current_HP -= projectile->damage;
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
									spawn_Particle_System(
										game_Data,
										"PT_BLOOD",
										target_Unit->rigid_Body.position_WS,
										0.5,
										15,
										15,
										target_Unit->handle,
										false
									);
									target_Unit->health_Bar.current_HP -= projectile->damage;
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
					if (unit->current_Attack_Cooldown < 0) {
						unit->current_Attack_Cooldown = unit->attack_Cooldown;
						castle->health_Bar.current_HP -= unit->damage;
					}
				} else {
					if (unit->current_Attack_Cooldown <= 0) {
						unit->current_Attack_Cooldown = unit->attack_Cooldown;
						V2 aim_Head = castle->rigid_Body.position_WS;
						aim_Head.x += get_Sprite_Radius(&castle->sprite_Sheet_Tracker);
						V2 arrow_Spawn_Location = unit->rigid_Body.position_WS;
						arrow_Spawn_Location.y -= get_Sprite_Radius(&castle->sprite_Sheet_Tracker) / 2;
						spawn_Projectile(game_Data, N_PLAYER, unit->projectile_Type, unit->damage, arrow_Spawn_Location, aim_Head);
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
							if (origin_Unit->current_Attack_Cooldown <= 0) {
								origin_Unit->current_Attack_Cooldown = origin_Unit->attack_Cooldown;
								target_Unit->health_Bar.current_HP -= origin_Unit->damage;
							}
						}
						else {
							// change_Animation(&player_Unit->sprite_Sheet_Tracker, "archer_Stop");
							if (origin_Unit->current_Attack_Cooldown <= 0) {
								origin_Unit->current_Attack_Cooldown = origin_Unit->attack_Cooldown;
								V2 aim_Head = target_Unit->rigid_Body.position_WS;
								aim_Head.x += get_Sprite_Radius(&target_Unit->sprite_Sheet_Tracker);
								V2 arrow_Spawn_Location = origin_Unit->rigid_Body.position_WS;
								arrow_Spawn_Location.y -= get_Sprite_Radius(&target_Unit->sprite_Sheet_Tracker) / 2;
								spawn_Projectile(game_Data, N_PLAYER, origin_Unit->projectile_Type, origin_Unit->damage, arrow_Spawn_Location, aim_Head);
							}
							// change_Animation(&player_Unit->sprite_Sheet_Tracker, "archer_Walk");
						}
					}
					//if (target_Unit->current_Attack_Cooldown <= 0) {
					//	target_Unit->current_Attack_Cooldown = target_Unit->attack_Cooldown;
					//	origin_Unit->health_Bar.current_HP -= target_Unit->damage;
					//}
				}
			}
		}
	}
}

Health_Bar create_Health_Bar(int width, int height, int y_Offset, int thickness, float hp) {
	Health_Bar result;

	result.width = width;
	result.height = height;
	result.y_Offset = y_Offset;
	result.thickness = thickness;
	result.max_HP = hp;
	result.current_HP = result.max_HP;

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

void spawn_Player_Castle(Game_Data& game_Data, V2 position_WS, Level level) {
	Castle castle = {};

	castle.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker("castle");

	castle.rigid_Body = create_Rigid_Body(position_WS, false);

	castle.health_Bar = create_Health_Bar(90, 20, 115, 3, castle_Stats_Array[level].hp);

	castle.fire_Cooldown = castle_Stats_Array[level].fire_Cooldown;
	castle.spawn_Cooldown = castle_Stats_Array[level].spawn_Cooldown;
	castle.arrow_Ammo = castle_Stats_Array[level].arrow_Ammo;
	castle.arrow_Ammo_Cooldown = castle_Stats_Array[level].arrow_Ammo_Cooldown;

	add_Collider(&castle.rigid_Body, { 0.0f, 0.0f }, get_Sprite_Radius(&castle.sprite_Sheet_Tracker));

	game_Data.player_Castle = castle;
}

void spawn_Enemy_Castle(Game_Data& game_Data, V2 position_WS, Level level) {
	Castle castle = {};

	// Would be the appropriate way to do it but it breaks the serialization
	// castle.stats_Info = &castle_Stats_Array[level];

	castle.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker("castle");

	castle.rigid_Body = create_Rigid_Body(position_WS, false);

	castle.health_Bar = create_Health_Bar(90, 20, 115, 3, castle_Stats_Array[level].hp);

	castle.fire_Cooldown = castle_Stats_Array[level].fire_Cooldown;
	castle.spawn_Cooldown = castle_Stats_Array[level].spawn_Cooldown;

	add_Collider(&castle.rigid_Body, { 0.0f, 0.0f }, get_Sprite_Radius(&castle.sprite_Sheet_Tracker));

	game_Data.enemy_Castle = castle;
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

void spawn_Unit(Game_Data& game_Data, Nation unit_Side, std::string unit_Type, int level, V2 spawn_Position, V2 target_Position) {
	Unit unit = {};

	Unit_Data unit_Data = get_Unit_Data(unit_Type);
	unit.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(unit_Data.sprite_Sheet_Name);
	unit.rigid_Body = create_Rigid_Body(spawn_Position, false);
	
	unit.speed = unit_Data.speed;

	// ***Level based ***
	float updated_HP = unit_Data.base_HP + ((level - 1.0f) * ((unit_Data.hp_Multiplier * unit_Data.base_HP) - unit_Data.base_HP));
	unit.health_Bar = create_Health_Bar(50, 13, 60, 2, updated_HP);
	
	float updated_Damage = unit_Data.base_Damage + ((level - 1.0f) * ((unit_Data.damage_Multiplier * unit_Data.base_Damage) - unit_Data.base_Damage));
	unit.damage = updated_Damage;
	//NOTE: 														  subtract this value because it's a cooldown
	float updated_Attack_Cooldown = unit_Data.base_Attack_Cooldown - ((level - 1.0f) * ((unit_Data.attack_Cooldown_Multiplier * unit_Data.base_Attack_Cooldown) - unit_Data.base_Attack_Cooldown));
	unit.attack_Cooldown = updated_Attack_Cooldown;
	// *****************

	unit.current_Attack_Cooldown = 0.0f;
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
	if (unit_Side == N_PLAYER) {
		unit.handle = create_Handle(game_Data.units);
		game_Data.units.arr[unit.handle.index] = unit;

		game_Data.player_Unit_IDS.push_back(unit.handle);
	} else if (unit_Side == N_ENEMY) {
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
			SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		}
		else if (color == CI_GREEN) {
			SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
		}
		else if (color == CI_BLUE) {
			SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
		}
		else {
			// Yellow is default
			SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
		}
		SDL_RenderDrawLine(Globals::renderer, x1, y1, x2, y2);
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

std::vector<int> create_Height_Map(const char* filename) {
	int channels = 0;
	int terrain_Width = 0;
	int terrain_Height = 0;
	unsigned char* data = stbi_load(filename, &terrain_Width, &terrain_Height, &channels, 4);

	if (data == NULL) {
		SDL_Log("ERROR: stbi_load returned NULL");
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
			if (player_Unit->current_Attack_Cooldown < 0) {
				player_Unit->current_Attack_Cooldown = player_Unit->attack_Cooldown;
				castle->health_Bar.current_HP -= player_Unit->damage;
			}
		}
	}
}

void fire_Projectile() {

}

void cast_Spell() {

}

// Doesn't account for empty height map
float get_Height_Map_Pos_Y(Game_Data& game_Data, int x_Pos) {
	if (x_Pos < 0) {
		x_Pos = 0;
	}
	if (x_Pos >= game_Data.terrain_Height_Map.size()) {
		x_Pos = (int)(game_Data.terrain_Height_Map.size() - 1);
	}
	return (float)game_Data.terrain_Height_Map[x_Pos];
}

// Array size will be determined based off total number of initializations
Type_Descriptor unit_Type_Descriptors[] = {
	FIELD(Unit_Data, DT_STRING, type),
	FIELD(Unit_Data, DT_STRING, sprite_Sheet_Name),
	FIELD(Unit_Data, DT_STRING, projectile_Type),
	FIELD(Unit_Data, DT_FLOAT, base_HP),
	FIELD(Unit_Data, DT_FLOAT, hp_Multiplier),
	FIELD(Unit_Data, DT_FLOAT, base_Damage),
	FIELD(Unit_Data, DT_FLOAT, damage_Multiplier),
	FIELD(Unit_Data, DT_FLOAT, speed),
	FIELD(Unit_Data, DT_FLOAT, base_Attack_Cooldown),
	FIELD(Unit_Data, DT_FLOAT, attack_Cooldown_Multiplier),
	FIELD(Unit_Data, DT_FLOAT, attack_Range),
	// FIELD(Unit_Data, MT_STRING, spell_Type),
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