#pragma once
#include <cmath>

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

struct V2{
	float x;
	float y;
};

V2 operator+(const V2& a, const V2& b);
V2 operator-(V2& a, V2& b);
V2 operator/(const V2& V2, float scalar);

void my_Memory_Copy(void* dest, const void* src, size_t count);

float calculate_Distance(float x1, float y1, float x2, float y2);

float linear_Interpolation(float left_Point, float right_Point, float percent);

V2 calculate_Center(float w, float h);

V2 calculate_Direction_V2(V2 target, V2 start);

void swap_Floats(float& a, float& b);

float random_Float_In_Range(float min, float max);

V2 random_Vector_In_Range(V2 min, V2 max);
