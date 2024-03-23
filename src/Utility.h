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