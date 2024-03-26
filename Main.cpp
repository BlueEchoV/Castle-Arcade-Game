#include <SDL.h>
#include <vector>
#include <string>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Utility.h"
#include "Entity_Data.h"

#define RESOLUTION_WIDTH 1920
#define RESOLUTION_HEIGHT 1080

const float GRAVITY = 300;

bool temp_Bool = false;

SDL_Renderer* renderer = NULL;

struct Color {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
};

struct Image {
	int width;
	int height;
    SDL_Texture* texture;
    unsigned char* pixel_Data;
};

struct Sprite {
	SDL_Rect source_Rect;
	float radius;
	Image* image;
};

enum class Button_State {
    NORMAL,
    HOVERED,
    IS_HOT,
    PRESSED
};

enum class Select_Color {
    BLACK,
    RED,
    GREEN,
    BLUE
};

enum class Current_Game_State {
    MENU,
    GAMELOOP
};

struct Font {
    int width;
    int height;
    SDL_Texture* texture;
    int char_Height;
    int char_Width;
};

struct Button {
    SDL_Rect rect;
    Font* font;
    Button_State state;
    const char* string;
};

struct Health_Bar {
	float max_HP;
	float current_HP;
    int width;
    int height;
    int y_Offset;
    int thickness;
};

struct Collider {
	Vector position_LS;
	float radius;
};

struct Rigid_Body {
    bool rigid_Body_Faces_Velocity;
    Vector position_WS;
    Vector velocity;
    float angle;
    std::vector<Collider> colliders;
};

struct Sprite_Sheet {
	int rows;
	int columns;
	std::vector<Sprite> sprites;
    int num_Of_Frames;
};

enum Sprite_Sheet_Selector {
    SKELETON_WALKING,
    SKELETON_STOP,
    SKELETON_ATTACKING,
    SKELETON_DYING,

	ARCHER_WALKING,
	ARCHER_STOP,
	ARCHER_ATTACKING,
	ARCHER_DYING,

    ARROW_DEFAULT,

	CASTLE_1,

    BKG_GAMELOOP_1,
    BKG_MENU_1,

    TERRAIN_1,

	TOTAL_SPRITE_SHEETS
};

Sprite_Sheet sprite_Sheet_Array[TOTAL_SPRITE_SHEETS] = {};

struct Sprite_Sheet_Tracker {
	// Make it const because nothing should change these values
	const Sprite_Sheet* sprite_Sheet_Array;
    Sprite_Sheet_Selector selected;
	float last_Frame_Update_Time;
	int current_Frame;
};

Sprite_Sheet_Tracker create_Sprite_Sheet_Tracker(Sprite_Sheet_Selector selected) {
    Sprite_Sheet_Tracker result;
    
    result.sprite_Sheet_Array = sprite_Sheet_Array;
    result.selected = selected;
    result.last_Frame_Update_Time = 0.0f;
    result.current_Frame = 0;

    return result;
}

enum Level {
	LEVEL_1,
	LEVEL_2,
	LEVEL_3,
	LEVEL_4,
	LEVEL_5,
	TOTAL_LEVELS
};                                      

struct Castle_Stats {
    float hp;
    float fire_Cooldown;
	float current_Fire_Cooldown;
	float spawn_Cooldown;
	float current_Spawn_Cooldown;
};

const Castle_Stats castle_Stats_Array[TOTAL_LEVELS] = {
	{.hp = 100.0f, .fire_Cooldown = 0.01f, .current_Fire_Cooldown = 0.0f, .spawn_Cooldown = 1.0f, .current_Spawn_Cooldown = 0.0f},
	{.hp = 100.0f, .fire_Cooldown = 1.0f, .current_Fire_Cooldown = 0.0f, .spawn_Cooldown = 1.0f, .current_Spawn_Cooldown = 0.0f},
	{.hp = 100.0f, .fire_Cooldown = 1.0f, .current_Fire_Cooldown = 0.0f, .spawn_Cooldown = 1.0f, .current_Spawn_Cooldown = 0.0f},
	{.hp = 100.0f, .fire_Cooldown = 1.0f, .current_Fire_Cooldown = 0.0f, .spawn_Cooldown = 1.0f, .current_Spawn_Cooldown = 0.0f},
	{.hp = 100.0f, .fire_Cooldown = 1.0f, .current_Fire_Cooldown = 0.0f, .spawn_Cooldown = 1.0f, .current_Spawn_Cooldown = 0.0f}
};

struct Castle {
    const Castle_Stats* castle_Stats_Array;

    Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;
	Health_Bar health_Bar;

	float fire_Cooldown;
	float current_Fire_Cooldown;
	float spawn_Cooldown;
	float current_Spawn_Cooldown;
};

struct Arrow_Stats {
	float speed;
	float damage;
	float life_Time;
};

const Arrow_Stats arrow_Stats_Array[TOTAL_LEVELS] = {
	{.speed = 800, .damage = 20, .life_Time = 10},
	{.speed = 800, .damage = 25, .life_Time = 10},
	{.speed = 800, .damage = 30, .life_Time = 10},
	{.speed = 800, .damage = 35, .life_Time = 10},
	{.speed = 800, .damage = 40, .life_Time = 10}
};

struct Arrow {
	const Arrow_Stats* arrow_Stats_Array;

	Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;

	float damage;
	float speed;
	float life_Time;

	bool stop;
	bool destroyed;
};

struct Skeleton_Stats {
	float speed;
	float damage;
	float max_HP;
	float current_HP;
	float attack_Cooldown;
	float attack_Range;
};

const Skeleton_Stats skeleton_Stats_Array[TOTAL_LEVELS] = {
	{.speed = 100, .damage = 20, .max_HP = 100, .attack_Cooldown = 1, .attack_Range = 150},
	{.speed = 200, .damage = 25, .max_HP = 125, .attack_Cooldown = 1, .attack_Range = 150},
	{.speed = 300, .damage = 30, .max_HP = 150, .attack_Cooldown = 1, .attack_Range = 150},
	{.speed = 400, .damage = 35, .max_HP = 175, .attack_Cooldown = 1, .attack_Range = 150},
	{.speed = 500, .damage = 40, .max_HP = 200, .attack_Cooldown = 1, .attack_Range = 150}
};

struct Skeleton {
	const Skeleton_Stats* skeleton_Stats_Array;

	Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;

	Health_Bar health_Bar;

	float speed;
	float damage;
	float attack_Cooldown;
	float current_Attack_Cooldown;
	float attack_Range;

	bool destroyed;
	bool stop;
};

struct Archer_Stats {
	float speed;
	float damage;
	float hp;
    float attack_Cooldown;
    float current_Attack_Cooldown;
    float attack_Range;
};

const Archer_Stats archer_Stats_Array[TOTAL_LEVELS] = {
	{.speed = 100, .damage = 10, .hp = 100, .attack_Cooldown = 1, .current_Attack_Cooldown = 0.0, .attack_Range = 750},
	{.speed = 150, .damage = 15, .hp = 125, .attack_Cooldown = 1, .current_Attack_Cooldown = 0.0, .attack_Range = 750},
	{.speed = 200, .damage = 20, .hp = 150, .attack_Cooldown = 1, .current_Attack_Cooldown = 0.0, .attack_Range = 750},
	{.speed = 250, .damage = 25, .hp = 175, .attack_Cooldown = 1, .current_Attack_Cooldown = 0.0, .attack_Range = 750},
	{.speed = 300, .damage = 30, .hp = 200, .attack_Cooldown = 1, .current_Attack_Cooldown = 0.0, .attack_Range = 750}
};

struct Archer {
    const Archer_Stats* archer_Stats_Array;

    Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;

	Health_Bar health_Bar;

	float speed;
	float damage;
    float attack_Cooldown;
	float current_Attack_Cooldown;
	float attack_Range;

	bool destroyed;
	bool stop;
};

struct Game_Data {
    Castle                      player_Castle;
    Castle                      enemy_Castle;
    std::vector<int>            terrain_Height_Map;
    std::vector<Arrow>          player_Arrows;
	std::vector<Skeleton>       enemy_Skeletons;
	std::vector<Skeleton>       player_Skeletons;
    std::vector<Archer>         player_Archers;
};

float return_Sprite_Radius(Sprite sprite) {
	float max_Distance = 0;
	for (int y = sprite.source_Rect.y; y < (sprite.source_Rect.h + sprite.source_Rect.y); y++) {
		for (int x = sprite.source_Rect.x; x < (sprite.source_Rect.w + sprite.source_Rect.x); x++) {
			int index = 0;
			index = (4 * ((y * sprite.image->width) + x)) + 3;
			if (sprite.image->pixel_Data[index] != 0) {
				float distance = 
                    calculate_Distance(
                        (float)x, 
                        (float)y, 
                        (float)(sprite.source_Rect.x + sprite.source_Rect.w / 2), 
                        (float)(sprite.source_Rect.y + sprite.source_Rect.h / 2)
                    );
				if (distance > max_Distance) {
					max_Distance = distance;
				}
			}
		}
	}
	return max_Distance;
}

