#include <SDL.h>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define RESOLUTION_WIDTH 1920
#define RESOLUTION_HEIGHT 1080
const float GRAVITY = 500;

SDL_Renderer* renderer = NULL;

// Suppress compiler warnings
#define REF(V) ((void)V)

template <typename T>
struct ExitScope
{
    T lambda;
    ExitScope(T lambda) : lambda(lambda) { }
    ~ExitScope() { lambda(); }
    // NO_COPY(ExitScope);
};

struct ExitScopeHelp
{
    template<typename T>
    ExitScope<T> operator+(T t) { return t; }
};

#define _SG_CONCAT(a, b) a ## b
#define SG_CONCAT(a, b) _SG_CONCAT(a, b)
#define DEFER auto SG_CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

struct Vector {
    float x;
    float y;
};

Vector operator+(const Vector& a, const Vector& b) {
    Vector result = {};
    result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

Vector operator-(Vector& a, Vector& b) {
	a.x -= b.x;
	a.y -= b.y;
    return a; 
}

struct Color {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
};

struct Sprite {
    int width;
    int height;
    float radius;
    Vector center;
    SDL_Texture* texture;
};

enum class Button_State {
    NORMAL,
    HOVERED,
    IS_HOT,
    PRESSED
};

enum class Select_Color {
    RED,
    GREEN,
    BLUE
};

enum class Body_Type {
    DYNAMIC,
    STATIC
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
    Body_Type body_Type;
    Vector position_WS;
    Vector velocity;
    std::vector<Collider> colliders;
};

struct Castle {
    Sprite sprite;

    Rigid_Body rigid_Body;
    Health_Bar health_Bar;

    float fire_Recharge;
    float current_Fire_Delay;
    float spawn_Recharge;
    float current_Spawn_Delay;
};

struct Arrow {
    Sprite sprite;

    Rigid_Body rigid_Body;
    
    float damage;
    float speed;
    float life_Time;
    float angle;

    bool stop_Arrow;
    bool destroyed;
};

struct Skeleton {
    Sprite sprite;
    Rigid_Body rigid_Body;
    Health_Bar health_Bar;

    float speed;
    float damage;
    float attack_Rate;
    float current_Cooldown;
    float angle;
   
    bool destroyed;
    bool stop_Skeleton;
};

struct Game_Data {
    Castle                  player_Castle;
    Castle                  enemy_Castle;
    std::vector<int>        terrain_Height_Map;
    std::vector<Arrow>      player_Arrows;
	std::vector<Skeleton>   enemy_Skeletons;
	std::vector<Skeleton>   player_Skeletons;
};

void my_Memory_Copy(void* dest, const void* src, size_t count) {
    unsigned char* destination = (unsigned char*)dest;
    unsigned char* source = (unsigned char*)src;
    for (int i = 0; i < count; i++) {
        destination[i] = source[i];
    }
}

float calculate_Distance(float x1, float y1, float x2, float y2) {
    return (float)sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void add_Collider(std::vector<Collider>* colliders, Vector position_LS, float radius) {
    Collider collider = {};
    collider.position_LS = position_LS;
    collider.radius = radius;
    
    colliders->push_back(collider);
}

// Advanced Technique:
// Another fix to returning a bad image is to have a backup image that 
// gets set that indicates bad things. (Can't load character model in wow,
// so the character model gets a cube. The backup image won't be loaded 
// off the disk. It will be something that shows up as a color like purple.
// You don't want the backup texture to be missing)
Sprite create_Sprite(const char* file_name) {
    Sprite result = {};

    int width, height, channels;
    unsigned char* data = stbi_load(file_name, &width, &height, &channels, 4);

    if (data == NULL) {
        SDL_Log("ERROR: stbi_load returned NULL: %s", SDL_GetError());
        return result;
    }

    result.width = width;
    result.height = height;

    result.center = { (float)width / 2, (float)height / 2 };
    
    float max_Distance = 0;

    for (int y = 0; y < result.height; y++) {
        for (int x = 0; x < result.width; x++) {
            int index = 0;
            index = (4 * ((y * width) + x)) + 3;
            if (data[index] != 0) {
                float distance = calculate_Distance((float)x, (float)y, result.center.x, result.center.y);
                if (distance > max_Distance) {
                    max_Distance = distance;
                }
            }
        }
    }

    result.radius = max_Distance;

    DEFER{
        stbi_image_free(data);
    };
    
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

void draw_Layer(Sprite sprite) {
    SDL_RenderCopyEx(renderer, sprite.texture, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
}

void draw_Castle(Castle* castle, bool flip) {
    SDL_Rect temp = {};
    temp = { 
		((int)castle->rigid_Body.position_WS.x - (int)castle->sprite.center.x),
		((int)castle->rigid_Body.position_WS.y - (int)castle->sprite.center.y),
        castle->sprite.width, 
        castle->sprite.height 
    };
    SDL_RenderCopyEx(renderer, castle->sprite.texture, NULL, &temp, 0, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

void draw_Arrow(Arrow* arrow, bool flip) {
    SDL_Rect temp = {};
    temp = {
        ((int)arrow->rigid_Body.position_WS.x - (int)arrow->sprite.center.x),
        ((int)arrow->rigid_Body.position_WS.y - (int)arrow->sprite.center.y),
        arrow->sprite.width,
        arrow->sprite.height
    };
    SDL_RenderCopyEx(renderer, arrow->sprite.texture, NULL, &temp, arrow->angle, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

void draw_Skeleton(Skeleton* skeleton, bool flip) {
    SDL_Rect temp = {};
    temp = {
        ((int)skeleton->rigid_Body.position_WS.x - (int)skeleton->sprite.center.x),
        ((int)skeleton->rigid_Body.position_WS.y - (int)skeleton->sprite.center.y),
        skeleton->sprite.width,
        skeleton->sprite.height
    };
    SDL_RenderCopyEx(renderer, skeleton->sprite.texture, NULL, &temp, skeleton->angle, NULL, (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

void update_Arrow_Position(Arrow* arrow, float delta_Time) {
	if (!arrow->stop_Arrow) {
		arrow->rigid_Body.velocity.y += GRAVITY * delta_Time;

		arrow->rigid_Body.position_WS.x += arrow->rigid_Body.velocity.x * delta_Time;
		arrow->rigid_Body.position_WS.y += arrow->rigid_Body.velocity.y * delta_Time;

		arrow->angle = (float)(atan2(arrow->rigid_Body.velocity.y, arrow->rigid_Body.velocity.x) * (180 / M_PI));

        float angle_In_Radians = arrow->angle * (float)(M_PI / 180.0);

        float length_Offset = arrow->sprite.radius - arrow->rigid_Body.colliders[0].radius;

		float tip_Offset_X = (float)cos(angle_In_Radians) * length_Offset;
		float tip_Offset_Y = (float)sin(angle_In_Radians) * length_Offset;

        // First collider is the tip
        if (!arrow->rigid_Body.colliders.empty()) {
			arrow->rigid_Body.colliders[0].position_LS.x = tip_Offset_X;
			arrow->rigid_Body.colliders[0].position_LS.y = tip_Offset_Y;
        }

	}
}

void update_Skeleton_Position(Skeleton* skeleton, float delta_Time) {
    if (!skeleton->stop_Skeleton) {
        skeleton->rigid_Body.position_WS.y += GRAVITY * delta_Time;
        skeleton->rigid_Body.position_WS.x += (skeleton->rigid_Body.velocity.x * delta_Time);
        skeleton->rigid_Body.position_WS.y += (skeleton->rigid_Body.velocity.y * delta_Time);
    }
}

void spawn_Arrow(Arrow* arrow, Vector* spawn_Position, Vector* target_Position) {
    arrow->rigid_Body.position_WS.x = spawn_Position->x;
    arrow->rigid_Body.position_WS.y = spawn_Position->y;

    Vector direction_Vector = {};
    direction_Vector.x = target_Position->x - arrow->rigid_Body.position_WS.x;
    direction_Vector.y = target_Position->y - arrow->rigid_Body.position_WS.y;
    
    float length = (float)sqrt((direction_Vector.x * direction_Vector.x) + (direction_Vector.y * direction_Vector.y));

    direction_Vector.x /= length;
    direction_Vector.y /= length;

    arrow->rigid_Body.velocity.x = direction_Vector.x * arrow->speed;
    arrow->rigid_Body.velocity.y = direction_Vector.y * arrow->speed;
}

void spawn_Skeleton(Skeleton* unit_Skeleton, Vector spawn_Location, Castle* target_Castle) {
    unit_Skeleton->rigid_Body.position_WS.x = spawn_Location.x;
    unit_Skeleton->rigid_Body.position_WS.y = spawn_Location.y;

    Vector direction_Vector = {};
    direction_Vector.x = target_Castle->rigid_Body.position_WS.x - unit_Skeleton->rigid_Body.position_WS.x;
    direction_Vector.y = target_Castle->rigid_Body.position_WS.y - unit_Skeleton->rigid_Body.position_WS.y;

    // Magnitude of the Vector
    float length = (float)sqrt((direction_Vector.x * direction_Vector.x) + (direction_Vector.y * direction_Vector.y));

    // Normalize
    direction_Vector.x /= length;
    direction_Vector.y /= length;

    // Set the new velocity
    unit_Skeleton->rigid_Body.velocity.x = direction_Vector.x * unit_Skeleton->speed;
    unit_Skeleton->rigid_Body.velocity.y = direction_Vector.y * unit_Skeleton->speed;
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
		Vector world_Position = rigid_Body->position_WS + collider.position_LS;
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

float linear_Interpolation(float left_Point, float right_Point, float percent) {
    // Lerp of T = A * (1 - T) + B * T
    // A is the left side, B is the right side, T is the percentage of the interpolation
    return ((left_Point) * (1 - percent) + (right_Point)*percent);
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

void draw_Timer(Font* font, Vector position, int timer_Size) {
    SDL_Rect temp = {};
    Uint64 time_Elapsed = SDL_GetTicks64() / 1000;

    /*
    temp.w = (int)(font->char_Width * timer_Size) + 6;
    temp.h = (int)(font->char_Height * timer_Size) + 6;
    temp.x = (int)((position.x) - ((font->char_Width + 6) / 2));
    temp.y = (int)((position.y) - ((font->char_Height + 6) / 2));
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &temp);
    */

    std::string str = std::to_string(time_Elapsed);
    const char* ptr = str.c_str();
    draw_String(font, ptr, (int)(position.x - (font->char_Width / 2)), (int)(position.y - (font->char_Height / 2)), timer_Size, true);
}

bool mouse_Down_This_Frame = false;
std::string current_frame_Hot_Name;
std::string next_Frame_Hot_Name;
bool draw_Rect = false;

bool button(Font* font, const char* string, int position_X, int position_Y, int w, int h) {
    SDL_Rect button_Area = {};
    // Centers the button
    button_Area.x = (position_X - (w / 2));
    button_Area.y = (position_Y - (h / 2));
    button_Area.w = w;
    button_Area.h = h;

    // Set background as black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &button_Area);

    // Calculate the size for the string based off the width of the button
    size_t length_Pixels = strlen(string);
    length_Pixels *= font->char_Width;
    int size = (w / (int)length_Pixels);

    draw_String(font, string, position_X, position_Y, size, true);
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
        SDL_Log("ERROR: stbi_load returned NULL: %s", SDL_GetError());
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
		Vector world_Position = rigid_Body->position_WS + collider.position_LS;

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
bool check_RB_Collision(const Rigid_Body* rigid_Body_1, const Rigid_Body* rigid_Body_2) {
	// Apply rotation at this point
    // LOCAL POSITION DOES NOT CHANGE
    // SET THE LOCAL POSITION ONE TIME BUT THAT'S IT. Unless I want to animate the collider.
    for (const auto& collider_1 : rigid_Body_1->colliders) {
		Vector world_Pos_1 = rigid_Body_1->position_WS + collider_1.position_LS;

		for (const auto& collider_2 : rigid_Body_2->colliders) {
			Vector world_Pos_2 = rigid_Body_2->position_WS + collider_2.position_LS;

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

    // Create sprites
    Sprite arrow_Sprite = create_Sprite("images/arrow.png");
    // Sprite castle_Sprite = create_Sprite(renderer, "images/player_Castle.png");
    Sprite castle_Sprite = create_Sprite("images/castle_2.png");
    Sprite skeleton_Sprite = create_Sprite("images/unit_Skeleton.png");
    Sprite archer_Sprite = create_Sprite("images/unit_Archer.png");

    Sprite background = create_Sprite("images/background.jpg");
    // Sprite background = create_Sprite(renderer, "images/background_2.png");
    // Sprite background = create_Sprite(renderer, "images/background_3.png");
    Sprite terrain = create_Sprite("images/collision_Terrain_1.png");
    
    Game_Data game_Data = {};

    game_Data.terrain_Height_Map = create_Height_Map("images/collision_Terrain_1.png");

    Health_Bar castle_Health_Bar = {
        .max_HP = 100,
        .current_HP = castle_Health_Bar.max_HP,
		.width = 90,
	    .height = 20,
        .y_Offset = 115,
        .thickness = 3
    };

    Rigid_Body player_Castle_RB = {
        .body_Type = Body_Type::STATIC,
        .position_WS = { (RESOLUTION_WIDTH * 0.05) , (float)game_Data.terrain_Height_Map[(int)(RESOLUTION_WIDTH * 0.05)] },
        .velocity = { 0.0f, 0.0f },
        .colliders = {}
    };
    game_Data.player_Castle = {
		.sprite = castle_Sprite,
		.rigid_Body = player_Castle_RB,
		.health_Bar = castle_Health_Bar,
		.fire_Recharge = 0.01f,
		.current_Fire_Delay = game_Data.player_Castle.fire_Recharge,
		.spawn_Recharge = 0.0f,
		.current_Spawn_Delay = game_Data.player_Castle.spawn_Recharge
    };
    add_Collider(&game_Data.player_Castle.rigid_Body.colliders, { 0.0f, 0.0f }, game_Data.player_Castle.sprite.radius);

    Rigid_Body enemy_Castle_RB = {
		.body_Type = Body_Type::STATIC,
		.position_WS = { (RESOLUTION_WIDTH * 0.95) , (float)game_Data.terrain_Height_Map[(int)(RESOLUTION_WIDTH * 0.95)] },
		.velocity = { 0.0f, 0.0f },
		.colliders = {}
	};
	game_Data.enemy_Castle= {
	    .sprite = castle_Sprite,
	    .rigid_Body = enemy_Castle_RB,
	    .health_Bar = castle_Health_Bar,
	    .fire_Recharge = 0.0f,
	    .current_Fire_Delay = game_Data.player_Castle.fire_Recharge,
	    .spawn_Recharge = 30.0f,
	    .current_Spawn_Delay = game_Data.player_Castle.spawn_Recharge
	};
	add_Collider(&game_Data.enemy_Castle.rigid_Body.colliders, { 0.0f, 0.0f }, game_Data.enemy_Castle.sprite.radius);

	Rigid_Body arrow_RB = {
		.body_Type = Body_Type::DYNAMIC,
		.position_WS = { 0.0f, 0.0f },
		.velocity = { 0.0f, 0.0f },
		.colliders = {}
	};
    Arrow player_Arrow = {
		.sprite = arrow_Sprite,
        .rigid_Body = arrow_RB,
        .damage = 5,
		.speed = 750,
		.life_Time = 10,
		.angle = 0.0f,
        .stop_Arrow = false,
	    .destroyed = false
    };
    // Arrow local position wouldn't be 0,0
    add_Collider(&player_Arrow.rigid_Body.colliders, { 0.0f, 0.0f }, (player_Arrow.sprite.radius * 0.25f));

	Health_Bar unit_Health_Bar = {
	    .max_HP = 100,
	    .current_HP = castle_Health_Bar.max_HP,
	    .width = 65,
	    .height = 15,
	    .y_Offset = 60,
	    .thickness = 2
	};

	Health_Bar skeleton_Health_Bar = unit_Health_Bar;
	Rigid_Body skeleton_RB = {
		.body_Type = Body_Type::DYNAMIC,
		.position_WS = { 0.0f , 0.0f },
		.velocity = { 0.0f, 0.0f },
		.colliders = {}
	};
    Skeleton unit_Skeleton = {
		.sprite = skeleton_Sprite,
		.rigid_Body = skeleton_RB,
		.health_Bar = skeleton_Health_Bar,

		.speed = 200,
		.damage = 20,
		.attack_Rate = 1,
		.current_Cooldown = 0.0f,
		.angle = 0.0f,

		.destroyed = false,
		.stop_Skeleton = false
    };
    add_Collider(&unit_Skeleton.rigid_Body.colliders, { 0.0f, 0.0f }, (unit_Skeleton.sprite.radius));

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

    float ticks = 0;
    float last_Ticks = 0;
    float delta_Time = 0;

    bool key_Space_Pressed = false;

    bool spawn_Skeleton_Pressed = false;

    bool running = true;
    while (running) {
        mouse_Down_This_Frame = false;
        SDL_Event event = {};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            // Key down gives events based off keyboard repeat-rate (Operating system)
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    key_Space_Pressed = true;
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
                    key_Space_Pressed = false;
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

        last_Ticks = ticks;
        ticks = (float)SDL_GetTicks64();
        delta_Time = ticks - last_Ticks;
        delta_Time /= 1000;

        current_frame_Hot_Name = next_Frame_Hot_Name;
        next_Frame_Hot_Name = "";
        SDL_GetMouseState(&mouse_X, &mouse_Y);

        // Spawn Arrows and update lifetime
        if (key_Space_Pressed) {
            if (game_Data.player_Castle.current_Fire_Delay < 0) {
				Vector target_Mouse = {};
				int x, y = 0;
				SDL_GetMouseState(&x, &y);
                target_Mouse = { (float)x,(float)y };
                spawn_Arrow(&player_Arrow, &game_Data.player_Castle.rigid_Body.position_WS, &target_Mouse);
                game_Data.player_Arrows.push_back(player_Arrow);
                game_Data.player_Castle.current_Fire_Delay = game_Data.player_Castle.fire_Recharge;
            }
            else {
                game_Data.player_Castle.current_Fire_Delay -= delta_Time;
            }
        }
        else {
            game_Data.player_Castle.current_Fire_Delay -= delta_Time;
        }

		// Spawn Player Skeletons
		if (spawn_Skeleton_Pressed) {
            spawn_Skeleton(
                &unit_Skeleton, 
                { (float)game_Data.player_Castle.rigid_Body.position_WS.x,
                ((float)game_Data.terrain_Height_Map[(int)game_Data.player_Castle.rigid_Body.position_WS.x] + game_Data.enemy_Castle.sprite.radius) },
                &game_Data.enemy_Castle);
            game_Data.player_Skeletons.push_back(unit_Skeleton);
			spawn_Skeleton_Pressed = false;
		}

        // Spawn enemy skeletons
        if (game_Data.enemy_Castle.current_Spawn_Delay < 0) {
            spawn_Skeleton(
                &unit_Skeleton, 
                { (float)game_Data.enemy_Castle.rigid_Body.position_WS.x,
                ((float)game_Data.terrain_Height_Map[(int)game_Data.enemy_Castle.rigid_Body.position_WS.x] + game_Data.enemy_Castle.sprite.radius) },
                &game_Data.player_Castle);
            game_Data.enemy_Skeletons.push_back(unit_Skeleton);
            game_Data.enemy_Castle.current_Spawn_Delay = game_Data.enemy_Castle.spawn_Recharge;
        }
        else {
            game_Data.enemy_Castle.current_Spawn_Delay -= delta_Time;
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
				update_Skeleton_Position(&game_Data.player_Skeletons[i], delta_Time);
			}
		}

        // Update enemy skeleton positions
        for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
            if (game_Data.enemy_Skeletons[i].destroyed == false) {
                update_Skeleton_Position(&game_Data.enemy_Skeletons[i], delta_Time);
            }
        }

        // Collision detection for arrows
        for (int i = 0; i < game_Data.player_Arrows.size(); i++) {
            if (check_RB_Collision(&game_Data.player_Arrows[i].rigid_Body, &game_Data.enemy_Castle.rigid_Body)) {
                game_Data.enemy_Castle.health_Bar.current_HP -= game_Data.player_Arrows[i].damage;
                game_Data.player_Arrows[i].destroyed = true;
            }
            // Collision with map
            if (check_Height_Map_Collision(
				&game_Data.player_Arrows[i].rigid_Body,
                game_Data.terrain_Height_Map))
            {
                game_Data.player_Arrows[i].stop_Arrow = true;
            }
            // Collision with skeletons and arrows
            for (int j = 0; j < game_Data.enemy_Skeletons.size(); j++) {
                if (check_RB_Collision(&game_Data.player_Arrows[i].rigid_Body, &game_Data.enemy_Skeletons[j].rigid_Body)) {
                    if (!game_Data.player_Arrows[i].stop_Arrow) {
                        game_Data.enemy_Skeletons[j].health_Bar.current_HP -= game_Data.player_Arrows[i].damage;
                        game_Data.player_Arrows[i].destroyed = true;
                    }
                }
            }
        }

        // Collision enemy skeleton with map
        for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
            if (check_Height_Map_Collision(&game_Data.enemy_Skeletons[i].rigid_Body, game_Data.terrain_Height_Map)) {
                game_Data.enemy_Skeletons[i].rigid_Body.position_WS.y = ((RESOLUTION_HEIGHT - (float)game_Data.terrain_Height_Map[(int)game_Data.enemy_Skeletons[i].rigid_Body.position_WS.x]) - game_Data.enemy_Skeletons[i].sprite.radius);
            }
        }

        // (check_Height_Map_Collision((int)game_Data.player_Skeletons[i].rigid_Body.position_WS.x, ((int)game_Data.player_Skeletons[i].rigid_Body.position_WS.y + (int)game_Data.player_Skeletons[i].sprite.radius), game_Data.terrain_Height_Map)
        // Collision player skeletons with map
        for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
            if (check_Height_Map_Collision(&game_Data.player_Skeletons[i].rigid_Body, game_Data.terrain_Height_Map)) {
                game_Data.player_Skeletons[i].rigid_Body.position_WS.y = ((RESOLUTION_HEIGHT - (float)game_Data.terrain_Height_Map[(int)game_Data.player_Skeletons[i].rigid_Body.position_WS.x]) - game_Data.player_Skeletons[i].sprite.radius);
            }
        }

		// Collision with skeletons
        // NOTE: Set stop_Skeleton false for all the skeletons
        // or else it overrides 'true' in the nested loop
		for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
			game_Data.player_Skeletons[i].stop_Skeleton = false;
		}
		for (int j = 0; j < game_Data.enemy_Skeletons.size(); j++) {
			game_Data.enemy_Skeletons[j].stop_Skeleton = false;
		}

        // Collision player skeleton with enemy castle
        for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
            if (check_RB_Collision(&game_Data.player_Skeletons[i].rigid_Body, &game_Data.enemy_Castle.rigid_Body)) {
                game_Data.player_Skeletons[i].stop_Skeleton = true;
                if (game_Data.player_Skeletons[i].current_Cooldown < 0) {
                    game_Data.player_Skeletons[i].current_Cooldown = game_Data.player_Skeletons[i].attack_Rate;
                    game_Data.enemy_Castle.health_Bar.current_HP -= game_Data.player_Skeletons[i].damage;
                }
                else {
                    game_Data.player_Skeletons[i].current_Cooldown -= delta_Time;
                }
            }
        }
		// Collision enemy skeleton with player castle
		for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
			if (check_RB_Collision(&game_Data.enemy_Skeletons[i].rigid_Body, &game_Data.player_Castle.rigid_Body)) {
				game_Data.enemy_Skeletons[i].stop_Skeleton = true;
				if (game_Data.enemy_Skeletons[i].current_Cooldown < 0) {
					game_Data.enemy_Skeletons[i].current_Cooldown = game_Data.enemy_Skeletons[i].attack_Rate;
					game_Data.player_Castle.health_Bar.current_HP -= game_Data.enemy_Skeletons[i].damage;
				}
				else {
					game_Data.enemy_Skeletons[i].current_Cooldown -= delta_Time;
				}
			}
		}

        // Skeletons colliding with eachother
        for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
            for (int j = 0; j < game_Data.enemy_Skeletons.size(); j++) {
                if (check_RB_Collision(&game_Data.player_Skeletons[i].rigid_Body, &game_Data.enemy_Skeletons[j].rigid_Body)) {
                    game_Data.player_Skeletons[i].stop_Skeleton = true;
                    game_Data.enemy_Skeletons[j].stop_Skeleton = true;
                    if (game_Data.player_Skeletons[i].current_Cooldown <= 0) {
                        game_Data.player_Skeletons[i].current_Cooldown = game_Data.player_Skeletons[i].attack_Rate;
                        game_Data.enemy_Skeletons[j].health_Bar.current_HP -= game_Data.player_Skeletons[i].damage;
                    }
                    else {
                        game_Data.player_Skeletons[i].current_Cooldown -= delta_Time;
                    }
                    if (game_Data.enemy_Skeletons[j].current_Cooldown <= 0) {
                        game_Data.enemy_Skeletons[j].current_Cooldown = game_Data.enemy_Skeletons[j].attack_Rate;
                        game_Data.player_Skeletons[i].health_Bar.current_HP -= game_Data.enemy_Skeletons[j].damage;
                    }
                    else {
                        game_Data.enemy_Skeletons[j].current_Cooldown -= delta_Time;
                    }
                    // Enemy skeletons with player castle
					if (check_RB_Collision(&game_Data.enemy_Skeletons[j].rigid_Body, &game_Data.player_Castle.rigid_Body)) {
						game_Data.enemy_Skeletons[i].stop_Skeleton = true;
						if (game_Data.enemy_Skeletons[i].current_Cooldown < 0) {
							game_Data.enemy_Skeletons[i].current_Cooldown = game_Data.enemy_Skeletons[i].attack_Rate;
							game_Data.player_Castle.health_Bar.current_HP -= game_Data.enemy_Skeletons[i].damage;
						}
						else {
							game_Data.enemy_Skeletons[i].current_Cooldown -= delta_Time;
						}
					}
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // ***Renderering happens here***
        draw_Layer(background);
        draw_Layer(terrain);
        draw_Castle(&game_Data.player_Castle, false);
        draw_Castle(&game_Data.enemy_Castle, true);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

        draw_Circle(game_Data.enemy_Castle.rigid_Body.position_WS.x, (float)(game_Data.enemy_Castle.rigid_Body.position_WS.y), game_Data.enemy_Castle.sprite.radius, Select_Color::GREEN);
		draw_Circle(game_Data.player_Castle.rigid_Body.position_WS.x, (float)((int)game_Data.player_Castle.rigid_Body.position_WS.y), game_Data.player_Castle.sprite.radius, Select_Color::GREEN);

        // Draw player arrows
        for (int i = 0; i < game_Data.player_Arrows.size(); i++) {
            draw_RigidBody_Colliders(&game_Data.player_Arrows[i].rigid_Body, Select_Color::GREEN);
            if (game_Data.player_Arrows[i].life_Time > 0) {
                draw_Arrow(&game_Data.player_Arrows[i], false);
                game_Data.player_Arrows[i].life_Time -= delta_Time;
            } 
        }

        // Draw enemy skeletons
        for (int i = 0; i < game_Data.enemy_Skeletons.size(); i++) {
            draw_RigidBody_Colliders(&game_Data.enemy_Skeletons[i].rigid_Body, Select_Color::GREEN);
            draw_Skeleton(&game_Data.enemy_Skeletons[i], false);
            draw_HP_Bar(&game_Data.enemy_Skeletons[i].rigid_Body.position_WS, &game_Data.enemy_Skeletons[i].health_Bar);
        }

        // Draw player skeletons
        for (int i = 0; i < game_Data.player_Skeletons.size(); i++) {
            draw_RigidBody_Colliders(&game_Data.player_Skeletons[i].rigid_Body, Select_Color::GREEN);
            draw_Skeleton(&game_Data.player_Skeletons[i], false);
            draw_HP_Bar(&game_Data.player_Skeletons[i].rigid_Body.position_WS, &game_Data.player_Skeletons[i].health_Bar);
        }

        draw_HP_Bar(&game_Data.player_Castle.rigid_Body.position_WS, &game_Data.player_Castle.health_Bar);
		draw_HP_Bar(&game_Data.enemy_Castle.rigid_Body.position_WS, &game_Data.enemy_Castle.health_Bar);

        draw_Timer(&font_1, { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 30 }, 6);

        if (button(&font_1, "Spawn Skeleton", (RESOLUTION_WIDTH / 8), ((RESOLUTION_HEIGHT / 16) * 15), 450, 100)) {
            spawn_Skeleton_Pressed = true;
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

        SDL_RenderPresent(renderer);
    }

    return 0;
}