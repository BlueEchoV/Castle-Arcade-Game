#include <vector>
#include <unordered_map>
#include <sys/stat.h>

#include "Game.h"
#include "Menu_System.h"
#include "Particle_System.h"

#include "soloud.h"
#include "soloud_wav.h"

#define WITH_MINIAUDIO

// Engine core
SoLoud::Soloud soloud;

int main(int argc, char** argv) {
    REF(argc);
    REF(argv);

    soloud.init(); // Initialize SoLoud

    // One wave file per sound
    SoLoud::Wav wav_Electronic_Song;
    wav_Electronic_Song.load("audio/electronic_Song.wav");

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Castle Defense", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RESOLUTION_WIDTH, RESOLUTION_HEIGHT, 0);
    if (window == NULL) {
        SDL_Log("ERROR: SDL_RenderClear returned returned NULL: %s", SDL_GetError());
        return 1;
    }

    Globals::renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (Globals::renderer == NULL) {
        SDL_Log("ERROR: SDL_CreateRenderer returned NULL: %s", SDL_GetError());
        return 1;
    }

    Font font_1 = load_Font_Bitmap("images/font_1.png");

    Image gameloop_BKG_Image = create_Image("images/background.jpg");
    Image menu_BKG_Image = create_Image("images/background_2.png");
    // Image menu_BKG_Image = create_Image("images/background_3.png");
    Image terrain_Image = create_Image("images/collision_Terrain_1.png");
    Image castle_Image = create_Image("images/player_Castle.png");
    Image game_Over_Image = create_Image("images/game_Over.png");
    Image arrow_Image = create_Image("images/arrow.png");
    Image skeleton_Image_Walking = create_Image("images/unit_Skeleton_Sprite_Sheet.png");
    // Image warrior_Image_Walking = create_Image("images/unit_Warrior.png");
    Image warrior_Image_Walking = create_Image("images/unit_Warrior_Short.png");
    Image skeleton_Image_Stop = create_Image("images/unit_Skeleton.png");
    // Image archer_Image_Walking = create_Image("images/unit_Archer_Sprite_Sheet.png");
    // Image archer_Image_Stop = create_Image("images/unit_Archer.png");
    Image archer_Image_Stop = create_Image("images/unit_Archer_Short.png");
    Image blood_Image = create_Image("images/blood_1.png");

    add_Sprite_Sheet_To_Array(SSS_BKG_GAMELOOP_1, &gameloop_BKG_Image, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_BKG_MENU_1, &menu_BKG_Image, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_TERRAIN_1, &terrain_Image, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_CASTLE_1, &castle_Image, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_BKG_GAMEOVER, &game_Over_Image, 1, 1);

    add_Sprite_Sheet_To_Array(SSS_SKELETON_WALKING, &warrior_Image_Walking, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_SKELETON_STOP, &warrior_Image_Walking, 1, 1);
    // add_Sprite_Sheet_To_Array(SSS_SKELETON_WALKING, &skeleton_Image_Walking, 1, 4);
    // add_Sprite_Sheet_To_Array(SSS_SKELETON_STOP, &skeleton_Image_Stop, 1, 1);

    add_Sprite_Sheet_To_Array(SSS_ARCHER_WALKING, &archer_Image_Stop, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_ARCHER_STOP, &archer_Image_Stop, 1, 1);
    // add_Sprite_Sheet_To_Array(SSS_ARCHER_WALKING, &archer_Image_Walking, 1, 2);
    // add_Sprite_Sheet_To_Array(SSS_ARCHER_STOP, &archer_Image_Stop, 1, 1);
    add_Sprite_Sheet_To_Array(SSS_ARROW_DEFAULT, &arrow_Image, 1, 1);

    Game_Data game_Data = {};

    start_Game(&game_Data);

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

    // This could just be an array
    std::unordered_map<std::string, Game_Data> saved_Games_Cache = {};

    Cache_Data save_Game_Cache_Data = create_Cache_Data(saved_Games_Cache);

	// spawn_Particle_Systems(game_Data, PT_WATER, { RESOLUTION_WIDTH / 4, RESOLUTION_HEIGHT / 4 }, 400, 400, &blood_Image);

    Game_State current_Game_State = GS_GAMELOOP;
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

		last_Ticks = ticks;
		ticks = (float)SDL_GetTicks64();
		delta_Time = ticks - last_Ticks;
        // Clamps the game time
        if (delta_Time > 250) {
            delta_Time = 250;
        }
		// Multiply by the scalar
		delta_Time *= time_Scalar;
		delta_Time /= 1000;

        for (Particle_System& particle_System : game_Data.particle_Systems) {
            if (particle_System.target_ID >= 0) {
                bool target_Exists = false;
                for (Skeleton& skeleton : game_Data.enemy_Skeletons) {
                    if (particle_System.target_ID == skeleton.ID) {
                        target_Exists = true;
                        update_Particle_System(particle_System, skeleton.rigid_Body.position_WS, delta_Time);
                    }
                }
                if (!target_Exists) {

                }
            }
        }

		if (current_Game_State == GS_GAMELOOP) {
			game_Data.timer += delta_Time;
		}

		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(Globals::renderer);

        // Load the cache once when the game starts
		load_Game_Data_Cache(save_Game_Cache_Data);

		if (key_States[SDLK_F1].pressed_This_Frame) {
            std::string save_Game_1_String = create_Save_Game_File_Name(SG_SAVE_GAME_1);
            const char* save_Game_1_Ptr = save_Game_1_String.c_str();
            if (check_If_File_Exists(save_Game_1_Ptr)) {
                remove(save_Game_1_Ptr);
            }
            save_Game_To_Cache(SG_SAVE_GAME_1, game_Data, save_Game_Cache_Data);
		}

		if (key_States[SDLK_F2].pressed_This_Frame) {
			std::string save_Game_1_String = create_Save_Game_File_Name(SG_SAVE_GAME_1);
            if (save_Game_Cache_Data.cache.find(save_Game_1_String) != save_Game_Cache_Data.cache.end()) {
                game_Data = save_Game_Cache_Data.cache[save_Game_1_String];
            }
		}

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
			SDL_RenderCopy(Globals::renderer, sprite_Sheet_Array[SSS_BKG_MENU_1].sprites[0].image->texture, NULL, NULL);
        
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
            
            V2 button_Pos = { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 };
            int button_Width = 300;
            int button_Height = 100;
            int string_Size = 4;

			if (button_Text(&font_1, "Play", button_Pos, button_Width, button_Height, string_Size)) {
                current_Game_State = GS_GAMELOOP;
                start_Game(&game_Data);
			}
			button_Pos.y += 100;
			if (button_Text(&font_1, "Load Game", button_Pos, button_Width, button_Height, string_Size)) {
                current_Game_State = GS_LOADGAME;
			}
            button_Pos.y += 100;
			if (button_Text(&font_1, "Options", button_Pos, button_Width, button_Height, string_Size)) {

			}
            button_Pos.y += 100;
			if (button_Text(&font_1, "Quit", button_Pos, button_Width, button_Height, string_Size)) {
				running = false;
			}
            button_Pos.y += 100;
        }
        else if (current_Game_State == GS_LOADGAME) {
            SDL_RenderCopy(Globals::renderer, sprite_Sheet_Array[SSS_BKG_MENU_1].sprites[0].image->texture, NULL, NULL);

            int button_Width = 325;
            int button_Height = 90;
            int offset = button_Height;
			V2 button_Pos = { RESOLUTION_WIDTH / 2 , RESOLUTION_HEIGHT / 10 * 3 };
            int size = 3;

            draw_String_With_Background(&font_1, "Saved Games", (int)button_Pos.x, (int)button_Pos.y, size, true, CI_BLACK, 3);
            button_Pos.y += offset;

            if (load_Game_Button(SG_SAVE_GAME_1, save_Game_Cache_Data, &font_1, button_Pos, button_Width, button_Height, size)) {
                load_Game(&game_Data, SG_SAVE_GAME_1);
				current_Game_State = GS_GAMELOOP;
			}
			button_Pos.y += offset;
			if (load_Game_Button(SG_SAVE_GAME_2, save_Game_Cache_Data, &font_1, button_Pos, button_Width, button_Height, size)) {
                load_Game(&game_Data, SG_SAVE_GAME_2);
				current_Game_State = GS_GAMELOOP;
			}
			button_Pos.y += offset;
			if (load_Game_Button(SG_SAVE_GAME_3, save_Game_Cache_Data, &font_1, button_Pos, button_Width, button_Height, size)) {
                load_Game(&game_Data, SG_SAVE_GAME_3);
				current_Game_State = GS_GAMELOOP;
			}
			button_Pos.y += offset;
			if (key_States[SDLK_ESCAPE].pressed_This_Frame) {
				current_Game_State = GS_MENU;
			}
        }
		else if (current_Game_State == GS_VICTORY || current_Game_State == GS_GAMEOVER) {
			SDL_RenderCopy(Globals::renderer, sprite_Sheet_Array[SSS_BKG_GAMEOVER].sprites[0].image->texture, NULL, NULL);
			if (current_Game_State == GS_VICTORY) {
				draw_String_With_Background(
					&font_1,
					"Victory!!!",
					RESOLUTION_WIDTH / 2,
					RESOLUTION_HEIGHT / 2,
					4,
					true,
					CI_BLACK,
					3
				);
			}
			if (current_Game_State == GS_GAMEOVER) {
				draw_String_With_Background(
					&font_1,
					"Game Over",
					RESOLUTION_WIDTH / 2,
					RESOLUTION_HEIGHT / 2,
					4,
					true,
					CI_BLACK,
					3
				);
			}
			if (button_Text(&font_1, "Return to Menu", { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 90 }, 325, 90, 3)) {
				current_Game_State = GS_MENU;
			}
		}
        else if (current_Game_State == GS_GAMELOOP || current_Game_State == GS_PAUSED || current_Game_State == GS_SAVEGAME) {
            if (key_States[SDLK_UP].held_Down == true) {
                time_Scalar += 0.01f;
            }
            if (key_States[SDLK_DOWN].held_Down == true) {
                if (time_Scalar > 0) {
                    time_Scalar -= 0.01f;
                }
            }
           
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

                // arrow collision
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
                                // On first hit, proc the damage
                                if (arrow->collision_Delay.remaining == arrow->collision_Delay.duration) {
                                    spawn_Particle_System_Target(game_Data, PT_BLOOD, 2, enemy_Skeleton->ID, 100, 100, &blood_Image);
                                    enemy_Skeleton->health_Bar.current_HP -= arrow->damage;
                                    arrow->target_ID = enemy_Skeleton->ID;
                                }
                                bool targeted_Unit_Still_Alive = false;
                                for (int e = 0; e < game_Data.enemy_Skeletons.size(); e++) {
                                    if (arrow->target_ID == game_Data.enemy_Skeletons[e].ID) {
                                        targeted_Unit_Still_Alive = true;
                                    }
                                }
                                // Won't always stick, but it will always proc the damage.
                                // This way, if the arrow is fast and goes through the target (which is fine),
                                // then the arrow is bound to the unit but will also die with the unit.
                                if (targeted_Unit_Still_Alive) {
                                    if (arrow->collision_Delay.remaining > 0) {
                                        arrow->collision_Delay.remaining -= delta_Time;
                                    }
                                    else {
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
                                else {
                                    arrow->destroyed = true;
                                }
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

            if (game_Data.player_Castle.health_Bar.current_HP <= 0) {
                current_Game_State = GS_GAMEOVER;
            }
			if (game_Data.enemy_Castle.health_Bar.current_HP <= 0) {
				current_Game_State = GS_VICTORY;
			}


            // ***Renderering happens here***
            draw_Layer(sprite_Sheet_Array[SSS_BKG_GAMELOOP_1].sprites[0].image->texture);
            draw_Layer(sprite_Sheet_Array[SSS_TERRAIN_1].sprites[0].image->texture);
            draw_Castle(&game_Data.player_Castle, false);
            draw_Castle(&game_Data.enemy_Castle, true);

            SDL_SetRenderDrawColor(Globals::renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

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
                    true
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
                    false
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
                &game_Data,
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
                    current_Game_State = GS_SAVEGAME;
				}
                button_Pos_Paused.y += button_Height_Paused;
            }
            if (current_Game_State == GS_SAVEGAME) {
                if (key_States[SDLK_ESCAPE].pressed_This_Frame) {
                    current_Game_State = GS_GAMELOOP;
                }
                int button_Width_Saved = 325;
				int button_Height_Saved = 90;
				int offset = button_Height_Saved;
				V2 button_Pos_Saved = { RESOLUTION_WIDTH / 2 , RESOLUTION_HEIGHT / 10 * 3 };
				int size = 3;

				draw_String_With_Background(&font_1, "Saved Games", (int)button_Pos_Saved.x, (int)button_Pos_Saved.y, size, true, CI_BLACK, 3);
                
                // This is a loop
                if (save_Game_Button(SG_SAVE_GAME_1, save_Game_Cache_Data, &font_1, button_Pos_Saved, button_Width_Saved, button_Height_Saved, size)) {
                    // Put this in the save_Game_Button
                    save_Game_To_Cache(SG_SAVE_GAME_1, game_Data, save_Game_Cache_Data);
				}
                button_Pos_Saved.y += offset;
				if (save_Game_Button(SG_SAVE_GAME_2, save_Game_Cache_Data, &font_1, button_Pos_Saved, button_Width_Saved, button_Height_Saved, size)) {
                    save_Game_To_Cache(SG_SAVE_GAME_2, game_Data, save_Game_Cache_Data);
				}
                button_Pos_Saved.y += offset;
				if (save_Game_Button(SG_SAVE_GAME_3, save_Game_Cache_Data, &font_1, button_Pos_Saved, button_Width_Saved, button_Height_Saved, size)) {
                    save_Game_To_Cache(SG_SAVE_GAME_3, game_Data, save_Game_Cache_Data);
				}
                button_Pos_Saved.y += offset;
                /*
				if (button_Text(&font_1, "Return to Menu", button_Pos_Saved, button_Width_Saved, button_Height_Saved, size)) {
					current_Game_State = GS_MENU;
				}
                */

            }

            draw_Particle_Systems(game_Data);

#if 0
			if (button_Text(&font_1, "Play", { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 }, 150, 100, 3)) {
				soloud.play(wav_Electronic_Song);
			}
			if (button_Text(&font_1, "Stop", { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 100 }, 150, 100, 3)) {
				soloud.stopAll();
			}
#endif

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

			// Erase destroy units
			std::erase_if(game_Data.particle_Systems, [](Particle_System& particle_System) {
				// Return if we want the value to be destroyed
				return particle_System.destroyed && particle_System.particles.size() == 0;
				});
        }
        SDL_RenderPresent(Globals::renderer);
    }

    soloud.deinit();
    return 0;
}