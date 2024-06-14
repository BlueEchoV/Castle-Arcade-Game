#include "Console.h"

std::unordered_map<SDL_Keycode, Key_State> console_Key_States = {};

// NOTE: Need a font

Console create_Console(Font* font, int text_Size, float max_Openness, float rate_Of_Openness_DT) {
	Console result;
	
	result.text_Size_Multiplier = text_Size;
	result.font = font;
	result.cursor_Indicator.duration = 0.25;
	result.cursor_Indicator.remaining = 0.0;

	result.bkg_Rect.x = 0;
	result.bkg_Rect.y = 0;
	result.bkg_Rect.w = RESOLUTION_WIDTH;
	result.bkg_Rect.h = 0;

	result.ipt_Rect.x = 0;
	result.ipt_Rect.y = 0;
	result.ipt_Rect.w = RESOLUTION_WIDTH;
	result.text_Height = font->char_Height * text_Size;
	result.ipt_Rect.h = 0;

	result.state = CS_Closed;
	result.max_Openness = max_Openness;
	result.current_Openness = 0.0f;
	result.target_Openness = 0.0f;
	result.rate_Of_Openness_DT = rate_Of_Openness_DT;
	result.input_Background_Color = { 34,34,34, SDL_ALPHA_OPAQUE };
	result.report_Background_Color = { 0, 0, 0, SDL_ALPHA_OPAQUE };
	result.history_Size = {};
	
	return result;
}

void add_Input_To_History(Console& console) {
	// Guard against array size
	if (console.history_Size < ARRAY_SIZE(console.history)) {
		std::string str = console.user_Input;
		console.history[console.history_Size] = str;
		console.history_Size++;
		
	} else {
		// Should never get here
		assert(false);
	}
}

void update_Openness(Console& console, float delta_Time) {
	float rate_Of_Openness = delta_Time * console.rate_Of_Openness_DT;
	if (console.current_Openness < console.target_Openness) {
		console.current_Openness += rate_Of_Openness;
		if (console.current_Openness > console.target_Openness) {
			console.current_Openness = console.target_Openness;
		} 
	} else if (console.current_Openness > console.target_Openness) {
		console.current_Openness -= rate_Of_Openness;
		if (console.current_Openness < 0) {
			console.current_Openness = 0;
		}
	}
	
	// Input rect
	if (console.current_Openness <= console.text_Height) {
		console.ipt_Rect.y = 0;
		console.ipt_Rect.h = (int)console.current_Openness;
	}
	else {
		console.ipt_Rect.y = (int)console.current_Openness - (int)console.text_Height;
		console.ipt_Rect.h = (int)console.text_Height;
	}
	
	// Background rect
	int background_Openness = (int)(console.current_Openness - console.text_Height);
	if (background_Openness >= 0) {
		console.bkg_Rect.h = background_Openness;
	} else {
		console.bkg_Rect.h = 0;
	}
}

void set_Render_Draw_Color(Color color) {
	SDL_SetRenderDrawColor(Globals::renderer, color.r, color.g, color.b, color.a);
}

