#pragma once

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
// Suppress compiler warnings
#define REF(V) ((void)V)

// DEFER
template <typename T>
struct ExitScope
{
	T lambda;
	ExitScope(T lambda) : lambda(lambda) { }
	~ExitScope() { lambda(); }
	// NO_COPY(ExitScope);
};

struct ExitScopeHelp
{
	template<typename T>
	ExitScope<T> operator+(T t) { return t; }
};

#define _SG_CONCAT(a, b) a ## b
#define SG_CONCAT(a, b) _SG_CONCAT(a, b)
#define DEFER auto SG_CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

struct Vector {
	float x;
	float y;
};

Vector operator+(const Vector& a, const Vector& b) {
	Vector result = {};
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

Vector operator-(Vector& a, Vector& b) {
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

Vector operator/(const Vector& vector, float scalar) {
	if (scalar == 0) {
		SDL_Log("ERROR: operator overload / - Division by zero");
		return vector; // or handle it in some other way
	}
	return { vector.x / scalar, vector.y / scalar };
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
// Multiply this vector by this scalar (0.5)
Vector calculate_Center(float w, float h) {
	Vector result = { w / 2, h / 2 };
	return result;
}
