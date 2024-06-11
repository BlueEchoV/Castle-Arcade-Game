#include "Console.h"

// NOTE: Need a font

Console create_Console(float max_Openness, float rate_Of_Openness_DT) {
	Console result;
	result.rect.x = 0;
	result.max_Openness = max_Openness;
	result.current_Openness = 0.0f;
	result.target_Openness = 0.0f;
	result.rate_Of_Openness_DT = rate_Of_Openness_DT;
	result.input_Background_Color = { 255, 0, 0, 0 };
	result.report_Background_Color = { 0, 255, 0, 0 };
	return result;
}

void update_Openness(Console console, float delta_Time) {
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
}

void draw_Console() {
	update_Openness();


}

float get_Console_Bottom(Console console) {

}