Image create_Image(const char* file_name) {
    Image result = {};

    int width, height, channels;
    unsigned char* data = stbi_load(file_name, &width, &height, &channels, 4);

    if (data == NULL) {
        SDL_Log("ERROR: stbi_load returned NULL");
        return result;
    }

    result.pixel_Data = data;

	result.width = width;
	result.height = height;

	DEFER{
		// stbi_image_free(data);
	};

	SDL_Texture* temp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, result.width, result.height);

	if (temp == NULL) {
		SDL_Log("ERROR: SDL_CreateTexture returned NULL: %s", SDL_GetError());
		return result;
	}

	if (SDL_SetTextureBlendMode(temp, SDL_BLENDMODE_BLEND) != 0) {
		SDL_Log("ERROR: SDL_SsetTextureBlendMode did not succeed: %s", SDL_GetError());
	}

	result.texture = temp;

	void* pixels;
	int pitch;
	SDL_LockTexture(temp, NULL, &pixels, &pitch);

	my_Memory_Copy(pixels, data, (result.width * result.height) * 4);

	SDL_UnlockTexture(temp);

	return result;
}

// Operator overload
// Multiply this vector by this scalar (0.5)
Vector calculate_Center(float w, float h) {
    Vector result = { w / 2, h / 2 };
    return result;
}


Sprite create_Sprite(Image* image, SDL_Rect* source_Rect) {
    Sprite result = {};

    result.image = image;
    // This is the width and height of the individual sprite
    result.source_Rect = *source_Rect;
	result.radius = return_Sprite_Radius(result);

    return result;
}

Sprite_Sheet create_Sprite_Sheet(Image* image, int rows, int columns) {
	Sprite_Sheet result = {
		.rows = rows,
		.columns = columns,
	};
    result.num_Of_Frames = rows * columns;
	for (int c = 0; c < columns; ++c)
	{
		for (int r = 0; r < rows; ++r)
		{
			SDL_Rect source;
			source.x = (c * (image->width / columns));
			source.y = (r * (image->height / rows));
			source.w = (image->width / columns);
			source.h = (image->height / rows);
			result.sprites.push_back(create_Sprite(image, &source));
		}
	}
	return result;
}

void add_Sprite_Sheet_To_Array(Sprite_Sheet_Selector selected, Image* image, int rows, int columns) {
    sprite_Sheet_Array[selected].rows = rows;
    sprite_Sheet_Array[selected].columns = columns;
    sprite_Sheet_Array[selected].num_Of_Frames = rows * columns;
	for (int c = 0; c < columns; ++c)
	{
		for (int r = 0; r < rows; ++r)
		{
			SDL_Rect source;
			source.x = (c * (image->width / columns));
			source.y = (r * (image->height / rows));
			source.w = (image->width / columns);
			source.h = (image->height / rows);
            sprite_Sheet_Array[selected].sprites.push_back(create_Sprite(image, &source));
		}
	}
}

void add_Collider(std::vector<Collider>* colliders, Vector position_LS, float radius) {
    Collider collider = {};
    collider.position_LS = position_LS;
    collider.radius = radius;
    
    colliders->push_back(collider);
}

