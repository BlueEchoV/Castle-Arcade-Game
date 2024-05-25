#include "Particle_System.h"
#include <algorithm> 
#include "Entity.h"
#include <assert.h>
#include <span>

std::unordered_map<std::string, Particle_Data> particle_Data_Map = {};

const Particle_Data bad_Particle_Data = {
	// type,		sprite_Sheet,		size	  particles_Per_Second max_Fade_In  lifetime_Min	lifetime_Max  velocity_Min		velocity_Max
	  "PT_RAINBOW", "basic_Particle_1", 20,		  1.0f,					0.5f,		3.0f,			3.0f,			{0.0f, 0.0f},	{0.0f, 0.0f}
};

const Particle_Data& get_Particle_Data(std::string key) {
	auto it = particle_Data_Map.find(key);
	if (it != particle_Data_Map.end()) {
		// Key found
		return it->second;
	}

	assert(false);
	// Return garbage values
	return bad_Particle_Data;
}

void spawn_Particle_System(Game_Data& game_Data, std::string particle_Type, V2 pos, float lifetime, int w, int h, Handle target_Handle, bool flip_Horizontally) {
	Particle_System particle_System = {};

	particle_System.rect.x = (int)pos.x;
	particle_System.rect.y = (int)pos.y;
	particle_System.rect.w = w;
	particle_System.rect.h = h;
	particle_System.particle_Type = particle_Type;
	particle_System.sprite_Sheet_Tracker = create_Sprite_Sheet_Tracker(particle_Data_Map[particle_Type].sprite_Sheet_Name);
	particle_System.time_Between_Spawns = 0.0f;
	particle_System.destroyed = false;
	particle_System.lifetime = lifetime;
	particle_System.target_Handle = target_Handle;
	particle_System.flip_Horizontally = flip_Horizontally;

	particle_System.handle = create_Handle(game_Data.particle_Systems);
	game_Data.particle_Systems.arr[particle_System.handle.index] = particle_System;
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
		
		const Particle_Data* data = &get_Particle_Data(particle_System.particle_Type);

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

			particle_System.time_Between_Spawns += (1.0f / data->particles_Per_Second);
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
	for (uint32_t i = 0; i < game_Data.particle_Systems.index_One_Past_Last; i++) {
		Particle_System* particle_System = get_Ptr_From_Particle_System_Storage(game_Data.particle_Systems, game_Data.particle_Systems.arr[i].handle);
		if (particle_System != nullptr) {
			const Particle_Data* particle_Data = &get_Particle_Data(particle_System->particle_Type);
			const Sprite_Sheet* sprite_Sheet_Data = &get_Sprite_Sheet(particle_Data->sprite_Sheet_Name);
			SDL_Texture* texture = sprite_Sheet_Data->sprites[0].image.texture;
			// Chris' trick for rendering backwards
			for (size_t j = particle_System->particles.size(); j--;) {
				SDL_Rect src_Rect = {};
				src_Rect.w = particle_System->particles[j].size;
				src_Rect.h = particle_System->particles[j].size;
				src_Rect.x = (int)particle_System->particles[j].position.x;
				src_Rect.y = (int)particle_System->particles[j].position.y;

				const Particle* particle = &particle_System->particles[j];

				F_Color color = {};
				float lifetime_Delta = particle->lifetime_Max - particle->lifetime;
				float fade_Percent = 0.0f;
				if (lifetime_Delta <= particle_Data->max_Fade) {
					fade_Percent = lifetime_Delta / particle_Data->max_Fade;
					color.a = linear_Interpolation(0, 1.0, fade_Percent);

				}
				else if (lifetime_Delta >= (particle->lifetime_Max - particle_Data->max_Fade)) {
					fade_Percent = (particle_Data->lifetime_Max - lifetime_Delta) / particle_Data->max_Fade;
					color.a = linear_Interpolation(0, 1.0, fade_Percent);
				}
				else {
					color.a = 1.0f;
				}

				SDL_SetTextureAlphaMod(texture, (Uint8)(255 * (color.a)));

				float percent_Life_time = particle_System->particles[j].lifetime / particle->lifetime_Max;

				if (particle_System->particle_Type == "PT_RAINBOW") {
					float hue = 360.0f * (1.0f - percent_Life_time);
					float saturation = 1.0f;
					float value = 1.0f;

					color = HSV_To_RGB(hue, saturation, value);
				}
				else if (particle_System->particle_Type == "PT_BLOOD") {
					color.r = 1.0f;
				}
				SDL_SetTextureColorMod(texture, (Uint8)(255 * color.r), (Uint8)(255 * color.g), (Uint8)(255 * color.b));


				SDL_RenderCopyEx(Globals::renderer, texture, NULL, &src_Rect, 0, NULL, SDL_FLIP_NONE);
			}
			SDL_SetTextureAlphaMod(texture, SDL_ALPHA_OPAQUE);
			SDL_SetTextureColorMod(texture, 0, 0, 0);
		}
	}
}

Type_Descriptor particle_Data_Type_Descriptors[] = {
	FIELD(Particle_Data, DT_STRING, type),
	FIELD(Particle_Data, DT_STRING, sprite_Sheet_Name),
	FIELD(Particle_Data, DT_INT, size),
	FIELD(Particle_Data, DT_FLOAT, particles_Per_Second),
	FIELD(Particle_Data, DT_FLOAT, max_Fade),
	FIELD(Particle_Data, DT_FLOAT, lifetime_Min),
	FIELD(Particle_Data, DT_FLOAT, lifetime_Max),
	FIELD(Particle_Data, DT_FLOAT, velocity_Min.x),
	FIELD(Particle_Data, DT_FLOAT, velocity_Min.y),
	FIELD(Particle_Data, DT_FLOAT, velocity_Max.x),
	FIELD(Particle_Data, DT_FLOAT, velocity_Max.y),
};

void load_Particle_Data_CSV(CSV_Data* csv_Data) {
	std::vector<Particle_Data> particle_Data;
	particle_Data.resize(csv_Data->rows);

	std::span<Type_Descriptor> particle_Descripters(particle_Data_Type_Descriptors);

	load_CSV_Data(csv_Data, (char*)particle_Data.data(), sizeof(particle_Data[0]), particle_Descripters);

	for (Particle_Data& iterator : particle_Data) {
		particle_Data_Map[iterator.type] = iterator;
	}
}

void attempt_Reload_Particle_CSV_File(CSV_Data* csv_Data) {
	size_t current_File_Time = file_Last_Modified(csv_Data->file_Path);
	if (current_File_Time != csv_Data->last_Modified_Time) {
		bool close_File = false;
		if (!csv_Data->file.is_open()) {
			open_CSV_File(csv_Data);
			close_File = true;
		}
		if (csv_Data->file.is_open()) {
			SDL_Log("About to call load_Particle");
			load_Particle_Data_CSV(csv_Data);
			// Only set the last_Modified_Time if we get to this point
			if (close_File) {
				close_CSV_File(csv_Data);
			}
		}
		else {
			SDL_Log("ERROR: Unable to open file");
		}
	}
}