#include "Utility.h"
#include "SDL.h"
#include <string>

V2 operator+(const V2& a, const V2& b) {
	V2 result = {};
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

V2 operator-(V2& a, V2& b) {
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

V2 operator/(const V2& V2, float scalar) {
	return { V2.x / scalar, V2.y / scalar };
}

void my_Memory_Copy(void* dest, const void* src, size_t count) {
	unsigned char* destination = (unsigned char*)dest;
	unsigned char* source = (unsigned char*)src;
	for (int i = 0; i < count; i++) {
		destination[i] = source[i];
	}
}

float calculate_Distance(float x1, float y1, float x2, float y2) {
	return (float)sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

float linear_Interpolation(float left_Point, float right_Point, float percent) {
	// Lerp of T = A * (1 - T) + B * T
	// A is the left side, B is the right side, T is the percentage of the interpolation
	return ((left_Point) * (1 - percent) + (right_Point)*percent);
}

// Operator overload
// Multiply this V2 by this scalar (0.5)
V2 calculate_Center(float w, float h) {
	V2 result = { w / 2, h / 2 };
	return result;
}

V2 calculate_Direction_V2(V2 target, V2 start) {
	V2 result = {};

	result.x = target.x - start.x;
	result.y = target.y - start.y;

	float length = (float)sqrt((result.x * result.x) + (result.y * result.y));

	result.x /= length;
	result.y /= length;

	return result;
}

float random_Float_In_Range(float min, float max) {
	if (min > max) {
		my_Swap(min, max);
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

size_t file_Last_Modified(std::string file_Name) {
	const char* ptr = file_Name.c_str();
	struct stat file_Buffer = {};

	if (stat(ptr, &file_Buffer) != 0) {
		char errorMessage[256];
		strerror_s(errorMessage, sizeof(errorMessage), errno);
		SDL_Log("ERROR: stat failed for %s, Error: %s", ptr, errorMessage);
	}

	return file_Buffer.st_mtime;
}