void draw_Layer(SDL_Texture* texture) {
    SDL_RenderCopyEx(renderer, texture, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
}

void draw_Castle(Castle* castle, bool flip) {
    SDL_Rect temp = {};
	SDL_Rect* src_Rect = const_cast<SDL_Rect*>(&castle->sprite_Sheet_Tracker.sprite_Sheet_Array[castle->sprite_Sheet_Tracker.selected].sprites[0].source_Rect);
    Vector sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
    sprite_Half_Size = sprite_Half_Size / 2;
    temp = { 
		((int)castle->rigid_Body.position_WS.x - (int)sprite_Half_Size.x),
		((int)castle->rigid_Body.position_WS.y - (int)sprite_Half_Size.y),
        castle->sprite_Sheet_Tracker.sprite_Sheet_Array[castle->sprite_Sheet_Tracker.selected].sprites[0].source_Rect.w,
        castle->sprite_Sheet_Tracker.sprite_Sheet_Array[castle->sprite_Sheet_Tracker.selected].sprites[0].source_Rect.h
    };
    SDL_RenderCopyEx(renderer, castle->sprite_Sheet_Tracker.sprite_Sheet_Array[castle->sprite_Sheet_Tracker.selected].sprites[0].image->texture, NULL, &temp, 0, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}


void draw_Arrow(Arrow* arrow, bool flip) {
    SDL_Rect temp = {};
    // Cast off the const
    Sprite* sprite = const_cast<Sprite*>(&arrow->sprite_Sheet_Tracker.sprite_Sheet_Array[arrow->sprite_Sheet_Tracker.selected].sprites[0]);
    SDL_Rect* src_Rect = &sprite->source_Rect;
	Vector sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
	sprite_Half_Size = sprite_Half_Size / 2;
    temp = {
        ((int)arrow->rigid_Body.position_WS.x - (int)sprite_Half_Size.x),
        ((int)arrow->rigid_Body.position_WS.y - (int)sprite_Half_Size.y),
		sprite->source_Rect.w,
		sprite->source_Rect.h
    };
    SDL_RenderCopyEx(renderer, sprite->image->texture, NULL, &temp, arrow->rigid_Body.angle, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

void update_Animation(Sprite_Sheet_Tracker* tracker, float unit_Speed, float delta_Time) {
    if (unit_Speed > 0) {
        float frames_Per_Second = unit_Speed / 25;
        float conversion_Frames = 1 / frames_Per_Second;
        // It's more precise to record a start time and subtract from
        // the new frames value
        tracker->last_Frame_Update_Time += delta_Time;

        // I can overshoot with delta_Time and it won't be 100% accurate
        // Modulus with floating point captures that overshot value
        if (tracker->last_Frame_Update_Time >= conversion_Frames) {
            tracker->current_Frame++;
            tracker->last_Frame_Update_Time = 0;
            if (tracker->current_Frame >= tracker->sprite_Sheet_Array[tracker->selected].num_Of_Frames) {
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

    const Sprite_Sheet* sprite_Sheet = &tracker->sprite_Sheet_Array[tracker->selected];
	SDL_Rect current_Frame_Rect = sprite_Sheet->sprites[sprite_Frame].source_Rect;

	const SDL_Rect* src_Rect = &sprite_Sheet->sprites[0].source_Rect;
	Vector sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
	sprite_Half_Size = sprite_Half_Size / 2;

	SDL_Rect destination_Rect = {
		(int)(rigid_Body->position_WS.x - (int)sprite_Half_Size.x),
		(int)(rigid_Body->position_WS.y - (int)sprite_Half_Size.y),
		current_Frame_Rect.w,
		current_Frame_Rect.h
	};

    Vector pivot = calculate_Center(
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
		renderer,
        sprite_Sheet->sprites[sprite_Frame].image->texture,
		&current_Frame_Rect,
		&destination_Rect,
		rigid_Body->angle,
		&pivot_Point,
		(flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)
	);
}

/*
void draw_Unit_Animated(Rigid_Body* rigid_Body, Unit_Animation_Data* Unit_Animation_Data, float play_Speed, bool flip) {
	if (play_Speed <= 0) {
		// SDL_Log("ERROR: draw_Unit_Animated() - Animation speed <= 0");
		return;
	}
	Uint64 ticks = SDL_GetTicks64();
	// Convert to percent
	float play_Speed_Percent = (play_Speed / 100.0f);
	if (play_Speed_Percent > 1.0f) {
		play_Speed_Percent = 1.0f;
	}
	if (play_Speed_Percent < 0.0f) {
		play_Speed_Percent = 0.0f;
	}
	// Invert
	play_Speed_Percent = 1.0f - play_Speed_Percent;
	float new_Play_Speed = linear_Interpolation(
		(float)Unit_Animation_Data->data.play_Range.min,
		(float)Unit_Animation_Data->data.play_Range.max,
		play_Speed_Percent
	);

	// 1000 / 250 = 4  %  4 = 0
	// 1250 / 250 = 5  %  4 = 1
	// 1500 / 250 = 6  %  4 = 2
    if (new_Play_Speed < 0) {
        SDL_Log("ERROR: draw_Unit_Animated - new_Play_Speed less than 0: %f", new_Play_Speed);
    }
    int ticks_Divided_Speed = (int)((float)ticks / (float)(new_Play_Speed));
	Uint32 sprite_Frame = (ticks_Divided_Speed % (int)Unit_Animation_Data->data.num_Of_Frames);
	if (temp_Bool) {
		SDL_Log("**************************************");
		SDL_Log("ticks = %i", ticks);
		SDL_Log("play_Speed = %f", play_Speed);
		SDL_Log("(ticks / new_Play_Speed) = %i", ticks_Divided_Speed);
		SDL_Log("new_Play_Speed = %f", new_Play_Speed);
		SDL_Log("sprite_Frame = %i", sprite_Frame);
	}

	SDL_Rect current_Frame_Rect = Unit_Animation_Data->data.sprite_Sheet.sprites[sprite_Frame].source_Rect;

	SDL_Rect destination_Rect = {
		(int)(rigid_Body->position_WS.x - Unit_Animation_Data->data.sprite_Sheet.sprites[sprite_Frame].center.x),
		(int)(rigid_Body->position_WS.y - Unit_Animation_Data->data.sprite_Sheet.sprites[sprite_Frame].center.y),
		current_Frame_Rect.w,
		current_Frame_Rect.h
	};

	// Set the center of the rotation
	SDL_Point center = {
		(int)Unit_Animation_Data->data.sprite_Sheet.sprites[sprite_Frame].center.x,
		(int)Unit_Animation_Data->data.sprite_Sheet.sprites[sprite_Frame].center.y
	};

	// Render the current frame of the animation
	SDL_RenderCopyEx(
		renderer,
		Unit_Animation_Data->data.sprite_Sheet.sprites[sprite_Frame].image->texture,
		&current_Frame_Rect,
		&destination_Rect,
		rigid_Body->angle,
		&center,
		(flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)
	);
}
*/

Vector get_WS_Position(Rigid_Body* rigid_Body, const Collider* collider) {
	Vector result = rigid_Body->position_WS;

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
		arrow->rigid_Body.velocity.y += GRAVITY * delta_Time;

		arrow->rigid_Body.position_WS.x += arrow->rigid_Body.velocity.x * delta_Time;
		arrow->rigid_Body.position_WS.y += arrow->rigid_Body.velocity.y * delta_Time;
	}
}

void update_Unit_Position(Rigid_Body* rigid_Body, bool stop_Unit, float delta_Time) {
    if (!stop_Unit) {
		rigid_Body->position_WS.y += GRAVITY * delta_Time;
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

Rigid_Body create_Rigid_Body(Vector position_WS, bool rigid_Body_Faces_Velocity) {
	Rigid_Body result = {};

	result.rigid_Body_Faces_Velocity = rigid_Body_Faces_Velocity;
	result.position_WS = { position_WS.x , position_WS.y };
	result.velocity = { 0.0f, 0.0f };
	result.angle = 0.0f;
	result.colliders = {};

	return result;
}

void spawn_Player_Castle(Sprite_Sheet_Selector selector, Game_Data* game_Data, Vector position_WS, Level level) {
    Castle castle = {};

    castle.castle_Stats_Array = castle_Stats_Array;

	castle.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(selector);

    castle.rigid_Body = create_Rigid_Body(position_WS, false);
	
    castle.health_Bar = create_Health_Bar(90, 20, 115, 3, castle.castle_Stats_Array[level].hp);

    castle.fire_Cooldown = castle.castle_Stats_Array[level].fire_Cooldown;
	castle.current_Fire_Cooldown = castle.castle_Stats_Array[level].current_Fire_Cooldown;
	castle.spawn_Cooldown = castle.castle_Stats_Array[level].spawn_Cooldown;
	castle.current_Spawn_Cooldown = castle.castle_Stats_Array[level].current_Spawn_Cooldown;

	add_Collider(&castle.rigid_Body.colliders, { 0.0f, 0.0f }, castle.sprite_Sheet_Tracker.sprite_Sheet_Array[selector].sprites[0].radius);

    game_Data->player_Castle = castle;
}

void spawn_Enemy_Castle(Sprite_Sheet_Selector selector, Game_Data* game_Data, Vector position_WS, Level level) {
	Castle castle = {};

	castle.castle_Stats_Array = castle_Stats_Array;

	castle.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(selector);

	castle.rigid_Body = create_Rigid_Body(position_WS, false);

	castle.health_Bar = create_Health_Bar(90, 20, 115, 3, castle.castle_Stats_Array[level].hp);

	castle.fire_Cooldown = castle.castle_Stats_Array[level].fire_Cooldown;
	castle.current_Fire_Cooldown = castle.castle_Stats_Array[level].current_Fire_Cooldown;
	castle.spawn_Cooldown = castle.castle_Stats_Array[level].spawn_Cooldown;
	castle.current_Spawn_Cooldown = castle.castle_Stats_Array[level].current_Spawn_Cooldown;

	add_Collider(&castle.rigid_Body.colliders, { 0.0f, 0.0f }, castle.sprite_Sheet_Tracker.sprite_Sheet_Array[selector].sprites[0].radius);

	game_Data->enemy_Castle = castle;
}

void spawn_Arrow(Game_Data* game_Data, Vector* spawn_Position, Vector* target_Position, Level level) {
    Arrow arrow = {};
    arrow.arrow_Stats_Array = arrow_Stats_Array;

	arrow.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(ARROW_DEFAULT);

    arrow.rigid_Body = create_Rigid_Body(*spawn_Position, true);

	arrow.damage = arrow_Stats_Array[level].damage;
	arrow.speed = arrow_Stats_Array[level].speed;
	arrow.life_Time = arrow_Stats_Array[level].life_Time;

    arrow.stop = false;
    arrow.destroyed = false;

    Vector direction_Vector = {};
    direction_Vector.x = target_Position->x - arrow.rigid_Body.position_WS.x;
    direction_Vector.y = target_Position->y - arrow.rigid_Body.position_WS.y;
    
    float length = (float)sqrt((direction_Vector.x * direction_Vector.x) + (direction_Vector.y * direction_Vector.y));

    direction_Vector.x /= length;
    direction_Vector.y /= length;

    arrow.rigid_Body.velocity.x = direction_Vector.x * arrow.speed;
    arrow.rigid_Body.velocity.y = direction_Vector.y * arrow.speed;

	add_Collider(
		&arrow.rigid_Body.colliders,
		{ (arrow.sprite_Sheet_Tracker.sprite_Sheet_Array[arrow.sprite_Sheet_Tracker.selected].sprites[0].radius * 0.75f), 0.0f },
		(arrow.sprite_Sheet_Tracker.sprite_Sheet_Array[arrow.sprite_Sheet_Tracker.selected].sprites[0].radius * 0.25f)
	);

    game_Data->player_Arrows.push_back(arrow);
}

void spawn_Player_Skeleton(Game_Data* game_Data, Vector spawn_Location, Rigid_Body* target_Rigid_Body, Level level) {
	Skeleton skeleton = {};

    skeleton.skeleton_Stats_Array = skeleton_Stats_Array;

    skeleton.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(SKELETON_WALKING);

	skeleton.rigid_Body = create_Rigid_Body(spawn_Location, false);

    skeleton.health_Bar = create_Health_Bar(50, 13, 60, 2, skeleton.skeleton_Stats_Array[level].max_HP);

    skeleton.speed = skeleton.skeleton_Stats_Array[level].speed;
    skeleton.damage = skeleton.skeleton_Stats_Array[level].damage;
    skeleton.attack_Cooldown = skeleton.skeleton_Stats_Array[level].attack_Cooldown;
    skeleton.current_Attack_Cooldown = 0.0f;
	skeleton.attack_Range = skeleton.skeleton_Stats_Array[level].attack_Range;

	skeleton.destroyed = false;
	skeleton.stop = false;

	Vector direction_Vector = {};
	direction_Vector.x = target_Rigid_Body->position_WS.x - skeleton.rigid_Body.position_WS.x;
	direction_Vector.y = target_Rigid_Body->position_WS.y - skeleton.rigid_Body.position_WS.y;

	// Magnitude of the Vector
	float length = (float)sqrt((direction_Vector.x * direction_Vector.x) + (direction_Vector.y * direction_Vector.y));

	// Normalize
	direction_Vector.x /= length;
	direction_Vector.y /= length;

	// Set the new velocity
	skeleton.rigid_Body.velocity.x = direction_Vector.x * skeleton.skeleton_Stats_Array[level].speed;
	skeleton.rigid_Body.velocity.y = direction_Vector.y * skeleton.skeleton_Stats_Array[level].speed;

	const Sprite* skeleton_Sprite_Radius = &skeleton.sprite_Sheet_Tracker.sprite_Sheet_Array[skeleton.sprite_Sheet_Tracker.selected].sprites[0];
	add_Collider(&skeleton.rigid_Body.colliders, { 0.0f, -(skeleton_Sprite_Radius->radius / 2) }, (skeleton_Sprite_Radius->radius / 2));
	add_Collider(&skeleton.rigid_Body.colliders, { 0.0f, 0.0f }, (skeleton_Sprite_Radius->radius / 2));
	add_Collider(&skeleton.rigid_Body.colliders, { 0.0f, (skeleton_Sprite_Radius->radius / 2) }, (skeleton_Sprite_Radius->radius / 2));

	game_Data->player_Skeletons.push_back(skeleton);
}


void spawn_Enemy_Skeleton(Game_Data* game_Data, Vector spawn_Location, Rigid_Body* target_Rigid_Body, Level level) {
	Skeleton skeleton = {};

	skeleton.skeleton_Stats_Array = skeleton_Stats_Array;

	skeleton.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(SKELETON_WALKING);

	skeleton.rigid_Body = create_Rigid_Body(spawn_Location, false);

	skeleton.health_Bar = create_Health_Bar(50, 13, 60, 2, skeleton.skeleton_Stats_Array[level].max_HP);

	skeleton.speed = skeleton.skeleton_Stats_Array[level].speed;
	skeleton.damage = skeleton.skeleton_Stats_Array[level].damage;
	skeleton.attack_Cooldown = skeleton.skeleton_Stats_Array[level].attack_Cooldown;
	skeleton.current_Attack_Cooldown = 0.0f;
	skeleton.attack_Range = skeleton.skeleton_Stats_Array[level].attack_Range;

	skeleton.destroyed = false;
	skeleton.stop = false;

	Vector direction_Vector = {};
	direction_Vector.x = target_Rigid_Body->position_WS.x - skeleton.rigid_Body.position_WS.x;
	direction_Vector.y = target_Rigid_Body->position_WS.y - skeleton.rigid_Body.position_WS.y;

	// Magnitude of the Vector
	float length = (float)sqrt((direction_Vector.x * direction_Vector.x) + (direction_Vector.y * direction_Vector.y));

	// Normalize
	direction_Vector.x /= length;
	direction_Vector.y /= length;

	// Set the new velocity
	skeleton.rigid_Body.velocity.x = direction_Vector.x * skeleton.skeleton_Stats_Array[level].speed;
	skeleton.rigid_Body.velocity.y = direction_Vector.y * skeleton.skeleton_Stats_Array[level].speed;

	const Sprite* skeleton_Sprite_Radius = &skeleton.sprite_Sheet_Tracker.sprite_Sheet_Array[skeleton.sprite_Sheet_Tracker.selected].sprites[0];
	add_Collider(&skeleton.rigid_Body.colliders, { 0.0f, -(skeleton_Sprite_Radius->radius / 2) }, (skeleton_Sprite_Radius->radius / 2));
	add_Collider(&skeleton.rigid_Body.colliders, { 0.0f, 0.0f }, (skeleton_Sprite_Radius->radius / 2));
	add_Collider(&skeleton.rigid_Body.colliders, { 0.0f, (skeleton_Sprite_Radius->radius / 2) }, (skeleton_Sprite_Radius->radius / 2));

	game_Data->enemy_Skeletons.push_back(skeleton);
}

void spawn_Archer(Game_Data* game_Data, Vector spawn_Location, Rigid_Body* target_Rigid_Body, Level level) {
	Archer archer = {};

	archer.archer_Stats_Array = archer_Stats_Array;

	archer.health_Bar = create_Health_Bar(50, 13, 60, 2, archer.archer_Stats_Array[level].hp);

	archer.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(ARCHER_WALKING);

	archer.rigid_Body = create_Rigid_Body(spawn_Location, false);

    archer.speed = archer.archer_Stats_Array[level].speed;
    archer.damage = archer.archer_Stats_Array[level].damage;
    archer.attack_Cooldown = archer.archer_Stats_Array[level].attack_Cooldown;
    archer.current_Attack_Cooldown = 0.0f;
    archer.attack_Range = archer.archer_Stats_Array[level].attack_Range;

    archer.destroyed = false;
    archer.stop = false;
    
	Vector direction_Vector = {};
	direction_Vector.x = target_Rigid_Body->position_WS.x - archer.rigid_Body.position_WS.x;
	direction_Vector.y = target_Rigid_Body->position_WS.y - archer.rigid_Body.position_WS.y;

	// Magnitude of the Vector
	float length = (float)sqrt((direction_Vector.x * direction_Vector.x) + (direction_Vector.y * direction_Vector.y));

	// Normalize
	direction_Vector.x /= length;
	direction_Vector.y /= length;

	// Set the new velocity
    archer.rigid_Body.velocity.x = direction_Vector.x * archer.archer_Stats_Array[level].speed;
    archer.rigid_Body.velocity.y = direction_Vector.y * archer.archer_Stats_Array[level].speed;

	const Sprite* archer_Sprite_Radius = &archer.sprite_Sheet_Tracker.sprite_Sheet_Array[archer.sprite_Sheet_Tracker.selected].sprites[0];
	add_Collider(&archer.rigid_Body.colliders, { 0.0f, -(archer_Sprite_Radius->radius / 2) }, (archer_Sprite_Radius->radius / 2));
	add_Collider(&archer.rigid_Body.colliders, { 0.0f, 0.0f }, (archer_Sprite_Radius->radius / 2));
	add_Collider(&archer.rigid_Body.colliders, { 0.0f, (archer_Sprite_Radius->radius / 2) }, (archer_Sprite_Radius->radius / 2));

    game_Data->player_Archers.push_back(archer);
}

void draw_Circle(float center_X, float center_Y, float radius, Select_Color color) {
    float total_Lines = 30;
    float line_Step = (float)(2 * M_PI) / total_Lines;
    int x1, y1, x2, y2 = 0;

    for (float angle = 0; angle < (2 * M_PI); angle += line_Step) {
        // sin and cos gives us the ratio of the length
        x1 = (int)(center_X + (radius * cos(angle - line_Step)));
        y1 = (int)(center_Y + (radius * sin(angle - line_Step)));
        x2 = (int)(center_X + (radius * cos(angle)));
        y2 = (int)(center_Y + (radius * sin(angle)));
        if (color == Select_Color::RED) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        } else if (color == Select_Color::GREEN) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        } else if (color == Select_Color::BLUE) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
        }
        else {
            // Yellow is default
            SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
        }
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}
}

void draw_RigidBody_Colliders(Rigid_Body* rigid_Body, Select_Color color) {
	for (const auto& collider : rigid_Body->colliders) {
		Vector world_Position = get_WS_Position(rigid_Body, &collider);
		draw_Circle(world_Position.x, world_Position.y, collider.radius, color);
	}
}

void outline_Rect(SDL_Rect* rect, int outline_Thickness) {
    SDL_Rect top_Rect = {};
    top_Rect.x = rect->x;
    top_Rect.y = rect->y;
    top_Rect.w = rect->w;
    top_Rect.h = outline_Thickness;
    SDL_RenderFillRect(renderer, &top_Rect);

    SDL_Rect bottom_Rect = {};
    bottom_Rect.x = rect->x;
    bottom_Rect.y = ((rect->y + rect->h) - outline_Thickness);
    bottom_Rect.w = rect->w;
    bottom_Rect.h = outline_Thickness;
    SDL_RenderFillRect(renderer, &bottom_Rect);

    SDL_Rect left_Rect = {};
    left_Rect.x = rect->x;
    left_Rect.y = rect->y;
    left_Rect.w = outline_Thickness;
    left_Rect.h = rect->h;
    SDL_RenderFillRect(renderer, &left_Rect);

    SDL_Rect right_Rect = {};
    right_Rect.x = ((rect->x + rect->w) - outline_Thickness);
    right_Rect.y = rect->y;
    right_Rect.w = outline_Thickness;
    right_Rect.h = rect->h;
    SDL_RenderFillRect(renderer, &right_Rect);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawRect(renderer, rect);
}

void draw_HP_Bar(Vector* position, Health_Bar* health_Bar) {
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
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect_Green);

    SDL_Rect rect_Red = rect_Green;
    rect_Red.w = health_Bar->width - rect_Green.w;
    rect_Red.x = (int)(rect_Green.x + rect_Green.w);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect_Red);

    // Outline HP bars
    SDL_Rect outline = {};
    outline.w = (int)health_Bar->width;
    outline.h = (int)health_Bar->height;
	outline.x = (int)((position->x) - health_Bar->width / 2);
	outline.y = (int)((position->y) - health_Bar->y_Offset);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    outline_Rect(&outline, health_Bar->thickness);
}

Font load_Font_Bitmap(const char* font_File_Path) {
    Font result = {};

    int width, height, channels;
    unsigned char* data = stbi_load(font_File_Path, &width, &height, &channels, 4);
    DEFER{
        stbi_image_free(data);
    };

    result.width = width;
    result.height = height;

    for (int y = 0; y < result.height; y++) {
        for (int x = 0; x < result.width; x++) {
            int index = 0;
            index = (4 * ((y * width) + x));
            // Check if the color is black
            if (data[index] == 0 && data[index + 1] == 0 && data[index + 2] == 0) {
                // Set alpha to 0 (Transparent)
                data[index + 3] = 0;
            }
        }
    }

    // Hard coded values (Potentially bad)
    // This function is for bitmap fonts so 
    // this could be okay.
    result.char_Width = result.width / 18;
    result.char_Height = result.height / 7;

    SDL_Texture* temp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    if (temp == NULL) {
        SDL_Log("ERROR: SDL_CreateTexture returned NULL: %s", SDL_GetError());
        return result;
    }

    if (SDL_SetTextureBlendMode(temp, SDL_BLENDMODE_BLEND) != 0) {
        SDL_Log("ERROR: SDL_SsetTextureBlendMode did not succeed: %s", SDL_GetError());
    }

    result.texture = temp;

    void* pixels;
    int pitch;
    SDL_LockTexture(temp, NULL, &pixels, &pitch);

    // Copy what's in my data into the pixels
    // my_Memory_Copy();
    // memcpy(pixels, data, (width * height) * 4);
    my_Memory_Copy(pixels, data, (width * height) * 4);

    SDL_UnlockTexture(temp);

    return result;
}

void draw_Character(Font* font, char character, int position_X, int position_Y, int size) {
    int ascii_Dec = (int)character - (int)' ';
    int chars_Per_Row = (font->width / font->char_Width);

    SDL_Rect src_Rect = {};
    // Position in the row
    src_Rect.x = (ascii_Dec % chars_Per_Row) * font->char_Width;
    // Which row the char is in
    src_Rect.y = (ascii_Dec / chars_Per_Row) * font->char_Height;
    src_Rect.w = font->char_Width;
    src_Rect.h = font->char_Height;

    SDL_Rect dest_Rect = {};
    // Position of the character on the screen
    dest_Rect.x = position_X;
    dest_Rect.y = position_Y;
    dest_Rect.w = (int)(font->char_Width * size);
    dest_Rect.h = (int)(font->char_Height * size);
    
    SDL_RenderCopyEx(renderer, font->texture, &src_Rect, &dest_Rect, 0, NULL, SDL_FLIP_NONE);
}

void draw_String(Font* font, const char* string, int position_X, int position_Y, int size, bool center) {
    int offset_X = 0;
    int index = 0;
    char iterator = string[index];
    size_t length_Pixels = strlen(string);
    length_Pixels *= font->char_Width * size;

    int char_Position_X = 0;
    int char_Position_Y = 0;
    if (center) {
        char_Position_X = (position_X - (int)(length_Pixels / 2));
        char_Position_Y = (position_Y - ((font->char_Height * size) / 2));
    }
    else {
        char_Position_X = position_X;
        char_Position_Y = position_Y;
    }

    while (iterator != '\0') {
        char_Position_X += offset_X;
        draw_Character(font, iterator, char_Position_X, char_Position_Y, size);
        offset_X = font->char_Width * size;
        index++;
        iterator = string[index];
    }
}

void draw_String_With_Background(Font* font, const char* string, int position_X, int position_Y, int size, bool center, Select_Color color, int outline_Padding) {
	size_t length_Pixels = strlen(string);
	length_Pixels *= font->char_Width;
    length_Pixels *= size;
    size_t height_Pixels = font->char_Height;
    height_Pixels *= size;

    SDL_Rect canvas_Area = {};
	// Centers the button
	canvas_Area.x = (int)((position_X - (length_Pixels / 2)) - (outline_Padding / 2));
	canvas_Area.y = (int)((position_Y - (height_Pixels / 2)) - (outline_Padding / 2));
	canvas_Area.w = (int)length_Pixels + outline_Padding;
    canvas_Area.h = (int)height_Pixels + outline_Padding;

	// Set background as black
    if (color == Select_Color::BLACK) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    } else if (color == Select_Color::RED) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	} else if (color == Select_Color::GREEN) {
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	} else if (color == Select_Color::BLUE) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	} else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderFillRect(renderer, &canvas_Area);
    
    draw_String(font, string, position_X, position_Y, size, center);
}

