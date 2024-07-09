#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Image create_Image(const char* file_Path) {
	Image result = {};
	result.file_Path = file_Path;

	int width, height, channels;
	unsigned char* data = stbi_load(file_Path, &width, &height, &channels, 4);

	if (data == NULL) {
		#ifndef USE_CUSTOM_SDL
			log("ERROR: stbi_load returned NULL");
		#else 
			SDL_Log("ERROR: stbi_load returned NULL");
		#endif
		return result;
	}

	result.pixel_Data = data;

	result.width = width;
	result.height = height;

	DEFER{
		// Freed at the end of main
		// stbi_image_free(data);
	};

	SDL_Texture* temp = MP_CreateTexture(Globals::renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, result.width, result.height);

	if (temp == NULL) {
		#ifndef USE_CUSTOM_SDL
			log("ERROR: SDL_CreateTexture returned NULL");
		#else 
			SDL_Log("ERROR: SDL_CreateTexture returned NULL");
		#endif
		return result;
	}

	if (MP_SetTextureBlendMode(temp, SDL_BLENDMODE_BLEND) != 0) {
		#ifndef USE_CUSTOM_SDL
			log("ERROR: SDL_SetTextureBlendMode did not succeed");
		#else 
			SDL_Log("ERROR: SDL_SetTextureBlendMode did not succeed");
		#endif
	}

	result.texture = temp;

	void* pixels;
	int pitch;
	MP_LockTexture(temp, NULL, &pixels, &pitch);

	my_Memory_Copy(pixels, data, (result.width * result.height) * 4);

	MP_UnlockTexture(temp);

	return result;
}
