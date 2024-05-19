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

    CSV_Data sprite_Sheet_CSV_Data = {};
    sprite_Sheet_CSV_Data.file_Path = "data/Sprite_Sheet_Data.csv";
	load_Sprite_Sheet_Data_CSV(&sprite_Sheet_CSV_Data);

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

    bool spawn_Warrior_Pressed = false;
    bool spawn_Archer_Pressed = false;
    bool spawn_Necromancer_Pressed = false;

    bool running = true;

    // This could just be an array
    std::unordered_map<std::string, Game_Data> saved_Games_Cache = {};

    Cache_Data save_Game_Cache_Data = create_Cache_Data(saved_Games_Cache);

    CSV_Data particle_CSV_Data = create_Open_CSV_File("data/Particle_Data.csv");
    load_Particle_Data_CSV(&particle_CSV_Data);
    close_CSV_File(&particle_CSV_Data);

    CSV_Data unit_CSV_Data = create_Open_CSV_File("data/Unit_Data.csv");
    load_Unit_Data_CSV(&unit_CSV_Data);
    close_CSV_File(&unit_CSV_Data);

    CSV_Data projectile_CSV_Data = create_Open_CSV_File("data/Projectile_Data.csv");
    load_Projectile_Data_CSV(&projectile_CSV_Data);
    close_CSV_File(&projectile_CSV_Data);

	spawn_Particle_System(
		game_Data,
		"PT_RAINBOW",
        { RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 4 },
		1000,
		500,
		100,
        -1,
        false
	);

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

        // Hot loading
        attempt_Reload_Particle_CSV_File(&particle_CSV_Data);

        if (current_Game_State == GS_GAMELOOP) {
			for (Particle_System& particle_System : game_Data.particle_Systems) {
				for (Unit& unit : game_Data.enemy_Units) {
					if (particle_System.target_ID == unit.ID) {
						particle_System.rect.x = (int)unit.rigid_Body.position_WS.x;
						particle_System.rect.y = (int)unit.rigid_Body.position_WS.y;
						break;
					}
				}
				update_Particle_System(particle_System, delta_Time);
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
			SDL_RenderCopy(Globals::renderer, get_Sprite_Sheet_Texture("bkg_Menu"), NULL, NULL);
        
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
            SDL_RenderCopy(Globals::renderer, get_Sprite_Sheet_Texture("bkg_Menu"), NULL, NULL);

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
			SDL_RenderCopy(Globals::renderer, get_Sprite_Sheet_Texture("bkg_Gameloop"), NULL, NULL);
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
                        spawn_Projectile(game_Data, N_PLAYER, "arrow_Short", 10, game_Data.player_Castle.rigid_Body.position_WS, target_Mouse);
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

                // Spawn Player Warriors
                if (spawn_Warrior_Pressed) {
                    Castle* player_Castle = &game_Data.player_Castle;
                    Castle* enemy_Castle = &game_Data.enemy_Castle;
                    spawn_Unit(
                        &game_Data,
                        N_PLAYER,
                        "warrior",
                        1,
                        {
                            (float)player_Castle->rigid_Body.position_WS.x,
							((float)game_Data.terrain_Height_Map[(int)player_Castle->rigid_Body.position_WS.x] + get_Sprite_Radius(&player_Castle->sprite_Sheet_Tracker))
                        },
                        enemy_Castle->rigid_Body.position_WS
                    );
                    spawn_Warrior_Pressed = false;
                }
                if (spawn_Archer_Pressed) {
					Castle* player_Castle = &game_Data.player_Castle;
					Castle* enemy_Castle = &game_Data.enemy_Castle;
					spawn_Unit(
						&game_Data,
                        N_PLAYER,
						"archer",
                        1,
						{
							(float)player_Castle->rigid_Body.position_WS.x,
							((float)game_Data.terrain_Height_Map[(int)player_Castle->rigid_Body.position_WS.x] + get_Sprite_Radius(&player_Castle->sprite_Sheet_Tracker))
						},
                        enemy_Castle->rigid_Body.position_WS
                    );
                    spawn_Archer_Pressed = false;
                }
                if (spawn_Necromancer_Pressed) {
					Castle* player_Castle = &game_Data.player_Castle;
					Castle* enemy_Castle = &game_Data.enemy_Castle;
					spawn_Unit(
						&game_Data,
                        N_PLAYER,
						"necromancer",
						1,
						{
							(float)player_Castle->rigid_Body.position_WS.x,
							((float)game_Data.terrain_Height_Map[(int)player_Castle->rigid_Body.position_WS.x] + get_Sprite_Radius(&player_Castle->sprite_Sheet_Tracker))
						},
						enemy_Castle->rigid_Body.position_WS
					);
					spawn_Necromancer_Pressed = false;
                }

                // Spawn enemy Warriors
                if (game_Data.enemy_Castle.spawn_Cooldown.remaining < 0) {
					Castle* player_Castle = &game_Data.player_Castle;
					Castle* enemy_Castle = &game_Data.enemy_Castle;
                    // Readability
					float x_Pos = enemy_Castle->rigid_Body.position_WS.x;
					float terrain_height = (float)game_Data.terrain_Height_Map[(int)x_Pos];
					float radius = get_Sprite_Radius(&enemy_Castle->sprite_Sheet_Tracker);
					float y_Pos = terrain_height + radius;
					spawn_Unit(
						&game_Data,
						N_ENEMY,
						"warrior",
                        1,
                        { x_Pos, y_Pos },
						player_Castle->rigid_Body.position_WS
					);
                    enemy_Castle->spawn_Cooldown.remaining = enemy_Castle->spawn_Cooldown.duration;
                }
                else {
                    game_Data.enemy_Castle.spawn_Cooldown.remaining -= delta_Time;
                }

                // Update player arrow positions
                for (int i = 0; i < game_Data.player_Projectiles.size(); i++) {
                    Projectile* projectile = &game_Data.player_Projectiles[i];
                    if (!projectile->destroyed) {
                        update_Projectile_Position(projectile, delta_Time);
                    }
                }
				// Update enemy arrow positions
				for (int i = 0; i < game_Data.enemy_Projectiles.size(); i++) {
					Projectile* projectile = &game_Data.enemy_Projectiles[i];
					if (!projectile->destroyed) {
						update_Projectile_Position(projectile, delta_Time);
					}
				}

#if 0
				if (arrow->stuck_To_Unit.is_Sticking) {
					bool arrow_Currently_Stuck = false;
					for (int j = 0; j < game_Data.enemy_Warriors.size(); j++) {
						Warrior* warrior = &game_Data.enemy_Warriors[j];
						if (arrow->stuck_To_Unit.ID == warrior->ID) {
							arrow->rigid_Body.position_WS.x = warrior->rigid_Body.position_WS.x + arrow->stuck_To_Unit.offset.x;
							arrow->rigid_Body.position_WS.y = warrior->rigid_Body.position_WS.y + arrow->stuck_To_Unit.offset.y;

							arrow_Currently_Stuck = true;
						}
					}
					if (!arrow_Currently_Stuck) {
						arrow->destroyed = true;
					}
				}
#endif

                // Update player units
                for (int i = 0; i < game_Data.player_Units.size(); i++) {
                    if (game_Data.player_Units[i].destroyed == false) {
                        update_Unit_Position(
                            &game_Data.player_Units[i].rigid_Body,
                            game_Data.player_Units[i].stop,
                            delta_Time
                        );
                    }
                }

                // Update enemy units
                for (int i = 0; i < game_Data.enemy_Units.size(); i++) {
                    if (game_Data.enemy_Units[i].destroyed == false) {
                        update_Unit_Position(
                            &game_Data.enemy_Units[i].rigid_Body,
                            game_Data.enemy_Units[i].stop,
                            delta_Time
                        );
                    }
                }

                // Player Projectile Collision
                for (int i = 0; i < game_Data.player_Projectiles.size(); i++) {
                    Projectile* projectile = &game_Data.player_Projectiles[i];
                    Castle* enemy_Castle = &game_Data.enemy_Castle;
                    if (check_RB_Collision(&projectile->rigid_Body, &enemy_Castle->rigid_Body)) {
                        enemy_Castle->health_Bar.current_HP -= projectile->damage;
                        projectile->destroyed = true;
                    }
                    // Collision with map
					if (check_Height_Map_Collision(&projectile->rigid_Body, game_Data.terrain_Height_Map))
                    {
                        projectile->stop= true;
                    }
                    // Collision with Warriors and arrows
                    for (int j = 0; j < game_Data.enemy_Units.size(); j++) {
                        Unit* enemy_Unit = &game_Data.enemy_Units[j];
                        if (check_RB_Collision(&projectile->rigid_Body, &enemy_Unit->rigid_Body)) {
							if (!projectile->stop) {
                                // On first hit, proc the damage
                                if (projectile->collision_Delay.remaining == projectile->collision_Delay.duration) {
                                    spawn_Particle_System(
                                        game_Data,
                                        "PT_BLOOD",
                                        enemy_Unit->rigid_Body.position_WS,
                                        0.5,
                                        15,
                                        15,
                                        enemy_Unit->ID,
                                        false
									);
                                    enemy_Unit->health_Bar.current_HP -= projectile->damage;
                                    projectile->target_ID = enemy_Unit->ID;
                                }
                                bool targeted_Unit_Still_Alive = false;
                                for (int e = 0; e < game_Data.enemy_Units.size(); e++) {
                                    if (projectile->target_ID == game_Data.enemy_Units[e].ID) {
                                        targeted_Unit_Still_Alive = true;
                                    }
                                }
                                // Won't always stick, but it will always proc the damage.
                                // This way, if the projectile is fast and goes through the target (which is fine),
                                // then the projectile is bound to the unit but will also die with the unit.
                                if (targeted_Unit_Still_Alive && projectile->can_Attach) {
                                    if (projectile->collision_Delay.remaining > 0) {
                                        projectile->collision_Delay.remaining -= delta_Time;
                                    }
                                    else {
                                        V2 offset = projectile->rigid_Body.position_WS - enemy_Unit->rigid_Body.position_WS;
                                        Attached_Entity attached_Entity = return_Attached_Entity(
                                            projectile->type,
                                            projectile->rigid_Body.angle,
                                            offset
                                        );
                                        enemy_Unit->attached_Entities[enemy_Unit->attached_Entities_Size++] = attached_Entity;
                                        projectile->destroyed = true;
                                    }
                                }
                                else {
                                    projectile->destroyed = true;
                                }
							}
                        }
                    }
                }

				// Collision player units with map
				for (int i = 0; i < game_Data.player_Units.size(); i++) {
					Unit* player_Unit = &game_Data.player_Units[i];
					if (check_Height_Map_Collision(&player_Unit->rigid_Body, game_Data.terrain_Height_Map)) {
						float radius = get_Sprite_Radius(&player_Unit->sprite_Sheet_Tracker);
						float pos_Y_HM = (float)game_Data.terrain_Height_Map[(int)player_Unit->rigid_Body.position_WS.x];

                        player_Unit->rigid_Body.position_WS.y = ((RESOLUTION_HEIGHT - pos_Y_HM) - radius);
					}
				}

                // Collision enemy units with map
                for (int i = 0; i < game_Data.enemy_Units.size(); i++) {
                    Unit* unit = &game_Data.enemy_Units[i];
                    if (check_Height_Map_Collision(&unit->rigid_Body, game_Data.terrain_Height_Map)) {
                        float radius = get_Sprite_Radius(&unit->sprite_Sheet_Tracker);
                        float pos_Y_HM = (float)game_Data.terrain_Height_Map[(int)unit->rigid_Body.position_WS.x];

                        unit->rigid_Body.position_WS.y = ((RESOLUTION_HEIGHT - pos_Y_HM) - radius);
                    }
                }

                // Initialize default values before collision check
                for (int i = 0; i < game_Data.player_Units.size(); i++) {
                    Unit* player_Unit = &game_Data.player_Units[i];
                    player_Unit->stop = false;
                    player_Unit->current_Attack_Cooldown -= delta_Time;
                }
                for (int i = 0; i < game_Data.enemy_Units.size(); i++) {
                    Unit* enemy_Unit = &game_Data.enemy_Units[i];
                    enemy_Unit->stop = false;
                    enemy_Unit->current_Attack_Cooldown -= delta_Time;
                }

                // Rigid Body Collision: Player units with enemy castle
                for (int i = 0; i < game_Data.player_Units.size(); i++) {
                    Unit* player_Unit = &game_Data.player_Units[i];
                    Castle* castle = &game_Data.enemy_Castle;
                    if (check_RB_Collision(&player_Unit->rigid_Body, &castle->rigid_Body)) {
                        player_Unit->stop = true;
                        if (player_Unit->current_Attack_Cooldown < 0) {
                            player_Unit->current_Attack_Cooldown = player_Unit->attack_Cooldown;
                            castle->health_Bar.current_HP -= player_Unit->damage;
                        }
                    }
                }
                // Rigid Body Collision: Enemy units with player castle
                for (int i = 0; i < game_Data.enemy_Units.size(); i++) {
                    Unit* enemy_Unit = &game_Data.enemy_Units[i];
                    Castle* castle = &game_Data.player_Castle;
                    if (check_RB_Collision(&enemy_Unit->rigid_Body, &castle->rigid_Body)) {
                        enemy_Unit->stop = true;
                        if (enemy_Unit->current_Attack_Cooldown < 0) {
                            enemy_Unit->current_Attack_Cooldown = enemy_Unit->attack_Cooldown;
                            castle->health_Bar.current_HP -= enemy_Unit->damage;
                        }
                    }
                }
                // Rigid Body Collision: Player units with enemy units
                for (int i = 0; i < game_Data.player_Units.size(); i++) {
                    Unit* player_Unit = &game_Data.player_Units[i];
                    for (int j = 0; j < game_Data.enemy_Units.size(); j++) {
                        Unit* enemy_Unit = &game_Data.enemy_Units[j];
                        if (check_RB_Collision(&player_Unit->rigid_Body, &enemy_Unit->rigid_Body)) {
                            player_Unit->stop = true;
                            enemy_Unit->stop = true;
                            if (player_Unit->current_Attack_Cooldown <= 0) {
                                player_Unit->current_Attack_Cooldown = player_Unit->attack_Cooldown;
                                enemy_Unit->health_Bar.current_HP -= player_Unit->damage;
                            }
                            if (enemy_Unit->current_Attack_Cooldown <= 0) {
                                enemy_Unit->current_Attack_Cooldown = enemy_Unit->attack_Cooldown;
                                player_Unit->health_Bar.current_HP -= enemy_Unit->damage;
                            }
                        }
                    }
                }

                // Units that fire projectiles
                for (int i = 0; i < game_Data.player_Units.size(); i++) {
                    Unit* player_Unit = &game_Data.player_Units[i];
                    for (int j = 0; j < game_Data.enemy_Units.size(); j++) {
                        Unit* enemy_Unit = &game_Data.enemy_Units[j];
                        if (player_Unit->projectile_Type != "") {
                            if (check_Attack_Range_Collision(player_Unit->attack_Range, &player_Unit->rigid_Body, &enemy_Unit->rigid_Body)) {
                                // change_Animation(&player_Unit->sprite_Sheet_Tracker, "archer_Stop");
                                player_Unit->stop = true;
                                if (player_Unit->current_Attack_Cooldown <= 0) {
                                    player_Unit->current_Attack_Cooldown = player_Unit->attack_Cooldown;
                                    V2 aim_Head = enemy_Unit->rigid_Body.position_WS;
                                    aim_Head.x += get_Sprite_Radius(&enemy_Unit->sprite_Sheet_Tracker);
                                    V2 arrow_Spawn_Location = player_Unit->rigid_Body.position_WS;
                                    arrow_Spawn_Location.y -= get_Sprite_Radius(&enemy_Unit->sprite_Sheet_Tracker) / 2;
                                    spawn_Projectile(game_Data, N_PLAYER, player_Unit->projectile_Type, player_Unit->damage, arrow_Spawn_Location, aim_Head);
                                }
                            }
                            else {
                                // change_Animation(&player_Unit->sprite_Sheet_Tracker, "archer_Walk");
                            }
                            if (check_RB_Collision(&player_Unit->rigid_Body, &enemy_Unit->rigid_Body)) {
                                enemy_Unit->stop = true;
                                if (enemy_Unit->current_Attack_Cooldown <= 0) {
                                    enemy_Unit->current_Attack_Cooldown = enemy_Unit->attack_Cooldown;
                                    player_Unit->health_Bar.current_HP -= enemy_Unit->damage;
                                }
                            }
                        }
					}
				}

                // TODO: Need to do a attack range collision check for the enemy as well

				for (int i = 0; i < game_Data.player_Units.size(); i++) {
					Unit* player_Unit = &game_Data.player_Units[i];
					float speed = player_Unit->speed;
					if (!player_Unit->stop) {
						update_Animation(&player_Unit->sprite_Sheet_Tracker, speed, delta_Time);
					}
				}
                for (int i = 0; i < game_Data.enemy_Units.size(); i++) {
                    Unit* enemy_Unit = &game_Data.enemy_Units[i];
                    float speed = enemy_Unit->speed;
                    if (!enemy_Unit->stop) {
                        update_Animation(&enemy_Unit->sprite_Sheet_Tracker, speed, delta_Time);
                    }
                }
            }

            if (game_Data.player_Castle.health_Bar.current_HP <= 0) {
                current_Game_State = GS_GAMEOVER;
            }
			if (game_Data.enemy_Castle.health_Bar.current_HP <= 0) {
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
            for (int i = 0; i < game_Data.player_Projectiles.size(); i++) {
                Projectile* projectile = &game_Data.player_Projectiles[i];
                draw_RigidBody_Colliders(&projectile->rigid_Body, CI_GREEN);
                if (projectile->life_Time > 0) {
                    // draw_Circle(projectile->rigid_Body.position_WS.x, projectile->rigid_Body.position_WS.y, get_Sprite_Radius(&projectile->sprite_Sheet_Tracker), CI_RED);
                    draw_Projectile(projectile, false);
                    projectile->life_Time -= delta_Time;
                }
            }
			// Draw enemy projectiles
			for (int i = 0; i < game_Data.enemy_Projectiles.size(); i++) {
				Projectile* projectile = &game_Data.enemy_Projectiles[i];
				// draw_RigidBody_Colliders(&arrow->rigid_Body, CI_GREEN);
				if (projectile->life_Time > 0) {
					// draw_Circle(projectile->rigid_Body.position_WS.x, projectile->rigid_Body.position_WS.y, get_Sprite_Radius(&projectile->sprite_Sheet_Tracker), CI_RED);
					draw_Projectile(projectile, false);
                    projectile->life_Time -= delta_Time;
				}
			}

            // Draw player units
            for (int i = 0; i < game_Data.player_Units.size(); i++) {
                Unit* player_Unit = &game_Data.player_Units[i];
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
                draw_HP_Bar(&player_Unit->rigid_Body.position_WS, &player_Unit->health_Bar);
            }

            // Draw enemy Units
            for (int i = 0; i < game_Data.enemy_Units.size(); i++) {
                Unit* enemy_Unit = &game_Data.enemy_Units[i];
                // draw_Circle(warrior->rigid_Body.position_WS.x, warrior->rigid_Body.position_WS.y, 5, CI_RED);
                // draw_Circle(warrior->rigid_Body.position_WS.x, warrior->rigid_Body.position_WS.y, 6, CI_RED);
                // draw_Circle(warrior->rigid_Body.position_WS.x, warrior->rigid_Body.position_WS.y, 7, CI_RED);
                // draw_RigidBody_Colliders(&warrior->rigid_Body, CI_GREEN);
                draw_Unit_Animated(
                    &enemy_Unit->rigid_Body,
                    &enemy_Unit->sprite_Sheet_Tracker,
                    true
                );
                draw_HP_Bar(&enemy_Unit->rigid_Body.position_WS, &enemy_Unit->health_Bar);
                for (int j = 0; j < enemy_Unit->attached_Entities_Size; j++) {
                    draw_Attached_Entity(&enemy_Unit->attached_Entities[j], enemy_Unit->rigid_Body.position_WS, false);
				}
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
            if (button_Image(get_Sprite_Sheet_Texture("warrior_Stop"), "Spawn Warrior", button_Pos, button_Height_Unit_Spawn)) {
                spawn_Warrior_Pressed = true;
            } 
            button_Pos.x += button_Height_Unit_Spawn;
            if (button_Image(get_Sprite_Sheet_Texture("archer_Stop"), "Spawn Archer", button_Pos, button_Height_Unit_Spawn)) {
				spawn_Archer_Pressed = true;
			}
            button_Pos.x += button_Height_Unit_Spawn;
			if (button_Image(get_Sprite_Sheet_Texture("necromancer_Stop"), "Spawn Necromancer", button_Pos, button_Height_Unit_Spawn)) {
				spawn_Necromancer_Pressed = true;
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

            std::erase_if(game_Data.player_Projectiles, [](Projectile& projectile) {
                return projectile.destroyed || projectile.life_Time <= 0;
                });
			std::erase_if(game_Data.enemy_Projectiles, [](Projectile& projectile) {
				return projectile.destroyed || projectile.life_Time <= 0;
				});
			std::erase_if(game_Data.player_Units, [](Unit& unit) {
				return unit.destroyed || unit.health_Bar.current_HP <= 0;
				});
            std::erase_if(game_Data.enemy_Units, [](Unit& unit) {
                return unit.destroyed || unit.health_Bar.current_HP <= 0;
                });
			std::erase_if(game_Data.particle_Systems, [](Particle_System& particle_System) {
				return particle_System.destroyed && particle_System.particles.size() == 0;
				});
        }
        SDL_RenderPresent(Globals::renderer);
    }

    free_Pixel_Data_In_Sprite_Sheet_Map();

    soloud.deinit();
    return 0;

}