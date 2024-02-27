#include <SDL.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define RESOLUTION_WIDTH 1920
#define RESOLUTION_HEIGHT 1080

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

struct Sprite {
    int width;
    int height;
    float radius;
    Vector center;
    SDL_Texture* texture;
};

struct Font {
    int width;
    int height;
    SDL_Texture* texture;
    int char_Height;
    int char_Width;
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
};

struct Arrow {
    Sprite sprite;
    Vector position;
    Vector velocity;
    float speed;
    float life_Time;
    float angle;
    bool destroyed;
    float damage;
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
    return result;
}

Arrow create_Arrow(Sprite sprite, float speed, float damage) {
    Arrow result = {};
    result.sprite = sprite;
    result.speed = speed;
    result.life_Time = 5;
    result.destroyed = false;
    result.damage = damage;
    return result;
}   

void draw_Background(SDL_Renderer* renderer, Sprite sprite) {
    SDL_RenderCopyEx(renderer, sprite.texture, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
}

void draw_Castle(SDL_Renderer* renderer, Castle castle) {
    SDL_Rect temp = {};
    temp = { 
        ((int)castle.position.x - (int)castle.sprite.center.x), 
        ((int)castle.position.y - (int)castle.sprite.center.y), 
        castle.sprite.width, 
        castle.sprite.height 
    };

    SDL_RenderCopyEx(renderer, castle.sprite.texture, NULL, &temp, 0, NULL, SDL_FLIP_NONE);
}

void draw_Arrow(SDL_Renderer* renderer, Arrow arrow) {
    SDL_Rect temp = {};
    temp = {
        ((int)arrow.position.x - (int)arrow.sprite.center.x),
        ((int)arrow.position.y - (int)arrow.sprite.center.y),
        arrow.sprite.width,
        arrow.sprite.height
    };

    SDL_RenderCopyEx(renderer, arrow.sprite.texture, NULL, &temp, arrow.angle, NULL, SDL_FLIP_NONE);
}

void update_Arrow_Position(Arrow* arrow, float delta_Time) {
    const float gravity = 50.0f;

    arrow->velocity.y += gravity * delta_Time;
    arrow->position.x += (arrow->velocity.x * delta_Time);
    arrow->position.y += (arrow->velocity.y * delta_Time);

    arrow->angle = (float)(((float)atan2(arrow->velocity.y, arrow->velocity.x) * (180 / M_PI)));
}

void spawn_Arrow(Arrow* player_Arrow, Castle* player_Castle) {
    player_Arrow->position.x = player_Castle->position.x - player_Arrow->sprite.center.x;
    player_Arrow->position.y = player_Castle->position.y - player_Arrow->sprite.center.y;

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
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

void draw_Castle_HP_Bar(SDL_Renderer* renderer, Castle castle, int width, int height, int y_Offset) {
    // Lerp of T = A * (1 - T) + B * T
    // Linear interpolation
    // A is the left side, B is the right side, T is the health %
    // I only need one lerp, and just fill in the second rect with 
    // what remains.
    // LERP works in as many dimensions as possible (Vec2, Vec3, Vec4)
    if (castle.current_HP > 0) {
        // Draw first square
        float hp_Percent_Green = (castle.current_HP / castle.max_HP);

        SDL_Rect rect_Green = {};
        rect_Green.w = (width * hp_Percent_Green);
        rect_Green.h = height;
        rect_Green.x = ((castle.position.x) - width / 2);
        rect_Green.y = (castle.position.y - y_Offset);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &rect_Green);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawRect(renderer, &rect_Green);

        // Draw second square
        float hp_Percent_Red = (hp_Percent_Green - 1.00);

        SDL_Rect rect_Red = {};
        rect_Red.w = (width * hp_Percent_Red);
        rect_Red.h = height;
        rect_Red.x = ((castle.position.x) + width / 2);
        rect_Red.y = ((castle.position.y - y_Offset));

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &rect_Red);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawRect(renderer, &rect_Red);
    }
    else {
        SDL_Rect rectRed = {};
        // Multiply by minus 1 so it draws correctly
        rectRed.w = width * -1;
        rectRed.h = height;
        rectRed.x = ((castle.position.x) + width / 2);
        rectRed.y = ((castle.position.y - y_Offset));
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawRect(renderer, &rectRed);
        SDL_RenderFillRect(renderer, &rectRed);
    }
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

void draw_Character(SDL_Renderer* renderer, Font* font, char character, Vector position, int size) {
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
    dest_Rect.x = position.x;
    dest_Rect.y = position.y;
    dest_Rect.w = font->char_Width * size;
    dest_Rect.h = font->char_Height * size;

    SDL_RenderCopyEx(renderer, font->texture, &src_Rect, &dest_Rect, 0, NULL, SDL_FLIP_NONE);
}

void draw_String(SDL_Renderer* renderer, Font* font, const char* string, Vector position, int size) {
    int offset_X = 0;
    int index = 0;
    char iterator = string[index];
    while (iterator != '\0') {
        Vector char_Position = position;
        char_Position.x += (offset_X * size);
        draw_Character(renderer, font, iterator, char_Position, size);

        offset_X += font->char_Width;
        index++;
        iterator = string[index];
    }
}

int main(int argc, char** argv) {
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
    Sprite arrow = create_Sprite(renderer, "images/arrow.png");
    Sprite zergling_Sprite = create_Sprite(renderer, "images/zergling.jpg");
    Sprite background = create_Sprite(renderer, "images/background.jpg");
    Sprite castle_Sprite = create_Sprite(renderer, "images/player_Castle.jpg");

    // Create the Entities
    Vector player_Position = { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 };
    Player player = create_Player(zergling_Sprite, player_Position, 300);

    float player_Fire_Rate = 1;

    Vector player_Castle_Position = { RESOLUTION_WIDTH * 0.05, RESOLUTION_HEIGHT * 0.5 };
    Castle player_Castle = create_Castle(castle_Sprite, player_Castle_Position, 100, player_Fire_Rate);

    Vector enemy_Castle_Position = { RESOLUTION_WIDTH * 0.95, RESOLUTION_HEIGHT * 0.5 };
    Castle enemy_Castle = create_Castle(castle_Sprite, enemy_Castle_Position, 100, 1);

    float arrow_Speed = 600;
    Arrow player_Arrow = create_Arrow(arrow, arrow_Speed, 25);
    spawn_Arrow(&player_Arrow, &player_Castle);

    std::vector<Arrow> player_Arrows;
    player_Arrows.push_back(player_Arrow);

    float ticks = 0;
    float last_Ticks = 0;
    float delta_Time = 0;
    float FPS = 144;

    bool key_W_Pressed = false;
    bool key_A_Pressed = false;
    bool key_S_Pressed = false;
    bool key_D_Pressed = false;

    bool running = true;
    while (running) {
        SDL_Event Test_Event;
        while (SDL_PollEvent(&Test_Event)) {
            switch (Test_Event.type) {
            // Keydown gives events based off keyboard repeat-rate (Operating system)
            case SDL_KEYDOWN: {
                switch (Test_Event.key.keysym.sym) {
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
                default: {
                    break;
                }
                }  
                break;
            }
            case SDL_KEYUP: {
                switch (Test_Event.key.keysym.sym) {
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
                default: {
                    break;
                }
                }
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

        if (player_Castle.fire_Interval < 0) {
            spawn_Arrow(&player_Arrow, &player_Castle);
            player_Arrows.push_back(player_Arrow);
            player_Castle.fire_Interval = player_Fire_Rate;
        }
        else {
            player_Castle.fire_Interval -= delta_Time;
        }
       
        for (int i = 0; i < player_Arrows.size(); i++) {
            if (player_Arrows[i].destroyed == false) {
                update_Arrow_Position(&player_Arrows[i], delta_Time);
            }
            /*
            if (player_Arrows[i].life_Time > 0) {
                update_Arrow_Position(&player_Arrows[i], delta_Time);
            }
            */
        }

        // [] capture group (Don't need now)
        std::erase_if(player_Arrows, [](Arrow &arrow) {
            // Return if we want the value to be destroyed
            return arrow.destroyed;
            });

        // Collision detection
        for (int i = 0; i < player_Arrows.size(); i++) {
            float distance_Between = calculate_Distance(
                player_Arrows[i].position.x,
                player_Arrows[i].position.y,
                enemy_Castle.position.x,
                enemy_Castle.position.y);
            float radius_Sum = player_Arrows[i].sprite.radius + enemy_Castle.sprite.radius;
            if (distance_Between <= radius_Sum) {
                enemy_Castle.current_HP -= player_Arrows[i].damage;
                player_Arrows[i].destroyed = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // ***Renderering happens here***
        draw_Background(renderer, background);
        draw_Castle(renderer, player_Castle);
        draw_Castle(renderer, enemy_Castle);

        /*
        SDL_Rect temp = {};
        temp = { (int)player.position.x, (int)player.position.y, player.sprite.width, player.sprite.height };
        SDL_RenderCopyEx(renderer, player.sprite.texture, NULL, &temp, 0, NULL, SDL_FLIP_NONE);
        */
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
         
        draw_Circle(renderer, player_Castle.position.x, player_Castle.position.y, player_Castle.sprite.radius);

        for (int i = 0; i < player_Arrows.size(); i++) {
            draw_Arrow(renderer, player_Arrows[i]);
            draw_Circle(renderer, player_Arrows[i].position.x, player_Arrows[i].position.y, player_Arrows[i].sprite.radius);
            /*
            if (player_Arrows[i].life_Time > 0) {
                draw_Arrow(renderer, player_Arrows[i]);
            }
            */
        }

        draw_Circle(renderer, enemy_Castle.position.x, enemy_Castle.position.y, enemy_Castle.sprite.radius);

        draw_Castle_HP_Bar(renderer, enemy_Castle, 100, 25, 100);
        draw_Castle_HP_Bar(renderer, player_Castle, 100, 25, 100);

        draw_String(renderer, &font_1, "HELLO", { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 }, 5);

        SDL_RenderPresent(renderer);
    }

    return 0;
}