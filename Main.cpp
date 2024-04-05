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
const float ARCHER_ARROW_GRAVITY = 50;
const int MAX_ATTACHED_ENTITY = 1000;
const int MAX_COLLIDERS = 100;

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

enum Button_State {
    BS_NORMAL,
    BS_HOVERED,
    BS_IS_HOT,
    BS_PRESSED
};

enum Color_Index {
    CI_BLACK,
    CI_RED,
    CI_GREEN,
    CI_BLUE
};

enum Arrow_Type {
    AT_PLAYER_ARROW,
    AT_ARCHER_ARROW,
};

// Chris does this instead of enums
const Color BLACK = {0, 0, 0, 0};

enum Game_State {
    GS_MENU,
    GS_GAMELOOP,
    GS_PAUSED
};

struct Key_State {
    bool pressed_This_Frame;
    bool held_Down;
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
    V2 position_LS;
	float radius;
};

struct Rigid_Body {
    bool rigid_Body_Faces_Velocity;
    V2 position_WS;
    V2 velocity;
    float angle;
    // num_Colliders
    int colliders_Array_Size;
    Collider colliders_Array[MAX_COLLIDERS];
};

struct Sprite_Sheet {
    std::vector<Sprite> sprites;
};

enum Sprite_Sheet_Selector {
    SSS_SKELETON_WALKING,
    SSS_SKELETON_STOP,
    SSS_SKELETON_ATTACKING,
    SSS_SKELETON_DYING,

    SSS_ARCHER_WALKING,
    SSS_ARCHER_STOP,
    SSS_ARCHER_ATTACKING,
    SSS_ARCHER_DYING,

    SSS_ARROW_DEFAULT,

    SSS_CASTLE_1,

    SSS_BKG_GAMELOOP_1,
    SSS_BKG_MENU_1,

    SSS_TERRAIN_1,

    SSS_TOTAL_SPRITE_SHEETS
};

Sprite_Sheet sprite_Sheet_Array[SSS_TOTAL_SPRITE_SHEETS] = {};

struct Sprite_Sheet_Tracker {
    Sprite_Sheet_Selector selected;
	float animation_Time;
	int current_Frame;
};

Sprite_Sheet_Tracker create_Sprite_Sheet_Tracker(Sprite_Sheet_Selector selected) {
    Sprite_Sheet_Tracker result;

    result.selected = selected;
    result.animation_Time = 0.0f;
    result.current_Frame = 0;

    return result;
}

enum Level {
	LEVEL_1,
	LEVEL_2,
	TOTAL_LEVELS
};  

struct Stored_Units {
    int current;
    int max;
};

struct Cooldown {
    float duration;
    float remaining;
};

struct Castle_Stats {
    float hp;

    Cooldown fire_Cooldown;
    Cooldown spawn_Cooldown;

    int arrow_Ammo;
    Cooldown arrow_Ammo_Cooldown;

    Stored_Units stored_Units;
};

const Castle_Stats castle_Stats_Array[TOTAL_LEVELS] = {
	// hp    |    fire_Cooldown   |  spawn_Cooldown   |  arrow_Ammo    |   arrow_Ammo_Cooldown  |  stored_Units
    {  100.0f,    {0.01f, 0.0f},     {1.0f, 0.0f},       0,                {0.25f, 0.0f},          {0, 3}     },
	{  100.0f,    {1.0f, 0.0f},      {1.0f, 0.0f},       0,                {0.25f, 0.0f},          {0, 3}     }
};

struct Castle {
    Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;
	Health_Bar health_Bar;

	Cooldown fire_Cooldown;
	Cooldown spawn_Cooldown;

	int arrow_Ammo;
	Cooldown arrow_Ammo_Cooldown;

    Stored_Units stored_Units;
};

enum Current_Weapon {
    CW_ARROWS,
    CW_FIREBALLS,

    CW_TOTAL_WEAPONS
};

struct Arrow_Stats {
	float speed;
	float damage;
	float life_Time;
};

const Arrow_Stats arrow_Stats_Array[TOTAL_LEVELS] = {
    // speed    |   damage  |   life_Time
	{  800,         25,          100     },
	{  800,         100,         100     }
};

struct Arrow {
    Arrow_Type type;

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
    // speed    |   damage  |   max_HP  |   attack_Cooldown  |  attack_Range
	{  100,         20,         100,        1,                  150        },
	{  200,         25,         125,        1,                  150        }
};

struct Attached_Entity{
    Sprite_Sheet_Tracker tracker;
    float angle;
    V2 offset;
};

struct Skeleton {
	Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;

	Health_Bar health_Bar;

	float speed;
	float damage;
	float attack_Cooldown;
	float current_Attack_Cooldown;
	float attack_Range;

    Attached_Entity attached_Entities[MAX_ATTACHED_ENTITY] = {};
    int attached_Entities_Size = 0;

	bool destroyed;
	bool stop;

    int ID;
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
	// speed | damage |  hp  |  attack_Cooldown | current_Attack_Cooldown | attack_Range
	{  100,    10,      100,    1,                0.0,                      750        },
	{  150,    15,      125,    0.5,              0.0,                      1000       },
};

struct Archer {
    Sprite_Sheet_Tracker sprite_Sheet_Tracker;

	Rigid_Body rigid_Body;

