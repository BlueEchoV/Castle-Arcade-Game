#include "Particle_System.h"
#include <algorithm> 
#include "Entity.h"

namespace Globals {
	std::unordered_map<std::string, Particle_Data> particle_Data_Map = {};
}

void spawn_Particle_System(Game_Data& game_Data, std::string particle_Type, V2 pos, float lifetime, int w, int h, int target_ID, bool flip_Horizontally) {
	Particle_System particle_System = {};

	particle_System.rect.x = (int)pos.x;
	particle_System.rect.y = (int)pos.y;
	particle_System.rect.w = w;
	particle_System.rect.h = h;
	particle_System.particle_Type = particle_Type;
	particle_System.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(Globals::particle_Data_Map[particle_Type].sprite_Sheet_Name);
	particle_System.time_Between_Spawns = 0.0f;
	particle_System.destroyed = false;
	particle_System.lifetime = lifetime;
	particle_System.target_ID = target_ID;
	particle_System.flip_Horizontally = flip_Horizontally;

	game_Data.particle_Systems.push_back(particle_System);
}

// Update 
void update_Particle_System(Particle_System& particle_System, float delta_Time) {
	// Store the current position
	{
		// Update current particles
		for (int i = 0; i < particle_System.particles.size(); i++) {
			particle_System.particles[i].velocity.y += (100 * delta_Time);

			particle_System.particles[i].position.x += (particle_System.particles[i].velocity.x * delta_Time);
			particle_System.particles[i].position.y += (particle_System.particles[i].velocity.y * delta_Time);

			particle_System.particles[i].fade_In -= delta_Time;
		}
	}
	{
		particle_System.time_Between_Spawns -= delta_Time;
		// Spawn new particles based off time
		int max_Spawn = 1000;
		int current_Spawn = 0;
		
		const Particle_Data* data = &Globals::particle_Data_Map[particle_System.particle_Type];

		while (particle_System.time_Between_Spawns <= 0 
			&& current_Spawn <= max_Spawn
			&& particle_System.destroyed == false) {
			Particle particle = {};
			SDL_Rect* rect = &particle_System.rect;

			particle.lifetime = 
				random_Float_In_Range(
					data->lifetime_Min, 
					data->lifetime_Max
				);
			particle.lifetime_Max = particle.lifetime;
			particle.velocity =
				random_Vector_In_Range(
					data->velocity_Min,
					data->velocity_Max
				);
			if (particle_System.flip_Horizontally) {
				// Flip the sign so it goes the opposite way
				particle.velocity.x *= -1;
				particle.velocity.y *= -1;
			}
			particle.position =
				random_Vector_In_Range(
					{
						(float)rect->x - ((float)rect->w / 2),
						(float)rect->y - ((float)rect->h / 2)
					},
					{
						(float)rect->x + ((float)rect->w / 2),
						(float)rect->y + ((float)rect->h / 2)
					}
				);
			particle.size = data->size;
			particle.fade_In = data->max_Fade;
			particle_System.particles.push_back(particle);

			// Adding it binds it to the frames
			particle_System.time_Between_Spawns += data->time_Between_Spawns;
			current_Spawn++;
		}
	}
	{
		// update lifetime and destroy particles
		std::erase_if(particle_System.particles, [delta_Time](Particle& particle) {
			particle.lifetime -= delta_Time;
			return particle.lifetime <= 0;
			});
		// Move this check into the particles themselves so all of them don't just disappear
		particle_System.lifetime -= delta_Time;
		if (particle_System.lifetime <= 0) {
			particle_System.destroyed = true;
		}
	}
}

float clamp(float& a) {
	if (a > 1.0) {
		return a = 1.0;
	}
	else if (a < 0.0) {
		return a = 0.0;
	} 
	else {
		return a;
	}
}

//				   Color type	Vividness		Light/Darkness
//			       Hue			Saturation		Value
F_Color HSV_To_RGB(float h,		float s,		float v) {
	// NOTE: V is the max
	float max = v;
	// Intensity of the color
	// Chroma is the difference between max 
	// and min values of the RGB model
	float chroma = s * max;
	float min = v - chroma;
	// Transitioning from different hues								  
	float x = (float)(chroma * (1 - fabs(fmod(h / 60.0, 2) - 1)));
	float r, g, b;

	if (h < 60) {
		r = chroma;
		g = x; 
		b = 0;
	} else if (h < 120) { 
		r = x;
		g = chroma;
		b = 0;
	} else if (h < 180) { 
		r = 0; 
		g = chroma;
		b = x;
	} else if (h < 240) { 
		r = 0; 
		g = x;
		b = chroma;
	} else if (h < 300) { 
		r = x;
		g = 0;
		b = chroma;
	} else { 
		r = chroma;
		g = 0;
		b = x;
	}

	return { r + min, g + min, b + min };
}

