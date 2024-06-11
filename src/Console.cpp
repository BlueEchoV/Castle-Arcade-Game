#include "Console.h"

// NOTE: Need a font

Console create_Console(float max_Openness, float rate_Of_Openness_DT) {
	Console result;
	result.rect.x = 0;
	result.rect.y = 0;
	result.rect.w = RESOLUTION_WIDTH;
	result.rect.h = 0;
	result.max_Openness = max_Openness;
	result.current_Openness = 0.0f;
	result.target_Openness = 0.0f;
	result.rate_Of_Openness_DT = rate_Of_Openness_DT;
	result.input_Background_Color = { 255, 0, 0, 0 };
	result.report_Background_Color = { 0, 255, 0, 0 };
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
	console.rect.h = (int)console.current_Openness;
	console.rect.x = RESOLUTION_WIDTH / 2;
	console.rect.y = -(RESOLUTION_HEIGHT / 2);
}

void set_Render_Draw_Color(Color color) {
	SDL_SetRenderDrawColor(Globals::renderer, color.r, color.g, color.b, color.a);
}

void draw_Console(Console& console, float delta_Time) {
	open_Close_Console(console);
	update_Openness(console, delta_Time);
	
	set_Render_Draw_Color(console.input_Background_Color);
	SDL_RenderDrawRect(Globals::renderer, &console.rect);
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