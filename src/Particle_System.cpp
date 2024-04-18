#include "Particle_System.h"
#include <algorithm> 

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

void spawn_Particle_Systems(Game_Data& game_Data, Particle_Type type, V2 pos, int w, int h, Image image) {
	int max_Particles = 1000;
	int particle_Size = 100;

	Particle_System particle_System = {};
	Particle particle = {};

	particle.image = image;
	particle.life_Time = random_Float_In_Range(particle_Data_Array[type].lifetime_Min, particle_Data_Array[type].lifetime_Max);
	particle.velocity =
		random_Vector_In_Range(
			{
				particle_Data_Array[type].velocity_Min.x,
				particle_Data_Array[type].velocity_Max.x
			},
			{
				particle_Data_Array[type].velocity_Min.y,
				particle_Data_Array[type].velocity_Max.y
			}
			);
	particle.position =
		random_Vector_In_Range(
			{
				pos.x - (w / 2),
				pos.x + (w / 2)
			},
			{
				pos.y - (h / 2),
				pos.y + (h / 2)
			}
			);
	particle.size = particle_Size;

	particle_System.particle_Info = particle;
	particle_System.type = type;
	particle_System.time_Between_Spawns = 0.0f;

	game_Data.particle_Systems.push_back(particle_System);
}

// Update 
void update_Particle_Systems(Particle_System particle_System, float delta_Time) {
	{
		// Update current particles
		for (int i = 0; i < particle_System.particles.size(); i++) {
			particle_System.particles[i].velocity.y += Globals::GRAVITY;

			particle_System.particles[i].position.x = (particle_System.particles[i].velocity.x);
			particle_System.particles[i].position.y = (particle_System.particles[i].velocity.y);
		}
	}
	{
		// Spawn new particles based off time
		if (particle_System.time_Between_Spawns <= 0) {
			particle_System.particles.push_back(particle_System.particle_Info);

			particle_System.time_Between_Spawns = particle_Data_Array[particle_System.type].time_Between_Spawns;
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

// Render
