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

    font_1 = load_Font_Bitmap("images/font_1.png");
    init_Sprites();

    CSV_Data sprite_Sheet_CSV_Data = {};
    sprite_Sheet_CSV_Data.file_Path = "data/Sprite_Sheet_Data.csv";
	load_Sprite_Sheet_Data_CSV(&sprite_Sheet_CSV_Data);

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

    CSV_Data particle_CSV_Data = create_Open_CSV_File("data/Particle_Data.csv");
    load_Particle_Data_CSV(&particle_CSV_Data);
    close_CSV_File(&particle_CSV_Data);

    CSV_Data unit_CSV_Data = create_Open_CSV_File("data/Unit_Data.csv");
    load_Unit_Data_CSV(&unit_CSV_Data);
    close_CSV_File(&unit_CSV_Data);

    CSV_Data projectile_CSV_Data = create_Open_CSV_File("data/Projectile_Data.csv");
    load_Projectile_Data_CSV(&projectile_CSV_Data);
    close_CSV_File(&projectile_CSV_Data);

	CSV_Data spell_CSV_Data = create_Open_CSV_File("data/Spell_Data.csv");
	load_Spell_Data_CSV(&spell_CSV_Data);
	close_CSV_File(&spell_CSV_Data);

    CSV_Data castle_CSV_Data = create_Open_CSV_File("data/Castle_Data.csv");
    load_Castle_Data_CSV(&castle_CSV_Data);
    close_CSV_File(&castle_CSV_Data);

	start_Game(game_Data);

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

        // Chose to handle every game state for now
		if (key_States[SDLK_ESCAPE].pressed_This_Frame) 
        {
            if (current_Game_State == GS_GAMELOOP) 
            {
				push_To_Menu_Stack(MM_Sub_Menu_Paused);
				current_Game_State = GS_PAUSED;
			} 
            else if (current_Game_State == GS_PAUSED) 
            {
				// Only change the game_State with the menu stack is at 1
                size_t stack_Size = get_Menu_Stack_Size();
                if (stack_Size <= 1) 
                {
					current_Game_State = GS_GAMELOOP;
				}
                pop_Menu_From_Stack();
			} 
            else if (current_Game_State == GS_MAIN_MENU) 
            {
                pop_Menu_From_Stack_Keep_First();
            }
            else if (current_Game_State == GS_VICTORY)
            {
                pop_Menu_From_Stack_Keep_First();
            } 
            else if (current_Game_State == GS_GAMEOVER) 
            {
                pop_Menu_From_Stack_Keep_First();
            }
		}

        for (Game_Level& game_Level : game_Data.game_Level_Map.game_Levels) {
            if (game_Level.is_Pressed) {
                load_Game_Level(game_Data.game_Level_Map, game_Data.player_Castle, game_Level);
                game_Level.is_Pressed = false;
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

        // Hot loadinggame_Data.
        attempt_Reload_Particle_CSV_File(&particle_CSV_Data);

        if (current_Game_State == GS_GAMELOOP) {
            for (uint32_t i = 0; i < game_Data.particle_System_IDS.size(); i++) {
                Particle_System* particle_System = get_Particle_System(game_Data.particle_Systems, game_Data.particle_System_IDS[i]);
                if (particle_System != nullptr) {
                    // Check if the handle is valid
                    if (particle_System->parent.generation != 0) {
                        Unit* enemy_Unit = get_Unit(game_Data.units, particle_System->parent);
                        if (enemy_Unit != nullptr) {
                            particle_System->rect.x = (int)enemy_Unit->rigid_Body.position_WS.x;
                            particle_System->rect.y = (int)enemy_Unit->rigid_Body.position_WS.y;
                        }
                    }
                    update_Particle_System(game_Data, *particle_System, delta_Time);
                }
			}
        }

		if (current_Game_State == GS_GAMELOOP) {
			game_Data.timer += delta_Time;
		}

		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(Globals::renderer);

        // Load the cache once when the game starts
		// load_Game_Data_Cache(save_Game_Cache_Data);

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

        if (current_Game_State == GS_GAMELOOP || current_Game_State == GS_PAUSED) {
            if (key_States[SDLK_UP].held_Down == true) {
                time_Scalar += 0.05f;
            }
            if (key_States[SDLK_DOWN].held_Down == true) {
                if (time_Scalar > 0) {
                    time_Scalar -= 0.05f;
                }
            }
           
            // Updating the game logic
            if (current_Game_State == GS_GAMELOOP && time_Scalar > 0) 
            {
                // Spawn Arrows and update lifetime
                if (key_States[SDLK_SPACE].held_Down == true && game_Data.player_Castle.projectile_Ammo > 0) {
                    if (game_Data.player_Castle.fire_Cooldown.remaining < 0) {
                        V2 target_Mouse = {};
                        int x, y = 0;
                        SDL_GetMouseState(&x, &y);
                        target_Mouse = { (float)x,(float)y };
                        spawn_Projectile(game_Data, N_PLAYER, "arrow_Short", 10, game_Data.player_Castle.rigid_Body.position_WS, target_Mouse);
                        game_Data.player_Castle.fire_Cooldown.remaining = game_Data.player_Castle.fire_Cooldown.duration;
                        if (game_Data.player_Castle.projectile_Ammo > 0) {
                            game_Data.player_Castle.projectile_Ammo--;
                        } else {
                            game_Data.player_Castle.projectile_Ammo = 0;
                        }
                    }
                }
                game_Data.player_Castle.fire_Cooldown.remaining -= delta_Time;
                
                if (game_Data.player_Castle.projectile_Ammo_Cooldown.remaining < 0) {
                    Castle* player_Castle = &game_Data.player_Castle;
                    player_Castle->projectile_Ammo++;
                    player_Castle->projectile_Ammo_Cooldown.remaining = player_Castle->projectile_Ammo_Cooldown.duration;
                }
                game_Data.player_Castle.projectile_Ammo_Cooldown.remaining -= delta_Time;

                // Spawn Player Units
                for (Summonable_Unit& summonable_Unit : game_Data.player_Castle.summonable_Units) {
                    Resource_Bar* food_Bar = &game_Data.player_Castle.food_Bar;
                    if (summonable_Unit.is_Pressed) {
                        if (food_Bar->current_Resource >= summonable_Unit.food_Cost) {
                            food_Bar->current_Resource -= summonable_Unit.food_Cost;
                            spawn_Unit_At_Castle(game_Data, summonable_Unit);
                        } else {
                            summonable_Unit.is_Pressed = false;
                        }
                    }
                }
                // Spawn Enemy Units
				for (Summonable_Unit& summonable_Unit : game_Data.enemy_Castle.summonable_Units) {
                    Resource_Bar* food_Bar = &game_Data.enemy_Castle.food_Bar;
					if (game_Data.enemy_Castle.spawn_Cooldown.remaining < 0 && food_Bar->current_Resource >= summonable_Unit.food_Cost) {
						food_Bar->current_Resource -= summonable_Unit.food_Cost;
						game_Data.enemy_Castle.spawn_Cooldown.remaining = game_Data.enemy_Castle.spawn_Cooldown.duration;
                        spawn_Unit_At_Castle(game_Data, summonable_Unit);
					}
				}
			    game_Data.enemy_Castle.spawn_Cooldown.remaining -= delta_Time;

                // Cast Units Spells
                cast_Units_Spells(game_Data, game_Data.player_Unit_IDS, delta_Time);
                cast_Units_Spells(game_Data, game_Data.enemy_Unit_IDS, delta_Time);

                // Update Castle Food bars
                update_Resource_Bar(game_Data.player_Castle.food_Bar, delta_Time);
                update_Resource_Bar(game_Data.enemy_Castle.food_Bar, delta_Time);

                // Update Units Spell Bars
                update_Units_Spell_Bars(game_Data, game_Data.player_Unit_IDS, delta_Time);
                update_Units_Spell_Bars(game_Data, game_Data.enemy_Unit_IDS, delta_Time);

                // Update Units
                update_Units_Positions(game_Data, game_Data.player_Unit_IDS, delta_Time);
                update_Units_Positions(game_Data, game_Data.enemy_Unit_IDS, delta_Time);

				// Update Projectiles
                update_Projectiles_Positions(game_Data, game_Data.player_Proj_IDS, delta_Time);
                update_Projectiles_Positions(game_Data, game_Data.enemy_Proj_IDS, delta_Time);

                // Projectile Collision
                check_Projectiles_Collisions(game_Data, game_Data.player_Proj_IDS, game_Data.enemy_Castle, game_Data.enemy_Unit_IDS, delta_Time);
                check_Projectiles_Collisions(game_Data, game_Data.enemy_Proj_IDS, game_Data.player_Castle, game_Data.player_Unit_IDS, delta_Time);

                // Units colliding with Terrain
                check_Units_Collisions_With_Terrain(game_Data, game_Data.player_Unit_IDS);
                check_Units_Collisions_With_Terrain(game_Data, game_Data.enemy_Unit_IDS);

                // Initialize default values before the below collision checks
                update_Units_Variables(game_Data, game_Data.player_Unit_IDS, delta_Time);
                update_Units_Variables(game_Data, game_Data.enemy_Unit_IDS, delta_Time);

                // Units colliding with castle
                check_Units_Collisions_With_Castle(game_Data, game_Data.player_Unit_IDS, game_Data.enemy_Castle);
                check_Units_Collisions_With_Castle(game_Data, game_Data.enemy_Unit_IDS, game_Data.player_Castle);

                // Units colliding with Units
                check_Units_Collisions_With_Units(game_Data, game_Data.player_Unit_IDS, game_Data.enemy_Unit_IDS);
                check_Units_Collisions_With_Units(game_Data, game_Data.enemy_Unit_IDS, game_Data.player_Unit_IDS);

                // Updating animations
				for (uint32_t i = 0; i < game_Data.player_Unit_IDS.size(); i++) {
					Unit* player_Unit = get_Unit(game_Data.units, game_Data.player_Unit_IDS[i]);
                    if (player_Unit != nullptr) {
                        float speed = player_Unit->speed;
                        if (!player_Unit->stop) {
                            update_Animation(&player_Unit->sprite_Sheet_Tracker, speed, delta_Time);
                        }
                    }
				}
				for (uint32_t i = 0; i < game_Data.enemy_Unit_IDS.size(); i++) {
					Unit* enemy_Unit = get_Unit(game_Data.units, game_Data.enemy_Unit_IDS[i]);
                    if (enemy_Unit != nullptr) {
                        float speed = enemy_Unit->speed;
                        if (!enemy_Unit->stop) {
                            update_Animation(&enemy_Unit->sprite_Sheet_Tracker, speed, delta_Time);
                        }
                    }
                }

            }

            if (game_Data.player_Castle.health_Bar.current_Resource <= 0) {
                push_To_Menu_Stack(MM_Menu_Game_Over_Screen);
                current_Game_State = GS_GAMEOVER;
            }
			if (game_Data.enemy_Castle.health_Bar.current_Resource <= 0) {
                push_To_Menu_Stack(MM_Menu_Victory_Screen);
                current_Game_State = GS_VICTORY;
			}

            // ***Rendering happens here***
            draw_Layer(get_Sprite_Sheet_Texture("bkg_Gameloop"));
            draw_Layer(get_Sprite_Sheet_Texture("collision_Terrain_1"));
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

            // Draw player projectiles
			for (uint32_t i = 0; i < game_Data.player_Proj_IDS.size(); i++) {
				Projectile* projectile = get_Projectile(game_Data.projectiles, game_Data.player_Proj_IDS[i]);
                if (projectile != nullptr) {
                    draw_RigidBody_Colliders(&projectile->rigid_Body, CI_GREEN);
                    if (projectile->life_Time > 0) {
                        // draw_Circle(projectile->rigid_Body.position_WS.x, projectile->rigid_Body.position_WS.y, get_Sprite_Radius(&projectile->sprite_Sheet_Tracker), CI_RED);
                        draw_Projectile(projectile, false);
                        projectile->life_Time -= delta_Time;
                    }
                }
            }
			// Draw enemy projectiles
			for (uint32_t i = 0; i < game_Data.enemy_Proj_IDS.size(); i++) {
				Projectile* projectile = get_Projectile(game_Data.projectiles, game_Data.enemy_Proj_IDS[i]);
                if (projectile != nullptr) {
                    // draw_RigidBody_Colliders(&arrow->rigid_Body, CI_GREEN);
                    if (projectile->life_Time > 0) {
                        // draw_Circle(projectile->rigid_Body.position_WS.x, projectile->rigid_Body.position_WS.y, get_Sprite_Radius(&projectile->sprite_Sheet_Tracker), CI_RED);
                        draw_Projectile(projectile, false);
                        projectile->life_Time -= delta_Time;
                    }
                }
			}

            // Draw player units
            for (uint32_t i = 0; i < game_Data.player_Unit_IDS.size(); i++) {
                Unit* player_Unit = get_Unit(game_Data.units, game_Data.player_Unit_IDS[i]);
                if (player_Unit != nullptr) {
                    /*
                    // Debugging circles for colliders
                    for (int j = 0; j < player_Warrior->animation_Tracker.animations_Array[game_Data.player_Warriors[i].animation_Tracker.type].sprite_Sheet.sprites.size(); j++) {
                        Animation_Type type = game_Data.player_Warriors[i].animation_Tracker.type;
                        draw_Circle(
                            game_Data.player_Warriors[i].rigid_Body.position_WS.x,
                            game_Data.player_Warriors[i].rigid_Body.position_WS.y,
                            game_Data.player_Warriors[i].animation_Tracker.animations_Array[type].sprite_Sheet.sprites[j].radius,
                            CI_Color_Index::RED
                        );
                    }
                    */
                    draw_RigidBody_Colliders(&player_Unit->rigid_Body, CI_GREEN);
                    draw_Unit_Animated(
                        &player_Unit->rigid_Body,
                        &player_Unit->sprite_Sheet_Tracker,
                        false
                    );
                    // draw_Resource_Bar(&player_Unit->rigid_Body.position_WS, &player_Unit->health_Bar);
                    // draw_Resource_Bar_With_Data(&player_Unit->rigid_Body.position_WS, &player_Unit->health_Bar);
                    draw_Unit_Data(*player_Unit, player_Unit->rigid_Body.position_WS);
					for (int j = 0; j < player_Unit->attached_Entities_Size; j++) {
						draw_Attached_Entity(&player_Unit->attached_Entities[j], player_Unit->rigid_Body.position_WS, false);
					}
                }
            }

            // Draw enemy Units
			for (uint32_t i = 0; i < game_Data.enemy_Unit_IDS.size(); i++) {
				Unit* enemy_Unit = get_Unit(game_Data.units, game_Data.enemy_Unit_IDS[i]);
				//Unit* enemy_Unit = get_Ptr_From_Handle_In_Storage(game_Data.units, game_Data.units.arr[i].handle);
                if (enemy_Unit != nullptr) {
                    // draw_Circle(warrior->rigid_Body.position_WS.x, warrior->rigid_Body.position_WS.y, 5, CI_RED);
                    // draw_Circle(warrior->rigid_Body.position_WS.x, warrior->rigid_Body.position_WS.y, 6, CI_RED);
                    // draw_Circle(warrior->rigid_Body.position_WS.x, warrior->rigid_Body.position_WS.y, 7, CI_RED);
                    // draw_RigidBody_Colliders(&warrior->rigid_Body, CI_GREEN);
                    draw_Unit_Animated(
                        &enemy_Unit->rigid_Body,
                        &enemy_Unit->sprite_Sheet_Tracker,
                        true
                    );
                    draw_Unit_Data(*enemy_Unit, enemy_Unit->rigid_Body.position_WS);
                    for (int j = 0; j < enemy_Unit->attached_Entities_Size; j++) {
                        draw_Attached_Entity(&enemy_Unit->attached_Entities[j], enemy_Unit->rigid_Body.position_WS, false);
                    }
                }
			}

            // DEBUGGING UI CODE
			draw_Time_Scalar(
				time_Scalar,
				(int)((RESOLUTION_WIDTH / 16) * 14),
				(int)(RESOLUTION_HEIGHT / 9 * 0.5),
				3
			);

            draw_Player_Hud();
            draw_Particle_Systems(game_Data);
           
#if 0
			if (button_Text(&font_1, "Play", { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 }, 150, 100, 3)) {
				soloud.play(wav_Electronic_Song);
			}
			if (button_Text(&font_1, "Stop", { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2 + 100 }, 150, 100, 3)) {
				soloud.stopAll();
			}
#endif

			delete_Expired_Entity_Handles(game_Data);
        }

        draw_Menu();

        SDL_RenderPresent(Globals::renderer);
    }

    free_Pixel_Data_In_Sprite_Sheet_Map();
    soloud.deinit();
    return 0;
}