void draw_Timer(Font* font, Vector position, int timer_Size, Select_Color color, int outline_Padding) {
    SDL_Rect temp = {};
    Uint64 time_Elapsed = SDL_GetTicks64() / 1000;

    std::string str = std::to_string(time_Elapsed);
    const char* ptr = str.c_str();
    draw_String_With_Background(font, ptr, (int)(position.x - (font->char_Width / 2)), (int)(position.y - (font->char_Height / 2)), timer_Size, true, color, outline_Padding);
}

bool mouse_Down_This_Frame = false;
std::string current_frame_Hot_Name;
std::string next_Frame_Hot_Name;
bool draw_Rect = false;

bool button(Font* font, const char* string, int position_X, int position_Y, int w, int h, int string_Size) {
	SDL_Rect button_Area = {};
	// Centers the button
	button_Area.x = (position_X - (w / 2));
	button_Area.y = (position_Y - (h / 2));
	button_Area.w = w;
	button_Area.h = h;

	// Set background as black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &button_Area);

	draw_String(font, string, position_X, position_Y, string_Size, true);
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

	int x, y;
	Uint32 mouse = SDL_GetMouseState(&x, &y);
	bool button_Pressed = false;

	bool was_Hot = (current_frame_Hot_Name == string);

	if (x >= button_Area.x && x <= (button_Area.x + button_Area.w)
		&& y >= button_Area.y && y <= (button_Area.y + button_Area.h)) {
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		if (mouse_Down_This_Frame) {
			next_Frame_Hot_Name = string;
		}
		else if (was_Hot && !(mouse & SDL_BUTTON_LMASK)) {
			button_Pressed = true;
		}
		else
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
		}
	}
	if (was_Hot && (mouse & SDL_BUTTON_LMASK)) {
		next_Frame_Hot_Name = string;
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	}

	outline_Rect(&button_Area, 10);

	return button_Pressed;
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
	for (const auto& collider : rigid_Body->colliders) {
		Vector world_Position = get_WS_Position(rigid_Body, &collider);

		int collider_X = (int)world_Position.x;
		int collider_Y = (int)world_Position.y + (int)collider.radius;

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
    for (const auto& collider_1 : rigid_Body_1->colliders) {
        Vector world_Pos_1 = get_WS_Position(rigid_Body_1, &collider_1);

		for (const auto& collider_2 : rigid_Body_2->colliders) {
            Vector world_Pos_2 = get_WS_Position(rigid_Body_2, &collider_2);

			float distance_Between = calculate_Distance(
                world_Pos_1.x, world_Pos_1.y,
                world_Pos_2.x, world_Pos_2.y);
			float radius_Sum = collider_1.radius + collider_2.radius;

			if (distance_Between <= radius_Sum) {
				return true;
			}
		}
	}
	return false;
}

