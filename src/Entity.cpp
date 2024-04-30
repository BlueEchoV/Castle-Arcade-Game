#include "Entity.h"
void add_Collider(Rigid_Body* rigid_Body, V2 position_LS, float radius) {
	// assert(rigid_Body->num_Colliders < Globals::MAX_COLLIDERS);

	Collider* collider = &rigid_Body->colliders[rigid_Body->num_Colliders++];
	collider->position_LS = position_LS;
	collider->radius = radius;
}

void draw_Castle(Castle* castle, bool flip) {
	SDL_Rect temp = {};
	Sprite* sprite = &Globals::sprite_Sheet_Array[castle->sprite_Sheet_Tracker.sprite_Sheet_Selector].sprites[0];
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

void draw_Arrow(Arrow* arrow, bool flip) {
	SDL_Rect temp = {};
	Sprite* sprite = &Globals::sprite_Sheet_Array[arrow->sprite_Sheet_Tracker.sprite_Sheet_Selector].sprites[0];
	SDL_Rect* src_Rect = &sprite->source_Rect;
	V2 sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
	sprite_Half_Size = sprite_Half_Size / 2;
	temp = {
		((int)arrow->rigid_Body.position_WS.x - (int)sprite_Half_Size.x),
		((int)arrow->rigid_Body.position_WS.y - (int)sprite_Half_Size.y),
		sprite->source_Rect.w,
		sprite->source_Rect.h
	};
	SDL_RenderCopyEx(Globals::renderer, sprite->image.texture, NULL, &temp, arrow->rigid_Body.angle, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

Attached_Entity return_Attached_Entity(Sprite_Sheet_Selector sprite_Sheet_Selector, float angle, V2 offset) {
	Attached_Entity result = {};

	//                 selected     animation_Time      current_Frame
	result.tracker = { sprite_Sheet_Selector,    0.0f,               0 };
	result.angle = angle;
	result.offset = offset;

	return result;
}

void draw_Attached_Entity(Attached_Entity* attached_Entity, V2 position_WS, bool flip) {
	SDL_Rect temp = {};
	Sprite* sprite = &Globals::sprite_Sheet_Array[attached_Entity->tracker.sprite_Sheet_Selector].sprites[0];
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
			if (tracker->current_Frame >= Globals::sprite_Sheet_Array[tracker->sprite_Sheet_Selector].sprites.size()) {
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

	const Sprite_Sheet* sprite_Sheet = &Globals::sprite_Sheet_Array[tracker->sprite_Sheet_Selector];
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

void change_Animation(Sprite_Sheet_Tracker* tracker, Sprite_Sheet_Selector sprite_Sheet_Selector) {
	if (tracker->sprite_Sheet_Selector != sprite_Sheet_Selector) {
		tracker->sprite_Sheet_Selector = sprite_Sheet_Selector;
		tracker->current_Frame = 0;
	}
}

V2 get_WS_Position(Rigid_Body* rigid_Body, const Collider* collider) {
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


void update_Arrow_Position(Arrow* arrow, float delta_Time) {
	if (!arrow->stop) {
		if (arrow->type == AT_PLAYER_ARROW) {
			arrow->rigid_Body.velocity.y += Globals::GRAVITY * delta_Time;
		}
		else if (arrow->type == AT_ARCHER_ARROW) {
			arrow->rigid_Body.velocity.y += Globals::ARCHER_ARROW_GRAVITY * delta_Time;
		}
		else {
			SDL_Log("ERROR: Arrow type not specified. update_Arrow_Position()");
			return;
		}

		arrow->rigid_Body.position_WS.x += arrow->rigid_Body.velocity.x * delta_Time;
		arrow->rigid_Body.position_WS.y += arrow->rigid_Body.velocity.y * delta_Time;
	}
}

void update_Unit_Position(Rigid_Body* rigid_Body, bool stop_Unit, float delta_Time) {
	if (!stop_Unit) {
		rigid_Body->position_WS.y += Globals::GRAVITY * delta_Time;
		rigid_Body->position_WS.x += (rigid_Body->velocity.x * delta_Time);
		rigid_Body->position_WS.y += (rigid_Body->velocity.y * delta_Time);
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


void spawn_Player_Castle(Sprite_Sheet_Selector sprite_Sheet_Selector, Game_Data* game_Data, V2 position_WS, Level level) {
	Castle castle = {};

	castle.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(sprite_Sheet_Selector);

	castle.rigid_Body = create_Rigid_Body(position_WS, false);

	castle.health_Bar = create_Health_Bar(90, 20, 115, 3, castle_Stats_Array[level].hp);

	castle.fire_Cooldown = castle_Stats_Array[level].fire_Cooldown;
	castle.spawn_Cooldown = castle_Stats_Array[level].spawn_Cooldown;
	castle.arrow_Ammo = castle_Stats_Array[level].arrow_Ammo;
	castle.arrow_Ammo_Cooldown = castle_Stats_Array[level].arrow_Ammo_Cooldown;

	add_Collider(&castle.rigid_Body, { 0.0f, 0.0f }, Globals::sprite_Sheet_Array[sprite_Sheet_Selector].sprites[0].radius);

	game_Data->player_Castle = castle;
}

void spawn_Enemy_Castle(Sprite_Sheet_Selector sprite_Sheet_Selector, Game_Data* game_Data, V2 position_WS, Level level) {
	Castle castle = {};

	// Would be the appropriate way to do it but it breaks the serialization
	// castle.stats_Info = &castle_Stats_Array[level];

	castle.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(sprite_Sheet_Selector);

	castle.rigid_Body = create_Rigid_Body(position_WS, false);

	castle.health_Bar = create_Health_Bar(90, 20, 115, 3, castle_Stats_Array[level].hp);

	castle.fire_Cooldown = castle_Stats_Array[level].fire_Cooldown;
	castle.spawn_Cooldown = castle_Stats_Array[level].spawn_Cooldown;

	add_Collider(&castle.rigid_Body, { 0.0f, 0.0f }, Globals::sprite_Sheet_Array[sprite_Sheet_Selector].sprites[0].radius);

	game_Data->enemy_Castle = castle;
}


void spawn_Arrow(Game_Data* game_Data, Arrow_Type type, Sprite_Sheet_Selector sprite_Sheet_Selector, V2 spawn_Position, V2 target_Position, Level level) {
	Arrow arrow = {};

	arrow.type = type;

	arrow.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(sprite_Sheet_Selector);

	arrow.rigid_Body = create_Rigid_Body(spawn_Position, true);

	arrow.damage = arrow_Stats_Array[level].damage;
	arrow.speed = arrow_Stats_Array[level].speed;
	arrow.life_Time = arrow_Stats_Array[level].life_Time;
	// 90000 is a pretty garbage number. I may want to come up with a better means of measuring speed
	arrow.collision_Delay.duration = 0.02f;
	arrow.collision_Delay.remaining = arrow.collision_Delay.duration;

	arrow.stop = false;
	arrow.destroyed = false;

	V2 direction_V2 = calculate_Direction_V2(target_Position, spawn_Position);

	arrow.rigid_Body.velocity.x = direction_V2.x * arrow.speed;
	arrow.rigid_Body.velocity.y = direction_V2.y * arrow.speed;

	// Improved readability
	float radius = get_Sprite_Radius(&arrow.sprite_Sheet_Tracker);

	add_Collider(
		&arrow.rigid_Body,
		{ (radius * 0.75f), 0.0f },
		(radius * 0.25f)
	);

	game_Data->player_Arrows.push_back(arrow);
}

void spawn_Player_Warrior(Game_Data* game_Data, Sprite_Sheet_Selector sprite_Sheet_Selector, V2 spawn_Position, V2 target_Position, Level level) {
	Warrior warrior = {};

	warrior.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(sprite_Sheet_Selector);

	warrior.rigid_Body = create_Rigid_Body(spawn_Position, false);

	warrior.health_Bar = create_Health_Bar(50, 13, 60, 2, warrior_Stats_Array[level].max_HP);

	warrior.speed = warrior_Stats_Array[level].speed;
	warrior.damage = warrior_Stats_Array[level].damage;
	warrior.attack_Cooldown = warrior_Stats_Array[level].attack_Cooldown;
	warrior.current_Attack_Cooldown = 0.0f;
	warrior.attack_Range = warrior_Stats_Array[level].attack_Range;

	warrior.destroyed = false;
	warrior.stop = false;

	V2 direction_V2 = calculate_Direction_V2(target_Position, spawn_Position);

	// Set the new velocity
	warrior.rigid_Body.velocity.x = direction_V2.x * warrior_Stats_Array[level].speed;
	warrior.rigid_Body.velocity.y = direction_V2.y * warrior_Stats_Array[level].speed;

	float radius = get_Sprite_Radius(&warrior.sprite_Sheet_Tracker);

	add_Collider(&warrior.rigid_Body, { 0.0f, -(radius / 2) }, (radius / 2));
	add_Collider(&warrior.rigid_Body, { 0.0f, 0.0f }, (radius / 2));
	add_Collider(&warrior.rigid_Body, { 0.0f, (radius / 2) }, (radius / 2));

	warrior.ID = game_Data->next_Entity_ID++;
	game_Data->player_Warriors.push_back(warrior);
}

void spawn_Enemy_Warrior(Game_Data* game_Data, Sprite_Sheet_Selector sprite_Sheet_Selector, V2 spawn_Position, V2 target_Position, Level level) {
	Warrior warrior = {};

	warrior.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(sprite_Sheet_Selector);

	warrior.rigid_Body = create_Rigid_Body(spawn_Position, false);

	warrior.health_Bar = create_Health_Bar(50, 13, 60, 2, warrior_Stats_Array[level].max_HP);

	warrior.speed = warrior_Stats_Array[level].speed;
	warrior.damage = warrior_Stats_Array[level].damage;
	warrior.attack_Cooldown = warrior_Stats_Array[level].attack_Cooldown;
	warrior.current_Attack_Cooldown = 0.0f;
	warrior.attack_Range = warrior_Stats_Array[level].attack_Range;

	warrior.destroyed = false;
	warrior.stop = false;

	V2 direction_V2 = calculate_Direction_V2(target_Position, spawn_Position);

	// Set the new velocity
	warrior.rigid_Body.velocity.x = direction_V2.x * warrior_Stats_Array[level].speed;
	warrior.rigid_Body.velocity.y = direction_V2.y * warrior_Stats_Array[level].speed;

	float radius = get_Sprite_Radius(&warrior.sprite_Sheet_Tracker);

	add_Collider(&warrior.rigid_Body, { 0.0f, -(radius / 2) }, (radius / 2));
	add_Collider(&warrior.rigid_Body, { 0.0f, 0.0f }, (radius / 2));
	add_Collider(&warrior.rigid_Body, { 0.0f, (radius / 2) }, (radius / 2));

	warrior.ID = game_Data->next_Entity_ID++;
	game_Data->enemy_Warriors.push_back(warrior);
}

void spawn_Archer(Game_Data* game_Data, Sprite_Sheet_Selector sprite_Sheet_Selector, V2 spawn_Position, V2 target_Position, Level level) {
	Archer archer = {};

	archer.health_Bar = create_Health_Bar(50, 13, 60, 2, archer_Stats_Array[level].hp);

	archer.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(sprite_Sheet_Selector);

	archer.rigid_Body = create_Rigid_Body(spawn_Position, false);

	archer.speed = archer_Stats_Array[level].speed;
	archer.attack_Cooldown = archer_Stats_Array[level].attack_Cooldown;
	archer.current_Attack_Cooldown = 0.0f;
	archer.attack_Range = archer_Stats_Array[level].attack_Range;

	archer.destroyed = false;
	archer.stop = false;

	V2 direction_V2 = calculate_Direction_V2(target_Position, spawn_Position);

	// Set the new velocity
	archer.rigid_Body.velocity.x = direction_V2.x * archer_Stats_Array[level].speed;
	archer.rigid_Body.velocity.y = direction_V2.y * archer_Stats_Array[level].speed;

	float radius = get_Sprite_Radius(&archer.sprite_Sheet_Tracker);

	add_Collider(&archer.rigid_Body, { 0.0f, -(radius / 2) }, (radius / 2));
	add_Collider(&archer.rigid_Body, { 0.0f, 0.0f }, (radius / 2));
	add_Collider(&archer.rigid_Body, { 0.0f, (radius / 2) }, (radius / 2));

	game_Data->player_Archers.push_back(archer);
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
		V2 world_Position = get_WS_Position(rigid_Body, collider);
		draw_Circle(world_Position.x, world_Position.y, collider->radius, color);
	}
}

void outline_Rect(SDL_Rect* rect, int outline_Thickness) {
	SDL_Rect top_Rect = {};
	top_Rect.x = rect->x;
	top_Rect.y = rect->y;
	top_Rect.w = rect->w;
	top_Rect.h = outline_Thickness;
	SDL_RenderFillRect(Globals::renderer, &top_Rect);

	SDL_Rect bottom_Rect = {};
	bottom_Rect.x = rect->x;
	bottom_Rect.y = ((rect->y + rect->h) - outline_Thickness);
	bottom_Rect.w = rect->w;
	bottom_Rect.h = outline_Thickness;
	SDL_RenderFillRect(Globals::renderer, &bottom_Rect);

	SDL_Rect left_Rect = {};
	left_Rect.x = rect->x;
	left_Rect.y = rect->y;
	left_Rect.w = outline_Thickness;
	left_Rect.h = rect->h;
	SDL_RenderFillRect(Globals::renderer, &left_Rect);

	SDL_Rect right_Rect = {};
	right_Rect.x = ((rect->x + rect->w) - outline_Thickness);
	right_Rect.y = rect->y;
	right_Rect.w = outline_Thickness;
	right_Rect.h = rect->h;
	SDL_RenderFillRect(Globals::renderer, &right_Rect);

	SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(Globals::renderer, rect);
}

void draw_HP_Bar(V2* position, Health_Bar* health_Bar) {
	float remaining_HP_Percent = (health_Bar->current_HP / health_Bar->max_HP);
	if (remaining_HP_Percent < 0) {
		remaining_HP_Percent = 0;
	}

	// Lerp of T = A * (1 - T) + B * T
	// A is the left side, B is the right side, T is the health %
	float lerp = linear_Interpolation(0, (float)health_Bar->width, remaining_HP_Percent);

	SDL_Rect rect_Green = {};
	rect_Green.w = (int)lerp;
	rect_Green.h = (int)health_Bar->height;
	rect_Green.x = (int)((position->x) - health_Bar->width / 2);
	rect_Green.y = (int)((position->y) - health_Bar->y_Offset);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &rect_Green);

	SDL_Rect rect_Red = rect_Green;
	rect_Red.w = health_Bar->width - rect_Green.w;
	rect_Red.x = (int)(rect_Green.x + rect_Green.w);
	SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(Globals::renderer, &rect_Red);

	// Outline HP bars
	SDL_Rect outline = {};
	outline.w = (int)health_Bar->width;
	outline.h = (int)health_Bar->height;
	outline.x = (int)((position->x) - health_Bar->width / 2);
	outline.y = (int)((position->y) - health_Bar->y_Offset);
	SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	outline_Rect(&outline, health_Bar->thickness);
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
		V2 world_Position = get_WS_Position(rigid_Body, collider);

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
		V2 world_Pos_1 = get_WS_Position(rigid_Body_1, collider_1);

		for (int j = 0; j < rigid_Body_2->num_Colliders; j++) {
			Collider* collider_2 = &rigid_Body_2->colliders[j];
			V2 world_Pos_2 = get_WS_Position(rigid_Body_2, collider_2);

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

// Doesn't account for empty height map
float get_Height_Map_Pos_Y(Game_Data* game_Data, int x_Pos) {
	if (x_Pos < 0) {
		x_Pos = 0;
	}
	if (x_Pos >= game_Data->terrain_Height_Map.size()) {
		x_Pos = (int)(game_Data->terrain_Height_Map.size() - 1);
	}
	return (float)game_Data->terrain_Height_Map[x_Pos];
}
