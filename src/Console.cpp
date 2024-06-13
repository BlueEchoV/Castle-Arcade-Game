#include "Console.h"

// NOTE: Need a font

Console create_Console(Font* font, int text_Size, float max_Openness, float rate_Of_Openness_DT) {
	Console result;
	
	result.text_Size_Multiplier = text_Size;
	result.font = font;

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
	result.input_Background_Color = { 0, 139, 0, SDL_ALPHA_OPAQUE };
	result.report_Background_Color = { 139, 0, 0, SDL_ALPHA_OPAQUE };
	result.history_Size = {};
	
	return result;
}

void add_Input_To_History(Console& console) {
	// Guard against array size
	if (console.history_Size < ARRAY_SIZE(console.history)) {
		std::string str = console.user_Input;
		console.history[console.history_Size] = str;
		console.history_Size++;
		for (int i = 0; i < strlen(console.user_Input); i++) {
			console.user_Input[i] = 0;
		}
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

		// Draw history
		int text_Y_Offset = console.bkg_Rect.h - console.text_Height;
		for (int i = 0; i < console.history_Size; i++) {
			// Draw +1 off the top so there is a nice transition and ONLY draw what is necessary for the given rect
			if (text_Y_Offset < console.bkg_Rect.h){
				std::string current_Str = console.history[i];
				draw_String(console.font, current_Str.c_str(), console.bkg_Rect.x, text_Y_Offset, console.text_Size_Multiplier, false);
				text_Y_Offset -= console.text_Height;
			} else {
				break;
			}
		}
		draw_String(console.font, console.user_Input, console.ipt_Rect.x, console.ipt_Rect.y, console.text_Size_Multiplier, false);
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

//float get_Console_Bottom(Console console) {
//
//}