void change_Animation(Sprite_Sheet_Tracker* tracker, Sprite_Sheet_Selector selected) {
	if (tracker->selected != selected) {
        tracker->selected = selected;
        tracker->current_Frame = 0;
    }
}

void draw_Time_Scalar(Font* font, float time_Scalar, int pos_X, int pos_Y, int size) {
    float converted_Time_Scalar;
    if (time_Scalar > 0) {
        converted_Time_Scalar = time_Scalar * 100;
    } else {
        converted_Time_Scalar = 0;
    }
    std::string str_1 = "Time Scalar = %";
    std::string str_2 = std::to_string((int)converted_Time_Scalar);
    std::string str_3 = str_1 + str_2;
    const char* ptr = str_3.c_str();
    draw_String(font, ptr, pos_X, pos_Y, size, true);
}

int main(int argc, char** argv) {
    REF(argc);
    REF(argv);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Castle Defense", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RESOLUTION_WIDTH, RESOLUTION_HEIGHT, 0);
    if (window == NULL) {
        SDL_Log("ERROR: SDL_RenderClear returned returned NULL: %s", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_Log("ERROR: SDL_CreateRenderer returned NULL: %s", SDL_GetError());
        return 1;
    }

    Font font_1 = load_Font_Bitmap("images/font_1.png");

    Image gameloop_BKG_Image = create_Image("images/background.jpg");
    Image menu_BKG_Image = create_Image("images/background_2.png");
    // Image menu_BKG_Image = create_Image("images/background_3.png");
    Image terrain_Image = create_Image("images/collision_Terrain_1.png");
    Image castle_Image = create_Image("images/player_Castle.png");
    Image arrow_Image = create_Image("images/arrow.png");
    Image skeleton_Image_Walking = create_Image("images/unit_Skeleton_Sprite_Sheet.png");
    Image skeleton_Image_Stop = create_Image("images/skeleton_Image_Walking");
    Image archer_Image_Walking = create_Image("images/unit_Archer_Sprite_Sheet.png");
    Image archer_Image_Stop = create_Image("images/unit_Archer.png");

	add_Sprite_Sheet_To_Array(BKG_GAMELOOP_1, &gameloop_BKG_Image, 1, 1);
	add_Sprite_Sheet_To_Array(BKG_MENU_1, &menu_BKG_Image, 1, 1);
    add_Sprite_Sheet_To_Array(TERRAIN_1, &terrain_Image, 1, 1);
    add_Sprite_Sheet_To_Array(CASTLE_1, &castle_Image, 1, 1);

    add_Sprite_Sheet_To_Array(SKELETON_WALKING, &skeleton_Image_Walking, 1, 4);
    add_Sprite_Sheet_To_Array(SKELETON_STOP, &skeleton_Image_Stop, 1, 1);
    add_Sprite_Sheet_To_Array(ARCHER_WALKING, &archer_Image_Walking, 1, 2);
    add_Sprite_Sheet_To_Array(ARCHER_STOP, &archer_Image_Stop, 1, 1);
    add_Sprite_Sheet_To_Array(ARROW_DEFAULT, &arrow_Image, 1, 1);

    Game_Data game_Data = {};

    game_Data.terrain_Height_Map = create_Height_Map("images/collision_Terrain_1.png");

    spawn_Player_Castle(
        CASTLE_1,
        &game_Data,
        { (RESOLUTION_WIDTH * 0.05) , ((float)game_Data.terrain_Height_Map[(int)(RESOLUTION_WIDTH * 0.05)] + (int)25) },
        LEVEL_1
    );

    spawn_Enemy_Castle(
        CASTLE_1, 
        &game_Data, 
        { (RESOLUTION_WIDTH * 0.95) , ((float)game_Data.terrain_Height_Map[(int)(RESOLUTION_WIDTH * 0.95)] + (int)25) }, 
        LEVEL_1
    );

    // Buttons
    SDL_Rect test = {};
    test.x = RESOLUTION_WIDTH / 2;
    test.y = RESOLUTION_HEIGHT / 2;
    test.w = 200;
    test.h = 100;
    Color color = { 0, 0, 255, SDL_ALPHA_OPAQUE };
   
    int mouse_X = 0;
    int mouse_Y = 0;
    SDL_GetMouseState(&mouse_X, &mouse_Y);

    float ticks = 0.0f;
    float last_Ticks = 0.0f;
    float delta_Time = 0.0f;
    // 0 - 1
    float time_Scalar = 1.0f;

    bool spawn_Skeleton_Pressed = false;
    bool spawn_Archer_Pressed = false;

    bool running = true;

    std::unordered_map<SDL_Keycode, bool> key_States;
    key_States[SDLK_ESCAPE] = false;
	// Debugging visualization code
	// Sprite_Sheet_Tracker sprite_Sheet_Tracker = { skeleton_Animations, WALKING, 0.0f, 0 };

    Current_Game_State current_Game_State = Current_Game_State::MENU;

    while (running) {
        mouse_Down_This_Frame = false;
        SDL_Event event = {};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    key_States[event.key.keysym.sym] = true;
                    break;
                }
                case SDLK_UP: {
                    key_States[event.key.keysym.sym] = true;
                    break;
                }
				case SDLK_DOWN: {
                    key_States[event.key.keysym.sym] = true;
					break;
				}
                case SDLK_ESCAPE: {
                    if (event.key.repeat == 0) {
                        key_States[event.key.keysym.sym] = !key_States[event.key.keysym.sym];
                    }
                    break;
                }
                default: {
                    break;
                }
                }  
                break;
            }
            case SDL_KEYUP: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    key_States[event.key.keysym.sym] = false;
                    break;
                }
				case SDLK_UP: {
                    key_States[event.key.keysym.sym] = false;
					break;
				}
				case SDLK_DOWN: {
                    key_States[event.key.keysym.sym] = false;
					break;
				}
                default: {
                    break;
                }
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouse_Down_This_Frame = true;
                }
                break;
            }
            case SDL_QUIT: {
                running = false;
                break;
            }
            default: {
                break;
            }
            }
        }

		current_frame_Hot_Name = next_Frame_Hot_Name;
		next_Frame_Hot_Name = "";
		SDL_GetMouseState(&mouse_X, &mouse_Y);

        if (current_Game_State == Current_Game_State::MENU) {
			// No game logic
            SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, sprite_Sheet_Array[BKG_MENU_1].sprites[0].image->texture, NULL, NULL);
        
            draw_String_With_Background(
                &font_1, 
                "Castle Defense", 
                RESOLUTION_WIDTH / 2, 
                RESOLUTION_HEIGHT / 4, 
                8, 
                true, 
                Select_Color::BLACK,
                20
            );
            
			if (button(&font_1, "Play", RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 50, 300, 100, 4)) {
                current_Game_State = Current_Game_State::GAMELOOP;
			}
			if (button(&font_1, "Options", RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 150, 300, 100, 4)) {

			}
			if (button(&font_1, "Quit", RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 250, 300, 100, 4)) {
				running = false;
			}

            SDL_RenderPresent(renderer);
        
        }
        else if (current_Game_State == Current_Game_State::GAMELOOP) {
            if (key_States[SDLK_UP] == true) {
                time_Scalar += 0.01f;
            }
            if (key_States[SDLK_DOWN] == true) {
                if (time_Scalar > 0) {
                    time_Scalar -= 0.01f;
                }
            }
            
            last_Ticks = ticks;
            ticks = (float)SDL_GetTicks64();
            delta_Time = ticks - last_Ticks;
            // Multiply by the scalar
            delta_Time *= time_Scalar;
            delta_Time /= 1000;

            if (key_States[SDLK_ESCAPE] == false && time_Scalar > 0) {
                // Spawn Arrows and update lifetime
                if (key_States[SDLK_SPACE] == true) {
                    if (game_Data.player_Castle.current_Fire_Cooldown < 0) {
                        Vector target_Mouse = {};
                        int x, y = 0;
                        SDL_GetMouseState(&x, &y);
                        target_Mouse = { (float)x,(float)y };
                        spawn_Arrow(
                            &game_Data,
                            &game_Data.player_Castle.rigid_Body.position_WS,
                            &target_Mouse,
                            LEVEL_1
                        );
                        game_Data.player_Castle.current_Fire_Cooldown = game_Data.player_Castle.fire_Cooldown;
                    }
                    else {
                        game_Data.player_Castle.current_Fire_Cooldown -= delta_Time;
                    }
                }
                else {
                    game_Data.player_Castle.current_Fire_Cooldown -= delta_Time;
                }

                // Spawn Player Skeletons
                if (spawn_Skeleton_Pressed) {
                    Castle* player_Castle = &game_Data.player_Castle;
                    Castle* enemy_Castle = &game_Data.enemy_Castle;
                    spawn_Player_Skeleton(
                        &game_Data,
                        {
                            (float)player_Castle->rigid_Body.position_WS.x,
							((float)game_Data.terrain_Height_Map[(int)player_Castle->rigid_Body.position_WS.x] + player_Castle->sprite_Sheet_Tracker.sprite_Sheet_Array[player_Castle->sprite_Sheet_Tracker.selected].sprites[0].radius)
                        },
                        &enemy_Castle->rigid_Body,
                        LEVEL_3
                    );
                    spawn_Skeleton_Pressed = false;
                }
                if (spawn_Archer_Pressed) {
					Castle* player_Castle = &game_Data.player_Castle;
					Castle* enemy_Castle = &game_Data.enemy_Castle;
                    spawn_Archer(
                        &game_Data,
						{
							(float)player_Castle->rigid_Body.position_WS.x,
							((float)game_Data.terrain_Height_Map[(int)player_Castle->rigid_Body.position_WS.x] + player_Castle->sprite_Sheet_Tracker.sprite_Sheet_Array[player_Castle->sprite_Sheet_Tracker.selected].sprites[0].radius)
						},
                        &enemy_Castle->rigid_Body,
                        LEVEL_2
                    );
                    spawn_Archer_Pressed = false;
                }

                // Spawn enemy skeletons
                if (game_Data.enemy_Castle.current_Spawn_Cooldown < 0) {
					Castle* player_Castle = &game_Data.player_Castle;
					Castle* enemy_Castle = &game_Data.enemy_Castle;
                    spawn_Enemy_Skeleton(
						&game_Data,
						{
							(float)enemy_Castle->rigid_Body.position_WS.x,
							((float)game_Data.terrain_Height_Map[(int)enemy_Castle->rigid_Body.position_WS.x] + enemy_Castle->sprite_Sheet_Tracker.sprite_Sheet_Array[enemy_Castle->sprite_Sheet_Tracker.selected].sprites[0].radius)
						},
						&player_Castle->rigid_Body,
						LEVEL_2
					);
                    enemy_Castle->current_Spawn_Cooldown = enemy_Castle->spawn_Cooldown;
                }
                else {
                    game_Data.enemy_Castle.current_Spawn_Cooldown -= delta_Time;
                }

                // Update arrow positions
                for (int i = 0; i < game_Data.player_Arrows.size(); i++) {
                    if (game_Data.player_Arrows[i].destroyed == false) {
                        update_Arrow_Position(&game_Data.player_Arrows[i], delta_Time);
                    }
                }

                // Update player skeleton positions
                for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
                    if (game_Data.player_Skeletons[i].destroyed == false) {
                        update_Unit_Position(
                            &game_Data.player_Skeletons[i].rigid_Body,
                            game_Data.player_Skeletons[i].stop,
                            delta_Time
                        );
                    }
                }

                // Update enemy skeleton positions
                for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
                    if (game_Data.enemy_Skeletons[i].destroyed == false) {
                        update_Unit_Position(&game_Data.enemy_Skeletons[i].rigid_Body,
                            game_Data.enemy_Skeletons[i].stop,
                            delta_Time
                        );
                    }
                }

                // Update player archer positions
                for (int i = 0; i < game_Data.player_Archers.size(); i++) {
                    if (game_Data.player_Archers[i].destroyed == false) {
                        update_Unit_Position(
                            &game_Data.player_Archers[i].rigid_Body,
                            game_Data.player_Archers[i].stop,
                            delta_Time
                        );
                    }
                }

                // Collision detection for arrows
                for (int i = 0; i < game_Data.player_Arrows.size(); i++) {
                    Arrow* arrow = &game_Data.player_Arrows[i];
                    Castle* enemy_Castle = &game_Data.enemy_Castle;
                    if (check_RB_Collision(&arrow->rigid_Body, &enemy_Castle->rigid_Body)) {
                        enemy_Castle->health_Bar.current_HP -= arrow->damage;
                        arrow->destroyed = true;
                    }
                    // Collision with map
                    if (check_Height_Map_Collision(
                        &arrow->rigid_Body,
                        game_Data.terrain_Height_Map))
                    {
                        arrow->stop= true;
                    }
                    // Collision with skeletons and arrows
                    for (int j = 0; j < game_Data.enemy_Skeletons.size(); j++) {
                        Skeleton* enemy_Skeletons = &game_Data.enemy_Skeletons[j];
                        if (check_RB_Collision(&arrow->rigid_Body, &enemy_Skeletons->rigid_Body)) {
                            if (!arrow->stop) {
                                enemy_Skeletons->health_Bar.current_HP -= arrow->damage;
                                arrow->destroyed = true;
                            }
                        }
                    }
                }

                // Collision enemy skeleton with map
                for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.enemy_Skeletons[i];
                    if (check_Height_Map_Collision(&skeleton->rigid_Body, game_Data.terrain_Height_Map)) {
                        float radius = skeleton->sprite_Sheet_Tracker.sprite_Sheet_Array[skeleton->sprite_Sheet_Tracker.selected].sprites[0].radius;
                        float pos_Y_HM = (float)game_Data.terrain_Height_Map[(int)skeleton->rigid_Body.position_WS.x];

                        skeleton->rigid_Body.position_WS.y = ((RESOLUTION_HEIGHT - pos_Y_HM) - radius);
                    }
                }

                // Collision player skeletons with map
                for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.player_Skeletons[i];
                    if (check_Height_Map_Collision(&game_Data.player_Skeletons[i].rigid_Body, game_Data.terrain_Height_Map)) {
                        float radius = skeleton->sprite_Sheet_Tracker.sprite_Sheet_Array[skeleton->sprite_Sheet_Tracker.selected].sprites[0].radius;
                        float pos_Y_HM = (float)game_Data.terrain_Height_Map[(int)skeleton->rigid_Body.position_WS.x];

                        skeleton->rigid_Body.position_WS.y = ((RESOLUTION_HEIGHT - pos_Y_HM) - radius);
                    }
                }

                // Collision player archers with map
                for (int i = 0; i < game_Data.player_Archers.size(); i++) {
                    Archer* archer = &game_Data.player_Archers[i];
                    if (check_Height_Map_Collision(&game_Data.player_Archers[i].rigid_Body, game_Data.terrain_Height_Map)) {
                        // Function: Pass in an archer and get the radius of the animation / sprite
                        // OR pass in the animation tracker (Makes sense)
                        float radius = archer->sprite_Sheet_Tracker.sprite_Sheet_Array[archer->sprite_Sheet_Tracker.selected].sprites[0].radius;
                        float pos_Y_HM = (float)game_Data.terrain_Height_Map[(int)archer->rigid_Body.position_WS.x];

                        archer->rigid_Body.position_WS.y = ((RESOLUTION_HEIGHT - pos_Y_HM) - radius);
                    }
                }

                // Initialize default values before collision check
                for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.player_Skeletons[i];
                    skeleton->stop = false;
                    skeleton->current_Attack_Cooldown -= delta_Time;
                }
                for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.enemy_Skeletons[i];
                    skeleton->stop = false;
                    skeleton->current_Attack_Cooldown -= delta_Time;
                }
                for (int i = 0; i < game_Data.player_Archers.size(); i++) {
                    Archer* archer = &game_Data.player_Archers[i];
                    archer->stop = false;
                    archer->current_Attack_Cooldown -= delta_Time;
                }

                // Collision player skeleton with enemy castle
                for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.player_Skeletons[i];
                    Castle* castle = &game_Data.enemy_Castle;
                    if (check_RB_Collision(&skeleton->rigid_Body, &castle->rigid_Body)) {
                        skeleton->stop = true;
                        if (skeleton->current_Attack_Cooldown < 0) {
                            skeleton->current_Attack_Cooldown = skeleton->attack_Cooldown;
                            castle->health_Bar.current_HP -= skeleton->damage;
                        }
                    }
                }

                // Collision enemy skeleton with player castle
                for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.enemy_Skeletons[i];
                    Castle* castle = &game_Data.player_Castle;
                    if (check_RB_Collision(&skeleton->rigid_Body, &castle->rigid_Body)) {
                        skeleton->stop = true;
                        if (skeleton->current_Attack_Cooldown < 0) {
                            skeleton->current_Attack_Cooldown = skeleton->attack_Cooldown;
                            castle->health_Bar.current_HP -= skeleton->damage;
                        }
                    }
                }

                // Skeletons colliding with each other
                for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
                    Skeleton* player_Skeleton = &game_Data.player_Skeletons[i];
                    for (int j = 0; j < game_Data.enemy_Skeletons.size(); j++) {
                        Skeleton* enemy_Skeleton = &game_Data.enemy_Skeletons[j];
                        if (check_RB_Collision(&player_Skeleton->rigid_Body, &enemy_Skeleton->rigid_Body)) {
                            player_Skeleton->stop = true;
                            enemy_Skeleton->stop = true;
                            if (player_Skeleton->current_Attack_Cooldown <= 0) {
                                player_Skeleton->current_Attack_Cooldown = player_Skeleton->attack_Cooldown;
                                enemy_Skeleton->health_Bar.current_HP -= player_Skeleton->damage;
                            }
                            if (enemy_Skeleton->current_Attack_Cooldown <= 0) {
                                enemy_Skeleton->current_Attack_Cooldown = enemy_Skeleton->attack_Cooldown;
                                player_Skeleton->health_Bar.current_HP -= enemy_Skeleton->damage;
                            }
                        }
                    }
                }

                // Player archers and enemy skeletons colliding with each other
                for (int i = 0; i < game_Data.player_Archers.size(); i++) {
                    Archer* archer = &game_Data.player_Archers[i];
                    for (int j = 0; j < game_Data.enemy_Skeletons.size(); j++) {
                        Skeleton* skeleton = &game_Data.enemy_Skeletons[j];
                        float distance_Between = calculate_Distance(
                            archer->rigid_Body.position_WS.x,
                            archer->rigid_Body.position_WS.y,
                            skeleton->rigid_Body.position_WS.x,
                            skeleton->rigid_Body.position_WS.y
                        );
                        float range_Sum = archer->attack_Range;
                        if (distance_Between <= range_Sum) {
                            change_Animation(&archer->sprite_Sheet_Tracker, ARCHER_STOP);
                            archer->stop = true;
                            if (archer->current_Attack_Cooldown <= 0) {
                                archer->current_Attack_Cooldown = archer->attack_Cooldown;
                                Vector aim_Head = skeleton->rigid_Body.position_WS;
                                Sprite_Sheet_Selector selected = game_Data.enemy_Skeletons[0].sprite_Sheet_Tracker.selected;
                                aim_Head.x += game_Data.enemy_Skeletons[0].sprite_Sheet_Tracker.sprite_Sheet_Array[selected].sprites[0].radius;
                                spawn_Arrow(
                                    &game_Data,
                                    &archer->rigid_Body.position_WS,
                                    &aim_Head,
                                    LEVEL_1
                                );
                            }
                        }
                        if (check_RB_Collision(&archer->rigid_Body, &skeleton->rigid_Body)) {
                            game_Data.enemy_Skeletons[j].stop = true;
                            if (skeleton->current_Attack_Cooldown <= 0) {
                                skeleton->current_Attack_Cooldown = skeleton->attack_Cooldown;
                                archer->health_Bar.current_HP -= skeleton->damage;
                            }
                        }
                    }
                }

                for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.enemy_Skeletons[i];
                    float speed = skeleton->speed;
                    if (!skeleton->stop) {
                        update_Animation(&skeleton->sprite_Sheet_Tracker, speed, delta_Time);
                    }
                }

                for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.player_Skeletons[i];
                    float speed = skeleton->speed;
                    if (!skeleton->stop) {
                        update_Animation(&skeleton->sprite_Sheet_Tracker, speed, delta_Time);
                    }
                }

                for (int i = 0; i < game_Data.player_Archers.size(); i++) {
                    Archer* archer = &game_Data.player_Archers[i];
                    float speed = archer->speed;
                    update_Animation(&archer->sprite_Sheet_Tracker, speed, delta_Time);
                }

            }

            SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);

            // ***Renderering happens here***
            draw_Layer(sprite_Sheet_Array[BKG_GAMELOOP_1].sprites[0].image->texture);
            draw_Layer(sprite_Sheet_Array[TERRAIN_1].sprites[0].image->texture);
            draw_Castle(&game_Data.player_Castle, false);
            draw_Castle(&game_Data.enemy_Castle, true);

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

            draw_Circle(game_Data.enemy_Castle.rigid_Body.position_WS.x, (float)(game_Data.enemy_Castle.rigid_Body.position_WS.y), game_Data.enemy_Castle.sprite_Sheet_Tracker.sprite_Sheet_Array[game_Data.enemy_Castle.sprite_Sheet_Tracker.selected].sprites[0].radius, Select_Color::GREEN);
            draw_Circle(game_Data.player_Castle.rigid_Body.position_WS.x, (float)((int)game_Data.player_Castle.rigid_Body.position_WS.y), game_Data.enemy_Castle.sprite_Sheet_Tracker.sprite_Sheet_Array[game_Data.enemy_Castle.sprite_Sheet_Tracker.selected].sprites[0].radius, Select_Color::GREEN);

            // Draw player arrows
            for (int i = 0; i < game_Data.player_Arrows.size(); i++) {
                Arrow* arrow = &game_Data.player_Arrows[i];
                draw_RigidBody_Colliders(&arrow->rigid_Body, Select_Color::GREEN);
                if (arrow->life_Time > 0) {
                    draw_Arrow(arrow, false);
                    arrow->life_Time -= delta_Time;
                }
            }

            // Draw enemy skeletons
            for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
                Skeleton* skeleton = &game_Data.enemy_Skeletons[i];
                draw_RigidBody_Colliders(&skeleton->rigid_Body, Select_Color::GREEN);
                draw_Unit_Animated(
                    &skeleton->rigid_Body,
                    &skeleton->sprite_Sheet_Tracker,
                    false
                );
                draw_HP_Bar(&skeleton->rigid_Body.position_WS, &skeleton->health_Bar);
            }

            // Draw player skeletons
            for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
                Skeleton* player_Skeleton = &game_Data.player_Skeletons[i];
                /*
                // Debugging circles for colliders 
                for (int j = 0; j < player_Skeleton->animation_Tracker.animations_Array[game_Data.player_Skeletons[i].animation_Tracker.type].sprite_Sheet.sprites.size(); j++) {
                    Animation_Type type = game_Data.player_Skeletons[i].animation_Tracker.type;
                    draw_Circle(
                        game_Data.player_Skeletons[i].rigid_Body.position_WS.x,
                        game_Data.player_Skeletons[i].rigid_Body.position_WS.y,
                        game_Data.player_Skeletons[i].animation_Tracker.animations_Array[type].sprite_Sheet.sprites[j].radius,
                        Select_Color::RED
                    );
                }
                */
                draw_RigidBody_Colliders(&player_Skeleton->rigid_Body, Select_Color::GREEN);
                draw_Unit_Animated(
                    &player_Skeleton->rigid_Body,
                    &player_Skeleton->sprite_Sheet_Tracker,
                    true
                );
                draw_HP_Bar(&player_Skeleton->rigid_Body.position_WS, &player_Skeleton->health_Bar);
            }

            // Draw player archers
            for (int i = 0; i < game_Data.player_Archers.size(); i++) {
                Archer* archer = &game_Data.player_Archers[i];
                draw_RigidBody_Colliders(&archer->rigid_Body, Select_Color::GREEN);
                draw_Unit_Animated(
                    &archer->rigid_Body,
                    &archer->sprite_Sheet_Tracker,
                    false
                );
                draw_HP_Bar(&archer->rigid_Body.position_WS, &archer->health_Bar);
                draw_HP_Bar(&archer->rigid_Body.position_WS, &archer->health_Bar);
                draw_HP_Bar(&archer->rigid_Body.position_WS, &archer->health_Bar);
            }

            draw_HP_Bar(&game_Data.player_Castle.rigid_Body.position_WS, &game_Data.player_Castle.health_Bar);
            draw_HP_Bar(&game_Data.enemy_Castle.rigid_Body.position_WS, &game_Data.enemy_Castle.health_Bar);

            draw_Timer(
                &font_1, 
                { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 30 }, 
                6, 
                Select_Color::BLACK, 
                4
            );

            draw_Time_Scalar(
                &font_1, 
                time_Scalar, 
                RESOLUTION_WIDTH / 2, 
                RESOLUTION_HEIGHT / 2,
                4
            );

            // Debugging visualization code