	Health_Bar health_Bar;

	float speed;
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
    int                         next_Entity_ID;
};

void save_Game(Game_Data* game_Data, const char* file_Name) {
	FILE* file = NULL;
	errno_t err = fopen_s(&file, file_Name, "wb");

    DEFER{
        fclose(file);
    };

	if (err != 0 || !file) {
		SDL_Log("ERROR: Unable to open file %s in save_Game", file_Name);
		return;
	}

    fwrite(&game_Data->player_Castle, sizeof(game_Data->player_Castle), 1, file);
    fwrite(&game_Data->enemy_Castle, sizeof(game_Data->enemy_Castle), 1, file);
     
    size_t terrain_Size = game_Data->terrain_Height_Map.size();
    fwrite(&terrain_Size, sizeof(terrain_Size), 1, file);
    fwrite(game_Data->terrain_Height_Map.data(), sizeof(game_Data->terrain_Height_Map[0]), terrain_Size, file);

	size_t player_Arrows_Size = game_Data->player_Arrows.size();
	fwrite(&player_Arrows_Size, sizeof(player_Arrows_Size), 1, file);
	fwrite(game_Data->player_Arrows.data(), sizeof(game_Data->player_Arrows[0]), player_Arrows_Size, file);

    size_t enemy_Skeletons_Size = game_Data->enemy_Skeletons.size();
    fwrite(&enemy_Skeletons_Size, sizeof(enemy_Skeletons_Size), 1, file);
    fwrite(game_Data->enemy_Skeletons.data(), sizeof(game_Data->enemy_Skeletons[0]), enemy_Skeletons_Size, file);

    size_t player_Skeletons_Size = game_Data->player_Skeletons.size();
    fwrite(&player_Skeletons_Size, sizeof(player_Skeletons_Size), 1, file);
    fwrite(game_Data->player_Skeletons.data(), sizeof(game_Data->player_Skeletons[0]), player_Skeletons_Size, file);
    
    size_t player_Archer_Size = game_Data->player_Archers.size();
    fwrite(&player_Archer_Size , sizeof(player_Archer_Size), 1, file);
    fwrite(game_Data->player_Archers.data(), sizeof(game_Data->player_Archers[0]), player_Archer_Size, file);
 
    fwrite(&game_Data->next_Entity_ID, sizeof(game_Data->next_Entity_ID), 1, file);
}

template <typename T>
void read_Vector(std::vector<T>& vector, FILE* file) {
	size_t vector_Size = 0;
	fread(&vector_Size, sizeof(vector_Size), 1, file);
	vector.clear();
	vector.resize(vector_Size);
	fread(vector.data(), sizeof(vector[0]), vector_Size, file);
}

void load_Game(Game_Data* game_Data, const char* file_Name) {
	FILE* file = NULL;
	errno_t err = fopen_s(&file, file_Name, "rb");
	
    DEFER{
	    fclose(file);
	};

	if (err != 0 || !file) {
		SDL_Log("ERROR: Unable to open file %s in save_Game", file_Name);
		return;
	}

	fread(&game_Data->player_Castle, sizeof(game_Data->player_Castle), 1, file);
	fread(&game_Data->enemy_Castle, sizeof(game_Data->enemy_Castle), 1, file);

    read_Vector(game_Data->terrain_Height_Map, file);

    read_Vector(game_Data->player_Arrows, file);

    read_Vector(game_Data->enemy_Skeletons, file);

    read_Vector(game_Data->player_Skeletons, file);

    read_Vector(game_Data->player_Archers, file);

    game_Data->next_Entity_ID = 0;
	fread(&game_Data->next_Entity_ID, sizeof(game_Data->next_Entity_ID), 1, file);
}

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