void draw_Console(Console& console, float delta_Time) {
	open_Close_Console(console);
	update_Openness(console, delta_Time);
	
	if (console.bkg_Rect.h > 0 || console.ipt_Rect.h > 0) {
		// Background
		set_Render_Draw_Color(console.report_Background_Color);
		SDL_RenderDrawRect(Globals::renderer, &console.bkg_Rect);
		SDL_RenderFillRect(Globals::renderer, &console.bkg_Rect);

		// Input Text Area
		set_Render_Draw_Color(console.input_Background_Color);
		SDL_RenderDrawRect(Globals::renderer, &console.ipt_Rect);
		SDL_RenderFillRect(Globals::renderer, &console.ipt_Rect);
		SDL_SetTextureColorMod(console.font->texture, 0, 255, 0);
		// Draw history
		int text_Y_Offset = console.bkg_Rect.h - (console.font->char_Height * console.text_Size_Multiplier);
		// Draw them in reverse order
		for (int i = console.history_Size; i > 0; i--) {
			// Draw +1 off the top so there is a nice transition and ONLY draw what is necessary for the given rect
			if (text_Y_Offset < console.bkg_Rect.h){
				std::string current_Str = console.history[i - 1];
				draw_String(console.font, current_Str.c_str(), console.bkg_Rect.x, text_Y_Offset, console.text_Size_Multiplier, false);
				text_Y_Offset -= console.text_Height;
			} else {
				break;
			}
		}

		int ipt_Y_Offset = 0;
		if (console.current_Openness < console.text_Height) {
			ipt_Y_Offset = (int)console.current_Openness - console.text_Height;
		} else {
			ipt_Y_Offset = console.ipt_Rect.y;
		}
		draw_String(console.font, console.user_Input, console.ipt_Rect.x, ipt_Y_Offset, console.text_Size_Multiplier, false);
		
		SDL_Rect cursor;
		cursor.w = console.font->char_Width * console.text_Size_Multiplier;
		cursor.h = console.font->char_Height * console.text_Size_Multiplier;
		size_t ipt_Length = strlen(console.user_Input);
		if (ipt_Length > 0) {
			cursor.x = (console.font->char_Width * 2) * (int)((ipt_Length));
		} else {
			cursor.x = 0;
		}
		cursor.y = ipt_Y_Offset;
		// NOTE: THIS IS IF I WANT THE CURSOR TO BLINK. STILL DECIDING.
		//progress_CD(console.cursor_Indicator, delta_Time);
		//if (console.cursor_Indicator.remaining > (console.cursor_Indicator.duration / 2)) {
			// SDL_SetRenderDrawColor(Globals::renderer, 57, 255, 20, 100);
			// SDL_RenderDrawRect(Globals::renderer, &cursor);
			// SDL_RenderFillRect(Globals::renderer, &cursor);
		//}
		
		SDL_SetRenderDrawColor(Globals::renderer, 57, 255, 20, 100);
		SDL_RenderFillRect(Globals::renderer, &cursor);
		SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 0, 0);
		SDL_RenderDrawRect(Globals::renderer, &cursor);
		
		// Reset the color mod
		SDL_SetTextureColorMod(console.font->texture, 0, 0, 0);
	}
}

void open_Close_Console(Console& console) {
	if (console.state == CS_Closed) {
		console.target_Openness = 0.0f;
	} else if (console.state == CS_Open_Small) {
		console.target_Openness = RESOLUTION_HEIGHT / 4;
	} else if (console.state == CS_Open_Big) {
		console.target_Openness = (RESOLUTION_HEIGHT / 4) * 3;
	} else {
		assert(false);
	}
}

bool is_Console_Open(Console& console) {
	if (console.state != CS_Closed) {
		return true;
	}
	return false;
}

// I could have this return a string that says what's missing
bool process_Command_Spawn(std::string command) {
	std::istringstream stream(command);
	std::string word;

	// Skip the first word
	if (!(stream >> word)) {
		return false;
	} 

	// Nation
	Nation nation = {};
	if (stream >> word) {
		if (word == "player") {
			nation = N_PLAYER;
		}
		else if (word == "enemy") {
			nation = N_ENEMY;
		}
	} else {
		return false;
	}

	// Unit Type
	std::string unit_Type = "";
	if ((stream >> word)) {
		std::vector<std::string> unit_Types = get_Unit_Types();
		for (std::string type : unit_Types) {
			if (type == word) {
				unit_Type = word;
				break;
			}
		}
	} else {
		return false;
	}

	// Level?
//	if (stream >> word) {
//
//	}
	
	// If we have any more input, this is an error
	if (stream >> word) {
		return false;
	}

	if (nation == N_PLAYER) {
		spawn_Unit(game_Data, nation, unit_Type, 1, game_Data.player_Castle.rigid_Body.position_WS, game_Data.enemy_Castle.rigid_Body.position_WS);
	} else if (nation == N_PLAYER) {
		spawn_Unit(game_Data, nation, unit_Type, 1, game_Data.enemy_Castle.rigid_Body.position_WS, game_Data.player_Castle.rigid_Body.position_WS);
	}
	return true;
}

bool process_Console_Command(std::string command) {
	// Get the first word to see what the command type is
	std::istringstream stream(command);
	std::string first_Word;
	stream >> first_Word;
	if (first_Word == "test") {
		printf("Test command\n");
		return true;
	} else if (first_Word == "spawn") {
		if (process_Command_Spawn(command)) {
			return true;
		}
	}
	return false;
}

void process_Console_Input(Console& console) {
	add_Input_To_History(console);
	if (process_Console_Command(console.user_Input)) {
		printf("Valid command: %s", console.user_Input);
		// Reset the user input
		for (int i = 0; i < strlen(console.user_Input); i++) {
			console.user_Input[i] = 0;
		}
	} else {
		printf("Invalid command: %s", console.user_Input);
		// Don't reset user input if the command doesn't work
	}
	
}