#if 0
            Rigid_Body temp_RB = {};
            temp_RB.position_WS = { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 };
			static float frames_Per_Seconds = 100.0f;
            if (temp_W_Pressed) {
                frames_Per_Seconds += 1.0f;
            }
            if (temp_S_Pressed) {
                frames_Per_Seconds -= 1.0f;
            }
            temp_Bool = false;
            if (temp_W_Pressed || temp_S_Pressed) {
                temp_Bool = true;
            }
            update_Animation(
                &animation_Tracker,
                frames_Per_Seconds, 
                delta_Time
            );
            draw_String(&font_1, std::to_string(frames_Per_Seconds).c_str(), RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 75, 4, true);
            draw_Unit_Animated(&temp_RB, &animation_Tracker, false);
#endif

            int button_Pos_X = (RESOLUTION_WIDTH / 8);
            int button_Pos_Y = ((RESOLUTION_HEIGHT / 16) * 15);
            int button_Width = 450;
            int button_Height = 100;
            int string_Size = 4;
            int x_Offset = button_Width;
            if (button(&font_1, "Spawn Skeleton", button_Pos_X, button_Pos_Y, button_Width, button_Height, string_Size)) {
                spawn_Skeleton_Pressed = true;
            } 
            button_Pos_X += x_Offset;
			if (button(&font_1, "Spawn Archer", button_Pos_X, button_Pos_Y, button_Width, button_Height, string_Size)) {
				spawn_Archer_Pressed = true;
			}
            button_Pos_X += x_Offset;

            if (key_States[SDLK_ESCAPE] == true) {
                draw_String_With_Background(
                    &font_1,
                    "Game Paused",
                    RESOLUTION_WIDTH / 2,
                    RESOLUTION_HEIGHT / 2,
                    5,
                    true,
                    Select_Color::BLACK,
                    4
                );
            }

            // Erase destroyed arrows
            std::erase_if(game_Data.player_Arrows, [](Arrow& arrow) {
                // Return if we want the value to be destroyed
                return arrow.destroyed || arrow.life_Time <= 0;
                });

            // Erase destroy units
            std::erase_if(game_Data.enemy_Skeletons, [](Skeleton& skeletons) {
                // Return if we want the value to be destroyed
                return skeletons.destroyed || skeletons.health_Bar.current_HP <= 0;
                });

            // Erase destroy units
            std::erase_if(game_Data.player_Skeletons, [](Skeleton& skeletons) {
                // Return if we want the value to be destroyed
                return skeletons.destroyed || skeletons.health_Bar.current_HP <= 0;
                });

			// Erase destroy units
			std::erase_if(game_Data.player_Archers, [](Archer& archer) {
				// Return if we want the value to be destroyed
				return archer.destroyed || archer.health_Bar.current_HP <= 0;
				});

            SDL_RenderPresent(renderer);
        }
    }

    return 0;
}