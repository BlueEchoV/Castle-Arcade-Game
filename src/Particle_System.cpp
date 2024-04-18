#include "Particle_System.h"
#include <algorithm> 
#include "Entity.h"

void spawn_Particle_Systems(Game_Data& game_Data, Particle_Type type, V2 pos, int w, int h, Image* image) {
	Particle_System particle_System = {};

	particle_System.rect.x = (int)pos.x;
	particle_System.rect.y = (int)pos.y;
	particle_System.rect.w = w;
	particle_System.rect.h = h;
	particle_System.type = type;
	particle_System.time_Between_Spawns = 0.0f;
	particle_System.image = image;

	game_Data.particle_Systems.push_back(particle_System);
}

// Update 
void update_Particle_System(Particle_System& particle_System, float delta_Time) {
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
		while (particle_System.time_Between_Spawns <= 0 && current_Spawn <= max_Spawn) {
			Particle particle = {};
			SDL_Rect* rect = &particle_System.rect;

			particle.life_Time = 
				random_Float_In_Range(
					particle_Data_Array[particle_System.type].lifetime_Min, 
					particle_Data_Array[particle_System.type].lifetime_Max
				);
			particle.velocity =
				random_Vector_In_Range(
					{
						particle_Data_Array[particle_System.type].velocity_Min
					},
					{
						particle_Data_Array[particle_System.type].velocity_Max.y
					}
				);
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
			particle.size = particle_Data_Array[particle_System.type].size;
			particle.fade_In = particle_Data_Array[particle_System.type].max_Fade_In;
			particle_System.particles.push_back(particle);

			// Adding it binds it to the frames
			particle_System.time_Between_Spawns += particle_Data_Array[particle_System.type].time_Between_Spawns;
			current_Spawn++;
		}
	}
	{
		// update lifetime and destroy particles
		std::erase_if(particle_System.particles, [delta_Time](Particle& particle) {
			particle.life_Time -= delta_Time;
			return particle.life_Time <= 0;
		});

	}
}

void clamp(float& a) {
	if (a > 1.0)
		a = 1.0;
	if (a < 0.0) 
		a = 0.0;
}

// Render
void draw_Particle_Systems(Game_Data& game_Data) {
	for (Particle_System particle_System : game_Data.particle_Systems) {
		// Chris' trick for rendering backwards
		for (size_t i = particle_System.particles.size(); i--;) {
			SDL_Rect src_Rect = {};
			src_Rect.w = particle_System.particles[i].size;
			src_Rect.h = particle_System.particles[i].size;
			src_Rect.x = (int)particle_System.particles[i].position.x;
			src_Rect.y = (int)particle_System.particles[i].position.y;

			float fade_Percent = particle_System.particles[i].fade_In / particle_Data_Array[particle_System.type].max_Fade_In;
			clamp(fade_Percent);

			SDL_SetTextureAlphaMod(particle_System.image->texture, (Uint8)(255 * (1 - fade_Percent)));

			float percent = particle_System.particles[i].life_Time / particle_Data_Array[particle_System.type].lifetime_Max;
			
			SDL_SetTextureColorMod(particle_System.image->texture, (Uint8)(255 * percent), 0, (Uint8)(255 * (1 - percent)));
			SDL_RenderCopyEx(Globals::renderer, particle_System.image->texture, NULL, &src_Rect, 0, NULL, SDL_FLIP_NONE);
		}
	}
}
