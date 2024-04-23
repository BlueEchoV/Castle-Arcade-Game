#include "Particle_System.h"
#include <algorithm> 
#include "Entity.h"
#include <fstream>
#include <sstream>

void spawn_Particle_System(Game_Data& game_Data, Particle_Type type, V2 pos, float lifetime, int w, int h, Image* image) {
	Particle_System particle_System = {};

	particle_System.rect.x = (int)pos.x;
	particle_System.rect.y = (int)pos.y;
	particle_System.rect.w = w;
	particle_System.rect.h = h;
	particle_System.type = type;
	particle_System.time_Between_Spawns = 0.0f;
	particle_System.image = image;
	particle_System.destroyed = false;
	particle_System.lifetime = lifetime;
	particle_System.target_ID = -1;

	game_Data.particle_Systems.push_back(particle_System);
}

void spawn_Particle_System_Target(Game_Data& game_Data, Particle_Type type, V2 pos, int target_ID, float lifetime, int w, int h, Image* image) {
	Particle_System particle_System = {};

	particle_System.rect.x = (int)pos.x;
	particle_System.rect.y = (int)pos.y;
	particle_System.rect.w = w;
	particle_System.rect.h = h;
	particle_System.type = type;
	particle_System.time_Between_Spawns = 0.0f;
	particle_System.image = image;
	particle_System.destroyed = false;
	particle_System.lifetime = lifetime;
	particle_System.target_ID = target_ID;

	game_Data.particle_Systems.push_back(particle_System);
}

// Update 
void update_Particle_System(Particle_System& particle_System, V2 spawn_Position, float delta_Time) {
	particle_System.rect.x = (int)spawn_Position.x;
	particle_System.rect.y = (int)spawn_Position.y;
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
		
		const Particle_Data* data = &particle_Data_Array[particle_System.type];

		while (particle_System.time_Between_Spawns <= 0 
			&& current_Spawn <= max_Spawn
			&& particle_System.destroyed == false) {
			Particle particle = {};
			SDL_Rect* rect = &particle_System.rect;
			rect->x = (int)spawn_Position.x;
			rect->y = (int)spawn_Position.y;

			particle.lifetime = 
				random_Float_In_Range(
					data->lifetime_Min, 
					data->lifetime_Max
				);
			particle.velocity =
				random_Vector_In_Range(
					data->velocity_Min,
					data->velocity_Max
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

// Render
void draw_Particle_Systems(Game_Data& game_Data) {
	for (const Particle_System& particle_System : game_Data.particle_Systems) {
		// Chris' trick for rendering backwards
		for (size_t i = particle_System.particles.size(); i--;) {
			SDL_Rect src_Rect = {};
			src_Rect.w = particle_System.particles[i].size;
			src_Rect.h = particle_System.particles[i].size;
			src_Rect.x = (int)particle_System.particles[i].position.x;
			src_Rect.y = (int)particle_System.particles[i].position.y;

			const Particle* particle = &particle_System.particles[i];
			const Particle_Data* data = &particle_Data_Array[particle_System.type];
			F_Color color = {};

			float lifetime_Delta = data->lifetime_Max - particle->lifetime;
			float fade_Percent = 0.0f;
			if (lifetime_Delta <= data->max_Fade) {
				fade_Percent = lifetime_Delta / data->max_Fade;
				color.a = linear_Interpolation(0, 1.0, fade_Percent);

			} else if (lifetime_Delta >= (data->lifetime_Max - data->max_Fade)) {
				fade_Percent = (data->lifetime_Max - lifetime_Delta) / data->max_Fade;
				color.a = linear_Interpolation(0, 1.0, fade_Percent);
			} else {
				color.a = 1.0f;
			}
			SDL_SetTextureAlphaMod(particle_System.image->texture, (Uint8)(255 * (color.a)));

			float percent_Life_time = particle_System.particles[i].lifetime / data->lifetime_Max;
			SDL_SetTextureColorMod(particle_System.image->texture, (Uint8)(255 * percent_Life_time), 0, (Uint8)(255 * (1 - percent_Life_time)));

			SDL_RenderCopyEx(Globals::renderer, particle_System.image->texture, NULL, &src_Rect, 0, NULL, SDL_FLIP_NONE);
		}
	}
}

// Splits a string using a delimiter and returns a vector of strings
std::vector<std::string> split(const std::string& my_String, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	// Input stream class to operate on strings
	std::istringstream my_Stream(my_String);
	while (std::getline(my_Stream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

void load_CSV_File(std::unordered_map<std::string, Particle_Data>& particle_Data_Map, std::string file_Name) {
	std::string filename = file_Name;
	std::ifstream file(filename);
	std::vector<Particle_Data> particles;

	if (!file.is_open()) {
		SDL_Log("Error loading .csv file");
	}

	DEFER{
		file.close();
	};

	std::string line;
	// Skip the first line containing the headers
	// NOTE: getline reads characters from an input stream and places them into a string: 
	std::getline(file, line);

	Particle_Data particle_data = {};
	while (std::getline(file, line)) {
		// Types,size,max_Particles,time_Between Spawns,max_Fade,lifetime_Min,lifetime_Max
		std::vector<std::string> tokens = split(line, ',');
		std::string row_Name = tokens[0];

		if (tokens.size() == 8) {
			particle_data.size = std::stoi(tokens[1]);
			particle_data.time_Between_Spawns = std::stof(tokens[2]);
			particle_data.max_Fade = std::stof(tokens[3]);
			particle_data.lifetime_Min = std::stof(tokens[4]);
			particle_data.lifetime_Max = std::stof(tokens[5]);

			std::vector<std::string> velocityMinTokens = split(tokens[6], ' ');
			std::vector<std::string> velocityMaxTokens = split(tokens[7], ' ');

			if (velocityMinTokens.size() == 2) {
				particle_data.velocity_Min.x = std::stof(velocityMinTokens[0]);
				particle_data.velocity_Min.y = std::stof(velocityMinTokens[1]);
			}

			if (velocityMaxTokens.size() == 2) {
				particle_data.velocity_Max.x = std::stof(velocityMaxTokens[0]);
				particle_data.velocity_Max.y = std::stof(velocityMaxTokens[1]);
			}

			// Add the populated struct to the vector
			particles.push_back(particle_data);
		}
		else {
			SDL_Log("Error: Line does not have enough data");
		}
		particle_Data_Map[row_Name] = particle_data;
	}
}
