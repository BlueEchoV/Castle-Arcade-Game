#include "Console.h"

// NOTE: Need a font

Console create_Console(float max_Openness, float rate_Of_Openness_DT) {
	Console result;
	result.bkg_Rect.x = 0;
	result.bkg_Rect.y = 0;
	result.bkg_Rect.w = RESOLUTION_WIDTH;
	result.bkg_Rect.h = 0;
	result.ipt_Rect.x = 0;
	result.ipt_Rect.y = 0;
	result.ipt_Rect.w = RESOLUTION_WIDTH;
	result.ipt_Max_Height = RESOLUTION_HEIGHT / 32;
	result.ipt_Rect.h = 0;
	result.state = CS_Closed;
	result.max_Openness = max_Openness;
	result.current_Openness = 0.0f;
	result.target_Openness = 0.0f;
	result.rate_Of_Openness_DT = rate_Of_Openness_DT;
	result.input_Background_Color = { 0, 139, 0, SDL_ALPHA_OPAQUE };
	result.report_Background_Color = { 139, 0, 0, 125 };
	return result;
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
	// Background rect
	console.bkg_Rect.h = (int)console.current_Openness;
	// Input rect
	if (console.current_Openness <= console.ipt_Max_Height) {
		console.ipt_Rect.h = (int)console.current_Openness;
	} else {
		console.ipt_Rect.y = (int)console.current_Openness - (int)console.ipt_Max_Height;
		console.ipt_Rect.h = (int)console.ipt_Max_Height;
	}
}

void set_Render_Draw_Color(Color color) {
	SDL_SetRenderDrawColor(Globals::renderer, color.r, color.g, color.b, color.a);
}

void draw_Console(Console& console, float delta_Time) {
	open_Close_Console(console);
	update_Openness(console, delta_Time);
	
	set_Render_Draw_Color(console.report_Background_Color);
	SDL_RenderDrawRect(Globals::renderer, &console.bkg_Rect);
	SDL_RenderFillRect(Globals::renderer, &console.bkg_Rect);

	set_Render_Draw_Color(console.input_Background_Color);
	SDL_RenderDrawRect(Globals::renderer, &console.ipt_Rect);
	SDL_RenderFillRect(Globals::renderer, &console.ipt_Rect);
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