// Returns the radius of the first sprite in the sprite sheet
// get_Radius_Of_First_Sprite_In_Selected_Sheet???? Way too long
float get_Sprite_Radius(Sprite_Sheet_Tracker* tracker) {
	const Sprite_Sheet* arr = sprite_Sheet_Array;
	Sprite_Sheet_Selector selected = tracker->selected;
	Sprite sprite = arr[selected].sprites[0];
	float radius = sprite.radius;
	return radius;
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
    Sprite_Sheet result = {};
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

void add_Collider(Rigid_Body* rigid_Body, V2 position_LS, float radius) {
    assert(rigid_Body->colliders_Array_Size < MAX_COLLIDERS);

    Collider* collider = &rigid_Body->colliders_Array[rigid_Body->colliders_Array_Size++];
    collider->position_LS = position_LS;
	collider->radius = radius;
}

void draw_Layer(SDL_Texture* texture) {
    SDL_RenderCopyEx(renderer, texture, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
}

void draw_Castle(Castle* castle, bool flip) {
    SDL_Rect temp = {};
	SDL_Rect* src_Rect = &sprite_Sheet_Array[castle->sprite_Sheet_Tracker.selected].sprites[0].source_Rect;
    V2 sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
    sprite_Half_Size = sprite_Half_Size / 2;
    temp = { 
		((int)castle->rigid_Body.position_WS.x - (int)sprite_Half_Size.x),
		((int)castle->rigid_Body.position_WS.y - (int)sprite_Half_Size.y),
        sprite_Sheet_Array[castle->sprite_Sheet_Tracker.selected].sprites[0].source_Rect.w,
        sprite_Sheet_Array[castle->sprite_Sheet_Tracker.selected].sprites[0].source_Rect.h
    };
    SDL_RenderCopyEx(renderer,  sprite_Sheet_Array[castle->sprite_Sheet_Tracker.selected].sprites[0].image->texture, NULL, &temp, 0, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

void draw_Arrow(Arrow* arrow, bool flip) {
    SDL_Rect temp = {};
    Sprite* sprite = &sprite_Sheet_Array[arrow->sprite_Sheet_Tracker.selected].sprites[0];
    SDL_Rect* src_Rect = &sprite->source_Rect;
	V2 sprite_Half_Size = { (float)src_Rect->w, (float)src_Rect->h };
	sprite_Half_Size = sprite_Half_Size / 2;
    temp = {
        ((int)arrow->rigid_Body.position_WS.x - (int)sprite_Half_Size.x),
        ((int)arrow->rigid_Body.position_WS.y - (int)sprite_Half_Size.y),
		sprite->source_Rect.w,
		sprite->source_Rect.h
    };
    SDL_RenderCopyEx(renderer, sprite->image->texture, NULL, &temp, arrow->rigid_Body.angle, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

Attached_Entity return_Attached_Entity(Sprite_Sheet_Selector selected, float angle, V2 offset) {
    Attached_Entity result = {};

    //                 selected     animation_Time      current_Frame
    result.tracker = { selected,    0.0f,               0 };
    result.angle = angle;
    result.offset = offset;

    return result;
}

void draw_Attached_Entity(Attached_Entity* attached_Entity, V2 position_WS, bool flip) {
	SDL_Rect temp = {};
    Sprite* sprite = &sprite_Sheet_Array[attached_Entity->tracker.selected].sprites[0];
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
	SDL_RenderCopyEx(renderer, sprite->image->texture, NULL, &temp, attached_Entity->angle, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
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
            if (tracker->current_Frame >= sprite_Sheet_Array[tracker->selected].sprites.size()) {
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

    const Sprite_Sheet* sprite_Sheet = &sprite_Sheet_Array[tracker->selected];
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
            arrow->rigid_Body.velocity.y += GRAVITY * delta_Time;
        } else if (arrow->type == AT_ARCHER_ARROW) {
            arrow->rigid_Body.velocity.y += ARCHER_ARROW_GRAVITY * delta_Time;
        } else {
            SDL_Log("ERROR: Arrow type not specified. update_Arrow_Position()");
            return;
        }

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

Rigid_Body create_Rigid_Body(V2 position_WS, bool rigid_Body_Faces_Velocity) {
	Rigid_Body result = {};

	result.rigid_Body_Faces_Velocity = rigid_Body_Faces_Velocity;
	result.position_WS = { position_WS.x , position_WS.y };

	return result;
}

void spawn_Player_Castle(Sprite_Sheet_Selector selector, Game_Data* game_Data, V2 position_WS, Level level) {
    Castle castle = {};

	castle.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(selector);

    castle.rigid_Body = create_Rigid_Body(position_WS, false);
	
    castle.health_Bar = create_Health_Bar(90, 20, 115, 3, castle_Stats_Array[level].hp);

    castle.fire_Cooldown = castle_Stats_Array[level].fire_Cooldown;
	castle.spawn_Cooldown = castle_Stats_Array[level].spawn_Cooldown;
    castle.arrow_Ammo = castle_Stats_Array[level].arrow_Ammo;
    castle.arrow_Ammo_Cooldown = castle_Stats_Array[level].arrow_Ammo_Cooldown;

	add_Collider(&castle.rigid_Body, { 0.0f, 0.0f }, sprite_Sheet_Array[selector].sprites[0].radius);

    game_Data->player_Castle = castle;
}

void spawn_Enemy_Castle(Sprite_Sheet_Selector selector, Game_Data* game_Data, V2 position_WS, Level level) {
	Castle castle = {};

    // Would be the appropriate way to do it but it breaks the serialization
    // castle.stats_Info = &castle_Stats_Array[level];

	castle.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(selector);

	castle.rigid_Body = create_Rigid_Body(position_WS, false);

	castle.health_Bar = create_Health_Bar(90, 20, 115, 3, castle_Stats_Array[level].hp);

	castle.fire_Cooldown = castle_Stats_Array[level].fire_Cooldown;
	castle.spawn_Cooldown = castle_Stats_Array[level].spawn_Cooldown;

	add_Collider(&castle.rigid_Body, { 0.0f, 0.0f }, sprite_Sheet_Array[selector].sprites[0].radius);

	game_Data->enemy_Castle = castle;
}

V2 calculate_Direction_V2(V2 target, V2 start) {
	V2 result = {};

	result.x = target.x - start.x;
	result.y = target.y - start.y;

	float length = (float)sqrt((result.x * result.x) + (result.y * result.y));

    result.x /= length;
    result.y /= length;

    return result;
}

void spawn_Arrow(Arrow_Type type, Game_Data* game_Data, V2 spawn_Position, V2 target_Position, Level level) {
    Arrow arrow = {};

    arrow.type = type;

	arrow.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(SSS_ARROW_DEFAULT);

    arrow.rigid_Body = create_Rigid_Body(spawn_Position, true);

	arrow.damage = arrow_Stats_Array[level].damage;
	arrow.speed = arrow_Stats_Array[level].speed;
	arrow.life_Time = arrow_Stats_Array[level].life_Time;

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

void spawn_Player_Skeleton(Game_Data* game_Data, V2 spawn_Position, V2 target_Position, Level level) {
	Skeleton skeleton = {};

    skeleton.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(SSS_SKELETON_WALKING);

	skeleton.rigid_Body = create_Rigid_Body(spawn_Position, false);

    skeleton.health_Bar = create_Health_Bar(50, 13, 60, 2, skeleton_Stats_Array[level].max_HP);

    skeleton.speed = skeleton_Stats_Array[level].speed;
    skeleton.damage = skeleton_Stats_Array[level].damage;
    skeleton.attack_Cooldown = skeleton_Stats_Array[level].attack_Cooldown;
    skeleton.current_Attack_Cooldown = 0.0f;
	skeleton.attack_Range = skeleton_Stats_Array[level].attack_Range;

	skeleton.destroyed = false;
	skeleton.stop = false;

    V2 direction_V2 = calculate_Direction_V2(target_Position, spawn_Position);

	// Set the new velocity
	skeleton.rigid_Body.velocity.x = direction_V2.x * skeleton_Stats_Array[level].speed;
	skeleton.rigid_Body.velocity.y = direction_V2.y * skeleton_Stats_Array[level].speed;

	float radius = get_Sprite_Radius(&skeleton.sprite_Sheet_Tracker);

    add_Collider(&skeleton.rigid_Body, { 0.0f, -(radius / 2) }, (radius / 2));
	add_Collider(&skeleton.rigid_Body, { 0.0f, 0.0f }, (radius / 2));
	add_Collider(&skeleton.rigid_Body, { 0.0f, (radius / 2) }, (radius / 2));

    skeleton.ID = game_Data->next_Entity_ID++;
	game_Data->player_Skeletons.push_back(skeleton);
}

void spawn_Enemy_Skeleton(Game_Data* game_Data, V2 spawn_Position, V2 target_Position, Level level) {
	Skeleton skeleton = {};

	skeleton.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(SSS_SKELETON_WALKING);

	skeleton.rigid_Body = create_Rigid_Body(spawn_Position, false);

	skeleton.health_Bar = create_Health_Bar(50, 13, 60, 2, skeleton_Stats_Array[level].max_HP);

	skeleton.speed = skeleton_Stats_Array[level].speed;
	skeleton.damage = skeleton_Stats_Array[level].damage;
	skeleton.attack_Cooldown = skeleton_Stats_Array[level].attack_Cooldown;
	skeleton.current_Attack_Cooldown = 0.0f;
	skeleton.attack_Range = skeleton_Stats_Array[level].attack_Range;

	skeleton.destroyed = false;
	skeleton.stop = false;

    V2 direction_V2 = calculate_Direction_V2(target_Position, spawn_Position);

	// Set the new velocity
	skeleton.rigid_Body.velocity.x = direction_V2.x * skeleton_Stats_Array[level].speed;
	skeleton.rigid_Body.velocity.y = direction_V2.y * skeleton_Stats_Array[level].speed;

	float radius = get_Sprite_Radius(&skeleton.sprite_Sheet_Tracker);

	add_Collider(&skeleton.rigid_Body, { 0.0f, -(radius / 2) }, (radius / 2));
	add_Collider(&skeleton.rigid_Body, { 0.0f, 0.0f }, (radius / 2));
	add_Collider(&skeleton.rigid_Body, { 0.0f, (radius / 2) }, (radius / 2));

    skeleton.ID = game_Data->next_Entity_ID++;
	game_Data->enemy_Skeletons.push_back(skeleton);
}

void spawn_Archer(Game_Data* game_Data, V2 spawn_Position, V2 target_Position, Level level) {
	Archer archer = {};

	archer.health_Bar = create_Health_Bar(50, 13, 60, 2, archer_Stats_Array[level].hp);

	archer.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(SSS_ARCHER_WALKING);

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
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        } else if (color == CI_GREEN) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        } else if (color == CI_BLUE) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
        }
        else {
            // Yellow is default
            SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
        }
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}
}

void draw_RigidBody_Colliders(Rigid_Body* rigid_Body, Color_Index color) {
    // This is a little weird
    for (int i = 0; i < rigid_Body->colliders_Array_Size; i++) {
        Collider* collider = &rigid_Body->colliders_Array[i];
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

void draw_String_With_Background(Font* font, const char* string, int position_X, int position_Y, int size, bool center, Color_Index color, int outline_Padding) {
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
    if (color == CI_BLACK) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    } else if (color == CI_RED) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	} else if (color == CI_GREEN) {
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	} else if (color == CI_BLUE) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	} else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderFillRect(renderer, &canvas_Area);
    
    draw_String(font, string, position_X, position_Y, size, center);
}

void draw_Timer(Font* font, V2 position, int timer_Size, Color_Index color, int outline_Padding) {
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

bool button_Text(Font* font, const char* string, V2 pos, int w, int h, int string_Size) {
	SDL_Rect button_Area = {};
    int outline_Thickness = 5;

	// Centers the button
	button_Area.x = ((int)pos.x - (w / 2));
	button_Area.y = ((int)pos.y - (h / 2));
	button_Area.w = w;
	button_Area.h = h;

	// Set background as black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &button_Area);

	draw_String(font, string, (int)pos.x, (int)pos.y, string_Size, true);
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

	outline_Rect(&button_Area, outline_Thickness);

	return button_Pressed;
}

bool button_Image(SDL_Texture* texture, const char* string, V2 pos, int h) {
	SDL_Rect button_Area = {};
    int outline_Thickness = 5;
    
    // Centers the button
	button_Area.x = ((int)pos.x - (h / 2));
	button_Area.y = ((int)pos.y - (h / 2));
	button_Area.w = h;
	button_Area.h = h;

	SDL_Rect image_Area = button_Area;
    image_Area.w -= outline_Thickness * 2;
    image_Area.h -= outline_Thickness * 2;
	image_Area.x += outline_Thickness;
	image_Area.y += outline_Thickness;

	// Set background as black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &button_Area);

    SDL_RenderCopyEx(renderer, texture, NULL, &image_Area, 0, NULL, SDL_FLIP_NONE);
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

	outline_Rect(&button_Area, outline_Thickness);

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
    for (int i = 0; i < rigid_Body->colliders_Array_Size; i++) {
        Collider* collider = &rigid_Body->colliders_Array[i];
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
    for (int i = 0; i < rigid_Body_1->colliders_Array_Size; i++) {
        Collider* collider_1 = &rigid_Body_1->colliders_Array[i];
        V2 world_Pos_1 = get_WS_Position(rigid_Body_1, collider_1);

		for (int j = 0; j < rigid_Body_2->colliders_Array_Size; j++) {
            Collider* collider_2 = &rigid_Body_2->colliders_Array[j];
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

void change_Animation(Sprite_Sheet_Tracker* tracker, Sprite_Sheet_Selector selected) {
	if (tracker->selected != selected) {
        tracker->selected = selected;
        tracker->current_Frame = 0;
    }
}

void draw_Arrow_Ammo_Tracker(Font* font, int ammo, V2 pos, int size) {
	std::string str_1 = std::to_string(ammo);
	std::string str_2 = "Arrow ammo:" + str_1;
	const char* ptr = str_2.c_str();
	draw_String_With_Background(font, ptr, (int)pos.x, (int)pos.y, size, true, CI_BLACK, 5);
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
    draw_String_With_Background(font, ptr, pos_X, pos_Y, size, true, CI_BLACK, 5);
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

std::unordered_map<SDL_Keycode, Key_State> key_States;
void reset_Pressed_This_Frame() {
    for (auto& key_State : key_States) {
        key_States[key_State.first].pressed_This_Frame = false;
    }
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
    Image skeleton_Image_Stop = create_Image("images/unit_Skeleton.png");
    Image archer_Image_Walking = create_Image("images/unit_Archer_Sprite_Sheet.png");
    Image archer_Image_Stop = create_Image("images/unit_Archer.png");

	add_Sprite_Sheet_To_Array(SSS_BKG_GAMELOOP_1, &gameloop_BKG_Image, 1, 1);
	add_Sprite_Sheet_To_Array(SSS_BKG_MENU_1, &menu_BKG_Image, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_TERRAIN_1, &terrain_Image, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_CASTLE_1, &castle_Image, 1, 1);
                            
    add_Sprite_Sheet_To_Array(SSS_SKELETON_WALKING, &skeleton_Image_Walking, 1, 4);
    add_Sprite_Sheet_To_Array(SSS_SKELETON_STOP, &skeleton_Image_Stop, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_ARCHER_WALKING, &archer_Image_Walking, 1, 2);
    add_Sprite_Sheet_To_Array(SSS_ARCHER_STOP, &archer_Image_Stop, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_ARROW_DEFAULT, &arrow_Image, 1, 1);

    Game_Data game_Data = {};

    game_Data.terrain_Height_Map = create_Height_Map("images/collision_Terrain_1.png");

    spawn_Player_Castle(
        SSS_CASTLE_1,
        &game_Data,
        { (RESOLUTION_WIDTH * 0.05f) , get_Height_Map_Pos_Y(&game_Data, (int)((RESOLUTION_WIDTH * 0.05f))) + 25.0f },
        LEVEL_1
    );

    spawn_Enemy_Castle(
        SSS_CASTLE_1,
        &game_Data, 
        { (RESOLUTION_WIDTH * 0.95f) , get_Height_Map_Pos_Y(&game_Data, (int)((RESOLUTION_WIDTH * 0.95f))) + 25.0f },
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

    // Debugging visualization code
	// Sprite_Sheet_Tracker sprite_Sheet_Tracker = { skeleton_Animations, WALKING, 0.0f, 0 };

    Game_State current_Game_State = GS_MENU;
    while (running) {
        mouse_Down_This_Frame = false;
        reset_Pressed_This_Frame();
        // Reset pressed this frame to false every frame
        SDL_Event event = {};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN: {
                key_States[event.key.keysym.sym].pressed_This_Frame = true;
                key_States[event.key.keysym.sym].held_Down = true;
                break;
            }
            case SDL_KEYUP: {
                key_States[event.key.keysym.sym].held_Down = false;
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

        if (key_States[SDLK_ESCAPE].pressed_This_Frame) {
            if (current_Game_State == GS_GAMELOOP) {
                current_Game_State = GS_PAUSED;
            }
            else if (current_Game_State == GS_PAUSED) {
                current_Game_State = GS_GAMELOOP;
            }
        }

        if (current_Game_State == GS_MENU) {
			// No game logic
            SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, sprite_Sheet_Array[SSS_BKG_MENU_1].sprites[0].image->texture, NULL, NULL);
        
            draw_String_With_Background(
                &font_1, 
                "Castle Defense", 
                RESOLUTION_WIDTH / 2, 
                RESOLUTION_HEIGHT / 4, 
                8, 
                true, 
                CI_BLACK,
                20
            );
            
            V2 button_Pos = { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 50 };
            int button_Width = 300;
            int button_Height = 100;
            int string_Size = 4;

			if (button_Text(&font_1, "Play", button_Pos, button_Width, button_Height, string_Size)) {
                current_Game_State = GS_GAMELOOP;
			}
            button_Pos.y += 100;
			if (button_Text(&font_1, "Options", button_Pos, button_Width, button_Height, string_Size)) {

			}
            button_Pos.y += 100;
			if (button_Text(&font_1, "Quit", button_Pos, button_Width, button_Height, string_Size)) {
				running = false;
			}
            button_Pos.y += 100;
			if (button_Text(&font_1, "Save Game", button_Pos, button_Width, button_Height, string_Size)) {
				save_Game(&game_Data, "Save_Game.txt");
			}
			button_Pos.y += 100;
			if (button_Text(&font_1, "Load Game", button_Pos, button_Width, button_Height, string_Size)) {
				load_Game(&game_Data, "Save_Game.txt");
			}
            button_Pos.y += 100;

            SDL_RenderPresent(renderer);
        
        }
        else if (current_Game_State == GS_GAMELOOP || current_Game_State == GS_PAUSED) {
            if (key_States[SDLK_UP].held_Down == true) {
                time_Scalar += 0.01f;
            }
            if (key_States[SDLK_DOWN].held_Down == true) {
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

            if (current_Game_State == GS_GAMELOOP && time_Scalar > 0) {
                // Spawn Arrows and update lifetime
                if (key_States[SDLK_SPACE].held_Down == true && game_Data.player_Castle.arrow_Ammo > 0) {
                    if (game_Data.player_Castle.fire_Cooldown.remaining < 0) {
                        V2 target_Mouse = {};
                        int x, y = 0;
                        SDL_GetMouseState(&x, &y);
                        target_Mouse = { (float)x,(float)y };
                        spawn_Arrow(
                            AT_PLAYER_ARROW,
                            &game_Data,
                            game_Data.player_Castle.rigid_Body.position_WS,
                            target_Mouse,
                            LEVEL_1
                        );
                        game_Data.player_Castle.fire_Cooldown.remaining = game_Data.player_Castle.fire_Cooldown.duration;
                        if (game_Data.player_Castle.arrow_Ammo > 0) {
                            game_Data.player_Castle.arrow_Ammo--;
                        } else {
                            game_Data.player_Castle.arrow_Ammo = 0;
                        }
                    }
                }
                game_Data.player_Castle.fire_Cooldown.remaining -= delta_Time;
                
                if (game_Data.player_Castle.arrow_Ammo_Cooldown.remaining < 0) {
                    Castle* player_Castle = &game_Data.player_Castle;

                    player_Castle->arrow_Ammo++;
                    player_Castle->arrow_Ammo_Cooldown.remaining = player_Castle->arrow_Ammo_Cooldown.duration;
                }
                game_Data.player_Castle.arrow_Ammo_Cooldown.remaining -= delta_Time;

                // Spawn Player Skeletons
                if (spawn_Skeleton_Pressed) {
                    Castle* player_Castle = &game_Data.player_Castle;
                    Castle* enemy_Castle = &game_Data.enemy_Castle;
                    spawn_Player_Skeleton(
                        &game_Data,
                        {
                            (float)player_Castle->rigid_Body.position_WS.x,
							((float)game_Data.terrain_Height_Map[(int)player_Castle->rigid_Body.position_WS.x] + get_Sprite_Radius(&player_Castle->sprite_Sheet_Tracker))
                        },
                        enemy_Castle->rigid_Body.position_WS,
                        LEVEL_1
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
							((float)game_Data.terrain_Height_Map[(int)player_Castle->rigid_Body.position_WS.x] + get_Sprite_Radius(&player_Castle->sprite_Sheet_Tracker))
						},
                        enemy_Castle->rigid_Body.position_WS,
                        LEVEL_1
                    );
                    spawn_Archer_Pressed = false;
                }

                // Spawn enemy skeletons
                if (game_Data.enemy_Castle.spawn_Cooldown.remaining < 0) {
					Castle* player_Castle = &game_Data.player_Castle;
					Castle* enemy_Castle = &game_Data.enemy_Castle;
                    // Readability
					float x_Pos = enemy_Castle->rigid_Body.position_WS.x;
					float terrain_height = (float)game_Data.terrain_Height_Map[(int)x_Pos];
					float radius = get_Sprite_Radius(&enemy_Castle->sprite_Sheet_Tracker);
					float y_Pos = terrain_height + radius;
                    spawn_Enemy_Skeleton(
						&game_Data,
                        { x_Pos, y_Pos },
						player_Castle->rigid_Body.position_WS,
						LEVEL_1
					);
                    enemy_Castle->spawn_Cooldown.remaining = enemy_Castle->spawn_Cooldown.duration;
                }
                else {
                    game_Data.enemy_Castle.spawn_Cooldown.remaining -= delta_Time;
                }

                // Update arrow positions
                for (int i = 0; i < game_Data.player_Arrows.size(); i++) {
                    Arrow* arrow = &game_Data.player_Arrows[i];
                    if (!arrow->destroyed) {
                        update_Arrow_Position(arrow, delta_Time);
                    }
                }

#if 0
				if (arrow->stuck_To_Unit.is_Sticking) {
					bool arrow_Currently_Stuck = false;
					for (int j = 0; j < game_Data.enemy_Skeletons.size(); j++) {
						Skeleton* skeleton = &game_Data.enemy_Skeletons[j];
						if (arrow->stuck_To_Unit.ID == skeleton->ID) {
							arrow->rigid_Body.position_WS.x = skeleton->rigid_Body.position_WS.x + arrow->stuck_To_Unit.offset.x;
							arrow->rigid_Body.position_WS.y = skeleton->rigid_Body.position_WS.y + arrow->stuck_To_Unit.offset.y;

							arrow_Currently_Stuck = true;
						}
					}
					if (!arrow_Currently_Stuck) {
						arrow->destroyed = true;
					}
				}
#endif

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
					if (check_Height_Map_Collision(&arrow->rigid_Body, game_Data.terrain_Height_Map))
                    {
                        arrow->stop= true;
                    }
                    // Collision with skeletons and arrows
                    for (int j = 0; j < game_Data.enemy_Skeletons.size(); j++) {
                        Skeleton* enemy_Skeleton = &game_Data.enemy_Skeletons[j];
                        if (check_RB_Collision(&arrow->rigid_Body, &enemy_Skeleton->rigid_Body)) {
                            if (!arrow->stop) {
                                enemy_Skeleton->health_Bar.current_HP -= arrow->damage;
                                V2 offset = arrow->rigid_Body.position_WS - enemy_Skeleton->rigid_Body.position_WS;
                                Attached_Entity attached_Entity = return_Attached_Entity(
                                    SSS_ARROW_DEFAULT,
                                    arrow->rigid_Body.angle,
                                    offset
                                );
                                enemy_Skeleton->attached_Entities[enemy_Skeleton->attached_Entities_Size++] = attached_Entity;
                                arrow->destroyed = true;
                            }
                        }
                    }
                }

                // Collision enemy skeleton with map
                for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.enemy_Skeletons[i];
                    if (check_Height_Map_Collision(&skeleton->rigid_Body, game_Data.terrain_Height_Map)) {
                        float radius = get_Sprite_Radius(&skeleton->sprite_Sheet_Tracker);
                        float pos_Y_HM = (float)game_Data.terrain_Height_Map[(int)skeleton->rigid_Body.position_WS.x];

                        skeleton->rigid_Body.position_WS.y = ((RESOLUTION_HEIGHT - pos_Y_HM) - radius);
                    }
                }

                // Collision player skeletons with map
                for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
                    Skeleton* skeleton = &game_Data.player_Skeletons[i];
                    if (check_Height_Map_Collision(&game_Data.player_Skeletons[i].rigid_Body, game_Data.terrain_Height_Map)) {
                        float radius = get_Sprite_Radius(&skeleton->sprite_Sheet_Tracker);
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
                        float radius = get_Sprite_Radius(&archer->sprite_Sheet_Tracker);
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
                            change_Animation(&archer->sprite_Sheet_Tracker, SSS_ARCHER_STOP);
                            archer->stop = true;
                            if (archer->current_Attack_Cooldown <= 0) {
                                archer->current_Attack_Cooldown = archer->attack_Cooldown;
                                V2 aim_Head = skeleton->rigid_Body.position_WS;
                                Sprite_Sheet_Selector skeleton_Selected = game_Data.enemy_Skeletons[0].sprite_Sheet_Tracker.selected;
                                aim_Head.x += sprite_Sheet_Array[skeleton_Selected].sprites[0].radius;
                                V2 arrow_Spawn_Location = archer->rigid_Body.position_WS;
                                arrow_Spawn_Location.y -= sprite_Sheet_Array[archer->sprite_Sheet_Tracker.selected].sprites[0].radius / 2;
                                spawn_Arrow(
                                    AT_ARCHER_ARROW,
                                    &game_Data,
                                    arrow_Spawn_Location,
                                    aim_Head,
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
            draw_Layer(sprite_Sheet_Array[SSS_BKG_GAMELOOP_1].sprites[0].image->texture);
            draw_Layer(sprite_Sheet_Array[SSS_TERRAIN_1].sprites[0].image->texture);
            draw_Castle(&game_Data.player_Castle, false);
            draw_Castle(&game_Data.enemy_Castle, true);

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);


            draw_Circle(
                game_Data.enemy_Castle.rigid_Body.position_WS.x, 
                (float)(game_Data.enemy_Castle.rigid_Body.position_WS.y), 
                (float)get_Sprite_Radius(&game_Data.enemy_Castle.sprite_Sheet_Tracker), 
                CI_GREEN
            );
            draw_Circle(
                game_Data.player_Castle.rigid_Body.position_WS.x, 
                (float)(game_Data.player_Castle.rigid_Body.position_WS.y),
                (float)get_Sprite_Radius(&game_Data.player_Castle.sprite_Sheet_Tracker), 
                CI_GREEN
            );

            // Draw player arrows
            for (int i = 0; i < game_Data.player_Arrows.size(); i++) {
                Arrow* arrow = &game_Data.player_Arrows[i];
                // draw_RigidBody_Colliders(&arrow->rigid_Body, CI_GREEN);
                if (arrow->life_Time > 0) {
					// draw_Circle(arrow->rigid_Body.position_WS.x, arrow->rigid_Body.position_WS.y, 1, CI_RED); 
                    draw_Arrow(arrow, false);
                    arrow->life_Time -= delta_Time;
                }
            }

            // Draw enemy skeletons
            for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
                Skeleton* skeleton = &game_Data.enemy_Skeletons[i];
                // draw_Circle(skeleton->rigid_Body.position_WS.x, skeleton->rigid_Body.position_WS.y, 5, CI_RED);
                // draw_Circle(skeleton->rigid_Body.position_WS.x, skeleton->rigid_Body.position_WS.y, 6, CI_RED);
                // draw_Circle(skeleton->rigid_Body.position_WS.x, skeleton->rigid_Body.position_WS.y, 7, CI_RED);
                // draw_RigidBody_Colliders(&skeleton->rigid_Body, CI_GREEN);
                draw_Unit_Animated(
                    &skeleton->rigid_Body,
                    &skeleton->sprite_Sheet_Tracker,
                    false
                );
                draw_HP_Bar(&skeleton->rigid_Body.position_WS, &skeleton->health_Bar);
                for (int j = 0; j < skeleton->attached_Entities_Size; j++) {
                    draw_Attached_Entity(&skeleton->attached_Entities[j], skeleton->rigid_Body.position_WS, false);
				}
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
                        CI_Color_Index::RED
                    );
                }
                */
                draw_RigidBody_Colliders(&player_Skeleton->rigid_Body, CI_GREEN);
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
                draw_RigidBody_Colliders(&archer->rigid_Body, CI_GREEN);
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
            
            // UI
            draw_Timer(
                &font_1, 
                { RESOLUTION_WIDTH / 2, (RESOLUTION_HEIGHT / 9) * 0.5 }, 
                6, 
                CI_BLACK, 
                3
            );

            draw_Time_Scalar(
                &font_1, 
                time_Scalar, 
                (int)((RESOLUTION_WIDTH / 16) * 14),
                (int)(RESOLUTION_HEIGHT / 9 * 0.5),
                3
            );

			draw_Arrow_Ammo_Tracker(
				&font_1,
				game_Data.player_Castle.arrow_Ammo,
				{ ((RESOLUTION_WIDTH / 16) * 2), ((RESOLUTION_HEIGHT / 9) * 0.5) },
				3
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

            V2 button_Pos = { (RESOLUTION_WIDTH / 16), ((RESOLUTION_HEIGHT / 9) * 8) };
			int button_Height_Unit_Spawn = 150;
            // int x_Offset = button_Width;
            if (button_Image(sprite_Sheet_Array[SSS_SKELETON_STOP].sprites[0].image->texture, "Spawn Skeleton", button_Pos, button_Height_Unit_Spawn)) {
                spawn_Skeleton_Pressed = true;
            } 
            button_Pos.x += button_Height_Unit_Spawn;
            if (button_Image(sprite_Sheet_Array[SSS_ARCHER_STOP].sprites[0].image->texture, "Spawn Archer", button_Pos, button_Height_Unit_Spawn)) {
				spawn_Archer_Pressed = true;
			}
            button_Pos.x += button_Height_Unit_Spawn;

            if (current_Game_State == GS_PAUSED) {
                int button_Width_Paused = 325;
                int button_Height_Paused = 90;
                int string_Size_Paused = 3;
                V2 button_Pos_Paused = { RESOLUTION_WIDTH / 2 , RESOLUTION_HEIGHT / 2 };
                draw_String_With_Background(
                    &font_1,
                    "Game Paused",
                    RESOLUTION_WIDTH / 2,
                    RESOLUTION_HEIGHT / 2,
                    5,
                    true,
                    CI_BLACK,
                    5
                );
                button_Pos_Paused.y += button_Height_Paused;
				if (button_Text(&font_1, "Return to Menu", button_Pos_Paused, button_Width_Paused, button_Height_Paused, string_Size_Paused)) {
                    current_Game_State = GS_MENU;
				}
                button_Pos_Paused.y += button_Height_Paused;
				if (button_Text(&font_1, "Save Game", button_Pos_Paused, button_Width_Paused, button_Height_Paused, string_Size_Paused)) {
					save_Game(&game_Data, "Save_Game.txt");
				}
                button_Pos_Paused.y += button_Height_Paused;
				if (button_Text(&font_1, "Load Game", button_Pos_Paused, button_Width_Paused, button_Height_Paused, string_Size_Paused)) {
					load_Game(&game_Data, "Save_Game.txt");
				}
                button_Pos_Paused.y += button_Height_Paused;
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