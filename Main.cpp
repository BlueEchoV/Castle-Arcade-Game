#include <SDL.h>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define RESOLUTION_WIDTH 1920
#define RESOLUTION_HEIGHT 1080

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

struct Font {
    int width;
    int height;
    SDL_Texture* texture;
    int char_Height;
    int char_Width;
};

struct Button {
    // Size and position
    SDL_Rect rect;
    Font* font;
    Button_State state;
    const char* string;
    // Could a texture later down the road
};

struct Player {
    Sprite sprite;
    Vector position;
    float speed;
};

struct Castle {
    Sprite sprite;
    Vector position;
    float fire_Interval;
    float current_HP;
    float max_HP;
    float spawn_Interval;
};

struct Arrow {
    Sprite sprite;
    Vector position;
    Vector velocity;
    bool stop_Arrow;
    Vector collision_Offset;
    float collision_Radius;
    float speed;
    float life_Time;
    float angle;
    bool destroyed;
    float damage;
    float gravity;
};

struct Skeleton {
    Sprite sprite;
    Vector position;
    Vector velocity;
    float current_HP;
    float max_HP;
    float speed;
    bool destroyed;
    float gravity;
    float damage;
    float angle;
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

// Advanced Technique:
// Another fix to returning a bad image is to have a backup image that 
// gets set that indicates bad things. (Can't load character model in wow,
// so the character model gets a cube. The backup image won't be loaded 
// off the disk. It will be something that shows up as a color like purple.
// You don't want the backup texture to be missing)
Sprite create_Sprite(SDL_Renderer* renderer, const char* file_name) {
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

Player create_Player(Sprite sprite, Vector position, float speed) {
    Player result = {};
    result.sprite = sprite;
    result.speed = speed;
    // Image origin top left. Change it to the center.
    result.position.x = position.x - sprite.center.x;
    result.position.y = position.y - sprite.center.y;

    return result;
}

Castle create_Castle(Sprite sprite, Vector position, float max_HP, float fire_Interval) {
    Castle result = {};
    result.sprite = sprite;
    result.position.x = position.x;
    result.position.y = position.y;
    result.fire_Interval = fire_Interval;
    result.max_HP = max_HP;
    result.current_HP = max_HP;
    result.spawn_Interval = 0;
    return result;
}

Arrow create_Arrow(Sprite sprite, float speed, float damage, float gravity) {
    Arrow result = {};
    result.sprite = sprite;
    result.speed = speed;
    result.life_Time = 10;
    result.destroyed = false;
    result.stop_Arrow = false;
    result.damage = damage;
    result.gravity = gravity;
    return result;
}   

Skeleton create_Skeleton(Sprite sprite, float speed, float damage, float max_HP, float gravity) {
    Skeleton result = {};
    result.sprite = sprite;
    result.speed = speed;
    result.destroyed = false;
    result.damage = damage;
    result.max_HP = max_HP;
    result.current_HP = max_HP;
    result.gravity = gravity;
    return result;
}

void draw_Layer(SDL_Renderer* renderer, Sprite sprite) {
    SDL_RenderCopyEx(renderer, sprite.texture, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
}

void draw_Castle(SDL_Renderer* renderer, Castle castle, bool flip) {
    SDL_Rect temp = {};
    temp = { 
        ((int)castle.position.x - (int)castle.sprite.center.x), 
        ((int)castle.position.y - (int)castle.sprite.center.y),
        castle.sprite.width, 
        castle.sprite.height 
    };
    SDL_RenderCopyEx(renderer, castle.sprite.texture, NULL, &temp, 0, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void draw_Arrow(SDL_Renderer* renderer, Arrow* arrow, bool flip) {
    SDL_Rect temp = {};
    temp = {
        ((int)arrow->position.x - (int)arrow->sprite.center.x),
        ((int)arrow->position.y - (int)arrow->sprite.center.y),
        arrow->sprite.width,
        arrow->sprite.height
    };
    SDL_RenderCopyEx(renderer, arrow->sprite.texture, NULL, &temp, arrow->angle, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void draw_Skeleton(SDL_Renderer* renderer, Skeleton* skeleton, bool flip) {
    SDL_Rect temp = {};
    temp = {
        ((int)skeleton->position.x - (int)skeleton->sprite.center.x),
        ((int)skeleton->position.y - (int)skeleton->sprite.center.y),
        skeleton->sprite.width,
        skeleton->sprite.height
    };
    SDL_RenderCopyEx(renderer, skeleton->sprite.texture, NULL, &temp, skeleton->angle, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void update_Arrow_Position(Arrow* arrow, float delta_Time) {
    if (!arrow->stop_Arrow) {
        arrow->velocity.y += arrow->gravity * delta_Time;
        arrow->position.x += (arrow->velocity.x * delta_Time);
        arrow->position.y += (arrow->velocity.y * delta_Time);

        arrow->angle = (float)(((float)atan2(arrow->velocity.y, arrow->velocity.x) * (180 / M_PI)));
    }
}

void update_Skeleton_Position(Skeleton* skeleton, float delta_Time) {
    skeleton->velocity.y += skeleton->gravity * delta_Time;
    skeleton->position.x += (skeleton->velocity.x * delta_Time);
    skeleton->position.y += (skeleton->velocity.y * delta_Time);
}

void update_Arrow_Collision(Arrow* arrow) {
    if (!arrow->stop_Arrow) {
        float angle_In_Radians = (arrow->angle * (float)(M_PI / 180));
        float x_Offset = ((arrow->sprite.radius - arrow->collision_Radius) * (float)cos(angle_In_Radians));
        float y_Offset = ((arrow->sprite.radius - arrow->collision_Radius) * (float)sin(angle_In_Radians));
        arrow->collision_Offset.x = arrow->position.x + x_Offset;
        arrow->collision_Offset.y = arrow->position.y + y_Offset;
    }
}

void spawn_Arrow(Arrow* player_Arrow, Castle* castle) {
    player_Arrow->position.x = castle->position.x;
    player_Arrow->position.y = castle->position.y;

    // Calculate the direction Vector
    Vector mouse = {};
    int x, y = 0;
    SDL_GetMouseState(&x, &y);
    mouse = { (float)x,(float)y };

    Vector direction_Vector = {};
    direction_Vector.x = mouse.x - player_Arrow->position.x;
    direction_Vector.y = mouse.y - player_Arrow->position.y;
    
    // Magnitude of the Vector
    float length = (float)sqrt((direction_Vector.x * direction_Vector.x) + (direction_Vector.y * direction_Vector.y));

    // Normalize
    direction_Vector.x /= length;
    direction_Vector.y /= length;

    // Set the new velocity
    player_Arrow->velocity.x = direction_Vector.x * player_Arrow->speed;
    player_Arrow->velocity.y = direction_Vector.y * player_Arrow->speed;
}


void spawn_Skeleton(Skeleton* unit_Skeleton, Castle* spawn_Castle, Castle* target_Castle) {
    unit_Skeleton->position.x = spawn_Castle->position.x;
    unit_Skeleton->position.y = spawn_Castle->position.y;

    Vector direction_Vector = {};
    direction_Vector.x = target_Castle->position.x - unit_Skeleton->position.x;
    direction_Vector.y = target_Castle->position.y - unit_Skeleton->position.y;

    // Magnitude of the Vector
    float length = (float)sqrt((direction_Vector.x * direction_Vector.x) + (direction_Vector.y * direction_Vector.y));

    // Normalize
    direction_Vector.x /= length;
    direction_Vector.y /= length;

    // Set the new velocity
    unit_Skeleton->velocity.x = direction_Vector.x * unit_Skeleton->speed;
    unit_Skeleton->velocity.y = direction_Vector.y * unit_Skeleton->speed;
}

void draw_Circle(SDL_Renderer* renderer, float center_X, float center_Y, float radius) {
    float total_Lines = 30;
    float line_Step = (float)(2 * M_PI) / total_Lines;
    int x1, y1, x2, y2 = 0;

    for (float angle = 0; angle < (2 * M_PI); angle += line_Step) {
        // sin and cos gives us the ratio of the length
        x1 = (int)(center_X + (radius * cos(angle - line_Step)));
        y1 = (int)(center_Y + (radius * sin(angle - line_Step)));
        x2 = (int)(center_X + (radius * cos(angle)));
        y2 = (int)(center_Y + (radius * sin(angle)));
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

void outline_Rect(SDL_Renderer* renderer, SDL_Rect* rect, int outline_Thickness) {

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

void draw_Castle_HP_Bar(SDL_Renderer* renderer, Castle* castle, int width, int height, int y_Offset, int thickness) {
    float remaining_HP_Percent = (castle->current_HP / castle->max_HP);
    if (remaining_HP_Percent < 0) {
        remaining_HP_Percent = 0;
    }

    // Lerp of T = A * (1 - T) + B * T
    // A is the left side, B is the right side, T is the health %
    float lerp = (0) * (1 - remaining_HP_Percent) + (width) * remaining_HP_Percent;

    SDL_Rect rect_Green = {};
    rect_Green.w = (int)lerp;
    rect_Green.h = (int)height;
    rect_Green.x = (int)((castle->position.x) - width / 2);
    rect_Green.y = (int)((castle->position.y) - y_Offset);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect_Green);

    SDL_Rect rect_Red = rect_Green;
    rect_Red.w = width - rect_Green.w;
    rect_Red.x = (int)(rect_Green.x + rect_Green.w);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect_Red);

    // Outline HP bars
    SDL_Rect outline = {};
    outline.w = (int)width;
    outline.h = (int)height;
    outline.x = (int)((castle->position.x) - width / 2);
    outline.y = (int)((castle->position.y) - y_Offset);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    outline_Rect(renderer, &outline, thickness);
}

void draw_Skeleton_HP_Bar(SDL_Renderer* renderer, Skeleton* skeleton, int width, int height, int y_Offset, int thickness) {
    float remaining_HP_Percent = (skeleton->current_HP / skeleton->max_HP);
    if (remaining_HP_Percent < 0) {
        remaining_HP_Percent = 0;
    }

    // Lerp of T = A * (1 - T) + B * T
    // A is the left side, B is the right side, T is the health %
    float lerp = (0) * (1 - remaining_HP_Percent) + (width)*remaining_HP_Percent;

    SDL_Rect rect_Green = {};
    rect_Green.w = (int)lerp;
    rect_Green.h = (int)height;
    rect_Green.x = (int)((skeleton->position.x) - width / 2);
    rect_Green.y = (int)((skeleton->position.y) - y_Offset);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect_Green);

    SDL_Rect rect_Red = rect_Green;
    rect_Red.w = width - rect_Green.w;
    rect_Red.x = (int)(rect_Green.x + rect_Green.w);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect_Red);

    // Outline HP bars
    SDL_Rect outline = {};
    outline.w = (int)width;
    outline.h = (int)height;
    outline.x = (int)((skeleton->position.x) - width / 2);
    outline.y = (int)((skeleton->position.y) - y_Offset);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    outline_Rect(renderer, &outline, thickness);
}

Font load_Font_Bitmap(SDL_Renderer* renderer, const char* font_File_Path) {
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

void draw_Character(SDL_Renderer* renderer, Font* font, char character, int position_X, int position_Y, int size) {
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

void draw_String(SDL_Renderer* renderer, Font* font, const char* string, int position_X, int position_Y, int size, bool center) {
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
        draw_Character(renderer, font, iterator, char_Position_X, char_Position_Y, size);
        offset_X = font->char_Width * size;
        index++;
        iterator = string[index];
    }
}

void draw_Timer(SDL_Renderer* renderer, Font* font, Vector position, int timer_Size) {
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
    draw_String(renderer, font, ptr, (int)(position.x - (font->char_Width / 2)), (int)(position.y - (font->char_Height / 2)), timer_Size, true);
}

bool mouse_Down_This_Frame = false;
std::string current_frame_Hot_Name;
std::string next_Frame_Hot_Name;
bool draw_Rect = false;

bool button(SDL_Renderer* renderer, Font* font, const char* string, int position_X, int position_Y, int w, int h) {
    SDL_Rect button_Area = {};
    // Centers the button
    button_Area.x = (position_X - (w / 2));
    button_Area.y = (position_Y - (h / 2));
    button_Area.w = w;
    button_Area.h = h;

    // Set background as black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &button_Area);

    draw_String(renderer, font, string, position_X, position_Y, 3, true);
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

    outline_Rect(renderer, &button_Area, 10);

    return button_Pressed;
}

std::vector<int> create_Height_Map(const char* filename, int& terrain_Width, int& terrain_Height) {
    int channels;
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

bool check_Height_Map_Collision(int entity_X, int entity_Y, std::vector<int> height_Map) {
    // Outside of the map range
    if (entity_X < 0 || entity_X >= height_Map.size()) {
        return false;
    }
    int terrain_Position = RESOLUTION_HEIGHT - height_Map[entity_X];
    bool result = (entity_Y >= terrain_Position);
    return result;
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

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_Log("ERROR: SDL_CreateRenderer returned NULL: %s", SDL_GetError());
        return 1;
    }

    Font font_1 = load_Font_Bitmap(renderer, "images/font_1.png");

    // Create sprites
    Sprite arrow_Sprite = create_Sprite(renderer, "images/arrow.png");
    Sprite castle_Sprite = create_Sprite(renderer, "images/player_Castle.png");
    Sprite zergling_Sprite = create_Sprite(renderer, "images/zergling.jpg");
    Sprite skeleton_Sprite = create_Sprite(renderer, "images/unit_Skeleton.png");

    Sprite background = create_Sprite(renderer, "images/background.jpg");
    Sprite terrain = create_Sprite(renderer, "images/collision_Terrain_1.png");
    int terrain_Width = 0;
    int terrain_Height = 0;
    std::vector<int> terrain_Height_Map = create_Height_Map("images/collision_Terrain_1.png", terrain_Width, terrain_Height);

    // Create the Entities
    Vector player_Position = { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 };
    Player player = create_Player(zergling_Sprite, player_Position, 300);

    float player_Fire_Rate = .1f;

    Vector player_Castle_Position = { RESOLUTION_WIDTH * 0.05, 0};
    Castle player_Castle = create_Castle(castle_Sprite, player_Castle_Position, 100, player_Fire_Rate);

    Vector enemy_Castle_Position = { RESOLUTION_WIDTH * 0.95, 0};
    Castle enemy_Castle = create_Castle(castle_Sprite, enemy_Castle_Position, 100, 1);

    // Lower the castles onto the map before we enter the loop
    while (!check_Height_Map_Collision((int)player_Castle.position.x, (int)player_Castle.position.y + (int)player_Castle.sprite.height / 4, terrain_Height_Map)) {
        // gravity
        player_Castle.position.y += 1;
    }
    while (!check_Height_Map_Collision((int)enemy_Castle.position.x, (int)enemy_Castle.position.y + (int)player_Castle.sprite.height / 4, terrain_Height_Map)) {
        // gravity
        enemy_Castle.position.y += 1;
    }

    // Arrows
    float arrow_Speed = 750;
    float arrow_Damage = 5;
    float arrow_Gravity = 300;
    Arrow player_Arrow = create_Arrow(arrow_Sprite, arrow_Speed, arrow_Damage, arrow_Gravity);
    // Make hitbox smaller
    player_Arrow.collision_Radius = (player_Arrow.sprite.radius * 0.25f);
    std::vector<Arrow> player_Arrows;

    // Unit - Skeleton
    float skeleton_Speed = 100;
    float skeleton_Damage = 5;
    float skeleton_Gravity = 50;
    float enemy_Unit_Spawn_Rate = 2;
    float skeleton_HP = 100;
    Skeleton unit_Skeleton = create_Skeleton(skeleton_Sprite, skeleton_Speed, skeleton_Damage, skeleton_HP, skeleton_Gravity);
    std::vector<Skeleton> enemy_Skeletons;

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
    // float FPS = 144;

    bool key_W_Pressed = false;
    bool key_A_Pressed = false;
    bool key_S_Pressed = false;
    bool key_D_Pressed = false;
   
    bool key_Space_Pressed = false;

    bool running = true;
    while (running) {
        mouse_Down_This_Frame = false;
        SDL_Event event = {};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            // Keydown gives events based off keyboard repeat-rate (Operating system)
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_w: {
                    key_W_Pressed = true;
                    break;
                }
                case SDLK_a: {
                    key_A_Pressed = true;
                    break;
                }
                case SDLK_s: {
                    key_S_Pressed = true;
                    break;
                }
                case SDLK_d: {
                    key_D_Pressed = true;
                    break;
                }
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
                case SDLK_w: {
                    key_W_Pressed = false;
                    break;
                }
                case SDLK_a: {
                    key_A_Pressed = false;
                    break;
                }
                case SDLK_s: {
                    key_S_Pressed = false;
                    break;
                }
                case SDLK_d: {
                    key_D_Pressed = false;
                    break;
                }
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
        // Convert delta_Time into seconds
        delta_Time /= 1000;

        current_frame_Hot_Name = next_Frame_Hot_Name;
        next_Frame_Hot_Name = "";

        SDL_GetMouseState(&mouse_X, &mouse_Y);

        if (key_W_Pressed) {
            if (player.position.y > 0) {
                player.position.y -= player.speed * delta_Time;
            }
        }
        if (key_A_Pressed) {
            if (player.position.x > 0) {
                player.position.x -= player.speed * delta_Time;
            }
        }
        if (key_S_Pressed) {
            if (player.position.y < RESOLUTION_HEIGHT) {
                player.position.y += player.speed * delta_Time;
            }
        }
        if (key_D_Pressed) {
            if (player.position.x < RESOLUTION_WIDTH) {
                player.position.x += player.speed * delta_Time;
            }
        }

        SDL_Rect move_Player = { (int)player.position.x, (int)player.position.y, player.sprite.width, player.sprite.height };

        // Spawn Arrows and update lifetime
        if (key_Space_Pressed) {
            if (player_Castle.fire_Interval < 0) {
                spawn_Arrow(&player_Arrow, &player_Castle);
                player_Arrows.push_back(player_Arrow);
                player_Castle.fire_Interval = player_Fire_Rate;
            }
            else {
                player_Castle.fire_Interval -= delta_Time;
            }
        }
        else {
            player_Castle.fire_Interval -= delta_Time;
        }

        // Spawn player skeletons
        if (enemy_Castle.spawn_Interval < 0) {
            spawn_Skeleton(&unit_Skeleton, &enemy_Castle, &player_Castle);
            enemy_Skeletons.push_back(unit_Skeleton);
            enemy_Castle.spawn_Interval = enemy_Unit_Spawn_Rate;
        }
        else {
            enemy_Castle.spawn_Interval -= delta_Time;
        }
       
        // Update arrow positions
        for (int i = 0; i < player_Arrows.size(); i++) {
            if (player_Arrows[i].destroyed == false) {
                update_Arrow_Position(&player_Arrows[i], delta_Time);
                update_Arrow_Collision(&player_Arrows[i]);
            }
            /*
            if (player_Arrows[i].life_Time > 0) {
                update_Arrow_Position(&player_Arrows[i], delta_Time);
            }
            */
        }

        // Update skeleton positions
        for (int i = 0; i < enemy_Skeletons.size(); i++) {
            if (enemy_Skeletons[i].destroyed == false) {
                update_Skeleton_Position(&enemy_Skeletons[i], delta_Time);
            }
        }

        // Erase destroyed arrows
        std::erase_if(player_Arrows, [](Arrow &arrow) {
            // Return if we want the value to be destroyed
            return arrow.destroyed || arrow.life_Time <= 0;
            });

        // Erase destroy units
        std::erase_if(enemy_Skeletons, [](Skeleton& skeletons) {
            // Return if we want the value to be destroyed
            return skeletons.destroyed || skeletons.current_HP <= 0;
            });

        // Collision detection
        for (int i = 0; i < player_Arrows.size(); i++) {
            // Collision with Enemy Castle
            float distance_Between = calculate_Distance(
                player_Arrows[i].collision_Offset.x,
                player_Arrows[i].collision_Offset.y,
                enemy_Castle.position.x,
                enemy_Castle.position.y);
            float radius_Sum = player_Arrows[i].collision_Radius + enemy_Castle.sprite.radius;
            if (distance_Between <= radius_Sum) {
                enemy_Castle.current_HP -= player_Arrows[i].damage;
                player_Arrows[i].destroyed = true;
            }
            // Collision with map
            if (check_Height_Map_Collision((int)player_Arrows[i].collision_Offset.x, (int)player_Arrows[i].collision_Offset.y, terrain_Height_Map)) {
                player_Arrows[i].stop_Arrow = true;
            }
            // Collision with skeletons
            for (int j = 0; j < enemy_Skeletons.size(); j++) {
                float distance_Between_Skeleton = calculate_Distance(
                    player_Arrows[i].collision_Offset.x,
                    player_Arrows[i].collision_Offset.y,
                    enemy_Skeletons[j].position.x,
                    enemy_Skeletons[j].position.y);
                float radius_Sum_Skeleton = player_Arrows[i].collision_Radius + enemy_Skeletons[j].sprite.radius;
                if (distance_Between_Skeleton <= radius_Sum_Skeleton) {
                    if (player_Arrows[i].velocity.x != 0 && player_Arrows[i].velocity.y != 0) {
                        enemy_Skeletons[j].current_HP -= player_Arrows[i].damage;
                        player_Arrows[i].destroyed = true;
                    }
                }
            }
        }

        // Collision skeleton with map
        for (int i = 0; i < enemy_Skeletons.size(); i++) {
            if (check_Height_Map_Collision((int)enemy_Skeletons[i].position.x, (int)enemy_Skeletons[i].position.y, terrain_Height_Map)) {
                enemy_Skeletons[i].position.y = (RESOLUTION_HEIGHT - (float)terrain_Height_Map[(int)enemy_Skeletons[i].position.x]);
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // ***Renderering happens here***
        draw_Layer(renderer, background);
        draw_Layer(renderer, terrain);
        draw_Castle(renderer, player_Castle, false);
        draw_Castle(renderer, enemy_Castle, true);

        /*
        SDL_Rect temp = {};
        temp = { (int)player.position.x, (int)player.position.y, player.sprite.width, player.sprite.height };
        SDL_RenderCopyEx(renderer, player.sprite.texture, NULL, &temp, 0, NULL, SDL_FLIP_NONE);
        */
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
         
        draw_Circle(renderer, player_Castle.position.x, (float)((int)player_Castle.position.y), player_Castle.sprite.radius);

        for (int i = 0; i < player_Arrows.size(); i++) {
            draw_Circle(renderer, player_Arrows[i].collision_Offset.x, player_Arrows[i].collision_Offset.y, player_Arrows[i].collision_Radius);
            if (player_Arrows[i].life_Time > 0) {
                draw_Arrow(renderer, &player_Arrows[i], false);
                player_Arrows[i].life_Time -= delta_Time;
            } 
        }

        // Draw enemy skeletons
        for (int i = 0; i < enemy_Skeletons.size(); i++) {
            draw_Circle(renderer, enemy_Skeletons[i].position.x, enemy_Skeletons[i].position.y, enemy_Skeletons[i].sprite.radius);
            draw_Skeleton(renderer, &enemy_Skeletons[i], false);
            draw_Skeleton_HP_Bar(renderer, &enemy_Skeletons[i], 45, 10, 50, 2);
        }

        draw_Circle(renderer, enemy_Castle.position.x, (float)(enemy_Castle.position.y), enemy_Castle.sprite.radius);
        // (float)((int)player_Castle.position.y - (int)player_Castle.sprite.center.y)  
        int hp_Bar_Width = 90;
        int hp_Bar_Height = 20;
        int hp_Bar_Y_Offset = 115;
        int hp_Bar_Thickness = 3;
        draw_Castle_HP_Bar(renderer, &enemy_Castle, hp_Bar_Width, hp_Bar_Height, hp_Bar_Y_Offset, hp_Bar_Thickness);
        draw_Castle_HP_Bar(renderer, &player_Castle, hp_Bar_Width, hp_Bar_Height, hp_Bar_Y_Offset, hp_Bar_Thickness);

        draw_Timer(renderer, &font_1, { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 30 }, 6);

        /*
        if (button(renderer, &font_1, "Testing", RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2, 200, 100)) {
            if (draw_Rect) {
                draw_Rect = false;
            }
            else {
                draw_Rect = true;
            }
        }
        if (draw_Rect) {
            SDL_Rect rect = {};
            rect.x = 500;
            rect.y = 500;
            rect.w = 100;
            rect.h = 100;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
        */

        SDL_RenderPresent(renderer);
    }

    return 0;
}