// Render
void draw_Particle_Systems(Game_Data& game_Data) {
	for (const Particle_System& particle_System : game_Data.particle_Systems) {
		const Particle_Data* particle_Data = &Globals::particle_Data_Map[particle_System.particle_Type];
		const Sprite_Sheet* sprite_Sheet_Data = &Globals::sprite_Sheet_Map[particle_Data->sprite_Sheet_Name];
		SDL_Texture* texture = sprite_Sheet_Data->sprites[0].image.texture;
		// Chris' trick for rendering backwards
		for (size_t i = particle_System.particles.size(); i--;) {
			SDL_Rect src_Rect = {};
			src_Rect.w = particle_System.particles[i].size;
			src_Rect.h = particle_System.particles[i].size;
			src_Rect.x = (int)particle_System.particles[i].position.x;
			src_Rect.y = (int)particle_System.particles[i].position.y;

			const Particle* particle = &particle_System.particles[i];

			F_Color color = {};
			float lifetime_Delta = particle->lifetime_Max - particle->lifetime;
			float fade_Percent = 0.0f;
			if (lifetime_Delta <= particle_Data->max_Fade) {
				fade_Percent = lifetime_Delta / particle_Data->max_Fade;
				color.a = linear_Interpolation(0, 1.0, fade_Percent);

			} else if (lifetime_Delta >= (particle->lifetime_Max - particle_Data->max_Fade)) {
				fade_Percent = (particle_Data->lifetime_Max - lifetime_Delta) / particle_Data->max_Fade;
				color.a = linear_Interpolation(0, 1.0, fade_Percent);
			} else {
				color.a = 1.0f;
			}

			SDL_SetTextureAlphaMod(texture, (Uint8)(255 * (color.a)));

			float percent_Life_time = particle_System.particles[i].lifetime / particle->lifetime_Max;

			if (particle_System.particle_Type == "PT_RAINBOW") {
				float hue = 360.0f * (1.0f - percent_Life_time);
				float saturation = 1.0f;
				float value = 1.0f;

				color = HSV_To_RGB(hue, saturation, value);
			} else if (particle_System.particle_Type == "PT_BLOOD") {
				color.r = 1.0f;
			}
			SDL_SetTextureColorMod(texture, (Uint8)(255 * color.r), (Uint8)(255 * color.g), (Uint8)(255 * color.b));
			
			// SDL_SetRenderDrawColor();

			SDL_RenderCopyEx(Globals::renderer, texture, NULL, &src_Rect, 0, NULL, SDL_FLIP_NONE);
		}
		SDL_SetTextureAlphaMod(texture, SDL_ALPHA_OPAQUE);
		SDL_SetTextureColorMod(texture, 0, 0, 0);
	}
}

#include <chrono>
#include <thread>
void load_Particle_Data_CSV(std::string file_Name) {
	std::ifstream file;
	
	int retries = 0;
	const int max_retries = 5;
	const int delay_ms = 200;

	// copy pasta for testing and it worked
	// The first run through is failing but the second runthrough isn't.
	while (retries < max_retries) {
		file.open(file_Name);
		if (file.is_open()) {
			break;
		}

		SDL_Log("Attempt %d: Error loading .csv file", retries + 1);
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
		retries++;
	}

	if (!file.is_open()) {
		SDL_Log("Error loading .csv file");
	}

	DEFER{
		file.close();
	};

	std::string line;

	std::getline(file, line);
	std::getline(file, line);

	Particle_Data particle_Data = {};
	while (std::getline(file, line)) {
		// Types,size,max_Particles,time_Between Spawns,max_Fade,lifetime_Min,lifetime_Max
		std::vector<std::string> tokens = split(line, ',');
		int row_Count = 0;
		std::string particle_Type = tokens[row_Count++];

		if (tokens.size() == 11) {
			particle_Data.sprite_Sheet_Name = tokens[row_Count++];
			particle_Data.size = std::stoi(tokens[row_Count++]);
			particle_Data.time_Between_Spawns = std::stof(tokens[row_Count++]);
			particle_Data.max_Fade = std::stof(tokens[row_Count++]);
			particle_Data.lifetime_Min = std::stof(tokens[row_Count++]);
			particle_Data.lifetime_Max = std::stof(tokens[row_Count++]);
			particle_Data.velocity_Min.x = std::stof(tokens[row_Count++]);
			particle_Data.velocity_Min.y = std::stof(tokens[row_Count++]);
			particle_Data.velocity_Max.x = std::stof(tokens[row_Count++]);
			particle_Data.velocity_Max.y = std::stof(tokens[row_Count++]);

			Globals::particle_Data_Map[particle_Type] = particle_Data;
		}
		else {
			SDL_Log("Error: Line does not have enough data");
		}
	}
}
