#include "Particle_System.h"
#include <algorithm> 
#include "Entity.h"

void swap_Floats(float& a, float& b) {
	float temp = a;
	a = b;
	b = temp;
}

float random_Float_In_Range(float min, float max) {
	if (min > max) {
		swap_Floats(min, max);
	}
	float result = max - min;
	// Random number between 0.0f - 1.0f
	float temp = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	result *= temp;
	result += min;

	return result;
}

V2 random_Vector_In_Range(V2 min, V2 max) {
	return { random_Float_In_Range(min.x, max.x), random_Float_In_Range(min.y, max.y) };
}

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
		}
	}
	{
		// Spawn new particles based off time
		if (particle_System.time_Between_Spawns <= 0) {
			Particle particle = {};
			SDL_Rect* rect = &particle_System.rect;

			// particle.life_Time = random_Float_In_Range(particle_Data_Array[type].lifetime_Min, particle_Data_Array[type].lifetime_Max);
			particle.life_Time = 10000;
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
			particle_System.particles.push_back(particle);

			particle_System.time_Between_Spawns = particle_Data_Array[particle_System.type].time_Between_Spawns;
		}
		// Decreases it right after setting (Bad)
		particle_System.time_Between_Spawns -= delta_Time;
	}
	{
		// update lifetime and destroy particles
		std::erase_if(particle_System.particles, [delta_Time](Particle& particle) {
			particle.life_Time -= delta_Time;
			return particle.life_Time <= 0;
		});

	}
}

// Render
void draw_Particle_Systems(Game_Data& game_Data) {
	for (Particle_System particle_System : game_Data.particle_Systems) {
		for (Particle particle : particle_System.particles) {
			SDL_Rect src_Rect = {};
			src_Rect.w = particle.size;
			src_Rect.h = particle.size;
			src_Rect.x = (int)particle.position.x;
			src_Rect.y = (int)particle.position.y;
			SDL_RenderCopyEx(Globals::renderer, particle_System.image->texture, NULL, &src_Rect, 0, NULL, SDL_FLIP_NONE);
		}
	}
}
